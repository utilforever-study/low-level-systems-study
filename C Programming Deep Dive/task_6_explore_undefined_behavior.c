#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* TODO: implement demonstrate_signed_overflow */
void demonstrate_signed_overflow()
{
    printf("=== Signed Overflow ===\n");
    printf("INT_MAX = %d\n", INT_MAX);
    printf("x + 1 would be UB in C; use checked arithmetic instead\n");

    int x = INT_MAX;
    if (x != INT_MAX)
    {
        printf("%d\n", x + 1);
    }
    else
    {
        printf("overflow would occur — refused\n");
    }
}

/* TODO: implement demonstrate_out_of_bounds */
void demonstrate_out_of_bounds()
{
    printf("=== Out of Bounds Access ===\n");

    int arr[5] = {1, 2, 3, 4, 5};
    int len = 5;
    int requested_index = 5;

    printf("array length = %d, requested index = %d\n", len, requested_index);

    if (requested_index >= 0 && requested_index < len) 
    {
        printf("%d\n", arr[requested_index]);
    }
    else
    {
        printf("bounds check rejected access\n");
    }
}

/* TODO: implement demonstrate_use_after_free */
void demonstrate_use_after_free()
{
    int *p = (int *)malloc(sizeof(int));
    *p = 1;

    if (p)
    {
        free(p);
        p = NULL;
    }
    
    printf("=== Use After Free ===\n");
    printf("pointer nulled after free\n");

    if (p)
    {
        int value = *p;
        printf("%d\n", value);
    }
    else
    {
        printf("use-after-free prevented\n");
    }
}

/* TODO: implement demonstrate_double_free */
void demonstrate_double_free()
{
    int *p = (int *)malloc(sizeof(int));
    *p = 1;

    if (p)
    {
        free(p);
        p = NULL;
    }

    printf("=== Double Free ===\n");

    if (p)
    {
        free(p);
        p = NULL;
    }
    else
    {
        printf("second free skipped (already NULL)\n");
    }
}

int main(void)
{
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }

    int run_all = strcmp(mode, "overflow") != 0 && strcmp(mode, "bounds") != 0 && strcmp(mode, "uaf") != 0 && strcmp(mode, "double") != 0;

    if (run_all || strcmp(mode, "overflow") == 0)
    {
        demonstrate_signed_overflow();
    }
    if (run_all || strcmp(mode, "bounds") == 0)
    {
        demonstrate_out_of_bounds();
    }
    if (run_all || strcmp(mode, "uaf") == 0)
    {
        demonstrate_use_after_free();
    }
    if (run_all || strcmp(mode, "double") == 0)
    {
        demonstrate_double_free();
    }
    if (run_all)
    {
        printf("Compile with sanitizers to detect UB:\n");
        printf("  gcc -fsanitize=undefined,address -g prog.c\n");
    }

    return 0;
}