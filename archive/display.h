#ifndef DISPLAY_H
#define DISPLAY_H

#include <ncurses.h>
#include <stdbool.h>
#include "document.h"

// Pointers for the UI sub-windows (defined in display.c)
extern WINDOW *editor_win, *chat_win, *chat_input_win;

/** Initialize ncurses, colors, and prepare the terminal for the UI. */
void init_display(void);

/** Restore the terminal to its original state and free window resources. */
void cleanup_display(void);

/**
 * Draw the entire UI layout: editor pane, chat pane, and status bar.
 * Automatically handles terminal resize.
 */
void draw_layout(Document* doc, const char** chat_history, int chat_lines,
                 const char* chat_input, bool is_chat_active);

#endif // DISPLAY_H
