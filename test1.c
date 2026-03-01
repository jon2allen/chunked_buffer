#include "chunked_buffer_lib/chunked_buffer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Test string to repeat
#define TEST_STRING "This is a test string. "

int main() {
    printf("Testing chunked buffer with 16KB chunks...\n");

    // Initialize the chunked buffer
    ChunkedBufferContext *ctx = cb_init();
    if (!ctx) {
        fprintf(stderr, "Failed to initialize chunked buffer context\n");
        return 1;
    }

    // Create a large string by repeating the test string
    size_t test_string_len = strlen(TEST_STRING);

    // Create a string that will exactly fill a chunk (minus one byte for null terminator)
    size_t reps_to_fill_chunk = (CHUNK_SIZE - 1) / test_string_len;
    size_t message_size = reps_to_fill_chunk * test_string_len;

    printf("Using %zu repetitions (%zu bytes per message)\n", reps_to_fill_chunk, message_size);

    // Add messages until we have at least 2 chunks
    for (int i = 0; i < 8; i++) {
        // Create a string that will fill a chunk
        char *message = malloc(message_size + 1); // +1 for null terminator
        if (!message) {
            fprintf(stderr, "Failed to allocate memory for test message\n");
            cb_free(ctx);
            return 1;
        }

        // Fill the message with repeating test strings
        for (size_t j = 0; j < reps_to_fill_chunk; j++) {
            strcpy(message + j * test_string_len, TEST_STRING);
        }
        message[message_size] = '\0'; // Ensure null termination

        // Add the message to the chunked buffer
        if (!cb_add_message(ctx, message)) {
            fprintf(stderr, "Failed to add message to chunked buffer\n");
            free(message);
            cb_free(ctx);
            return 1;
        }

        printf("Added message %d: %zu bytes\n", i + 1, message_size);
        free(message);
    }

    // Verify the messages
    printf("\nVerifying messages in the buffer:\n");
    Message *msg = ctx->msg_head;
    int msg_count = 0;
    char buffer[16384]; // Use 16KB buffer for retrieval

    while (msg) {
        size_t len = cb_get_message_text(ctx, msg, buffer, sizeof(buffer));
        printf("Message %d: %zu bytes\n", ++msg_count, len);

        // Verify the content
        for (size_t i = 0; i < len; i += test_string_len) {
            if (strncmp(buffer + i, TEST_STRING, test_string_len) != 0) {
                printf("Error at position %zu: Expected '%s', got '", i, TEST_STRING);
                for (size_t j = 0; j < test_string_len && (i + j) < len; j++) {
                    putchar(buffer[i + j]);
                }
                printf("'\n");
                cb_free(ctx);
                return 1;
            }
        }
        msg = msg->next;
    }

    // Verify the total bytes and message count
    printf("\nFinal stats:\n");
    printf("Total bytes stored: %zu\n", ctx->total_bytes);
    printf("Number of messages: %d\n", ctx->message_count);
    printf("Number of chunks used: ");

    // Count chunks manually
    size_t chunk_count = 0;
    Chunk *chunk = ctx->buf_head;
    while (chunk) {
        chunk_count++;
        chunk = chunk->next;
    }
    printf("%zu\n", chunk_count);

    // Free the context
    cb_free(ctx);
    printf("\nTest completed successfully!\n");

    return 0;
}

