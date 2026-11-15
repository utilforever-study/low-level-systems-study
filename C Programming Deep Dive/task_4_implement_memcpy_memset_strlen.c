#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): memcpy, memset, strlen, zero
 * Match the sample test output exactly. Hidden tests use the other inputs listed above.
 */

/* TODO: implement my_memcpy */
void *my_memcpy(void *dest, const void *src, size_t n) {
    for (unsigned int i = 0; i < n; i++)
    {
        *((unsigned char*)dest + i) = *((unsigned char*)src + i);
    }

    return dest;
}

/* TODO: implement my_memset */\
void *my_memset(void *ptr, int value, size_t n) {
    for (unsigned int i = 0; i < n; i++)
    {
        *((unsigned char*)ptr + i) = value;
    }
    return ptr;
}

/* TODO: implement my_strlen */
size_t my_strlen(const char *s) {
    unsigned int len = 0;
    for (unsigned char* c = s; *c != '\0'; c++)
    {
        len++;
    }

    return len;
}

int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    

    int run_all = mode[0] == '\0' || strcmp(mode, "memcpy") != 0 && strcmp(mode, "memset") != 0 && strcmp(mode, "strlen") != 0 && strcmp(mode, "zero") != 0;

    if (run_all || strcmp(mode, "memcpy") == 0)
    {
        char buf[32];
        my_memcpy(buf, "Hello, World!", sizeof("Hello, World!"));
        printf("memcpy: %s\n", buf);
    }

    if (run_all || strcmp(mode, "memset") == 0)
    {
        char buf[32];
        my_memset(buf, 'X', 9);
        buf[9] = '\0';
        printf("memset: %s\n", buf);
    }

    if (run_all || strcmp(mode, "strlen") == 0)
    {
        size_t len = my_strlen("Hello, World!");
        printf("strlen: %zu\n", len);
    }

    if (run_all || strcmp(mode, "zero") == 0)
    {
        char buf[32];
        my_memcpy(buf, "Hello, World!", 0);
        my_memset(buf, "Hello, World!", 0);
        printf("Zero size: OK\n");
    }

    return 0;
}
