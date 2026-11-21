#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): int, str, hex, pct
 * Match the sample test output exactly. Hidden tests use the other inputs
 * listed above.
 */

/* TODO: implement print_int */
int print_int(int value) {
  int count = 0;
  unsigned int magnitude = (unsigned int)value;
  if (value < 0) { // when negative, just print - and get magnitude
    putchar('-');
    magnitude = 0u - magnitude;
    count++;
  }

  int idx = 0;
  int arr[16];

  do {
    int digit = magnitude % 10;
    magnitude = magnitude / 10;
    arr[idx++] = digit;
  } while (magnitude > 0);

  while (idx > 0) {
    putchar(arr[--idx] + '0');
    count++;
  }

  return count;
}

/* TODO: implement print_hex */
int print_hex(unsigned int value) {
  int count = 0;
  int idx = 0;
  int arr[16];

  do {
    int digit = value % 16;
    value = value / 16;
    arr[idx++] = digit;
  } while (value > 0);

  while (idx > 0) {
    int digit = arr[--idx];
    if (digit < 10) {
      putchar(digit + '0');
    } else {
      putchar(digit - 10 + 'a');
    }
    count++;
  }

  return count;
}

/* TODO: implement my_printf */
int my_printf(const char *fmt, ...) {
  int count = 0;
  int idx = 0;
  va_list args;
  va_start(args, fmt);
  while (fmt[idx] != '\0') {
    if (fmt[idx] == '%') {
      idx++;
      char c = fmt[idx];
      switch (c) {
      case 'd': {
        int value = va_arg(args, int);
        count += print_int(value);
        idx++;
        break;
      }
      case 's': {
        char *s = va_arg(args, char *);
        int s_idx = 0;
        while (s[s_idx] != '\0') {
          putchar(s[s_idx]);
          s_idx++;
          count++;
        }
        idx++;
        break;
      }
      case 'x': {
        unsigned int xvalue = va_arg(args, unsigned int);
        count += print_hex(xvalue);
        idx++;
        break;
      }
      case '%':
        putchar('%');
        idx++;
        count++;
        break;
      default:
        putchar('%');
        count++;
        if (c != '\0') {
          putchar(c);
          count++;
          idx++;
        }
        break;
      }
    } else {
      putchar(fmt[idx]);
      idx++;
      count++;
    }
  }

  va_end(args);
  return count;
}

int main(void) {
  char mode[64] = "";
  if (scanf("%63s", mode) != 1) {
    mode[0] = '\0';
  }
  /* TODO: handle mode "int" (sample) and other modes */
  (void)mode;

  if (strcmp(mode, "int") == 0) {
    my_printf("Integer: %d\n", 42);
  } else if (strcmp(mode, "str") == 0) {
    my_printf("String: %s\n", "Hello");
  } else if (strcmp(mode, "hex") == 0) {
    my_printf("Hex: %x\n", 255u);
  } else if (strcmp(mode, "pct") == 0) {
    my_printf("Literal $$: %%\n");
  } else {
    my_printf("Integer: %d\n", 42);
    my_printf("String: %s\n", "Hello");
    my_printf("Hex: %x\n", 255u);
    my_printf("Literal %%: %%\n");
  }
  return 0;
}
