#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "string_buffer_lib/string_buffer.h"
#include "chunked_buffer_lib/chunked_buffer.h"

// Function to search for a word in StringBuffer and return matches
int search_stringbuffer(StringBuffer *sb, const char *word, ChunkedBufferContext *chunked) {
    char *line = strtok(sb->str, "\n");
    int match_count = 0;
    
    while (line != NULL) {
        // Skip empty lines
        if (strlen(line) == 0) {
            line = strtok(NULL, "\n");
            continue;
        }
        
        // Check if line contains the word
        if (strstr(line, word)) {
            match_count++;
            if (chunked) {
                cb_add_message(chunked, line);
            }
        }
        
        line = strtok(NULL, "\n");
    }
    
    return match_count;
}

// Function to load file into StringBuffer with timing
double load_file_into_stringbuffer(const char *filename, StringBuffer *sb, size_t initial_cap) {
    clock_t start = clock();
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open %s\n", filename);
        return -1.0;
    }
    
    sb_init(sb, initial_cap);
    
    char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        sb_append_len(sb, buffer, bytes_read);
    }
    fclose(file);
    
    clock_t end = clock();
    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

int main() {
    printf("Performance Test: StringBuffer + Chunked Memory\n");
    printf("================================================\n\n");
    
    const char *test_word = "gold";
    const char *filename = "sonnets.txt";
    
    // Test different chunk sizes
    size_t chunk_sizes[] = {1024, 4096, 16384, 32768};
    int num_tests = sizeof(chunk_sizes) / sizeof(chunk_sizes[0]);
    
    for (int i = 0; i < num_tests; i++) {
        size_t chunk_size = chunk_sizes[i];
        printf("Testing with %zu byte chunks:\n", chunk_size);
        printf("------------------------------\n");
        
        // Load file into StringBuffer with timing
        StringBuffer sb;
        double load_time = load_file_into_stringbuffer(filename, &sb, chunk_size);
        if (load_time < 0) {
            continue;
        }
        
        printf("Loaded %zu bytes in %.6f seconds\n", sb.len, load_time);
        
        // Search for the word with timing
        clock_t search_start = clock();
        ChunkedBufferContext *chunked = cb_init();
        int matches = search_stringbuffer(&sb, test_word, chunked);
        clock_t search_end = clock();
        double search_time = ((double)(search_end - search_start)) / CLOCKS_PER_SEC;
        
        printf("Found %d matches in %.6f seconds\n", matches, search_time);
        printf("Chunked buffer contains %d messages\n", chunked->message_count);
        
        // Cleanup
        cb_free(chunked);
        sb_free(&sb);
        
        printf("Total time: %.6f seconds\n\n", load_time + search_time);
    }
    
    // Comparative test: different search patterns
    printf("Comparative Search Tests:\n");
    printf("========================\n\n");
    
    const char *search_words[] = {"gold", "love", "time", "beauty", "heart"};
    int num_words = sizeof(search_words) / sizeof(search_words[0]);
    
    for (int i = 0; i < num_words; i++) {
        const char *word = search_words[i];
        
        // Load file
        StringBuffer sb;
        double load_time = load_file_into_stringbuffer(filename, &sb, 16384);
        if (load_time < 0) continue;
        
        // Search
        clock_t search_start = clock();
        ChunkedBufferContext *chunked = cb_init();
        int matches = search_stringbuffer(&sb, word, chunked);
        clock_t search_end = clock();
        double search_time = ((double)(search_end - search_start)) / CLOCKS_PER_SEC;
        
        printf("Word '%s': %d matches in %.6f seconds\n", word, matches, search_time);
        
        // Cleanup
        cb_free(chunked);
        sb_free(&sb);
    }
    
    printf("\nPerformance test completed!\n");
    return 0;
}
