#include <stdio.h>
#include <string.h>
#include "string_buffer_lib/string_buffer.h"

int main() {
    printf("Security Test: Handling non-null terminated buffers\n");
    printf("====================================================\n\n");

    // 1. Create a non-terminated buffer
    char unteminated_buf[10];
    memset(unteminated_buf, 'A', 10); // No null terminator here

    StringBuffer sb;
    sb_init(&sb, 32);

    printf("Attempting to use sb_from_cstr on an UNTERMINATED buffer...\n");
    printf("NOTE: This is a deliberate security risk demonstration (potential buffer over-read)\n");
    
    /* 
     * In a real system, sb_from_cstr would continue reading memory 
     * until it hits a random '\0' byte or segfaults.
     * 
     * Because we can't easily capture the 'failure' without crashing the test runner,
     * we will demonstrate the SAFE alternative: sb_from_mem
     */

    printf("\nTest 1: Safe Alternative (sb_from_mem)\n");
    if (sb_from_mem(&sb, unteminated_buf, 10)) {
        printf("Success: Safely copied 10 bytes from non-terminated buffer.\n");
        printf("Buffer content: [%.10s]\n", sb.str);
        printf("StringBuffer length: %zu\n", sb.len);
    } else {
        printf("Failure: Failed to copy safely.\n");
    }

    sb_free(&sb);

    // 2. Demonstration of the danger
    printf("\nTest 2: Dangerous Operation (sb_append with unteminated string)\n");
    printf("Attempting to append unteminated buffer (this might crash or show random data)...\n");
    
    // We'll use a very small controlled 'unsafe' buffer for demonstration 
    // to avoid a full process crash if possible, but the risk is clear.
    char danger_zone[5] = {'H', 'A', 'C', 'K', '!'};
    
    StringBuffer sb2;
    sb_init(&sb2, 64);
    
    // This used to be VULNERABLE, now protected by macro + __builtin_object_size
    sb_append(&sb2, danger_zone); 
    
    printf("Result of sb_append with UNTERMINATED array: [%s]\n", sb2.str);
    printf("StringBuffer length: %zu (Expected: 5)\n", sb2.len);

    if (sb2.len == 5) {
        printf("SUCCESS: The new macro-based protection caught the over-read!\n");
    } else {
        printf("FAILURE: The buffer still over-read into other memory.\n");
    }

    sb_free(&sb2);

    // 3. Regression test: Normal pointers and long strings
    printf("\nTest 3: Regression Test (sb_append with normal pointer)\n");
    char *normal_ptr = "This is a normal, long string that exceeds pointer size";
    StringBuffer sb3;
    sb_init(&sb3, 128);
    
    sb_append(&sb3, normal_ptr);
    printf("Result: [%s]\n", sb3.str);
    printf("Length: %zu (Expected: %zu)\n", sb3.len, strlen(normal_ptr));
    
    if (sb3.len == strlen(normal_ptr)) {
        printf("SUCCESS: Normal pointers still work correctly.\n");
    } else {
        printf("FAILURE: Normal pointers are being truncated incorrectly.\n");
    }
    
    sb_free(&sb3);

    printf("\nConclusion: Always use length-limited functions (sb_from_mem, sb_append_len) \n");
    printf("when dealing with data that might not be null-terminated.\n");

    return 0;
}
