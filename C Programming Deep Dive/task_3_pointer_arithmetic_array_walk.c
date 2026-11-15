#include <stdio.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): sum, max, reverse, count
 * Match the sample test output exactly. Hidden tests use the other inputs listed above.
 */

int arr[8] = {3, 1, 4, 1, 5, 9, 2, 6};

/* TODO: implement sum_array */
int sum_array(int *arr, int n)
{
    int sum = 0;

    for(int i = 0; i < n; i++)
    {
        sum += *(arr + i);
    }

    return sum;
}

/* TODO: implement find_max */
int find_max(int *arr, int n)
{
    int mx = *arr;

    for(int i = 1; i < n; i++)
    {
        if (mx < *(arr + i))
        {
            mx = *(arr + i);
        }
    }

    return mx;
}

/* TODO: implement reverse_array */
void reverse_array(int *arr, int n)
{
    int i = 0;
    int j = n - 1;

    while (i < j)
    {
        int* p = arr + i;
        int* q = arr + j;

        int t = *p;
        *p = *q;
        *q = t;

        i++; j--;
    }
}

/* TODO: implement count_occurrences */
int count_occurrences(int *arr, int n, int value)
{
    int cnt = 0;

    for(int i = 0; i < n; i++)
    {
        cnt += value == *(arr + i);
    }

    return cnt;
}

/* TODO: implement print_array */
void print_array(int *arr, int n)
{
    printf("[");
    for(int i = 0; i < n; i++)
    {
        printf("%d", *(arr+i));
        if(i != n-1)
        {
            printf(", ");
        }
    }
    printf("]\n");
}

int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    
    if (strcmp(mode, "sum") == 0 || strcmp(mode, "all") == 0)
    {
        printf("Original array: ");
        print_array(arr, 8);
        printf("Sum: %d\n", sum_array(arr, 8));
    }
    if (strcmp(mode, "max") == 0 || strcmp(mode, "all") == 0)
    {
        printf("Max: %d\n", find_max(arr, 8));
    }
    if (strcmp(mode, "reverse") == 0 || strcmp(mode, "all") == 0)
    {
        printf("Reversed array: ");
        reverse_array(arr, 8);
        print_array(arr, 8);
    }
    if (strcmp(mode, "count") == 0 || strcmp(mode, "all") == 0)
    {
        printf("Count of 1s: %d\n", count_occurrences(arr, 8, 1));
    }

    return 0;
}
