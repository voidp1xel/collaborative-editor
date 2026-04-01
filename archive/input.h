#ifndef INPUT_H
#define INPUT_H

#include "document.h"

/**
 * Handle a keyboard event and update the document accordingly.
 *
 * @param ch             Key code from getch().
 * @param doc            The active document.
 * @param cursor_y       Unused (cursor tracked internally by TextBuffer).
 * @param cursor_x       Unused.
 * @param scroll_offset  Unused.
 * @return 1 if the document was modified (screen needs redraw), 0 otherwise.
 */
int handle_user_input(int ch, Document* doc, int* cursor_y, int* cursor_x, int* scroll_offset);

#endif // INPUT_H
