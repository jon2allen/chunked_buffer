#include <stdio.h>
#include "string_buffer_lib/string_buffer.h"

int main() {
    // Create and fill a source buffer
    StringBuffer source_sb;
    sb_init(&source_sb, 128);
    sb_append(&source_sb, "This is a test of iteration and C language in STringbuf lib");

    // Initialize the iterator
    SB_Iterator it;
    sb_iterator_init(&it, &source_sb, ' ');

    // StringBuffer to hold each "word" yielded by the generator
    StringBuffer word_sb;
    sb_init(&word_sb, 32);

    printf("Iterating through StringBuffer words:\n");
    while (sb_iterator_next(&it, &word_sb)) {
        printf("Word found: [%s] (Length: %zu)\n", word_sb.str, word_sb.len);
    }

    // Cleanup
    sb_free(&source_sb);
    sb_free(&word_sb);  
    return 0;
}
