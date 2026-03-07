#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "string_buffer_lib/string_buffer.h"
#include "chunked_buffer_lib/chunked_buffer.h"

// Function to search for a word in a line and return true if found
bool contains_word(const char *line, const char *word) {
    char *copy = strdup(line);
    if (!copy) return false;
    
    bool found = false;
    char *token = strtok(copy, " ,.!?;:\"'\n\r\t");
    while (token != NULL) {
        if (strstr(token, word) != NULL) {
            found = true;
            break;
        }
        token = strtok(NULL, " ,.!?;:\"'\n\r\t");
    }
    
    free(copy);
    return found;
}

int main() {
    printf("Shakespeare Sonnets Search: Looking for 'gold'\n\n");
    
    // Open the sonnets file
    FILE *file = fopen("sonnets.txt", "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open sonnets.txt\n");
        return 1;
    }
    
    // Use StringBuffer to read and process the file
    StringBuffer file_content;
    sb_init(&file_content, 1024 * 1024);  // 1MB initial capacity
    
    // Read entire file into StringBuffer
    char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        sb_append_len(&file_content, buffer, bytes_read);
    }
    fclose(file);
    
    printf("Loaded %zu bytes into StringBuffer\n", file_content.len);
    
    // Also store in ChunkedBuffer for demonstration
    ChunkedBufferContext *chunked = cb_init();
    
    // Process line by line - use a different approach to avoid strtok issues
    char *ptr = file_content.str;
    int line_number = 0;
    int gold_count = 0;
    
    printf("\nProcessing file...\n");
    
    while (*ptr) {
        char *line_end = strchr(ptr, '\n');
        if (line_end) {
            *line_end = '\0'; // Temporarily terminate the line
        }
        
        line_number++;
        
        // Debug: Show progress
        if (line_number % 1000 == 0) {
            printf("Processed %d lines...\n", line_number);
        }
        
        // Skip empty lines
        if (strlen(ptr) == 0) {
            if (line_end) {
                *line_end = '\n'; // Restore
                ptr = line_end + 1;
            } else {
                break;
            }
            continue;
        }
        
        // Check for golden (debug removed)
        if (strstr(ptr, "golden")) {
            // This space intentionally left blank - debug removed
        }
        
        // Check if line contains "gold"
        if (contains_word(ptr, "gold")) {
            gold_count++;
            printf("Line %d: %s\n", line_number, ptr);
            
            // Also add to chunked buffer for demonstration
            cb_add_message(chunked, ptr);
        }
        
        if (line_end) {
            *line_end = '\n'; // Restore
            ptr = line_end + 1;
        } else {
            break;
        }
    }
    
    printf("\nLines containing 'gold':\n");
    printf("========================\n");
    
    printf("\nFound 'gold' in %d lines\n", gold_count);
    
    // Demonstrate reading from chunked buffer
    if (chunked->message_count > 0) {
        printf("\nLines stored in ChunkedBuffer (%d messages):\n", chunked->message_count);
        printf("============================================\n");
        
        Message *current_msg = chunked->msg_head;
        int msg_num = 1;
        
        while (current_msg != NULL) {
            char msg_buffer[512];
            size_t msg_len = cb_get_message_text(chunked, current_msg, msg_buffer, sizeof(msg_buffer));
            printf("Message %d: %.*s\n", msg_num++, (int)msg_len, msg_buffer);
            current_msg = current_msg->next;
        }
    }
    
    // Cleanup
    cb_free(chunked);
    sb_free(&file_content);
    
    printf("\nSearch completed!\n");
    return 0;
}