#include "document.h"
#include <stdlib.h>
#include <string.h>

Document* document_create(const char* filepath) {
    Document* doc = malloc(sizeof(Document));
    if (!doc) return NULL;

    doc->buffer = create_buffer();
    if (!doc->buffer) {
        free(doc);
        return NULL;
    }

    doc->filepath = filepath ? strdup(filepath) : NULL;
    doc->is_dirty = false;

    return doc;
}

void document_free(Document* doc) {
    if (!doc) return;
    destroy_buffer(doc->buffer);
    free(doc->filepath);
    free(doc);
}
