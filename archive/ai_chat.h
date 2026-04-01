#ifndef AI_CHAT_H
#define AI_CHAT_H

// Takes a user's prompt and returns a dynamically allocated string with the AI's response.
// The caller is responsible for freeing the returned string.
char* get_ai_response(const char* prompt);

#endif // AI_CHAT_H