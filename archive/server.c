/*
 * Collaborative Editor Server
 *
 * Manages client connections, authentication, and real-time document
 * synchronization between multiple editors using TCP sockets and select().
 */

#include "document.h"
#include "text_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <stdbool.h>

// Forward declare the auth function
int verify_credentials(const char* username, const char* password);

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 16384

typedef struct {
    int sock;
    int is_authenticated;
    char username[32];
} Client;

// Global state for signal handler cleanup
static volatile bool server_running = true;
static int master_socket_g = -1;
static Document* master_doc_g = NULL;

static void handle_signal(int sig) {
    (void)sig;
    printf("\nServer shutting down...\n");
    server_running = false;
}

int main() {
    int new_socket;
    struct sockaddr_in address;
    Client clients[MAX_CLIENTS];

    // Set up signal handlers for graceful shutdown
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Create the shared document
    Document* master_doc = document_create("shared_doc.txt");
    master_doc_g = master_doc;

    // Seed with welcome text
    insert_char_at_cursor(master_doc->buffer, 'W');
    insert_char_at_cursor(master_doc->buffer, 'e');
    insert_char_at_cursor(master_doc->buffer, 'l');
    insert_char_at_cursor(master_doc->buffer, 'c');
    insert_char_at_cursor(master_doc->buffer, 'o');
    insert_char_at_cursor(master_doc->buffer, 'm');
    insert_char_at_cursor(master_doc->buffer, 'e');
    insert_char_at_cursor(master_doc->buffer, '!');
    set_line_alignment(master_doc->buffer, ALIGN_CENTER);

    // Initialize client slots
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].sock = 0;
        clients[i].is_authenticated = 0;
        memset(clients[i].username, 0, sizeof(clients[i].username));
    }

    // Create master socket
    if ((master_socket_g = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Allow port reuse (prevents "Address already in use" on restart)
    int opt = 1;
    if (setsockopt(master_socket_g, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(master_socket_g, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(master_socket_g, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("=== Collaborative Editor Server ===\n");
    printf("Listening on port %d\n", PORT);
    printf("Waiting for connections...\n\n");

    socklen_t addrlen = sizeof(address);

    while (server_running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(master_socket_g, &readfds);
        int max_sd = master_socket_g;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].sock > 0) {
                FD_SET(clients[i].sock, &readfds);
            }
            if (clients[i].sock > max_sd) {
                max_sd = clients[i].sock;
            }
        }

        // Use a timeout so we can check server_running periodically
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0 && server_running) {
            perror("select error");
            continue;
        }

        // Handle new incoming connections
        if (FD_ISSET(master_socket_g, &readfds)) {
            if ((new_socket = accept(master_socket_g, (struct sockaddr*)&address, &addrlen)) < 0) {
                perror("accept");
                continue;
            }
            printf("[+] New connection from %s:%d (fd=%d)\n",
                   inet_ntoa(address.sin_addr), ntohs(address.sin_port), new_socket);

            // Find an empty slot
            bool slot_found = false;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].sock == 0) {
                    clients[i].sock = new_socket;
                    clients[i].is_authenticated = 0;
                    slot_found = true;
                    break;
                }
            }
            if (!slot_found) {
                printf("[!] Max clients reached, rejecting connection\n");
                const char* msg = "SERVER_FULL";
                send(new_socket, msg, strlen(msg), 0);
                close(new_socket);
            }
        }

        // Handle data from existing clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].sock;
            if (sd <= 0 || !FD_ISSET(sd, &readfds)) continue;

            char buffer[BUFFER_SIZE];
            int valread = read(sd, buffer, BUFFER_SIZE - 1);

            if (valread <= 0) {
                // Client disconnected
                printf("[-] %s disconnected (fd=%d)\n",
                       clients[i].username[0] ? clients[i].username : "unauthenticated client", sd);
                close(sd);
                clients[i].sock = 0;
                clients[i].is_authenticated = 0;
                clients[i].username[0] = '\0';
                continue;
            }

            buffer[valread] = '\0';

            if (!clients[i].is_authenticated) {
                // Expect: AUTH username password
                char username[32], password[32];
                if (sscanf(buffer, "AUTH %31s %31s", username, password) == 2) {
                    if (verify_credentials(username, password) != -1) {
                        clients[i].is_authenticated = 1;
                        strncpy(clients[i].username, username, sizeof(clients[i].username) - 1);
                        printf("[*] User '%s' authenticated (fd=%d)\n", username, sd);

                        // Send auth success
                        const char* success_msg = "AUTH_SUCCESS";
                        send(sd, success_msg, strlen(success_msg), 0);

                        // Small delay so client can distinguish the auth response
                        // from the subsequent document data
                        usleep(100000); // 100ms

                        // Send current document state
                        char* serialized_data = serialize_buffer(master_doc->buffer);
                        if (serialized_data) {
                            send(sd, serialized_data, strlen(serialized_data), 0);
                            free(serialized_data);
                        }
                    } else {
                        printf("[!] Auth failed for user '%s' (fd=%d)\n", username, sd);
                        const char* fail_msg = "AUTH_FAIL";
                        send(sd, fail_msg, strlen(fail_msg), 0);
                        close(sd);
                        clients[i].sock = 0;
                    }
                }
            } else {
                // Authenticated client sent a document update
                deserialize_into_buffer(master_doc->buffer, buffer);

                // Broadcast to all other authenticated clients
                char* serialized_data = serialize_buffer(master_doc->buffer);
                if (serialized_data) {
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j].is_authenticated && clients[j].sock != sd) {
                            send(clients[j].sock, serialized_data, strlen(serialized_data), 0);
                        }
                    }
                    free(serialized_data);
                }
            }
        }
    }

    // Graceful shutdown
    printf("Cleaning up...\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sock > 0) close(clients[i].sock);
    }
    close(master_socket_g);
    document_free(master_doc);

    return 0;
}
