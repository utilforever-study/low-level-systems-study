#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 4096

typedef struct {
  char *memory;
  size_t capacity;
  size_t offset;
} StackAllocator;

int valid_alignment(size_t alignment) {
  // valid if alignment is a power of two and non-zero
  return alignment != 0 && (alignment & (alignment - 1)) == 0;
}

void *aligned_malloc(size_t size, size_t alignment) {
  if (!valid_alignment(alignment) || alignment - 1 > SIZE_MAX - sizeof(void *))
    return NULL;

  size_t overhead = sizeof(void *) + alignment - 1;
  if (size > SIZE_MAX - overhead)
    return NULL;

  void *original = malloc(size + overhead);
  if (original == NULL)
    return NULL;

  // Work from a real pointer rather than rounding a potentially overflowing
  // integer representation of an address.
  char *start = (char *)original + sizeof(void *);
  size_t remainder = (uintptr_t)start & (alignment - 1);
  size_t padding = remainder == 0 ? 0 : alignment - remainder;
  char *result = start + padding;
  memcpy(result - sizeof original, &original, sizeof original);
  return result;
}

void aligned_free(void *ptr) {
  if (ptr != NULL) {
    void *original;
    memcpy(&original, (char *)ptr - sizeof original, sizeof original);
    free(original);
  }
}

StackAllocator *stack_init(void) {
  StackAllocator *allocator = malloc(sizeof *allocator);
  if (allocator == NULL)
    return NULL;

  allocator->memory = aligned_malloc(STACK_SIZE, STACK_SIZE);
  if (allocator->memory == NULL) {
    free(allocator);
    return NULL;
  }
  allocator->capacity = STACK_SIZE;
  allocator->offset = 0;
  return allocator;
}

void *stack_push(StackAllocator *allocator, size_t size, size_t alignment) {
  // Reject zero-sized pushes: otherwise multiple pushes can return the same
  // pointer and a pointer at the buffer end cannot identify an allocation.
  if (allocator == NULL || size == 0 || !valid_alignment(alignment) ||
      alignment > STACK_SIZE)
    return NULL;

  // The brief's equivalent power-of-two round-up trick is:
  // 	start = (offset + alignment - 1) & ~(alignment - 1);
  // Adding alignment - 1 can overflow size_t in a general allocator, wrapping
  // the result to a smaller offset. Also, even without overflow, rounding up
  // and adding size can exceed capacity; returning or using that address would
  // access outside the buffer. Here we calculate padding first and check the
  // remaining capacity before either addition. Our 4096-byte bounds would also
  // make the brief's round-up safe, provided the offset invariant holds
  size_t remainder =
      allocator->offset &
      (alignment - 1); // remainder trick only works on divisor is power of 2
  size_t padding = remainder == 0 ? 0 : alignment - remainder;
  if (padding > allocator->capacity - allocator->offset)
    return NULL;
  size_t start = allocator->offset + padding;
  if (size > allocator->capacity - start)
    return NULL;

  allocator->offset = start + size;
  return allocator->memory + start;
}

// ptr must be a currently live pointer returned by stack_push(allocator, ...).
//   This allocator does not track allocation boundaries to validate it.
void stack_pop(StackAllocator *allocator, void *ptr) {
  allocator->offset = (size_t)((char *)ptr - allocator->memory);
}

void stack_free(StackAllocator *allocator) {
  if (allocator != NULL) {
    aligned_free(allocator->memory);
    free(allocator);
  }
}

static int run_aligned(void) {
  int *p1 = aligned_malloc(sizeof *p1, 8);
  double *p2 = aligned_malloc(sizeof *p2, 16);
  if (p1 == NULL || p2 == NULL) {
    aligned_free(p1);
    aligned_free(p2);
    return 1;
  }

  *p1 = 42;
  *p2 = 3.14;
  printf("p1 aligned to 8: %s\n", (uintptr_t)p1 % 8 == 0 ? "YES" : "NO");
  printf("p1 = %d\n", *p1);
  printf("p2 aligned to 16: %s\n", (uintptr_t)p2 % 16 == 0 ? "YES" : "NO");
  printf("p2 = %f\n", *p2);
  aligned_free(p1);
  aligned_free(p2);
  puts("Aligned free successful");
  return 0;
}

// Combined stack and pop entrypoint
static int run_stack(int need_pop) {
  StackAllocator *allocator = stack_init();
  if (allocator == NULL)
    return 1;

  int *arr1 = stack_push(allocator, 10 * sizeof *arr1, 8);
  double *arr2 = stack_push(allocator, 5 * sizeof *arr2, 16);
  if (arr1 == NULL || arr2 == NULL) {
    stack_free(allocator);
    return 1;
  }

  if (need_pop) {
    size_t previous = allocator->offset;
    stack_pop(allocator, arr2);
    puts("Stack pop successful");
    printf("After pop: %zu bytes (was %zu)\n", allocator->offset, previous);
  } else {
    printf("Stack allocated %zu bytes\n", allocator->offset);
    printf("arr1 aligned to 8: %s\n", (uintptr_t)arr1 % 8 == 0 ? "YES" : "NO");
    printf("arr2 aligned to 16: %s\n",
           (uintptr_t)arr2 % 16 == 0 ? "YES" : "NO");
  }
  stack_free(allocator);
  return 0;
}

int main(void) {
  char mode[64] = "";
  (void)scanf("%63s", mode);

  int err;
  if (strcmp(mode, "aligned") == 0 || strcmp(mode, "free") == 0) {
    err = run_aligned();
  } else if (strcmp(mode, "stack") == 0) {
    err = run_stack(0);
  } else if (strcmp(mode, "pop") == 0) {
    err = run_stack(1);
  } else {
    err = run_aligned() || run_stack(0) || run_stack(1);
  }

  if (err) {
    fputs("Allocation failed\n", stderr);
    return 1;
  }
  return 0;
}
