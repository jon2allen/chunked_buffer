#include "chunked_buffer.h"
#include <stdlib.h>
#include <string.h>

// Initialize a new chunked buffer context
ChunkedBufferContext* cb_init() {
    return calloc(1, sizeof(ChunkedBufferContext));
}

// Add a message to the chunked buffer
bool cb_add_message(ChunkedBufferContext *ctx, const char *content) {
    if (!ctx || !content) return false;

    size_t len = strlen(content);
    if (len == 0) return true; // Empty message

    Message *msg = calloc(1, sizeof(Message));
    if (!msg) return false;
    msg->absolute_offset = ctx->total_bytes;
    msg->length = len;

    if (!ctx->msg_head) ctx->msg_head = msg;
    if (ctx->msg_tail) ctx->msg_tail->next = msg;
    ctx->msg_tail = msg;
    ctx->message_count++;

    size_t written = 0;
    while (written < len) {
        if (!ctx->buf_tail || ctx->buf_tail->used == CHUNK_SIZE) {
            Chunk *new_chunk = calloc(1, sizeof(Chunk));
            if (!new_chunk) {
                free(msg);
                return false;
            }
            if (!ctx->buf_head) ctx->buf_head = new_chunk;
            if (ctx->buf_tail) ctx->buf_tail->next = new_chunk;
            ctx->buf_tail = new_chunk;
        }

        size_t available = CHUNK_SIZE - ctx->buf_tail->used;
        size_t to_write = (len - written < available) ? (len - written) : available;

        memcpy(ctx->buf_tail->data + ctx->buf_tail->used, content + written, to_write);
        ctx->buf_tail->used += to_write;
        ctx->total_bytes += to_write;
        written += to_write;
    }
    return true;
}

// Retrieve a message from the chunked buffer
size_t cb_get_message_text(ChunkedBufferContext *ctx, Message *msg, char *dest, size_t dest_size) {
    if (!ctx || !msg || !dest || dest_size == 0 || msg->absolute_offset < ctx->discarded_bytes) return 0;

    size_t active_offset = msg->absolute_offset - ctx->discarded_bytes;
    size_t copied = 0;
    size_t current_chunk_offset = 0;
    Chunk *curr = ctx->buf_head;

    while (curr && current_chunk_offset + curr->used <= active_offset) {
        current_chunk_offset += curr->used;
        curr = curr->next;
    }

    while (curr && copied < msg->length && copied < (dest_size - 1)) {
        size_t chunk_start = (current_chunk_offset < active_offset) ? (active_offset - current_chunk_offset) : 0;
        size_t available = curr->used - chunk_start;

        size_t remaining_msg = msg->length - copied;
        size_t remaining_dest = (dest_size - 1) - copied;

        size_t to_copy = available;
        if (remaining_msg < to_copy) to_copy = remaining_msg;
        if (remaining_dest < to_copy) to_copy = remaining_dest;

        memcpy(dest + copied, curr->data + chunk_start, to_copy);
        copied += to_copy;
        current_chunk_offset += curr->used;
        curr = curr->next;
    }
    dest[copied] = '\0';
    return copied;
}

// Free the chunked buffer context
void cb_free(ChunkedBufferContext *ctx) {
    if (!ctx) return;
    Chunk *c = ctx->buf_head;
    while (c) {
        Chunk *next = c->next;
        free(c);
        c = next;
    }
    Message *m = ctx->msg_head;
    while (m) {
        Message *next = m->next;
        free(m);
        m = next;
    }
    free(ctx);
}


