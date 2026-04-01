#include "text_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ncurses.h> // For A_BOLD

#define INITIAL_LINE_CAP 128
#define INITIAL_LINES_CAP 32

// ============================================================================
// Private helpers
// ============================================================================

static Line* create_line(void) {
    Line* line = malloc(sizeof(Line));
    if (!line) return NULL;
    line->capacity = INITIAL_LINE_CAP;
    line->chars = malloc(line->capacity);
    line->attributes = calloc(line->capacity, sizeof(int));
    line->length = 0;
    line->chars[0] = '\0';
    line->alignment = ALIGN_LEFT;
    return line;
}

static void destroy_line(Line* line) {
    if (line) {
        free(line->chars);
        free(line->attributes);
        free(line);
    }
}

// Ensure line can hold at least `needed` characters (plus null terminator)
static void ensure_line_capacity(Line* line, int needed) {
    if (needed + 1 <= line->capacity) return;
    int new_cap = line->capacity;
    while (new_cap <= needed) new_cap *= 2;
    line->chars = realloc(line->chars, new_cap);
    line->attributes = realloc(line->attributes, new_cap * sizeof(int));
    // Zero-init newly allocated attribute slots
    memset(line->attributes + line->capacity, 0, (new_cap - line->capacity) * sizeof(int));
    line->capacity = new_cap;
}

// Ensure the buffer can hold at least `needed` lines
static void ensure_buffer_capacity(TextBuffer* buffer, int needed) {
    if (needed <= buffer->capacity) return;
    int new_cap = buffer->capacity;
    while (new_cap < needed) new_cap *= 2;
    buffer->lines = realloc(buffer->lines, sizeof(Line*) * new_cap);
    buffer->capacity = new_cap;
}

// ============================================================================
// Lifecycle
// ============================================================================

TextBuffer* create_buffer() {
    TextBuffer* buffer = malloc(sizeof(TextBuffer));
    if (!buffer) return NULL;
    buffer->capacity = INITIAL_LINES_CAP;
    buffer->lines = malloc(sizeof(Line*) * buffer->capacity);
    buffer->line_count = 1;
    buffer->lines[0] = create_line();
    buffer->cursor_line = 0;
    buffer->cursor_col = 0;
    buffer->is_dirty = false;
    return buffer;
}

void destroy_buffer(TextBuffer* buffer) {
    if (!buffer) return;
    for (int i = 0; i < buffer->line_count; i++) {
        destroy_line(buffer->lines[i]);
    }
    free(buffer->lines);
    free(buffer);
}

// ============================================================================
// Core editing — insert / delete at cursor position
// ============================================================================

void insert_char_at_cursor(TextBuffer* buffer, char ch) {
    Line* line = buffer->lines[buffer->cursor_line];
    ensure_line_capacity(line, line->length + 1);

    int col = buffer->cursor_col;

    // Shift chars and attributes right from cursor_col
    memmove(line->chars + col + 1, line->chars + col, line->length - col + 1); // +1 for '\0'
    memmove(line->attributes + col + 1, line->attributes + col, (line->length - col) * sizeof(int));

    // Insert the new character
    line->chars[col] = ch;
    line->attributes[col] = (col > 0) ? line->attributes[col - 1] : 0;
    line->length++;
    buffer->cursor_col++;
    buffer->is_dirty = true;
}

void delete_char_at_cursor(TextBuffer* buffer) {
    int col = buffer->cursor_col;
    int ln = buffer->cursor_line;

    if (col > 0) {
        // Normal backspace within a line: delete char at col-1
        Line* line = buffer->lines[ln];
        memmove(line->chars + col - 1, line->chars + col, line->length - col + 1);
        memmove(line->attributes + col - 1, line->attributes + col, (line->length - col) * sizeof(int));
        line->length--;
        buffer->cursor_col--;
        buffer->is_dirty = true;
    } else if (ln > 0) {
        // Backspace at column 0: merge current line into previous line
        Line* prev = buffer->lines[ln - 1];
        Line* curr = buffer->lines[ln];
        int prev_len = prev->length;

        ensure_line_capacity(prev, prev_len + curr->length);

        // Append current line's content to previous line
        memcpy(prev->chars + prev_len, curr->chars, curr->length + 1); // +1 for '\0'
        memcpy(prev->attributes + prev_len, curr->attributes, curr->length * sizeof(int));
        prev->length += curr->length;

        // Remove the current line from the array
        destroy_line(curr);
        memmove(buffer->lines + ln, buffer->lines + ln + 1, (buffer->line_count - ln - 1) * sizeof(Line*));
        buffer->line_count--;

        // Place cursor at the join point
        buffer->cursor_line--;
        buffer->cursor_col = prev_len;
        buffer->is_dirty = true;
    }
}

