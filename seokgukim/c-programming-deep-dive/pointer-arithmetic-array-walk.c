#include <stdio.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): sum, max, reverse, count
 * Match the sample test output exactly. Hidden tests use the other inputs
 * listed above.
 */

/* TODO: implement sum_array */
int sum_array(int *arr, int n) {
  int sum = 0;
  for (int *p = arr; p < arr + n; p++) {
    sum += *p;
  }
  return sum;
}

/* TODO: implement find_max */
int find_max(int *arr, int n) {
  int max = 1 << 31;
  for (int *p = arr; p < arr + n; p++) {
    if (*p > max) {
      max = *p;
    }
  }
  return max;
}

/* TODO: implement reverse_array */
void reverse_array(int *arr, int n) {
  int *p = arr;
  int *q = arr + n - 1;
  while (p < q) {
    int temp = *p;
    *p = *q;
    *q = temp;
    p++;
    q--;
  }
}

/* TODO: implement count_occurrences */
int count_occurrences(int *arr, int n, int value) {
  int count = 0;
  for (int *p = arr; p < arr + n; p++) {
    if (*p == value) {
      count++;
    }
  }
  return count;
}

/* TODO: implement print_array */
void print_array(int *arr, int n) {
  char buf[1024] = "";
  int idx = 1;
  for (int *p = arr; p < arr + n; p++) {
    sprintf(buf, "%s%s%d", buf, p == arr ? "" : ", ", *p);
  }
  printf("[%s]\n", buf);
}

int main(void) {
  char mode[64] = "";
  if (scanf("%63s", mode) != 1) {
    mode[0] = '\0';
  }
  /* TODO: handle mode "sum" (sample) and other modes */
  (void)mode;
  int arr[8] = {3, 1, 4, 1, 5, 9, 2, 6};
  if (strcmp(mode, "sum") == 0) {
    printf("Original array: ");
    print_array(arr, 8);
    printf("Sum: %d\n", sum_array(arr, 8));
  } else if (strcmp(mode, "max") == 0) {
    printf("Max: %d\n", find_max(arr, 8));
  } else if (strcmp(mode, "reverse") == 0) {
    reverse_array(arr, 8);
    printf("Reversed array: ");
    print_array(arr, 8);
  } else if (strcmp(mode, "count") == 0) {
    printf("Count of 1s: %d\n", count_occurrences(arr, 8, 1));
  } else {
    printf("Invalid mode: %s\n", mode);
  }

  return 0;
}
