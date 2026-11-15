#include <stdio.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): sum, max, reverse, count
 * Match the sample test output exactly. Hidden tests use the other inputs listed above.
 */

/* TODO: implement sum_array */
int sum_array(int *arr, int n) {
    int* end = arr + n, ret=0;
    for(;arr<end;arr++) ret+=*arr;
    return ret;
}
/* TODO: implement find_max */
int find_max(int *arr, int n) {
    int* end = arr + n, ret=1<<31;
    for(;arr<end;arr++) ret=ret>*arr?ret:*arr;
    return ret;
}
/* TODO: implement reverse_array */
void reverse_array(int *arr, int n) {
    int* end = arr + n;
    for(int* it = arr;it-arr<end-it;it++) {
        int t = *it;
        *it = *(end-1-(it-arr));
        *(end-1-(it-arr)) = t;
    }
}
/* TODO: implement count_occurrences */
int count_occurrences(int *arr, int n, int v) {
    int* end = arr + n, ret=0;
    for(;arr<end;arr++) ret=*arr==v?ret+1:ret;
    return ret;
}
/* TODO: implement print_array */
void print_array(int *arr, int n) {
    printf("[");
    int* end = arr + n;
    for(int* it=arr;it<end;it++) {
        if(arr<it) printf(", ");
        printf("%d", *it);
    }
    printf("]\n");
}
int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    /* TODO: handle mode "sum" (sample) and other modes */
    int arr[8] = {3,1,4,1,5,9,2,6};
    if (strcmp(mode, "sum") == 0) {
        printf("Original array: ");
        print_array(arr, 8);
        printf("Sum: %d\n", sum_array(arr, 8));
    }
    else if (strcmp(mode, "max") == 0) {
        printf("Max: %d\n", find_max(arr, 8));
    }
    else if (strcmp(mode, "reverse") == 0) {
        reverse_array(arr, 8);
        printf("Reversed array: ");
        print_array(arr, 8);
    }
    else if (strcmp(mode, "count") == 0) {
        printf("Count of 1s: %d\n", count_occurrences(arr, 8, 1));
    }
    else {
        printf("Original array: ");
        print_array(arr, 8);
        printf("Sum: %d\n", sum_array(arr, 8));
        printf("Max: %d\n", find_max(arr, 8));
        reverse_array(arr, 8);
        printf("Reversed array: ");
        print_array(arr, 8);
        printf("Count of 1s: %d\n", count_occurrences(arr, 8, 1));
    }
    return 0;
}
