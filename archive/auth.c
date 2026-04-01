/*
 * Authentication Module
 * Handles user credential verification against a flat-file database.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERNAME_LENGTH 32
#define USER_DATABASE_FILE "users.txt"

/**
 * Add a new user to the database file.
 * Returns 0 on success, -1 on failure.
 */
int add_user(const char* username, const char* password) {
    FILE* file = fopen(USER_DATABASE_FILE, "a");
    if (!file) {
        perror("Error: Could not open user database");
        return -1;
    }

    fprintf(file, "%s:%s\n", username, password);
    fclose(file);
    return 0;
}

/**
 * Verify user credentials against the database.
 * Returns a positive user_id on success, -1 on failure.
 */
int verify_credentials(const char* username, const char* password) {
    FILE* file = fopen(USER_DATABASE_FILE, "r");
    if (!file) {
        perror("Error: Could not open user database");
        return -1;
    }

    char line[256];
    char stored_username[MAX_USERNAME_LENGTH];
    char stored_password[MAX_USERNAME_LENGTH];
    int user_id = 1;

    while (fgets(line, sizeof(line), file)) {
        // Strip trailing newline
        line[strcspn(line, "\r\n")] = '\0';

        if (sscanf(line, "%31[^:]:%31s", stored_username, stored_password) == 2) {
            if (strcmp(username, stored_username) == 0 &&
                strcmp(password, stored_password) == 0) {
                fclose(file);
                return user_id;
            }
            user_id++;
        }
    }

    fclose(file);
    return -1;
}
