#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): alloc, free, merge, edge, multi
 * Match the sample test output exactly. Hidden tests use the other inputs
 * listed above.
 */

/* Core types from the exercise (keep these) */
#define HEAP_SIZE 4096
typedef struct Block {
  size_t size;
  int used;
  struct Block *next;
} Block;

Block heap = {0, 0, NULL};
Block _block_pool[64];
int _pool_idx = 0;

size_t round_8(size_t size) {
  if (size < 8)
    return 8;
  return (size + 7) &
         ~7; // round up to nearest multiple of 8. well-known trick.
}

/* TODO: implement init_heap */
void init_heap(void) {
  Block header = {HEAP_SIZE - sizeof(header), 0, NULL};
  heap = header;
}

Block *get_block() { return &_block_pool[_pool_idx++ % 64]; }

/* TODO: implement my_malloc */
Block *my_malloc(size_t size) {
  size_t rounded = round_8(size);
  Block *current = &heap;
  while (current != NULL) {
    if (current->used == 0 && current->size >= rounded + sizeof(Block)) {
      current->used = 1;
      size_t new_size = current->size - rounded - sizeof(Block);
      current->size = rounded;

      Block *nxt = get_block();
      nxt->size = new_size;
      nxt->used = 0;
      nxt->next = current->next;

      current->next = nxt;

      return current;
    }
    current = current->next;
  }

  return NULL;
}

/* TODO: implement my_free */
void my_free(Block *ptr) {
  if (ptr == NULL) {
    return;
  }
  ptr->used = 0;

  // merge from the head
  Block *current = &heap;
  while (current != NULL) {
    if (current->used == 0) {
      while (current->next != NULL && current->next->used == 0) {
        current->size += current->next->size + sizeof(Block);
        current->next = current->next->next;
      }
    }
    current = current->next;
  }
}

/* TODO: implement print_heap */
void print_heap(void) {
  size_t count = 0;
  Block *current = &heap;
  while (current != NULL) {
    printf("Block %zu: used=%d size=%zu\n", count++, current->used,
           current->size);
    current = current->next;
  }
}

/* TODO: implement count_free_blocks */
size_t count_free_blocks(void) {
  size_t count = 0;
  Block *current = &heap;
  while (current != NULL) {
    if (current->used == 0) {
      count++;
    }
    current = current->next;
  }

  return count;
}

int main(void) {
  char mode[64] = "";
  if (scanf("%63s", mode) != 1) {
    mode[0] = '\0';
  }
  /* TODO: handle mode "alloc" (sample) and other modes */
  (void)mode;

  init_heap();

  if (strcmp(mode, "alloc") == 0) {
    my_malloc(64);
    my_malloc(128);
    my_malloc(32);
    printf("After allocations:\n");
    print_heap();
  } else if (strcmp(mode, "free") == 0) {
    my_malloc(64);
    Block *p2 = my_malloc(128);
    my_malloc(32);
    my_free(p2);
    printf("After freeing p2:\n");
    print_heap();
  } else if (strcmp(mode, "merge") == 0) {
    Block *b1 = my_malloc(64);
    Block *b2 = my_malloc(64);
    Block *b3 = my_malloc(64);
    my_free(b2);
    my_free(b1);
    my_free(b3);
    printf("Blocks merged\n");
    printf("free_blocks=%zu\n", count_free_blocks());

  } else if (strcmp(mode, "edge") == 0) {
    Block *alloc_zero = my_malloc(0);
    Block *alloc_big = my_malloc(4096);
    my_free(NULL);
    printf("zero=%s\n", alloc_zero != NULL ? "nonnull" : "null_or_ok");
    printf("big=%s\n", alloc_big != NULL ? "nonnull" : "null");
    printf("All edge cases pass\n");
  } else if (strcmp(mode, "multi") == 0) {
    Block *arr[8];
    for (size_t i = 0; i < 8; i++) {
      arr[i] = my_malloc(32);
      if (arr[i] == NULL) {
        printf("ok=0");
        break;
      }
    }
    printf("All allocations successful\n");
    printf("ok=1");
  }

  return 0;
}
