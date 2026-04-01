#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "text_buffer.h"
#include <stdbool.h>

/**
 * The Document struct acts as a high-level container for a file being edited.
 * It wraps a TextBuffer (the actual content) along with file metadata.
 */
typedef struct {
    TextBuffer* buffer;      // The text content and formatting data
    char* filepath;          // Path on disk (NULL for untitled documents)
    bool is_dirty;           // Tracks whether there are unsaved changes
} Document;

/**
 * Creates a new, empty document.
 * @param filepath  Initial path for the document (NULL for untitled).
 * @return Pointer to the new Document, or NULL on allocation failure.
 */
Document* document_create(const char* filepath);

/**
 * Frees all memory associated with a document (buffer, filepath, struct).
 * @param doc  The document to free. Safe to pass NULL.
 */
void document_free(Document* doc);

#endif // DOCUMENT_H