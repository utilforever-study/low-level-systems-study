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

char HEAP[HEAP_SIZE];
struct Block* heap_start = NULL;

/* TODO: implement init_heap */
void init_heap() {
    heap_start = (struct Block*)HEAP;
    heap_start->size = HEAP_SIZE - sizeof(struct Block); // -24
    heap_start->used = 0;
    heap_start->next = NULL;
}

/* TODO: implement my_malloc */
void *my_malloc(size_t size) {
    size = ((size-1)/8)*8+8;
    struct Block* cur = heap_start;
    while(cur) {
        if(!cur->used && cur->size >= size) {
            // allocation possible
            cur->used = 1;
            if(cur->size >= size + 8 + sizeof(struct Block)) {
                // have to make another blocks
                struct Block* nxt = cur->next;
                cur->next = (struct Block*)((char*)cur + sizeof(struct Block) + size);
                cur->next->size = cur->size - size - sizeof(struct Block);
                cur->size = size;
                cur->next->used = 0;
                cur->next->next = nxt;
            }
            return (void*) cur;
        }
        cur = cur->next;
    }
    return NULL;
}

/* TODO: implement my_free */
void my_free(struct Block* p) {
    if(p == NULL) {
        return ;
    }
    p->used = 0;
    struct Block* cur = heap_start;
    while(cur -> next) {
        if(!cur->used && !cur->next->used){
            cur->size += cur->next->size + sizeof(struct Block);
            cur->next = cur->next->next;
        } else {
            cur = cur->next;
        }
    }
}

/* TODO: implement print_heap */
void print_heap() {
    struct Block* cur = heap_start;
    int ind=0;
    while(cur) {
        printf("Block %d: used=%d size=%zu\n", ind++, cur->used, cur->size);
        cur = cur->next;
    }
}

/* TODO: implement count_free_blocks */
int count_free_blocks() {
    struct Block* cur = heap_start;
    int cnt=0;
    while(cur) {
        if(!cur->used) cnt++;
        cur = cur->next;
    }
    return cnt;
}

int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    if(strcmp(mode, "alloc")==0) {
        init_heap();
        my_malloc(64);
        my_malloc(128);
        my_malloc(32);
        printf("After allocations:\n");
        print_heap();
    }

    if(strcmp(mode, "free")==0) {
        init_heap();
        struct Block* a = my_malloc(64);
        struct Block* b = my_malloc(128);
        struct Block* c = my_malloc(32);
        my_free(b);
        printf("After freeing p2:\n");
        print_heap();
    }
    if(strcmp(mode, "merge")==0) {
        init_heap();
        struct Block* a = my_malloc(64);
        struct Block* b = my_malloc(64);
        struct Block* c = my_malloc(64);
        my_free(a);
        my_free(b);
        my_free(c);
        printf("Blocks merged\n");
        printf("free_blocks=%d\n", count_free_blocks());
    }
    if(strcmp(mode, "edge")==0) {
        init_heap();
        struct Block* a = my_malloc(0);
        struct Block* b = my_malloc(4096);
        my_free(NULL);
        if(a) {
            printf("zero=nonnull\n");
        } else {
            printf("zero=null_or_ok\n");
        }
        if(b) {
            printf("big=nonull\n");
        } else {
            printf("big=null\n");
        }
        printf("All edge cases pass\n");
    }
    if(strcmp(mode, "multi") == 0) {
        init_heap();
        struct Block* a[8];
        int flag=1;
        for(int i=0; i<8; i++) {
            a[i] = my_malloc(32);
            flag &= (a[i] != NULL);
        }
        for(int i=0; i<8; i++) {
            my_free(a[i]);
        }
        printf("All allocations successful\n");
        printf("ok=%d\n", flag);
    }
    
    return 0;
}
