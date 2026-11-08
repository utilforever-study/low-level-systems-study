#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): aligned, stack, pop, free
 * The brief lists every input mode and the exact output.
 */

/* Core types from the exercise (keep these) */
#define STACK_SIZE 4096
typedef struct {
    char* memory;
    size_t capacity;
    size_t offset;
} StackAllocator;

StackAllocator* stack_allocator;

/* TODO: implement stack_init */
void stack_init()
{
    stack_allocator = (StackAllocator*) malloc(sizeof(StackAllocator));
    stack_allocator->memory = (char*) malloc(sizeof(STACK_SIZE));
    stack_allocator->capacity = STACK_SIZE;
    stack_allocator->offset = 0;
}

/* TODO: implement stack_push */
void* stack_push(size_t size, unsigned int alignment)
{
    int aligned = (stack_allocator->offset + alignment - 1) & ~(alignment - 1);
    stack_allocator->offset = aligned + size;
    return (void*) ((char*) stack_allocator->memory + aligned);
}

/* TODO: implement stack_pop */
void stack_pop(void* ptr)
{
    stack_allocator->offset = (size_t)((uintptr_t)ptr - (uintptr_t)stack_allocator->memory);
}

/* TODO: implement stack_free */
void stack_free()
{
    free(stack_allocator->memory);
    free(stack_allocator);
}

/* TODO: implement aligned_malloc */
void* aligned_malloc(size_t size, size_t alignment)
{
    void* raw = malloc(size + alignment + sizeof(void*));
    void* aligned = (void *)(((uintptr_t)raw + sizeof(void*) + alignment - 1) & ~(alignment - 1));
    *(((void**)aligned) - 1) = raw;
    
    return aligned;
}

/* TODO: implement aligned_free */
void aligned_free(void* ptr)
{
    void* raw = *((void**)ptr - 1);
    free(raw);
}

/* TODO: implement run_aligned */
void run_aligned()
{
    int* p1 = (int*) aligned_malloc(sizeof(int), 8);
    *p1 = 42;
    
    if((uintptr_t)p1 % 8 == 0)
        printf("p1 aligned to %d: YES\n", 8);
    printf("p1 = %d\n", *p1);
    
    double* p2 = (double*) aligned_malloc(sizeof(double), 16);
    *p2 = 3.14;
    
    if((uintptr_t)p2 % 16 == 0)
        printf("p2 aligned to %d: YES\n", 16);
    printf("p2 = %f\n", *p2);
    
    aligned_free(p1);
    aligned_free(p2);
    printf("Aligned free successful\n");
}

/* TODO: implement run_stack */
void* run_stack()
{
    stack_init();
    
    int* arr1 = (int*) stack_push(sizeof(int) * 10, 8);
    double* arr2 = (double*) stack_push(sizeof(double) * 5, 16);
    
    printf("Stack allocated %d bytes\n", stack_allocator->offset);
    if((size_t)((uintptr_t)arr1 - (uintptr_t)stack_allocator->memory) % 8 == 0)
        printf("arr1 aligned to %d: YES\n", 8);
    if((size_t)((uintptr_t)arr2 - (uintptr_t)stack_allocator->memory) % 16 == 0)
        printf("arr2 aligned to %d: YES\n", 16);
}

/* TODO: implement run_pop */
void run_pop()
{
    stack_init();
    
    int* arr1 = (int*) stack_push(sizeof(int) * 10, 8);
    double* arr2 = (double*) stack_push(sizeof(double) * 5, 16);
    
    int total = stack_allocator->offset;
    stack_pop(arr2);
    
    printf("Stack pop successful\n");
    printf("After pop: %d bytes (was %d)\n", stack_allocator->offset, total);
}

int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    
    int run_all = strcmp(mode, "aligned") != 0 
                && strcmp(mode, "free") != 0 
                && strcmp(mode, "stack") != 0 
                && strcmp(mode, "pop") != 0;
    
    if (run_all || strcmp(mode, "aligned") == 0 || strcmp(mode, "free") == 0)
    {
        run_aligned();
    }
    if (run_all || strcmp(mode, "stack") == 0)
    {
        run_stack();
    }
    if (run_all || strcmp(mode, "pop") == 0)
    {
        run_pop();
    }
    
    return 0;
}