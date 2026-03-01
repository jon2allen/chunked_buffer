#ifndef CHUNKED_BUFFER_H
#define CHUNKED_BUFFER_H

#include <stddef.h>
#include <stdbool.h>

// Define the chunk size (16KB)
#define CHUNK_SIZE 16384

// Data structure for a chunk
typedef struct Chunk {
    char data[CHUNK_SIZE];
    size_t used;
    struct Chunk *next;
} Chunk;

// Data structure for a message
typedef struct Message {
    size_t absolute_offset;
    size_t length;
    struct Message *next;
} Message;

// Data structure for the chunked buffer context
typedef struct {
    Chunk *buf_head;
    Chunk *buf_tail;
    size_t total_bytes;
    size_t discarded_bytes;
    Message *msg_head;
    Message *msg_tail;
    int message_count;
} ChunkedBufferContext;

// Function declarations
ChunkedBufferContext* cb_init();
bool cb_add_message(ChunkedBufferContext *ctx, const char *content);
size_t cb_get_message_text(ChunkedBufferContext *ctx, Message *msg, char *dest, size_t dest_size);
void cb_free(ChunkedBufferContext *ctx);

#endif // CHUNKED_BUFFER_H

