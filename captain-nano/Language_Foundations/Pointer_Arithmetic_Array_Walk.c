#include <stdio.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): sum, max, reverse, count
 * Match the sample test output exactly. Hidden tests use the other inputs listed above.
 */

/* TODO: implement sum_array */
static int sum_array(int *arr, int n) {
    int sum = 0;

    for (int *p = arr; p < arr + n; p++)
        sum += *p;

    return sum;
}

/* TODO: implement find_max */
static int find_max(int *arr, int n) {
    int max = *arr;

    for (int *p = arr + 1; p < arr + n; p++)
        if (*p > max)
            max = *p;

    return max;
}

/* TODO: implement reverse_array */
static void reverse_array (int *arr, int n) {
    if (n <= 1)
        return;
        
    for (int *pL = arr, *pR = arr + n - 1, tmp; pL < pR; pL++, pR--)
    {
        tmp = *pL;
        *pL = *pR;
        *pR = tmp;
    }
}

/* TODO: implement count_occurrences */
static int count_occurrences(int *arr, int n, int value) {
    int count = 0;

    for (int *p = arr; p < arr + n; p++)
        if (*p == value)
            count++;

    return count;
}

/* TODO: implement print_array */
static void print_array(const int *arr, int n) {
    printf("[");

    for (const int *p = arr; p < arr + n; p++) {
        if (p != arr)
            printf(", ");

        printf("%d", *p);
    }

    printf("]");
}


int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    /* TODO: handle mode "sum" (sample) and other modes */
    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6};
    int n = (int)(sizeof(arr) / sizeof(*arr));

    if (strcmp(mode, "sum") == 0) {
        printf("Original array: ");
        print_array(arr, n);
        printf("\nSum: %d\n", sum_array(arr, n));
    }
    else if (strcmp(mode, "max") == 0) {
        printf("Max: %d\n", find_max(arr, n));
    }
    else if (strcmp(mode, "reverse") == 0) {
        reverse_array(arr, n);
        printf("Reversed array: ");
        print_array(arr, n);
        printf("\n");
    }
    else if (strcmp(mode, "count") == 0) {
        printf("Count of 1s: %d\n",
               count_occurrences(arr, n, 1));
    }
    else {
        printf("Original array: ");
        print_array(arr, n);
        printf("\nSum: %d\n", sum_array(arr, n));
        printf("Max: %d\n", find_max(arr, n));

        reverse_array(arr, n);
        printf("Reversed array: ");
        print_array(arr, n);

        printf("\nCount of 1s: %d\n",
               count_occurrences(arr, n, 1));
    }

    return 0;
}
