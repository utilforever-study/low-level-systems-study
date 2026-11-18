#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): overflow, bounds, uaf, double, all
 * Match the sample test output exactly. Hidden tests use the other inputs listed above.
 */

/* TODO: implement demonstrate_signed_overflow */
/* TODO: implement demonstrate_out_of_bounds */
/* TODO: implement demonstrate_use_after_free */
/* TODO: implement demonstrate_double_free */

int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    /* TODO: handle mode "overflow" (sample) and other modes */
    int other = !((strcmp(mode, "overflow") == 0)
            || (strcmp(mode, "bounds") == 0)
            || (strcmp(mode, "uaf") == 0)
            || (strcmp(mode, "double") == 0));

    if(strcmp(mode, "overflow") == 0 || other) {
        printf("=== Signed Overflow ===\n");
        unsigned int x = INT_MAX;
        printf("INT_MAX = %u\n", INT_MAX);
        printf("x + 1 would be UB in C; use checked arithmetic instead\n");
        if (x < INT_MAX) {
            x = x + 1;
        } else {
            printf("overflow would occur — refused\n");
        }
    }
    if(strcmp(mode, "bounds") == 0 || other) {
        int arr[5] = {0,1,2,4,5};
        int ind = 5;
        printf("=== Out of Bounds Access ===\n");
        printf("array length = 5, requested index = %d\n", ind);
        if(ind<0 || 5<=ind) {
            printf("bounds check rejected access\n");
        }
    }
    if(strcmp(mode, "uaf") == 0 || other) {
        printf("=== Use After Free ===\n");
        int* x = malloc(sizeof(int));
        free(x);
        x = NULL;
        printf("pointer nulled after free\n");
        if(x == NULL) {
            printf("use-after-free prevented\n");
        } else {
            free(x);
        }
    }
    if(strcmp(mode,"double") == 0 || other) {
        printf("=== Double Free ===\n");
        int* x = malloc(sizeof(int));
        free(x);
        x = NULL;
        if(x == NULL) {
            printf("second free skipped (already NULL)\n");
        } else {
            free(x);
        }
    }
    if(other) {
        printf("Compile with sanitizers to detect UB:\n");
        printf("  gcc -fsanitize=undefined,address -g prog.c");
    }

    return 0;
}
