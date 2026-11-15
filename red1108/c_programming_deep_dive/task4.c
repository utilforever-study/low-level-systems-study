#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): memcpy, memset, strlen, zero
 * Match the sample test output exactly. Hidden tests use the other inputs listed above.
 */

/* TODO: implement my_memcpy */
/* TODO: implement my_memset */
/* TODO: implement my_strlen */

char buffer[1000];

void my_memcpy(void* dest, const void *src, size_t n) {
    char* it = (char*)src;
    for(int i=0; i<n; i++) {
        *((char*)(dest+i)) = *(it++);
    }
}

void my_memset(void* ptr, int value, size_t n) {
    char*it = (char*) ptr;
    for(;(size_t)(it-(char*)ptr) < n; it++) {
        *it = value;
    }
    *it = '\0'; // terminate
}

size_t my_strlen(const char *s) {
    char* it = s;
    for(size_t i = 0;;i++) {
        if((*it++) == '\0') return i;
    }
}

void print_buff() {
    for(char* it = buffer; *it!='\0';it++) {
        printf("%c", *it);
    }
}

int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    /* TODO: handle mode "memcpy" (sample) and other modes */
    if (strcmp(mode, "memcpy") == 0) {
        my_memcpy((void*)buffer, "Hello, World!", 14);
        printf("memcpy: ");
        print_buff();
    }
    if(strcmp(mode, "memset") == 0) {
        my_memset((void*)buffer, 'X', 9);
        printf("memset: ");
        print_buff();
    }
    if(strcmp(mode, "strlen") == 0) {
        my_memcpy((void*)buffer, "Hello, World!", 14);
        printf("strlen: %zu\n", strlen(buffer));
    }
    if(strcmp(mode, "zero") == 0) {
        my_memcpy((void*)buffer, "hi", 0);
        my_memset((void*)buffer, 'X', 0);
        if(strlen(buffer) == 0) {
            printf("Zero size: OK");
        }
    }
    return 0;
}
