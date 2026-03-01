#include "string_buffer_lib/string_buffer.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define TEST_STRING "Hello, world!"
#define SEARCH_STRING "world"
#define REPLACE_STRING "everyone"
#define SUBSTRING_START 7
#define SUBSTRING_LENGTH 5

void test_init() {
    printf("Testing sb_init...\n");
    StringBuffer sb;
    assert(sb_init(&sb, 10) == true);
    assert(sb.len == 0);
    assert(sb.capacity >= 10);
    assert(strcmp(sb.str, "") == 0);
    sb_free(&sb);
    printf("  OK\n");
}

void test_append() {
    printf("Testing sb_append...\n");
    StringBuffer sb;
    assert(sb_init(&sb, 10) == true);

    assert(sb_append(&sb, TEST_STRING) == true);
    assert(sb.len == strlen(TEST_STRING));
    assert(strcmp(sb.str, TEST_STRING) == 0);

    // Test automatic resizing
    char large_string[100];
    memset(large_string, 'a', sizeof(large_string) - 1);
    large_string[sizeof(large_string) - 1] = '\0';
    assert(sb_append(&sb, large_string) == true);
    assert(sb.len == strlen(TEST_STRING) + strlen(large_string));
    sb_free(&sb);
    printf("  OK\n");
}

void test_append_len() {
    printf("Testing sb_append_len...\n");
    StringBuffer sb;
    assert(sb_init(&sb, 10) == true);

    assert(sb_append_len(&sb, TEST_STRING, strlen(TEST_STRING)) == true);
    assert(sb.len == strlen(TEST_STRING));
    assert(strcmp(sb.str, TEST_STRING) == 0);

    // Test with exact length
    assert(sb_append_len(&sb, "123", 3) == true);
    assert(strcmp(sb.str, "Hello, world!123") == 0);
    sb_free(&sb);
    printf("  OK\n");
}

void test_append_escaped_json() {
    printf("Testing sb_append_escaped_json...\n");
    StringBuffer sb;
    assert(sb_init(&sb, 10) == true);

    const char *test_str = "Test \"string\". \\ \n \r \t";
    sb_append_escaped_json(&sb, test_str, strlen(test_str));
    assert(strcmp(sb.str, "Test \\\"string\\\". \\\\ \\n \\r \\t") == 0);
    sb_free(&sb);
    printf("  OK\n");
}

void test_copy() {
    printf("Testing sb_copy...\n");
    StringBuffer sb1, sb2;
    assert(sb_init(&sb1, 30) == true);
    assert(sb_init(&sb2, 10) == true);

    sb_append(&sb1, TEST_STRING);
    assert(sb_copy(&sb2, &sb1) == true);
    assert(strcmp(sb2.str, TEST_STRING) == 0);
    assert(sb2.len == sb1.len);

    // Test with different capacities
    StringBuffer sb3;
    assert(sb_init(&sb3, 50) == true);
    assert(sb_copy(&sb3, &sb1) == true);
    sb_free(&sb1);
    sb_free(&sb2);
    sb_free(&sb3);
    printf("  OK\n");
}

void test_search() {
    printf("Testing sb_search...\n");
    StringBuffer sb;
    assert(sb_init(&sb, 30) == true);
    sb_append(&sb, TEST_STRING);

    assert(sb_search(&sb, SEARCH_STRING) == 7);
    assert(sb_search(&sb, "not found") == (size_t)-1);
    assert(sb_search(&sb, "") == (size_t)-1);
    assert(sb_search(NULL, "test") == (size_t)-1);
    assert(sb_search(&sb, NULL) == (size_t)-1);
    sb_free(&sb);
    printf("  OK\n");
}

void test_replace() {
    printf("Testing sb_replace...\n");
    StringBuffer sb;
    assert(sb_init(&sb, 30) == true);
    sb_append(&sb, TEST_STRING);

    // Debug print before replacement
    printf("  Before replace: %s (len: %zu)\n", sb.str, sb.len);

    assert(sb_replace(&sb, SEARCH_STRING, REPLACE_STRING) == true);
    printf("  After replace: %s (len: %zu)\n", sb.str, sb.len);

    // The expected string after replacing "world" with "everyone"
    assert(strcmp(sb.str, "Hello, everyone!") == 0);

    // Correct the expected length to match the actual replacement
    // "Hello, world!" (13 chars) -> "Hello, everyone!" (15 chars)
    assert(sb.len == 16);

    // Test with equal length replacement
    assert(sb_replace(&sb, "everyone", "world") == true);
    printf("  After second replace: %s (len: %zu)\n", sb.str, sb.len);
    assert(strcmp(sb.str, "Hello, world!") == 0);
    assert(sb.len == 13);

    // Test with non-existent search string
    assert(sb_replace(&sb, "not found", "test") == false);
    sb_free(&sb);
    printf("  OK\n");
}

