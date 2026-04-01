/*
 * Input Handler
 *
 * Maps keyboard events to editing operations on the document's TextBuffer.
 */

#include "input.h"
#include <ncurses.h>
#include "document.h"
#include "text_buffer.h"

int handle_user_input(int ch, Document* doc, int* cursor_y, int* cursor_x, int* scroll_offset) {
    (void)cursor_y;
    (void)cursor_x;
    (void)scroll_offset;

    TextBuffer* buffer = doc->buffer;
    int needs_redraw = 1;

    switch (ch) {
        // --- Cursor movement ---
        case KEY_UP:    move_cursor(buffer, -1, 0);  break;
        case KEY_DOWN:  move_cursor(buffer, 1, 0);   break;
        case KEY_LEFT:  move_cursor(buffer, 0, -1);  break;
        case KEY_RIGHT: move_cursor(buffer, 0, 1);   break;

        // --- Deletion ---
        case KEY_BACKSPACE:
        case 127:
            delete_char_at_cursor(buffer);
            break;

        // --- Newline ---
        case '\n':
        case KEY_ENTER:
            insert_newline_at_cursor(buffer);
            break;

        // --- Formatting hotkeys ---
        case 'b' & 0x1F: // Ctrl+B — Bold
            toggle_bold_at_cursor(buffer);
            break;
        case 'l' & 0x1F: // Ctrl+L — Left align
            set_line_alignment(buffer, ALIGN_LEFT);
            break;
        case 'e' & 0x1F: // Ctrl+E — Center align
            set_line_alignment(buffer, ALIGN_CENTER);
            break;
        case 'r' & 0x1F: // Ctrl+R — Right align
            set_line_alignment(buffer, ALIGN_RIGHT);
            break;

        // --- Printable characters ---
        default:
            if (ch >= 32 && ch <= 126) {
                insert_char_at_cursor(buffer, (char)ch);
            } else {
                needs_redraw = 0;
            }
            break;
    }

    if (needs_redraw) {
        doc->is_dirty = true;
    }
    return needs_redraw;
}
