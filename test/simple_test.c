#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../string_buffer_lib/string_buffer.h"

int main() {
    printf("Simple StringBuffer test...\n");
    
    StringBuffer sb;
    bool result = sb_init(&sb, 1024);
    printf("Init result: %d\n", result);
    assert(result == true);
    
    result = sb_append(&sb, "Hello");
    printf("Append result: %d\n", result);
    printf("Length: %zu\n", sb.len);
    printf("Capacity: %zu\n", sb.capacity);
    printf("String: '%s'\n", sb.str);
    
    sb_free(&sb);
    printf("Test completed successfully!\n");
    return 0;
}