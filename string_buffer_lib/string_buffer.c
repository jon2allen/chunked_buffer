#define _GNU_SOURCE
#include "string_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Initialize a StringBuffer with an initial capacity
bool sb_init(StringBuffer *sb, size_t initial_cap) {
    if (!sb) return false;
    sb->str = malloc(initial_cap);
    if (!sb->str) return false;
    sb->capacity = initial_cap;
    sb->len = 0;
    sb->str[0] = '\0';
    return true;
}

// Append text with a specified length to a StringBuffer
bool sb_append_len(StringBuffer *sb, const char *text, size_t text_len) {
    if (!sb || !text) return false;
    if (sb->len + text_len + 1 > sb->capacity) {
        size_t new_cap = (sb->capacity + text_len) * 2;
        if (new_cap < sb->capacity) return false; // Overflow check
        char *new_str = realloc(sb->str, new_cap);
        if (!new_str) return false;
        sb->str = new_str;
        sb->capacity = new_cap;
    }
    memcpy(sb->str + sb->len, text, text_len);
    sb->len += text_len;
    sb->str[sb->len] = '\0';
    return true;
}

// Append a string to a StringBuffer with boundary safety
bool _sb_append_internal(StringBuffer *sb, const char *text, size_t bos) {
    if (!sb || !text) return false;
    
    size_t actual_len;
    if (bos == (size_t)-1) {
        // Unknown size (pointer), must trust null terminator
        actual_len = strlen(text);
    } else {
        // Known size (array), use strnlen for safety
        actual_len = strnlen(text, bos);
    }
    
    return sb_append_len(sb, text, actual_len);
}

// Append a string with JSON escaping to a StringBuffer
void sb_append_escaped_json(StringBuffer *sb, const char *text, size_t len) {
    for (size_t i = 0; i < len; i++) {
        char c = text[i];
        switch (c) {
            case '"':  sb_append(sb, "\\\""); break;
            case '\\': sb_append(sb, "\\\\"); break;
            case '\n': sb_append(sb, "\\n"); break;
            case '\r': sb_append(sb, "\\r"); break;
            case '\t': sb_append(sb, "\\t"); break;
            default:   sb_append_len(sb, &c, 1); break;
        }
    }
}

// Free the memory used by a StringBuffer
void sb_free(StringBuffer *sb) {
    if (sb) free(sb->str);
}

// Copy the contents of one StringBuffer to another
bool sb_copy(StringBuffer *dest, const StringBuffer *src) {
    if (!dest || !src) return false;

    // Make sure destination has enough capacity
    if (src->len + 1 > dest->capacity) {
        char *new_str = realloc(dest->str, src->len + 1);
        if (!new_str) return false;
        dest->str = new_str;
        dest->capacity = src->len + 1;
    }

    memcpy(dest->str, src->str, src->len + 1);
    dest->len = src->len;
    return true;
}

// Search for a substring in a StringBuffer
// Returns the starting index or -1 if not found
size_t sb_search(const StringBuffer *sb, const char *substring) {
    if (!sb || !substring || sb->len == 0 || strlen(substring) == 0) return -1;

    char *result = strstr(sb->str, substring);
    if (result) {
        return result - sb->str;
    }
    return -1;
}

// Search and replace in a StringBuffer
bool sb_replace(StringBuffer *sb, const char *search, const char *replace) {
    if (!sb || !search || !replace) return false;

    size_t pos = sb_search(sb, search);
    if (pos == (size_t)-1) return false; // Not found

    size_t search_len = strlen(search);
    size_t replace_len = strlen(replace);

    // Special case: same length replacement
    if (search_len == replace_len) {
        memcpy(sb->str + pos, replace, replace_len);
        return true;
    }

    // Need to adjust buffer size
    size_t new_len = sb->len - search_len + replace_len;
    if (new_len + 1 > sb->capacity) {
        size_t new_cap = (sb->capacity + (new_len - sb->len)) * 2;
        char *new_str = realloc(sb->str, new_cap);
        if (!new_str) return false;
        sb->str = new_str;
        sb->capacity = new_cap;
    }

    // Move the tail of the string
    memmove(sb->str + pos + replace_len, sb->str + pos + search_len, sb->len - pos - search_len + 1);

    // Insert the replacement
    memcpy(sb->str + pos, replace, replace_len);

    // Update length
    sb->len = new_len;
    sb->str[sb->len] = '\0';
    return true;
}

