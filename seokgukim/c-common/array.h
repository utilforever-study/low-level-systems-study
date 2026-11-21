#ifndef SEOKGUKIM_C_COMMON_ARRAY_H
#define SEOKGUKIM_C_COMMON_ARRAY_H

#include <stdint.h>
#include <stdlib.h>

// Grow a heap allocation geometrically. item_size must be nonzero.
// On failure, the original allocation and capacity remain unchanged.
// Initialize generated vectors with {0}. Push returns 0 or -1.
// Push copies the element; the caller manages any nested allocations.
static inline void *c_array_grow(void *items, size_t *capacity,
                                 size_t item_size) {
    if (*capacity > SIZE_MAX / 2)
        return NULL;

    size_t next = *capacity ? *capacity * 2 : 8;
    if (next > SIZE_MAX / item_size)
        return NULL;

    void *grown = realloc(items, next * item_size);
    if (grown == NULL)
        return NULL;

    *capacity = next;
    return grown;
}

#define C_COMMON_DEFINE_VECTOR(TypeName, ElementType, PushName)                \
    typedef struct {                                                           \
        ElementType *items;                                                    \
        size_t count;                                                          \
        size_t capacity;                                                       \
    } TypeName;                                                                \
                                                                               \
    static inline int PushName(TypeName *vec, ElementType item) {              \
        if (vec->count == vec->capacity) {                                     \
            void *grown =                                                      \
                c_array_grow(vec->items, &vec->capacity, sizeof *vec->items);  \
            if (grown == NULL)                                                 \
                return -1;                                                     \
            vec->items = grown;                                                \
        }                                                                      \
        vec->items[vec->count++] = item;                                       \
        return 0;                                                              \
    }

#endif /* SEOKGUKIM_C_COMMON_ARRAY_H */
