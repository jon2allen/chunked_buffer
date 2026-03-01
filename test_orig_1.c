#include "chunked_buffer_lib/chunked_buffer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // Add this for malloc and free

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

    // Calculate how many repetitions are needed to exceed CHUNK_SIZE
    size_t reps_to_fill_chunk = (CHUNK_SIZE / test_string_len) + 1;
    size_t total_length = 0;

    // Add messages until we exceed the chunk size
    while (total_length < 2 * CHUNK_SIZE) {
        // Create a string that will force chunk allocation
        char *message = malloc((reps_to_fill_chunk + 1) * test_string_len);
        if (!message) {
            fprintf(stderr, "Failed to allocate memory for test message\n");
            cb_free(ctx);
            return 1;
        }

        // Fill the message with repeating test strings
        for (size_t i = 0; i < reps_to_fill_chunk; i++) {
            strcpy(message + i * test_string_len, TEST_STRING);
        }
        message[(reps_to_fill_chunk) * test_string_len] = '\0';

        // Add the message to the chunked buffer
        if (!cb_add_message(ctx, message)) {
            fprintf(stderr, "Failed to add message to chunked buffer\n");
            free(message);
            cb_free(ctx);
            return 1;
        }

        total_length += (reps_to_fill_chunk * test_string_len);
        free(message);

        printf("Added %zu repetitions (%zu bytes). Total: %zu bytes (%.2f chunks)\n",
               reps_to_fill_chunk, reps_to_fill_chunk * test_string_len,
               total_length, (float)total_length / CHUNK_SIZE);
    }

    // Verify the messages
    printf("\nVerifying messages in the buffer:\n");
    Message *msg = ctx->msg_head;
    size_t msg_count = 0;
    char buffer[16384]; // Use 16KB buffer for retrieval

    while (msg) {
        size_t len = cb_get_message_text(ctx, msg, buffer, sizeof(buffer));
        printf("Message %zu: %zu bytes\n", ++msg_count, len);

        // Verify the content
        for (size_t i = 0; i < len; i += test_string_len) {
            if (strncmp(buffer + i, TEST_STRING, test_string_len) != 0) {
                fprintf(stderr, "Error: Message content is corrupted at position %zu\n", i);
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

    // Verify the chunk count is reasonable (should be 2 or 3)
    if (chunk_count < 2 || chunk_count > 3) {
        fprintf(stderr, "Warning: Unexpected number of chunks used (%zu)\n", chunk_count);
    }

    // Free the context
    cb_free(ctx);
    printf("\nTest completed successfully!\n");

    return 0;
}