void insert_newline_at_cursor(TextBuffer* buffer) {
    int ln = buffer->cursor_line;
    int col = buffer->cursor_col;
    Line* current = buffer->lines[ln];

    // Create the new line that will hold the text after the cursor
    Line* new_line = create_line();
    int trailing_len = current->length - col;

    if (trailing_len > 0) {
        ensure_line_capacity(new_line, trailing_len);
        memcpy(new_line->chars, current->chars + col, trailing_len + 1); // +1 for '\0'
        memcpy(new_line->attributes, current->attributes + col, trailing_len * sizeof(int));
        new_line->length = trailing_len;
    }
    new_line->alignment = current->alignment;

    // Truncate the current line at the cursor
    current->length = col;
    current->chars[col] = '\0';

    // Insert the new line into the buffer
    ensure_buffer_capacity(buffer, buffer->line_count + 1);
    memmove(buffer->lines + ln + 2, buffer->lines + ln + 1, (buffer->line_count - ln - 1) * sizeof(Line*));
    buffer->lines[ln + 1] = new_line;
    buffer->line_count++;

    // Move cursor to the beginning of the new line
    buffer->cursor_line++;
    buffer->cursor_col = 0;
    buffer->is_dirty = true;
}

// ============================================================================
// Cursor movement
// ============================================================================

void move_cursor(TextBuffer* buffer, int dr, int dc) {
    buffer->cursor_line += dr;
    if (buffer->cursor_line < 0) buffer->cursor_line = 0;
    if (buffer->cursor_line >= buffer->line_count) buffer->cursor_line = buffer->line_count - 1;

    buffer->cursor_col += dc;
    int line_len = buffer->lines[buffer->cursor_line]->length;
    if (buffer->cursor_col < 0) buffer->cursor_col = 0;
    if (buffer->cursor_col > line_len) buffer->cursor_col = line_len;
}

// ============================================================================
// Formatting
// ============================================================================

void toggle_bold_at_cursor(TextBuffer* buffer) {
    Line* line = buffer->lines[buffer->cursor_line];
    if (buffer->cursor_col > 0 && buffer->cursor_col <= line->length) {
        int idx = buffer->cursor_col - 1;
        line->attributes[idx] = (line->attributes[idx] == A_BOLD) ? 0 : A_BOLD;
        buffer->is_dirty = true;
    }
}

void set_line_alignment(TextBuffer* buffer, Alignment align) {
    if (buffer->cursor_line >= 0 && buffer->cursor_line < buffer->line_count) {
        buffer->lines[buffer->cursor_line]->alignment = align;
        buffer->is_dirty = true;
    }
}

// ============================================================================
// Bulk operations (backward-compatible with old Document API)
// ============================================================================

void insert_text(TextBuffer* buffer, int position, const char* text) {
    (void)position; // Reset and load from scratch
    // Clear existing content
    for (int i = 0; i < buffer->line_count; i++) destroy_line(buffer->lines[i]);
    buffer->line_count = 0;

    // Parse text into lines
    const char* start = text;
    while (*start) {
        const char* end = strchr(start, '\n');
        int len = end ? (int)(end - start) : (int)strlen(start);

        ensure_buffer_capacity(buffer, buffer->line_count + 1);
        Line* line = create_line();
        ensure_line_capacity(line, len);
        memcpy(line->chars, start, len);
        line->chars[len] = '\0';
        line->length = len;
        buffer->lines[buffer->line_count++] = line;

        if (end) start = end + 1;
        else break;
    }

    if (buffer->line_count == 0) {
        ensure_buffer_capacity(buffer, 1);
        buffer->lines[0] = create_line();
        buffer->line_count = 1;
    }

    buffer->cursor_line = 0;
    buffer->cursor_col = 0;
}

