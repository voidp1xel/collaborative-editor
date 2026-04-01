/*
 * Display Module
 *
 * Manages the ncurses UI layout: a 2/3-width editor pane on the left,
 * a 1/3-width AI chat pane on the right, and a status bar at the bottom.
 */

#include "display.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "document.h"
#include "text_buffer.h"

// Color pair identifiers
#define COLOR_EDITOR   1
#define COLOR_STATUS   2
#define COLOR_CHAT_BOX 3

// Window pointers (extern'd in display.h)
WINDOW *editor_win = NULL;
WINDOW *chat_win = NULL;
WINDOW *chat_input_win = NULL;

// Track last known terminal size for resize detection
static int last_rows = 0;
static int last_cols = 0;

void init_display(void) {
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    start_color();
    curs_set(1);

    init_pair(COLOR_EDITOR, COLOR_WHITE, COLOR_CYAN);
    init_pair(COLOR_STATUS, COLOR_BLACK, COLOR_GREEN);
    init_pair(COLOR_CHAT_BOX, COLOR_WHITE, COLOR_BLUE);

    bkgd(COLOR_PAIR(COLOR_EDITOR));

    getmaxyx(stdscr, last_rows, last_cols);
}

static void create_windows(int rows, int cols) {
    // Safely delete existing windows
    if (editor_win)     { delwin(editor_win);     editor_win = NULL; }
    if (chat_win)       { delwin(chat_win);       chat_win = NULL; }
    if (chat_input_win) { delwin(chat_input_win); chat_input_win = NULL; }

    int editor_width = cols * 2 / 3;
    int chat_width = cols - editor_width;

    // Editor: full height minus status bar row
    editor_win = newwin(rows - 1, editor_width, 0, 0);
    // Chat history: top-right, minus 3 rows for input box and 1 for status
    chat_win = newwin(rows - 4, chat_width, 0, editor_width);
    // Chat input: bottom-right, 3 rows
    chat_input_win = newwin(3, chat_width, rows - 4, editor_width);

    wbkgd(editor_win, COLOR_PAIR(COLOR_EDITOR));
    wbkgd(chat_win, COLOR_PAIR(COLOR_CHAT_BOX));
    wbkgd(chat_input_win, COLOR_PAIR(COLOR_CHAT_BOX));

    // Enable keypad for all windows
    keypad(editor_win, TRUE);
    keypad(chat_win, TRUE);
    keypad(chat_input_win, TRUE);
}

void cleanup_display(void) {
    if (editor_win)     delwin(editor_win);
    if (chat_win)       delwin(chat_win);
    if (chat_input_win) delwin(chat_input_win);
    editor_win = chat_win = chat_input_win = NULL;
    endwin();
}

void draw_layout(Document* doc, const char** chat_history, int chat_lines, const char* chat_input, bool is_chat_active) {
    int screen_rows, screen_cols;
    getmaxyx(stdscr, screen_rows, screen_cols);

    // Recreate windows on resize or first call
    if (screen_rows != last_rows || screen_cols != last_cols || !editor_win) {
        last_rows = screen_rows;
        last_cols = screen_cols;
        create_windows(screen_rows, screen_cols);
    }

    int editor_width = screen_cols * 2 / 3;
    int chat_width = screen_cols - editor_width;

    // Clear all panes
    werase(stdscr);
    werase(editor_win);
    werase(chat_win);
    werase(chat_input_win);

    // ----- Draw Editor Pane -----
    TextBuffer* buffer = doc->buffer;
    int max_editor_rows = getmaxy(editor_win);

    for (int i = 0; i < buffer->line_count && i < max_editor_rows; i++) {
        Line* line = buffer->lines[i];

        // Calculate horizontal offset for alignment
        int start_x = 0;
        if (line->alignment == ALIGN_CENTER) {
            start_x = (editor_width - line->length) / 2;
            if (start_x < 0) start_x = 0;
        } else if (line->alignment == ALIGN_RIGHT) {
            start_x = editor_width - line->length - 1;
            if (start_x < 0) start_x = 0;
        }

        // Render each character with its attributes
        for (int j = 0; j < line->length && (start_x + j) < editor_width; j++) {
            if (line->attributes[j]) wattron(editor_win, line->attributes[j]);
            mvwaddch(editor_win, i, start_x + j, line->chars[j]);
            if (line->attributes[j]) wattroff(editor_win, line->attributes[j]);
        }
    }

    // ----- Draw Chat Panes -----
    box(chat_win, 0, 0);
    box(chat_input_win, 0, 0);

    // Titles
    wattron(chat_win, A_BOLD);
    mvwprintw(chat_win, 0, 2, " AI Chat ");
    wattroff(chat_win, A_BOLD);

    mvwprintw(chat_input_win, 0, 2, is_chat_active ? " Message (Tab=Editor) " : " Tab to chat ");

    // Chat history — show the most recent messages that fit
    int chat_display_rows = getmaxy(chat_win) - 2; // minus top and bottom border
    int start_line = (chat_lines > chat_display_rows) ? chat_lines - chat_display_rows : 0;
    for (int i = start_line; i < chat_lines; i++) {
        if (chat_history[i]) {
            mvwprintw(chat_win, 1 + (i - start_line), 1, "%.*s", chat_width - 2, chat_history[i]);
        }
    }

    // Chat input prompt
    mvwprintw(chat_input_win, 1, 1, "> %.*s", chat_width - 4, chat_input);

    // ----- Draw Status Bar -----
    attron(COLOR_PAIR(COLOR_STATUS));
    char status[256];
    snprintf(status, sizeof(status),
             " F1:Quit | Tab:Switch | Ctrl+B:Bold | Ctrl+L/E/R:Align | Ln:%d Col:%d | %s ",
             buffer->cursor_line + 1, buffer->cursor_col + 1,
             is_chat_active ? "CHAT" : "EDITOR");

    mvprintw(screen_rows - 1, 0, "%-*s", screen_cols, status);
    attroff(COLOR_PAIR(COLOR_STATUS));

    // ----- Refresh and position cursor -----
    wnoutrefresh(stdscr);
    wnoutrefresh(editor_win);
    wnoutrefresh(chat_win);
    wnoutrefresh(chat_input_win);

    if (is_chat_active) {
        int cursor_pos = 2 + (int)strlen(chat_input);
        if (cursor_pos >= chat_width - 1) cursor_pos = chat_width - 2;
        wmove(chat_input_win, 1, cursor_pos);
    } else {
        wmove(editor_win, buffer->cursor_line, buffer->cursor_col);
    }

    doupdate();
}
