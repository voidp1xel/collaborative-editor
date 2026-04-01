#include "network_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h> // For non-blocking sockets

int connect_to_server(const char* address, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        close(sock);
        return -1;
    }
    
    // Set the socket to be non-blocking
    fcntl(sock, F_SETFL, O_NONBLOCK);
    
    return sock;
}

void send_message_to_server(int sock, const char* message) {
    send(sock, message, strlen(message), 0);
}

int receive_message_from_server(int sock, char* buffer, int size) {
    return recv(sock, buffer, size - 1, 0);
}

void close_connection(int sock) {
    close(sock);
}