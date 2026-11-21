#include <stdio.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): basic, pointers, arrays, struct, all
 * Match the sample test output exactly. Hidden tests use the other inputs
 * listed above.
 */

/* Core types from the exercise (keep these) */
struct MyStruct {
  int a;
  char b;
};

void do_print(char *s, size_t len) { printf("%s: %zu bytes\n", s, len); }

void do_basic(void) {
  do_print("char", sizeof(char));
  do_print("short", sizeof(short));
  do_print("int", sizeof(int));
  do_print("long", sizeof(long));
  do_print("long long", sizeof(long long));
  do_print("float", sizeof(float));
  do_print("double", sizeof(double));
}

void do_pointers(void) {
  do_print("int*", sizeof(int *));
  do_print("char*", sizeof(char *));
  do_print("double*", sizeof(double *));
}

void do_arrays(void) {
  do_print("int[10]", sizeof(int[10]));
  do_print("char[10]", sizeof(char[10]));
  do_print("double[5]", sizeof(double[5]));
}

void do_struct(void) {
  do_print("struct", sizeof(struct MyStruct));
  printf("sizeof(2 + 3.5): %zu\n", sizeof(2 + 3.5));
  printf("sizeof(\"hello\"): %zu\n", sizeof("hello"));
}

int main(void) {
  char mode[64] = "";
  if (scanf("%63s", mode) != 1) {
    mode[0] = '\0';
  }
  /* TODO: handle mode "basic" (sample) and other modes */
  (void)mode;

  if (strcmp(mode, "basic") == 0) {
    do_basic();
  } else if (strcmp(mode, "pointers") == 0) {
    do_pointers();
  } else if (strcmp(mode, "arrays") == 0) {
    do_arrays();
  } else if (strcmp(mode, "struct") == 0) {
    do_struct();
  } else if (strcmp(mode, "all") == 0) {
    do_basic();
    do_pointers();
    do_arrays();
    do_struct();
    printf("All sizes printed correctly\n");
  } else {
    /* TODO: handle other modes */
    printf("Unknown mode: %s\n", mode);
  }
  return 0;
}
