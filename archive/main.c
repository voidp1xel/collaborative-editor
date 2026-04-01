/*
 * Collaborative Editor Client
 *
 * A terminal-based text editor with real-time collaborative editing
 * and an integrated AI chat assistant. Connects to a server to sync
 * edits with other clients.
 *
 * Usage: ./collaborative_editor [server_ip]
 */

#include "display.h"
#include "input.h"
#include "network_client.h"
#include "ai_chat.h"
#include "document.h"
#include "text_buffer.h"
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define DEFAULT_SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define MAX_CHAT_HISTORY 50

// --- Chat history helper ---
static void add_to_history(const char** history, int* count, const char* message, const char* prefix) {
    if (*count >= MAX_CHAT_HISTORY) {
        // Shift everything up, discard oldest
        free((void*)history[0]);
        for (int i = 0; i < MAX_CHAT_HISTORY - 1; i++) {
            history[i] = history[i + 1];
        }
        (*count)--;
    }
    char temp[512];
    snprintf(temp, sizeof(temp), "%s%s", prefix, message);
    history[*count] = strdup(temp);
    (*count)++;
}

int main(int argc, char* argv[]) {
    const char* server_ip = (argc > 1) ? argv[1] : DEFAULT_SERVER_IP;

    // ---------------------------------------------------------------
    // 1. Initialize ncurses and show login screen
    // ---------------------------------------------------------------
    init_display();

    char username[32] = {0};
    char password[32] = {0};

    mvprintw(1, 1, "=== Collaborative Text Editor ===");
    mvprintw(3, 1, "Username: ");
    echo();
    getnstr(username, sizeof(username) - 1);
    mvprintw(4, 1, "Password: ");
    noecho();
    getnstr(password, sizeof(password) - 1);
    clear();
    refresh();

    // ---------------------------------------------------------------
    // 2. Connect and authenticate
    // ---------------------------------------------------------------
    mvprintw(1, 1, "Connecting to %s:%d ...", server_ip, SERVER_PORT);
    refresh();

    int server_sock = connect_to_server(server_ip, SERVER_PORT);
    if (server_sock < 0) {
        cleanup_display();
        fprintf(stderr, "Error: Failed to connect to server at %s:%d\n", server_ip, SERVER_PORT);
        return 1;
    }

    char auth_msg[128];
    snprintf(auth_msg, sizeof(auth_msg), "AUTH %s %s", username, password);
    send_message_to_server(server_sock, auth_msg);

    // Wait for auth response (blocking for up to 3 seconds)
    sleep(1);
    char auth_response[1024] = {0};
    int bytes_read = receive_message_from_server(server_sock, auth_response, sizeof(auth_response) - 1);
    if (bytes_read <= 0 || strncmp(auth_response, "AUTH_SUCCESS", 12) != 0) {
        cleanup_display();
        fprintf(stderr, "Authentication failed. Please check your credentials.\n");
        close_connection(server_sock);
        return 1;
    }

    // ---------------------------------------------------------------
    // 3. Receive initial document state
    // ---------------------------------------------------------------
    Document* doc = document_create(NULL);
    char initial_buffer[16384] = {0};
    bytes_read = receive_message_from_server(server_sock, initial_buffer, sizeof(initial_buffer) - 1);
    if (bytes_read > 0) {
        initial_buffer[bytes_read] = '\0';
        deserialize_into_buffer(doc->buffer, initial_buffer);
    }

    // ---------------------------------------------------------------
    // 4. Set up main loop state
    // ---------------------------------------------------------------
    nodelay(stdscr, TRUE); // Non-blocking getch()
    int ch;

    // Chat state
    const char* chat_history[MAX_CHAT_HISTORY];
    for (int i = 0; i < MAX_CHAT_HISTORY; i++) chat_history[i] = NULL;
    int chat_line_count = 0;
    char chat_input_buffer[256] = {0};
    int chat_input_len = 0;
    bool is_chat_active = false;

    // ---------------------------------------------------------------
    // 5. Main event loop
    // ---------------------------------------------------------------
    while ((ch = getch()) != KEY_F(1)) {
        int doc_changed = 0;

        if (ch == '\t') {
            // Toggle focus between editor and chat
            is_chat_active = !is_chat_active;
        } else if (is_chat_active && ch != ERR) {
            // --- Chat input handling ---
            if (ch == '\n') {
                if (chat_input_len > 0) {
                    chat_input_buffer[chat_input_len] = '\0';
                    add_to_history(chat_history, &chat_line_count, chat_input_buffer, "You: ");

                    char* response = get_ai_response(chat_input_buffer);
                    if (response) {
                        add_to_history(chat_history, &chat_line_count, response, "AI: ");
                        free(response);
                    }
                    chat_input_buffer[0] = '\0';
                    chat_input_len = 0;
                }
            } else if ((ch == KEY_BACKSPACE || ch == 127) && chat_input_len > 0) {
                chat_input_len--;
                chat_input_buffer[chat_input_len] = '\0';
            } else if (ch >= 32 && ch <= 126 && chat_input_len < (int)sizeof(chat_input_buffer) - 1) {
                chat_input_buffer[chat_input_len] = (char)ch;
                chat_input_len++;
                chat_input_buffer[chat_input_len] = '\0';
            }
        } else if (!is_chat_active && ch != ERR) {
            // --- Editor input handling ---
            if (handle_user_input(ch, doc, NULL, NULL, NULL)) {
                doc_changed = 1;
            }
        }

        // Send edits to server
        if (doc_changed) {
            char* serialized_data = serialize_buffer(doc->buffer);
            if (serialized_data) {
                send_message_to_server(server_sock, serialized_data);
                free(serialized_data);
            }
        }

        // Receive collaborative edits from server
        char net_buffer[16384];
        int net_bytes = receive_message_from_server(server_sock, net_buffer, sizeof(net_buffer) - 1);
        if (net_bytes > 0) {
            net_buffer[net_bytes] = '\0';
            deserialize_into_buffer(doc->buffer, net_buffer);
        }

        // Redraw UI
        draw_layout(doc, chat_history, chat_line_count, chat_input_buffer, is_chat_active);
        usleep(16000); // ~60fps cap, prevents 100% CPU
    }

    // ---------------------------------------------------------------
    // 6. Cleanup
    // ---------------------------------------------------------------
    cleanup_display();
    close_connection(server_sock);

    for (int i = 0; i < chat_line_count; i++) {
        free((void*)chat_history[i]);
    }
    document_free(doc);

    return 0;
}
