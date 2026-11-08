#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>


struct MyStruct {
    int a;
    char b;
};

typedef struct MyStruct MyStruct;
enum {
    BASIC = 1 << 0,
    POINTERS = 1 << 1,
    ARRAYS = 1 << 2,
    STRUCT = 1 << 3,
    ALL = BASIC | POINTERS | ARRAYS | STRUCT
};


int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }

    unsigned flags = 0;
    if (strcmp(mode, "basic") == 0) flags = BASIC;
    else if (strcmp(mode, "pointers") == 0) flags = POINTERS;
    else if (strcmp(mode, "arrays") == 0) flags = ARRAYS;
    else if (strcmp(mode, "struct") == 0) flags = STRUCT;
    else flags = ALL;

    if (flags & BASIC)
    {
        printf("%s: %zu bytes\n", "char", sizeof(char));
        printf("%s: %zu bytes\n", "short", sizeof(short));
        printf("%s: %zu bytes\n", "int", sizeof(int));
        printf("%s: %zu bytes\n", "long", sizeof(long));
        printf("%s: %zu bytes\n", "long long", sizeof(long long));
        printf("%s: %zu bytes\n", "float", sizeof(float));
        printf("%s: %zu bytes\n", "double", sizeof(double));
    }
    if (flags & POINTERS)
    {
        printf("%s: %zu bytes\n", "int*", sizeof(int*));
        printf("%s: %zu bytes\n", "char*", sizeof(char*));
        printf("%s: %zu bytes\n", "double*", sizeof(double*));
    }
    if (flags & ARRAYS)
    {
        printf("%s: %zu bytes\n", "int[10]", sizeof(int[10]));
        printf("%s: %zu bytes\n", "char[10]", sizeof(char[10]));
        printf("%s: %zu bytes\n", "double[5]", sizeof(double[5]));
    }
    if (flags & STRUCT)
    {
        printf("struct: %zu bytes\n", sizeof(MyStruct));
        printf("sizeof(2 + 3.5): %zu\n", sizeof(2 + 3.5));
        printf("sizeof(\"hello\"): %zu\n", sizeof("hello"));
    }
    if (flags == ALL)
    {
        printf("All sizes printed correctly\n");
    }
    
    return 0;
}
