#include <stdio.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): basic, pointers, arrays, struct, all
 * Match the sample test output exactly. Hidden tests use the other inputs listed above.
 */

/* Core types from the exercise (keep these) */
struct MyStruct {
    int a;
    char b;
};


int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    /* TODO: handle mode "basic" (sample) and other modes */
    if(strcmp(mode, "basic")==0 || strcmp(mode, "all")==0) {
        printf("char: %zu bytes\n", sizeof(char));
        printf("short: %zu bytes\n", sizeof(short));
        printf("int: %zu bytes\n", sizeof(int));
        printf("long: %zu bytes\n", sizeof(long));
        printf("long long: %zu bytes\n", sizeof(long long));
        printf("float: %zu bytes\n", sizeof(float));
        printf("double: %zu bytes\n", sizeof(double));
    } if(strcmp(mode, "pointers")==0 || strcmp(mode, "all")==0) {
        printf("int*: %zu bytes\n", sizeof(int*));
        printf("char*: %zu bytes\n", sizeof(char*));
        printf("double*: %zu bytes\n", sizeof(double*));

    } if(strcmp(mode, "arrays")==0 || strcmp(mode, "all")==0) {
        printf("int[10]: %zu bytes\n", sizeof(int[10]));
        printf("char[10]: %zu bytes\n", sizeof(char[10]));
        printf("double[5]: %zu bytes\n", sizeof(double[5]));
    } if(strcmp(mode, "struct")==0 || strcmp(mode, "all")==0) {
        printf("struct: %zu bytes\n", sizeof(struct MyStruct));
        printf("sizeof(2 + 3.5): %zu\n", sizeof(2+3.5));
        printf("sizeof(\"hello\"): %zu\n", sizeof("hello"));
    }
    if(strcmp(mode, "all") == 0){
        printf("All sizes printed correctly\n");
    }
    return 0;
}