void delete_text(TextBuffer* buffer, int position, int length) {
    (void)position;
    (void)length;
    // Clear all content
    for (int i = 0; i < buffer->line_count; i++) destroy_line(buffer->lines[i]);
    buffer->line_count = 1;
    buffer->lines[0] = create_line();
    buffer->cursor_line = 0;
    buffer->cursor_col = 0;
    buffer->is_dirty = true;
}

const char* get_text_content(TextBuffer* buffer) {
    // Build a flat string from all lines (caller should NOT free — use static buffer)
    static char* flat = NULL;
    free(flat);

    int total = 0;
    for (int i = 0; i < buffer->line_count; i++) {
        total += buffer->lines[i]->length + 1; // +1 for '\n'
    }

    flat = malloc(total + 1);
    flat[0] = '\0';
    for (int i = 0; i < buffer->line_count; i++) {
        strcat(flat, buffer->lines[i]->chars);
        if (i < buffer->line_count - 1) strcat(flat, "\n");
    }
    return flat;
}

// ============================================================================
// Networking serialization
//
// Format per line: [alignment]attr_bits|text_content
// Lines separated by newline character.
// ============================================================================

char* serialize_buffer(TextBuffer* buffer) {
    // Calculate required size
    int total_size = 0;
    for (int i = 0; i < buffer->line_count; i++) {
        Line* line = buffer->lines[i];
        // [X] + attr chars + | + text + \n
        total_size += 3 + line->length + 1 + line->length + 1;
    }
    total_size += 1; // null terminator

    char* data = malloc(total_size);
    if (!data) return NULL;
    data[0] = '\0';

    char meta[16];
    for (int i = 0; i < buffer->line_count; i++) {
        Line* line = buffer->lines[i];

        // Alignment tag
        sprintf(meta, "[%d]", line->alignment);
        strcat(data, meta);

        // Attribute bits
        for (int j = 0; j < line->length; j++) {
            strcat(data, (line->attributes[j] == A_BOLD) ? "1" : "0");
        }

        // Separator and text
        strcat(data, "|");
        strcat(data, line->chars);

        // Newline between lines
        if (i < buffer->line_count - 1) {
            strcat(data, "\n");
        }
    }
    return data;
}

void deserialize_into_buffer(TextBuffer* buffer, const char* data) {
    // Clear existing content
    for (int i = 0; i < buffer->line_count; i++) {
        destroy_line(buffer->lines[i]);
    }
    buffer->line_count = 0;

    char* data_copy = strdup(data);
    char* saveptr;
    char* line_str = strtok_r(data_copy, "\n", &saveptr);

    while (line_str != NULL) {
        ensure_buffer_capacity(buffer, buffer->line_count + 1);

        Line* current_line = create_line();
        buffer->lines[buffer->line_count] = current_line;

        // Parse alignment tag: [X]
        int align = 0;
        if (sscanf(line_str, "[%d]", &align) == 1) {
            current_line->alignment = (Alignment)align;
        }

        // Find the ']' and '|' markers
        char* attr_start = strchr(line_str, ']');
        char* separator = strchr(line_str, '|');

        if (attr_start && separator && separator > attr_start) {
            attr_start++; // skip past ']'
            char* text_start = separator + 1;
            int text_len = (int)strlen(text_start);

            ensure_line_capacity(current_line, text_len);
            strncpy(current_line->chars, text_start, text_len);
            current_line->chars[text_len] = '\0';
            current_line->length = text_len;

            // Parse attribute bits
            int attr_len = (int)(separator - attr_start);
            for (int i = 0; i < current_line->length && i < attr_len; i++) {
                current_line->attributes[i] = (attr_start[i] == '1') ? A_BOLD : 0;
            }
        }

        buffer->line_count++;
        line_str = strtok_r(NULL, "\n", &saveptr);
    }

    free(data_copy);

    // Ensure at least one line exists
    if (buffer->line_count == 0) {
        ensure_buffer_capacity(buffer, 1);
        buffer->lines[0] = create_line();
        buffer->line_count = 1;
    }

    // Reset cursor to safe position
    buffer->cursor_line = 0;
    buffer->cursor_col = 0;
}
