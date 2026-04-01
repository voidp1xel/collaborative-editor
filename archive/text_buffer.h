#ifndef TEXT_BUFFER_H
#define TEXT_BUFFER_H

#include <stdbool.h>

// Enum for text alignment
typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
} Alignment;

// A structure to hold one line of text and its formatting
typedef struct {
    char* chars;         // The character data (null-terminated)
    int* attributes;     // Parallel array for ncurses attributes (e.g., A_BOLD)
    int length;          // Number of characters (excluding null terminator)
    int capacity;        // Allocated memory for chars and attributes
    Alignment alignment; // Alignment for the entire line
} Line;

// The main text buffer manages a dynamic array of Lines
typedef struct {
    Line** lines;        // Dynamic array of pointers to Line structs
    int line_count;      // How many lines are in the document
    int capacity;        // Allocated space for lines array
    int cursor_line;     // Cursor's current line index
    int cursor_col;      // Cursor's current column index
    bool is_dirty;       // Flag for unsaved changes
} TextBuffer;


// --- Lifecycle ---
TextBuffer* create_buffer();
void destroy_buffer(TextBuffer* buffer);

// --- Editing ---
void insert_char_at_cursor(TextBuffer* buffer, char ch);
void delete_char_at_cursor(TextBuffer* buffer);
void insert_newline_at_cursor(TextBuffer* buffer);

// --- Cursor movement ---
void move_cursor(TextBuffer* buffer, int dr, int dc);

// --- Formatting ---
void toggle_bold_at_cursor(TextBuffer* buffer);
void set_line_alignment(TextBuffer* buffer, Alignment align);

// --- Bulk operations (used by storage / old API compatibility) ---
void insert_text(TextBuffer* buffer, int position, const char* text);
void delete_text(TextBuffer* buffer, int position, int length);
const char* get_text_content(TextBuffer* buffer);

// --- Networking serialization ---
char* serialize_buffer(TextBuffer* buffer);
void deserialize_into_buffer(TextBuffer* buffer, const char* data);

#endif // TEXT_BUFFER_H
