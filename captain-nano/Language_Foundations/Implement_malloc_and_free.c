#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): alloc, free, merge, edge, multi
 * Match the sample test output exactly. Hidden tests use the other inputs listed above.
 */

/* Core types from the exercise (keep these) */
#define HEAP_SIZE 4096
typedef struct Block {
    size_t size;
    int used;
    struct Block* next;
} Block;

/* TODO: implement init_heap */
static _Alignas(Block) unsigned char heap[HEAP_SIZE];

static void init_heap(void) {
    Block *head = (Block *)(void *)heap;
    head->size = HEAP_SIZE - sizeof(Block);
    head->used = 0;
    head->next = NULL;
}

/* TODO: implement my_malloc */
static void *my_malloc(size_t size) {
    if (size > HEAP_SIZE - sizeof(Block))
        return NULL;
    
    if (size == 0)
        size = 8;
    else
        size = (size + 7) & ~(size_t)7;

    for (Block *b = (Block *)(void *)heap; b != NULL; b = b->next) {
        if (b->used || b->size < size)
            continue;
        
        if (b->size - size >= sizeof(Block) + 8) {
            Block *rest = (Block *)(void *)((unsigned char*)b + sizeof(Block) + size);
            rest->size = b->size - size - sizeof(Block);
            rest->used = 0;
            rest->next = b->next;

            b->size = size;
            b->next = rest;
        }

        b->used = 1;
        return (unsigned char *)b + sizeof(Block);
    }

    return NULL;
}

/* TODO: implement my_free */
static void my_free(void *ptr) {
    if (ptr == NULL)
        return;

    Block *b = (Block *)(void *)((unsigned char *)ptr - sizeof(Block));
    b->used = 0;

    for (Block *cur = (Block *)(void *)heap; cur != NULL; cur = cur->next) {
        while (!cur->used && cur->next != NULL && !cur->next->used) {
            Block *next = cur->next;
            cur->size += sizeof(Block) + next->size;
            cur->next = next->next;
        }
    }
}

/* TODO: implement print_heap */
static void print_heap(void) {
    int i = 0;
    for (Block *b = (Block *)(void *)heap; b != NULL; b = b->next, ++i)
        printf("Block %d: used=%d size=%zu\n", i, b->used, b->size);
}

/* TODO: implement count_free_blocks */
static int count_free_blocks(void) {
    int count = 0;
    for (Block *b = (Block *)(void *)heap; b != NULL; b = b->next)
        if (!b->used)
            ++count;
    return count;
}


int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    
    /* TODO: handle mode "alloc" (sample) and other modes */
    init_heap();

    if (strcmp(mode, "alloc") == 0 || strcmp(mode, "free") == 0) {
        void *p1 = my_malloc(64);
        void *p2 = my_malloc(128);
        void *p3 = my_malloc(32);

        if (strcmp(mode, "free") == 0) {
            my_free(p2);
            printf("After freeing p2:\n");
        }
        else {
            printf("After allocations:\n");
        }
        print_heap();

    }
    else if (strcmp(mode, "merge") == 0) {
        void *p1 = my_malloc(64);
        void *p2 = my_malloc(64);
        void *p3 = my_malloc(64);
        my_free(p1);
        my_free(p2);
        my_free(p3);
        printf("Blocks merged\n");
        printf("free_blocks=%d\n", count_free_blocks());

    }
    else if (strcmp(mode, "edge") == 0) {
        void *zero = my_malloc(0);
        void *big = my_malloc(4096);
        my_free(NULL);
        printf("zero=%s\n", zero ? "nonnull" : "null");
        printf("big=%s\n", big ? "nonnull" : "null");
        if (zero != NULL && big == NULL)
            printf("All edge cases pass\n");

    }
    else if (strcmp(mode, "multi") == 0) {
        void *p[8];
        int ok = 1;
        for (int i = 0; i < 8; ++i) {
            p[i] = my_malloc(32);
            if (p[i] == NULL)
                ok = 0;
        }
        for (int i = 0; i < 8; ++i)
            my_free(p[i]);
        if (ok)
            printf("All allocations successful\n");
        printf("ok=%d\n", ok);
    }

    return 0;
}
