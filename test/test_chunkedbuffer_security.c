#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "../chunked_buffer_lib/chunked_buffer.h"

// Test message size limits
void test_message_size_limits() {
    printf("Testing message size limits...\n");
    
    // Test with 1KB message limit
    ChunkedBufferContext *ctx = cb_init_with_limits(1024, 0); // 1KB max message
    assert(ctx != NULL);
    
    // Create a message that exceeds the limit
    char *large_msg = malloc(1500);
    memset(large_msg, 'A', 1499);
    large_msg[1499] = '\0';
    
    bool result = cb_add_message(ctx, large_msg);
    printf("  Oversized message (1.5KB with 1KB limit): %s\n", result ? "FAIL" : "PASS");
    assert(result == false); // Should be rejected
    
    // Test with acceptable message size
    char normal_msg[512];
    memset(normal_msg, 'B', sizeof(normal_msg) - 1);
    normal_msg[sizeof(normal_msg) - 1] = '\0';
    
    bool normal_result = cb_add_message(ctx, normal_msg);
    printf("  Normal message (512B with 1KB limit): %s\n", normal_result ? "PASS" : "FAIL");
    assert(normal_result == true);
    assert(ctx->message_count == 1);
    
    free(large_msg);
    cb_free(ctx);
    printf("  Message size limits: PASS\n\n");
}

// Test total size limits
void test_total_size_limits() {
    printf("Testing total size limits...\n");
    
    // Test with 10KB total limit
    ChunkedBufferContext *ctx = cb_init_with_limits(0, 10240); // 10KB total
    assert(ctx != NULL);
    
    // Add messages until we hit the limit
    char buffer[1024];
    memset(buffer, 'C', sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    int message_count = 0;
    while (true) {
        bool result = cb_add_message(ctx, buffer);
        if (!result) break;
        message_count++;
    }
    
    printf("  Added %d messages before hitting 10KB limit\n", message_count);
    assert(message_count > 0 && message_count < 20); // Should be around 10
    
    // Verify total size is at or near limit
    assert(ctx->total_bytes <= 10240);
    printf("  Total size limit enforcement: PASS\n");
    
    cb_free(ctx);
    printf("  Total size limits: PASS\n\n");
}

// Test null pointer handling
void test_null_pointer_handling() {
    printf("Testing null pointer handling...\n");
    
    // Test cb_init returns non-null
    ChunkedBufferContext *ctx = cb_init();
    printf("  cb_init(): %s\n", ctx ? "PASS" : "FAIL");
    assert(ctx != NULL);
    
    // Test cb_add_message with null context
    bool null_ctx_result = cb_add_message(NULL, "test");
    printf("  cb_add_message(NULL context): %s\n", null_ctx_result ? "FAIL" : "PASS");
    assert(null_ctx_result == false);
    
    // Test cb_add_message with null content
    bool null_content_result = cb_add_message(ctx, NULL);
    printf("  cb_add_message(NULL content): %s\n", null_content_result ? "FAIL" : "PASS");
    assert(null_content_result == false);
    
    // Test cb_get_message_text with null parameters
    Message msg;
    msg.absolute_offset = 0;
    msg.length = 10;
    char dest[100];
    
    size_t null_result = cb_get_message_text(NULL, &msg, dest, sizeof(dest));
    printf("  cb_get_message_text(NULL context): %s\n", null_result == 0 ? "PASS" : "FAIL");
    assert(null_result == 0);
    
    size_t null_msg_result = cb_get_message_text(ctx, NULL, dest, sizeof(dest));
    printf("  cb_get_message_text(NULL message): %s\n", null_msg_result == 0 ? "PASS" : "FAIL");
    assert(null_msg_result == 0);
    
    size_t null_dest_result = cb_get_message_text(ctx, &msg, NULL, sizeof(dest));
    printf("  cb_get_message_text(NULL dest): %s\n", null_dest_result == 0 ? "PASS" : "FAIL");
    assert(null_dest_result == 0);
    
    cb_free(ctx);
    printf("  Null pointer handling: PASS\n\n");
}

// Test empty message handling
void test_empty_message_handling() {
    printf("Testing empty message handling...\n");
    
    ChunkedBufferContext *ctx = cb_init();
    assert(ctx != NULL);
    
    // Test empty string
    bool empty_result = cb_add_message(ctx, "");
    printf("  Empty message: %s\n", empty_result ? "PASS" : "FAIL");
    assert(empty_result == true);
    assert(ctx->message_count == 0); // Empty messages are not counted
    
    // Test single character
    bool single_result = cb_add_message(ctx, "A");
    printf("  Single character message: %s\n", single_result ? "PASS" : "FAIL");
    assert(single_result == true);
    assert(ctx->message_count == 1);
    
    cb_free(ctx);
    printf("  Empty message handling: PASS\n\n");
}

// Test memory management
void test_memory_management() {
    printf("Testing memory management...\n");
    
    ChunkedBufferContext *ctx = cb_init();
    assert(ctx != NULL);
    
    // Add many small messages to test chunk management
    for (int i = 0; i < 100; i++) {
        char msg[50];
        snprintf(msg, sizeof(msg), "Message %d", i);
        bool result = cb_add_message(ctx, msg);
        assert(result == true);
    }
    
    assert(ctx->message_count == 100);
    printf("  Added 100 messages: PASS\n");
    
    // Verify message retrieval
    Message *msg = ctx->msg_head;
    int count = 0;
    while (msg != NULL) {
        count++;
        msg = msg->next;
    }
    assert(count == 100);
    printf("  Message count verification: PASS\n");
    
    // Verify chunk chain
    Chunk *chunk = ctx->buf_head;
    int chunk_count = 0;
    while (chunk != NULL) {
        chunk_count++;
        chunk = chunk->next;
    }
    printf("  Created %d chunks for 100 messages\n", chunk_count);
    assert(chunk_count > 0);
    
    cb_free(ctx);
    printf("  Memory management: PASS\n\n");
}

// Test edge cases
void test_edge_cases() {
    printf("Testing edge cases...\n");
    
    // Test with very small limits
    ChunkedBufferContext *ctx = cb_init_with_limits(10, 100); // 10B msg, 100B total
    assert(ctx != NULL);
    
    // Test message exactly at limit
    char limit_msg[11];
    memset(limit_msg, 'X', 10);
    limit_msg[10] = '\0';
    
    bool limit_result = cb_add_message(ctx, limit_msg);
    printf("  Message at size limit (10B): %s\n", limit_result ? "PASS" : "FAIL");
    assert(limit_result == true);
    
    // Test message one byte over limit
    char over_limit_msg[12];
    memset(over_limit_msg, 'Y', 11);
    over_limit_msg[11] = '\0';
    
    bool over_limit_result = cb_add_message(ctx, over_limit_msg);
    printf("  Message over size limit (11B): %s\n", over_limit_result ? "FAIL" : "PASS");
    assert(over_limit_result == false);
    
    cb_free(ctx);
    printf("  Edge cases: PASS\n\n");
}

int main() {
    printf("ChunkedBuffer Security Test Suite\n");
    printf("==================================\n\n");
    
    test_message_size_limits();
    test_total_size_limits();
    test_null_pointer_handling();
    test_empty_message_handling();
    test_memory_management();
    test_edge_cases();
    
    printf("All security tests completed successfully!\n");
    return 0;
}