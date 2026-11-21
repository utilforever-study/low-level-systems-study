#ifndef SEOKGUKIM_C_COMMON_TEXT_H
#define SEOKGUKIM_C_COMMON_TEXT_H

#include <stdlib.h>
#include <string.h>

// Copy a null-terminated string. The caller owns the result; NULL means
// allocation failure. This is not a byte-buffer copy for embedded NULs.
static inline char *c_text_copy(const char *text) {
    size_t length = strlen(text);
    char *copy = malloc(length + 1);
    if (copy != NULL)
        memcpy(copy, text, length + 1);
    return copy;
}

#endif /* SEOKGUKIM_C_COMMON_TEXT_H */
