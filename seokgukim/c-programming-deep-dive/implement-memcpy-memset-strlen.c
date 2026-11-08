#include <asm-generic/errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): memcpy, memset, strlen, zero
 * Match the sample test output exactly. Hidden tests use the other inputs
 * listed above.
 */

char buf[64];

/* TODO: implement my_memcpy */
void my_memcpy(void *dest, const void *src, size_t n) {
  for (size_t i = 0; i < n; i++) {
    ((char *)dest)[i] = ((char *)src)[i];
  }
}

/* TODO: implement my_memset */
void my_memset(void *dest, int c, size_t n) {
  for (size_t i = 0; i < n; i++) {
    ((char *)dest)[i] = (char)c;
  }
}

/* TODO: implement my_strlen */
size_t my_strlen(const char *s) {
  size_t count = 0;
  while (s[count] != '\0') {
    count++;
  }
  return count;
}

int main(void) {
  char mode[64] = "";
  if (scanf("%63s", mode) != 1) {
    mode[0] = '\0';
  }
  /* TODO: handle mode "memcpy" (sample) and other modes */
  (void)mode;

  char src[64] = "Hello, World!";
  if (strcmp(mode, "memcpy") == 0) {
    my_memcpy(buf, src, 13);
    printf("memcpy: %s\n", buf);
  } else if (strcmp(mode, "memset") == 0) {
    my_memset(buf, 'X', 9);
    printf("memset: %s\n", buf);
  } else if (strcmp(mode, "strlen") == 0) {
    printf("strlen: %zu\n", my_strlen(src));

  } else if (strcmp(mode, "zero") == 0) {
    my_memcpy(buf, src, 0);
    my_memset(buf, 'x', 0);
    printf("Zero size: OK\n");

  } else {
    printf("Unknown mode %s\n", mode);
  }
  return 0;
}
