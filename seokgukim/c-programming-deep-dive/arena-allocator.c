#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): alloc, reset, align, oom
 * The brief lists every input mode and the exact output.
 */

/* Core types from the exercise (keep these) */
#define ARENA_SIZE 1024
typedef struct Arena {
    char *buffer;
    size_t size;
    size_t offset;
} Arena;

Arena *arena_create(size_t size) {
    Arena *arena = malloc(sizeof *arena);
    if (!arena) {
        return NULL;
    }
    arena->buffer = malloc(size);
    if (!arena->buffer) {
        free(arena);
        return NULL;
    }
    arena->size = size;
    arena->offset = 0;
    return arena;
}

/* TODO: implement arena_alloc */
void *arena_alloc(Arena *arena, size_t size, size_t alignment) {
    if (!alignment || (alignment & (alignment - 1)) != 0) {
        return NULL;
    }

    char *next = arena->buffer + arena->offset;
    size_t padding = (-(uintptr_t)next) & (alignment - 1);
    size_t remaining = arena->size - arena->offset;
    if (padding > remaining || size > remaining - padding) {
        return NULL;
    }

    arena->offset += padding + size;
    return next + padding;
}

/* TODO: implement arena_reset */
void arena_reset(Arena *arena) { arena->offset = 0; }

/* TODO: implement arena_free */
void arena_free(Arena *arena) {
    free(arena->buffer);
    free(arena);
}

/* TODO: implement run_alloc */
int run_alloc(Arena *arena) {
    if (!arena_alloc(arena, 10 * sizeof(int), 8) ||
        !arena_alloc(arena, 5 * sizeof(double), 16) ||
        !arena_alloc(arena, 100, 1)) {
        return EXIT_FAILURE;
    }
    printf("Allocated %zu bytes\n", arena->offset);
    return EXIT_SUCCESS;
}

/* TODO: implement run_reset */
int run_reset(Arena *arena) {
    int status = run_alloc(arena);
    if (status == EXIT_SUCCESS) {
        arena_reset(arena);
        printf("After reset: %zu bytes\n", arena->offset);
    }
    return status;
}

/* TODO: implement run_align */
void run_align(Arena *arena) {
    void *ints = arena_alloc(arena, 10 * sizeof(int), 8);
    void *doubles = arena_alloc(arena, 5 * sizeof(double), 16);
    printf("8-byte aligned: %s\n",
           ints && (uintptr_t)ints % 8 == 0 ? "YES" : "NO");
    printf("16-byte aligned: %s\n",
           doubles && (uintptr_t)doubles % 16 == 0 ? "YES" : "NO");
}

/* TODO: implement run_oom */
void run_oom(Arena *arena) {
    puts(arena_alloc(arena, ARENA_SIZE + 1, 1) ? "OOM not detected"
                                               : "Out of memory handled");
}

int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    /* TODO: handle mode "alloc" (sample) and other modes */
    Arena *arena = arena_create(ARENA_SIZE);
    if (!arena) {
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;
    if (strcmp(mode, "alloc") == 0) {
        status = run_alloc(arena);
    } else if (strcmp(mode, "reset") == 0) {
        status = run_reset(arena);
    } else if (strcmp(mode, "align") == 0) {
        run_align(arena);
    } else if (strcmp(mode, "oom") == 0) {
        run_oom(arena);
    } else {
        status = run_reset(arena);
        if (status == EXIT_SUCCESS) {
            run_align(arena);
            run_oom(arena);
        }
    }
    arena_free(arena);
    if (status != EXIT_SUCCESS) {
        return status;
    }
    (void)mode;
    return 0;
}
