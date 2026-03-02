#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_buffer_lib/string_buffer.h"
#include "chunked_buffer_lib/chunked_buffer.h"

// Test reading sonnets into StringBuffer and storing in ChunkedBuffer
int main() {
    printf("Sonnet Integration Test: StringBuffer → ChunkedBuffer\n");
    printf("===================================================\n\n");

    // Initialize chunked buffer with reasonable limits
    ChunkedBufferContext *cb_ctx = cb_init();
    if (!cb_ctx) {
        printf("ERROR: Failed to initialize chunked buffer\n");
        return 1;
    }

    // Note: Current chunked buffer implementation doesn't have explicit size limits
    // It will grow as needed, but we'll monitor the total size

    FILE *file = fopen("sonnets.txt", "r");
    if (!file) {
        printf("ERROR: Could not open sonnets.txt\n");
        cb_free(cb_ctx);
        return 1;
    }

    StringBuffer line_buffer;
    if (!sb_init(&line_buffer, 256)) {
        printf("ERROR: Failed to initialize string buffer\n");
        fclose(file);
        cb_free(cb_ctx);
        return 1;
    }

    char temp_line[1024];
    int line_count = 0;
    int message_count = 0;

    printf("Reading sonnets.txt and storing lines in chunked buffer...\n");

    while (fgets(temp_line, sizeof(temp_line), file)) {
        line_count++;
        
        // Clear the string buffer for each line
        line_buffer.len = 0;
        line_buffer.str[0] = '\0';
        
        // Append the line to string buffer
        if (!sb_append(&line_buffer, temp_line)) {
            printf("ERROR: Failed to append line %d to string buffer\n", line_count);
            break;
        }
        
        // Add the string buffer content to chunked buffer
        if (!cb_add_message(cb_ctx, line_buffer.str)) {
            printf("ERROR: Failed to add line %d to chunked buffer\n", line_count);
            break;
        }
        
        message_count++;
        
        // Show progress every 100 lines
        if (line_count % 100 == 0) {
            printf("  Processed %d lines...\n", line_count);
        }
    }

    fclose(file);

    // Count chunks used
    int chunk_count = 0;
    Chunk *current_chunk = cb_ctx->buf_head;
    while (current_chunk) {
        chunk_count++;
        current_chunk = current_chunk->next;
    }

    printf("\nResults:\n");
    printf("  Lines read from file: %d\n", line_count);
    printf("  Messages stored in chunked buffer: %d\n", message_count);
    printf("  Total size in chunked buffer: %zu bytes\n", cb_ctx->total_bytes);
    printf("  Number of messages stored: %d\n", cb_ctx->message_count);
    printf("  Number of chunks used: %d\n", chunk_count);
    printf("  Average bytes per chunk: %zu\n", 
           chunk_count > 0 ? cb_ctx->total_bytes / chunk_count : 0);

    // Verify we can retrieve messages
    printf("\nVerifying message retrieval...\n");
    
    StringBuffer retrieve_buffer;
    if (!sb_init(&retrieve_buffer, 256)) {
        printf("ERROR: Failed to initialize retrieval buffer\n");
        cb_free(cb_ctx);
        return 1;
    }

    // Test retrieving a few messages
    int test_messages = message_count < 5 ? message_count : 5;
    for (int i = 0; i < test_messages; i++) {
        // Get message by traversing the message list
        Message *msg = cb_ctx->msg_head;
        int current_msg = 0;
        while (msg && current_msg < i) {
            msg = msg->next;
            current_msg++;
        }
        
        if (!msg) {
            printf("ERROR: Failed to find message %d\n", i);
            break;
        }
        
        // Clear and reuse the retrieval buffer
        retrieve_buffer.len = 0;
        retrieve_buffer.str[0] = '\0';
        
        // Get message text (using the buffer's capacity as dest_size)
        size_t text_len = cb_get_message_text(cb_ctx, msg, retrieve_buffer.str, retrieve_buffer.capacity);
        if (text_len > 0) {
            retrieve_buffer.len = text_len;
            printf("  Message %d: %.50s...\n", i, retrieve_buffer.str);
        } else {
            printf("ERROR: Failed to get text for message %d\n", i);
        }
    }

    // String search test - find lines containing specific words
    printf("\nString Search Test:\n");
    printf("Searching for lines containing 'gold' and 'love'...\n");
    
    const char *search_terms[] = {"gold", "love"};
    int search_term_count = sizeof(search_terms) / sizeof(search_terms[0]);
    
    StringBuffer search_buffer;
    if (!sb_init(&search_buffer, 256)) {
        printf("ERROR: Failed to initialize search buffer\n");
        cb_free(cb_ctx);
        return 1;
    }
    
    int found_counts[search_term_count];
    for (int i = 0; i < search_term_count; i++) {
        found_counts[i] = 0;
    }
    
    // Search through all messages
    Message *msg = cb_ctx->msg_head;
    int msg_index = 0;
    
    while (msg) {
        // Get the message text
        search_buffer.len = 0;
        search_buffer.str[0] = '\0';
        
        size_t text_len = cb_get_message_text(cb_ctx, msg, search_buffer.str, search_buffer.capacity);
        if (text_len > 0) {
            search_buffer.len = text_len;
            
            // Search for each term in this message
            for (int term_idx = 0; term_idx < search_term_count; term_idx++) {
                if (strstr(search_buffer.str, search_terms[term_idx])) {
                    found_counts[term_idx]++;
                    
                    // Show first few matches for each term
                    if (found_counts[term_idx] <= 3) {
                        printf("  Found '%s' in message %d: %.60s...\n", 
                               search_terms[term_idx], msg_index, search_buffer.str);
                    }
                }
            }
        }
        
        msg = msg->next;
        msg_index++;
    }
    
    printf("\nSearch Results:\n");
    for (int i = 0; i < search_term_count; i++) {
        printf("  '%s': found in %d lines\n", search_terms[i], found_counts[i]);
    }

    printf("\nIntegration test completed successfully!\n");

    // Cleanup
    cb_free(cb_ctx);
    // No need to free string buffers - they're on the stack
    
    return 0;
}