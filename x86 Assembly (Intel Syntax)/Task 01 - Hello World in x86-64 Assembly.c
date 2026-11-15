#include <stdio.h>
#include <string.h>

/* stdin: one line, the message your .data section would hold.
 * The real buffer ends with '\n' (like db "Hello, World!", 10).
 *
 * TODO: simulate the syscall setup on x86-64 Linux:
 *   rax = write syscall number (see the writeup)
 *   rdi = fd for stdout
 *   rsi = simulated buffer address 0x402000
 *   rdx = byte count: message length + the embedded newline
 * Print the four mov lines, "bytes:" plus each buffer byte as two
 * uppercase hex digits, then "write returned rax=N".
 */
int main(void) {
    char msg[512];
    size_t n;
    if (!fgets(msg, sizeof msg, stdin)) msg[0] = '\0';
    n = strlen(msg);
    while (n > 0 && (msg[n - 1] == '\n' || msg[n - 1] == '\r')) n--;
    
    printf("mov rax, 1           ; sys_write\n");
    printf("mov rdi, 1           ; fd = stdout\n");
    printf("mov rsi, msg         ; msg @ 0x402000\n");
    printf("mov rdx, %d         ; byte count\n", n + 1);
    printf("bytes:");
    for (int i = 0; i < n; i++)
    {
        printf(" %02X", msg[i]);
    }
    printf(" %02X\n", '\n');
    printf("write returned rax=%d\n", n + 1);

    return 0;
}