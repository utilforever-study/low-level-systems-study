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

char heap[HEAP_SIZE];

/* TODO: implement init_heap */
void init_heap()
{
    Block* block = (struct Block*) heap;

    block->size = HEAP_SIZE - sizeof(Block);
    block->used = 0;
    block->next = NULL;
}

/* TODO: implement my_malloc */
void* my_malloc(int size)
{
    if (size <= 0) { size = 8; }
    int allocSize = size + (size % 8 != 0) * (8 - size % 8);

    Block* block;
    for (block = (Block*)heap; block != NULL; block = block->next) {
        if (block->used == 1)
            continue;

        size_t freeSize = block->size;
        if (freeSize < allocSize)
            continue;

        int isSeparate = freeSize >= allocSize + 8 + sizeof(Block);
        block->used = 1;

        if (isSeparate)
        {
            Block* newBlock = (Block*) ((char*)block + sizeof(Block) + allocSize);

            if (block->next != NULL)
                newBlock->size = (char*)(block->next) - (char*)newBlock - sizeof(Block);
            else
                newBlock->size = heap + HEAP_SIZE - (char*)newBlock - sizeof(Block);

            newBlock->used = 0;

            newBlock->next = block->next;
            block->next = newBlock;
            block->size = allocSize;
        }

        return (void*) (block + 1);
    }

    return NULL;
}

/* TODO: implement my_free */
void my_free(void* p)
{
    if (p == NULL)
        return;

    Block* block = (Block*) p - 1;
    block->used = 0;

    Block* b = (Block*)heap;
    while (b->next != NULL) {
        if (b->used == 1 || b->next->used == 1)
        {
            b = b->next;
            continue;
        }

        b->size = b->size + sizeof(Block) + b->next->size;
        b->next = b->next->next;
    }
}

/* TODO: implement print_heap */
void print_heap()
{
    Block* block;
    int id = 0;
    for (block = heap; block != NULL; block = block->next) {
        printf("Block %d: used=%d size=%d\n", id, block->used, block->size);
        id++;
    }
}

/* TODO: implement count_free_blocks */
int count_free_blocks()
{
    Block* block;
    int freeBlockCount = 0;
    for (block = heap; block != NULL; block = block->next) {
        if (block->used == 0)
        {
            freeBlockCount++;
        }
    }

    return freeBlockCount;
}

int main(void) {
    init_heap();

    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    
    if (strcmp(mode, "alloc") == 0)
    {
        my_malloc(64);
        my_malloc(128);
        my_malloc(32);

        printf("After allocations:\n");
        print_heap();
    }
    else if (strcmp(mode, "free") == 0)
    {
        void* p1 = my_malloc(64);
        void* p2 = my_malloc(128);
        void* p3 = my_malloc(32);

        my_free(p2);
        printf("After freeing p2:\n");
        print_heap();
    }
    else if (strcmp(mode, "merge") == 0)
    {
        void* p1 = my_malloc(64);
        void* p2 = my_malloc(64);
        void* p3 = my_malloc(64);

        my_free(p1);
        my_free(p2);
        my_free(p3);

        printf("Blocks merged\n");
        printf("free_blocks=%d\n", count_free_blocks());
    }
    else if (strcmp(mode, "edge") == 0)
    {
        void* p1 = my_malloc(0);
        if (p1 == NULL)
        {
            printf("zero=null_or_ok\n");
        }
        else
        {
            printf("zero=nonnull\n");
        }

        void* p2 = my_malloc(4096);
        if (p2 == NULL)
        {
            printf("big=null\n");
        }
        else
        {
            printf("big=nonnull\n");
        }

        my_free(NULL);

        printf("All edge cases pass\n");
    }
    else if (strcmp(mode, "multi") == 0)
    {
        Block* blocks[8];
        int ok = 1;
        for (int i = 0; i < 8; i++)
        {
            blocks[i] = my_malloc(32);
            if (blocks[i] == 0)
            {
                ok = 0;
            }
        }
        
        for (int i = 0; i < 8; i++)
        {
            my_free(blocks[i]);
        }

        printf("All allocations successful\n");

        Block* first_block = (Block*) heap;
        ok = ok == 1 && first_block->size == HEAP_SIZE - sizeof(Block) && first_block->used == 0 && first_block->next == NULL;

        printf("ok=%d", ok);
    }
    
    return 0;
}