void test_substr() {
    printf("Testing sb_substr...\n");
    StringBuffer sb1, sb2;
    assert(sb_init(&sb1, 30) == true);
    assert(sb_init(&sb2, 30) == true);
    sb_append(&sb1, TEST_STRING);

    assert(sb_substr(&sb1, &sb2, SUBSTRING_START, SUBSTRING_LENGTH) == true);
    assert(strcmp(sb2.str, "world") == 0);
    assert(sb2.len == 5);

    // Test edge cases
    assert(sb_substr(&sb1, &sb2, 0, 0) == true); // Empty substring at start
    assert(sb2.len == 0);

    assert(sb_substr(&sb1, &sb2, 100, 5) == true); // Beyond end of string
    assert(sb2.len == 0);

    sb_free(&sb1);
    sb_free(&sb2);
    printf("  OK\n");
}

void test_substr_c() {
    printf("Testing sb_substr_c...\n");
    StringBuffer sb;
    assert(sb_init(&sb, 30) == true);
    sb_append(&sb, TEST_STRING);

    // Test normal substring
    char dest[20];
    assert(sb_substr_c(&sb, dest, sizeof(dest), SUBSTRING_START, SUBSTRING_LENGTH) == true);
    printf("  Substring (7,5): %s\n", dest);
    assert(strcmp(dest, "world") == 0);

    // Test edge cases
    assert(sb_substr_c(&sb, dest, sizeof(dest), 0, 0) == true); // Empty substring
    printf("  Empty substring: %s\n", dest);
    assert(strcmp(dest, "") == 0);

    assert(sb_substr_c(&sb, dest, sizeof(dest), 100, 5) == true); // Beyond end
    printf("  Beyond end: %s\n", dest);
    assert(strcmp(dest, "") == 0);

    // Test truncation - adjust our expectation to match actual behavior
    memset(dest, 0, sizeof(dest)); // Clear the buffer
    assert(sb_substr_c(&sb, dest, 3, SUBSTRING_START, SUBSTRING_LENGTH) == true);
    printf("  Truncated substring: %s\n", dest);

    // The actual substring from position 7 is "world"
    // The first 2 characters that fit in our 3-byte buffer are "wo"
    assert(strcmp(dest, "wo") == 0);

    sb_free(&sb);
    printf("  OK\n");
}


void test_from_cstr() {
    printf("Testing sb_from_cstr...\n");
    StringBuffer sb;
    assert(sb_init(&sb, 10) == true);

    assert(sb_from_cstr(&sb, TEST_STRING) == true);
    assert(strcmp(sb.str, TEST_STRING) == 0);
    assert(sb.len == strlen(TEST_STRING));

    // Test with empty string
    assert(sb_from_cstr(&sb, "") == true);
    assert(strcmp(sb.str, "") == 0);
    assert(sb.len == 0);

    // Test with NULL (should fail)
    assert(sb_from_cstr(&sb, NULL) == false);
    sb_free(&sb);
    printf("  OK\n");
}

void test_from_mem() {
    printf("Testing sb_from_mem...\n");
    StringBuffer sb;
    assert(sb_init(&sb, 10) == true);

    const char mem_block[] = "Memory block";
    assert(sb_from_mem(&sb, mem_block, sizeof(mem_block)) == true);
    assert(strcmp(sb.str, "Memory block") == 0);
    assert(sb.len == sizeof(mem_block));

    // Test with empty memory block
    assert(sb_from_mem(&sb, mem_block, 0) == true);
    assert(strcmp(sb.str, "") == 0);
    assert(sb.len == 0);

    // Test with NULL (should fail)
    assert(sb_from_mem(&sb, NULL, 10) == false);
    sb_free(&sb);
    printf("  OK\n");
}

void run_all_tests() {
    printf("Running StringBuffer tests...\n\n");
    test_init();
    test_append();
    test_append_len();
    test_append_escaped_json();
    test_copy();
    test_search();
    test_replace();
    test_substr();
    test_substr_c();
    test_from_cstr();
    test_from_mem();
    printf("\nAll tests passed!\n");
}

int main() {
    run_all_tests();
    return 0;
}

