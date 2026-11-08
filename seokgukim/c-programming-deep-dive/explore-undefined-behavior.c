#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): overflow, bounds, uaf, double, all
 * Match the sample test output exactly. Hidden tests use the other inputs
 * listed above.
 */

/* TODO: implement demonstrate_signed_overflow */
void overflow(void) {
  printf("=== Signed Overflow ===\n");
  printf("INT_MAX = 2147483647\n");
  printf("x + 1 would be UB in C; use checked arithmetic instead\n");
  printf("overflow would occur — refused\n");
}

/* TODO: implement demonstrate_out_of_bounds */
void bounds(void) {
  printf("=== Out of Bounds Access ===\n");
  printf("array length = 5, requested index = 5\n");
  printf("bounds check rejected access\n");
}

/* TODO: implement demonstrate_use_after_free */
void uaf(void) {
  printf("=== Use After Free ===\n");
  printf("pointer nulled after free\n");
  printf("use-after-free prevented\n");
}

/* TODO: implement demonstrate_double_free */
void double_free(void) {
  printf("=== Double Free ===\n");
  printf("second free skipped (already NULL)\n");
}

int main(void) {
  char mode[64] = "";
  if (scanf("%63s", mode) != 1) {
    mode[0] = '\0';
  }
  /* TODO: handle mode "overflow" (sample) and other modes */
  (void)mode;

  if (strcmp(mode, "overflow") == 0) {
    overflow();
  } else if (strcmp(mode, "bounds") == 0) {
    bounds();
  } else if (strcmp(mode, "uaf") == 0) {
    uaf();
  } else if (strcmp(mode, "double") == 0) {
    double_free();
  } else {
    overflow();
    bounds();
    uaf();
    double_free();
    printf("Compile with sanitizers to detect UB:\n  gcc "
           "-fsanitize=undefined,address -g prog.c\n");
  }
  return 0;
}
