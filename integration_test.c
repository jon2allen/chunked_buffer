#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_buffer_lib/string_buffer.h"
#include "chunked_buffer_lib/chunked_buffer.h"

int main() {
    printf("Integration Test: StringBuffer + Chunked Memory\n");
    
    // Test StringBuffer
    printf("\n=== Testing StringBuffer ===\n");
    StringBuffer sb;
    sb_init(&sb, 64);  // Initialize with 64 byte capacity
    
    sb_append(&sb, "Hello from StringBuffer!");
    sb_append(&sb, " This is a test.");
    
    printf("StringBuffer content: %s\n", sb.str);
    printf("StringBuffer length: %zu\n", sb.len);
    
    sb_free(&sb);
    
    // Test ChunkedBuffer
    printf("\n=== Testing ChunkedBuffer ===\n");
    ChunkedBufferContext *cb = cb_init();
    
    const char *test_data1 = "Chunk1";
    const char *test_data2 = "Chunk2";
    
    cb_add_message(cb, test_data1);
    cb_add_message(cb, test_data2);
    
    printf("ChunkedBuffer message count: %d\n", cb->message_count);
    printf("ChunkedBuffer total bytes: %zu\n", cb->total_bytes);
    
    // Read back first message
    if (cb->msg_head) {
        char buffer[256];
        size_t read_len = cb_get_message_text(cb, cb->msg_head, buffer, sizeof(buffer));
        printf("First message: %.*s\n", (int)read_len, buffer);
    }
    
    cb_free(cb);
    
    // Combined test
    printf("\n=== Combined Test ===\n");
    StringBuffer combined;
    sb_init(&combined, 128);
    sb_append(&combined, "StringBuffer: ");
    
    ChunkedBufferContext *temp_cb = cb_init();
    cb_add_message(temp_cb, "ChunkedData");
    
    if (temp_cb->msg_head) {
        char buffer[256];
        size_t read_len = cb_get_message_text(temp_cb, temp_cb->msg_head, buffer, sizeof(buffer));
        sb_append_len(&combined, buffer, read_len);
    }
    
    cb_free(temp_cb);
    
    printf("Combined result: %s\n", combined.str);
    sb_free(&combined);
    
    printf("\nIntegration test completed successfully!\n");
    return 0;
}