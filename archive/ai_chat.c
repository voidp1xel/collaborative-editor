/*
 * AI Chat Module
 *
 * Provides an AI assistant interface using the DeepSeek API.
 * The API key is read from the DEEPSEEK_API_KEY environment variable.
 * Requires libcurl and libjansson.
 */

#include "ai_chat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

#define API_URL "sk-89e7da6bc2234725922a941ae738b95c"
#define API_MODEL "deepseek-chat"

// Buffer to accumulate HTTP response data
struct ResponseBuffer {
    char* data;
    size_t size;
};

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct ResponseBuffer* buf = (struct ResponseBuffer*)userp;

    char* ptr = realloc(buf->data, buf->size + realsize + 1);
    if (!ptr) return 0; // Out of memory

    buf->data = ptr;
    memcpy(buf->data + buf->size, contents, realsize);
    buf->size += realsize;
    buf->data[buf->size] = '\0';

    return realsize;
}

/**
 * Escape special characters in user input for safe JSON embedding.
 * Caller must free the returned string.
 */
static char* json_escape(const char* input) {
    // Worst case: every char becomes a 2-char escape
    size_t len = strlen(input);
    char* escaped = malloc(len * 2 + 1);
    if (!escaped) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        switch (input[i]) {
            case '"':  escaped[j++] = '\\'; escaped[j++] = '"';  break;
            case '\\': escaped[j++] = '\\'; escaped[j++] = '\\'; break;
            case '\n': escaped[j++] = '\\'; escaped[j++] = 'n';  break;
            case '\r': escaped[j++] = '\\'; escaped[j++] = 'r';  break;
            case '\t': escaped[j++] = '\\'; escaped[j++] = 't';  break;
            default:   escaped[j++] = input[i]; break;
        }
    }
    escaped[j] = '\0';
    return escaped;
}

char* get_ai_response(const char* prompt) {
    // Read API key from environment
    const char* api_key = getenv("DEEPSEEK_API_KEY");
    if (!api_key || strlen(api_key) == 0) {
        return strdup("[Set DEEPSEEK_API_KEY env var to enable AI chat]");
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        return strdup("[Error: Could not initialize HTTP client]");
    }

    struct ResponseBuffer response = { .data = malloc(1), .size = 0 };
    response.data[0] = '\0';

    // Escape user input for safe JSON embedding
    char* safe_prompt = json_escape(prompt);
    if (!safe_prompt) {
        curl_easy_cleanup(curl);
        free(response.data);
        return strdup("[Error: Memory allocation failed]");
    }

    // Build JSON payload
    char* json_payload = malloc(strlen(safe_prompt) + 256);
    sprintf(json_payload,
            "{\"model\": \"%s\", \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}]}",
            API_MODEL, safe_prompt);
    free(safe_prompt);

    // Set up HTTP headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);
    headers = curl_slist_append(headers, auth_header);

    // Configure request
    curl_easy_setopt(curl, CURLOPT_URL, API_URL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // 30 second timeout

    // Execute request
    CURLcode res = curl_easy_perform(curl);
    char* ai_response = NULL;

    if (res != CURLE_OK) {
        ai_response = strdup(curl_easy_strerror(res));
    } else {
        // Parse the JSON response
        json_error_t error;
        json_t* root = json_loads(response.data, 0, &error);
        if (!root) {
            ai_response = strdup("[Error: Invalid API response]");
        } else {
            json_t* choices = json_object_get(root, "choices");
            if (json_is_array(choices) && json_array_size(choices) > 0) {
                json_t* first = json_array_get(choices, 0);
                json_t* message = json_object_get(first, "message");
                json_t* content = json_object_get(message, "content");
                if (json_is_string(content)) {
                    ai_response = strdup(json_string_value(content));
                }
            }
            json_decref(root);
        }
    }

    // Cleanup
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(json_payload);
    free(response.data);

    return ai_response ? ai_response : strdup("[No response from AI]");
}