// Extract a substring into another StringBuffer
bool sb_substr(const StringBuffer *sb, StringBuffer *dest, size_t start, size_t length) {
    if (!sb || !dest) return false;

    // Check bounds
    if (start >= sb->len) {
        dest->len = 0;
        if (dest->capacity > 0) {
            dest->str[0] = '\0';
        }
        return true;
    }

    // Adjust length if it would extend past end of string
    size_t actual_length = (start + length > sb->len) ? (sb->len - start) : length;

    // Make sure destination has enough capacity
    if (actual_length + 1 > dest->capacity) {
        char *new_str = realloc(dest->str, actual_length + 1);
        if (!new_str) return false;
        dest->str = new_str;
        dest->capacity = actual_length + 1;
    }

    memcpy(dest->str, sb->str + start, actual_length);
    dest->str[actual_length] = '\0';
    dest->len = actual_length;
    return true;
}

// Extract a substring into a C string
bool sb_substr_c(const StringBuffer *sb, char *dest, size_t dest_size, size_t start, size_t length) {
    if (!sb || !dest || dest_size == 0) return false;

    // Check bounds
    if (start >= sb->len) {
        dest[0] = '\0';
        return true;
    }

    // Adjust length if it would extend past end of string or destination size
    size_t actual_length = (start + length > sb->len) ? (sb->len - start) : length;
    if (actual_length >= dest_size) {
        actual_length = dest_size - 1;
    }

    memcpy(dest, sb->str + start, actual_length);
    dest[actual_length] = '\0';
    return true;
}

// Convert C string to StringBuffer (with null termination checking)
bool sb_from_cstr(StringBuffer *sb, const char *cstr) {
    if (!sb || !cstr) return false;

    // Find length of the C string, ensuring it's null-terminated
    size_t len = 0;
    while (cstr[len] != '\0') {
        len++;
        // Verify we don't go past any reasonable limit for a string
        if (len > 1024 * 1024 * 1024) { // 1GB limit for safety
            return false;
        }
    }

    // Make sure we have enough capacity
    if (len + 1 > sb->capacity) {
        char *new_str = realloc(sb->str, len + 1);
        if (!new_str) return false;
        sb->str = new_str;
        sb->capacity = len + 1;
    }

    // Copy the string, including null terminator
    memcpy(sb->str, cstr, len + 1);
    sb->len = len;
    return true;
}

// Convert memory block to StringBuffer (no null termination required)
bool sb_from_mem(StringBuffer *sb, const char *mem, size_t length) {
    if (!sb || !mem) return false;

    // Make sure we have enough capacity
    if (length + 1 > sb->capacity) {
        char *new_str = realloc(sb->str, length + 1);
        if (!new_str) return false;
        sb->str = new_str;
        sb->capacity = length + 1;
    }

    // Copy the memory block and add null terminator
    memcpy(sb->str, mem, length);
    sb->str[length] = '\0';
    sb->len = length;
    return true;
}


// iterator init - takes a iterator struct, buffer and delim
//

void sb_iterator_init(SB_Iterator *it, const StringBuffer *sb, char delimiter) {
    it->source = sb;
    it->current_pos = 0;
    it->delimiter = delimiter;
}

 //Yields the next "word" into the provided dest StringBuffer.
 // Returns true if a word was found, false if at the end of the buffer.

bool sb_iterator_next(SB_Iterator *it, StringBuffer *dest) {
    if (!it || !it->source || it->current_pos >= it->source->len) {
        return false;
    }

    // 1. Skip leading delimiters
    while (it->current_pos < it->source->len && 
           it->source->str[it->current_pos] == it->delimiter) {
        it->current_pos++;
    }

    // 2. Check if we hit the end after skipping
    if (it->current_pos >= it->source->len) {
        return false;
    }

    // 3. Find the end of the current word
    size_t start = it->current_pos;
    while (it->current_pos < it->source->len && 
           it->source->str[it->current_pos] != it->delimiter) {
        it->current_pos++;
    }

    size_t length = it->current_pos - start;

    // 4. Use the  sb_substr to return the word as a StringBuffer
    return sb_substr(it->source, dest, start, length);
}
