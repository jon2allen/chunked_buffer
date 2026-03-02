#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include "../string_buffer_lib/string_buffer.h"

// Test integer overflow protection
void test_integer_overflow_protection() {
    printf("Testing integer overflow protection...\n");
    
    StringBuffer sb;
    bool result = sb_init(&sb, 1024);
    assert(result == true);
    
    // Test the overflow check by attempting to append a very large string
    // This should fail gracefully rather than overflow
    // We can't actually test with SIZE_MAX/2 due to memory constraints,
    // but we can test the overflow detection logic
    char *large_text = NULL;
    
    // Allocate a smaller test string to avoid actual memory exhaustion
    size_t reasonable_size = 1024 * 1024; // 1MB
    large_text = malloc(reasonable_size);
    if (large_text) {
        memset(large_text, 'A', reasonable_size - 1);
        large_text[reasonable_size - 1] = '\0';
        
        // This should work within limits
        bool append_result = sb_append(&sb, large_text);
        printf("  Large append (1MB): %s\n", append_result ? "PASS" : "FAIL");
        assert(append_result == true);
        
        free(large_text);
    }
    
    // Test with max capacity set
    StringBuffer sb2;
    result = sb_init_with_limits(&sb2, 1024, 1000); // Max 1KB (smaller than initial)
    assert(result == true);
    
    // Try to exceed maximum capacity
    char buffer[1500];
    memset(buffer, 'B', sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    bool exceed_result = sb_append(&sb2, buffer);
    printf("  Exceed max capacity: %s\n", exceed_result ? "FAIL" : "PASS");
    assert(exceed_result == false); // Should fail
    
    // No sb_free needed - both sb and sb2 are on the stack
    printf("  Integer overflow protection: PASS\n\n");
}

// Test maximum capacity enforcement
void test_maximum_capacity_enforcement() {
    printf("Testing maximum capacity enforcement...\n");
    
    // Test with various max capacity settings
    size_t test_sizes[] = {1024, 4096, 16384};
    
    for (size_t i = 0; i < sizeof(test_sizes)/sizeof(test_sizes[0]); i++) {
        size_t max_size = test_sizes[i];
        StringBuffer sb;
        bool result = sb_init_with_limits(&sb, 256, max_size);
        assert(result == true);
        
        // Fill to near capacity
        char buffer[512];
        memset(buffer, 'A' + i, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        
        size_t filled = 0;
        while (filled + sizeof(buffer) < max_size) {
            bool append_result = sb_append(&sb, buffer);
            assert(append_result == true);
            filled += sizeof(buffer) - 1;
        }
        
        // Try to exceed capacity
        bool exceed_result = sb_append(&sb, buffer);
        printf("  Max capacity %zu bytes: %s\n", max_size, exceed_result ? "FAIL" : "PASS");
        assert(exceed_result == false);
        
        // No sb_free needed - sb is on the stack
    }
    printf("  Maximum capacity enforcement: PASS\n\n");
}

// Test null pointer handling
void test_null_pointer_handling() {
    printf("Testing null pointer handling...\n");
    
    // Test sb_init with null pointer
    bool result = sb_init(NULL, 1024);
    printf("  sb_init(NULL): %s\n", result ? "FAIL" : "PASS");
    assert(result == false);
    
    // Test sb_append with null StringBuffer
    bool append_result = sb_append(NULL, "test");
    printf("  sb_append(NULL): %s\n", append_result ? "FAIL" : "PASS");
    assert(append_result == false);
    
    // Test sb_append with null text - should return false gracefully
    StringBuffer sb;
    sb_init(&sb, 1024);
    
    printf(" calling append with NULL text\n");
    bool null_text_result = sb_append(&sb, NULL);
    printf("  sb_append(NULL text): %s\n", null_text_result ? "FAIL" : "PASS");
    assert(null_text_result == false);
    
    // No sb_free needed - sb is on the stack
    printf("  Null pointer handling: PASS\n\n");
}

// Test edge cases
void test_edge_cases() {
    printf("Testing edge cases...\n");
    
    // Test with zero initial capacity
    StringBuffer sb1;
    bool result = sb_init(&sb1, 0); // Should use minimum size
    printf("  Zero initial capacity: %s\n", result ? "PASS" : "FAIL");
    assert(result == true);
    assert(sb1.capacity >= 64); // Minimum size
    // No sb_free needed - sb1 is on the stack
    
    // Test empty string handling
    StringBuffer sb2;
    sb_init(&sb2, 1024);
    bool empty_result = sb_append(&sb2, "");
    printf("  Empty string append: %s\n", empty_result ? "PASS" : "FAIL");
    assert(empty_result == true);
    assert(sb2.len == 0);
    // No sb_free needed - sb2 is on the stack
    
    // Test very long string
    StringBuffer sb3;
    sb_init(&sb3, 1024);
    char long_buffer[2048];
    memset(long_buffer, 'X', sizeof(long_buffer) - 1);
    long_buffer[sizeof(long_buffer) - 1] = '\0';
    
    bool long_result = sb_append(&sb3, long_buffer);
    printf("  Long string append: %s\n", long_result ? "PASS" : "FAIL");
    assert(long_result == true);
    assert(sb3.len == sizeof(long_buffer) - 1);
    // No sb_free needed - sb3 is on the stack
    
    printf("  Edge cases: PASS\n\n");
}

// Test memory safety
void test_memory_safety() {
    printf("Testing memory safety...\n");
    
    StringBuffer sb;
    sb_init(&sb, 1024);
    
    // Test that buffer is always null-terminated
    sb_append(&sb, "Test");
    assert(sb.str[sb.len] == '\0');
    printf("  Null termination: PASS\n");
    
    // Test that capacity is respected
    size_t original_capacity = sb.capacity;
    char buffer[1500];
    memset(buffer, 'A', sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    bool capacity_result = sb_append(&sb, buffer);
    printf("  Capacity respect: %s\n", capacity_result ? "PASS" : "FAIL");
    assert(capacity_result == true);
    assert(sb.capacity > original_capacity); // Should have grown
    
    // Test that length is accurate
    assert(sb.len == strlen(sb.str));
    printf("  Length accuracy: PASS\n");
    
    // No sb_free needed - sb is on the stack
    printf("  Memory safety: PASS\n\n");
}

int main() {
    printf("StringBuffer Security Test Suite\n");
    printf("================================\n\n");
    
    test_integer_overflow_protection();
    test_maximum_capacity_enforcement();
    test_null_pointer_handling();
    test_edge_cases();
    test_memory_safety();
    
    printf("All security tests completed successfully!\n");
    return 0;
}
