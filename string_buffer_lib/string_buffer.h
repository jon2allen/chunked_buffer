#ifndef STRING_BUFFER_H
#define STRING_BUFFER_H

#include <stddef.h>
#include <stdbool.h>

// StringBuffer structure
typedef struct {
    char *str;       // Pointer to the dynamically allocated string
    size_t len;      // Current length of the string
    size_t capacity; // Total allocated capacity
    size_t max_capacity; // Maximum allowed capacity (0 = unlimited)
} StringBuffer;

// Function declarations
bool sb_init(StringBuffer *sb, size_t initial_cap);
bool sb_init_with_limits(StringBuffer *sb, size_t initial_cap, size_t max_capacity);
bool sb_append_len(StringBuffer *sb, const char *text, size_t text_len);
bool sb_append(StringBuffer *sb, const char *text);
void sb_append_escaped_json(StringBuffer *sb, const char *text, size_t len);
void sb_free(StringBuffer *sb);

// New functions
bool sb_copy(StringBuffer *dest, const StringBuffer *src);
size_t sb_search(const StringBuffer *sb, const char *substring);
bool sb_replace(StringBuffer *sb, const char *search, const char *replace);
bool sb_substr(const StringBuffer *sb, StringBuffer *dest, size_t start, size_t length);
bool sb_substr_c(const StringBuffer *sb, char *dest, size_t dest_size, size_t start, size_t length);

// Utility to convert C string to StringBuffer
bool sb_from_cstr(StringBuffer *sb, const char *cstr);
bool sb_from_mem(StringBuffer *sb, const char *mem, size_t length);

#endif // STRING_BUFFER_H

