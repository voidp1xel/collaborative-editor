#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

// Connects to the server at the given address and port.
// Returns the socket descriptor on success, -1 on failure.
int connect_to_server(const char* address, int port);

// Sends a message to the server.
void send_message_to_server(int sock, const char* message);

// Tries to receive a message from the server (non-blocking).
// Returns the number of bytes read, 0 if no data, -1 on error.
int receive_message_from_server(int sock, char* buffer, int size);

// Closes the connection to the server.
void close_connection(int sock);

#endif // NETWORK_CLIENT_H