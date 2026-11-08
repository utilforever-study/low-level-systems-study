#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): int, str, hex, pct
 * Match the sample test output exactly. Hidden tests use the other inputs listed above.
 */

void reverse_str(char* str)
{
    int i = 0;
    int j = strlen(str) - 1;

    while(i < j)
    {
        char temp = *(str + i);
        *(str + i) = *(str + j);
        *(str + j) = temp;

        i++; j--;
    }
}

/* TODO: implement print_int */
void print_int(int value)
{
    char* value_str = (char*) malloc(sizeof(char) * 9);

    if (value == 0)
    {
        value_str = "0";
    }

    int index = 0;
    while (value > 0)
    {
        int v = value % 10;
        value_str[index] = '0' + v;
        value_str[index + 1] = '\0';
        value = value / 10;
        index++;
    }

    reverse_str(value_str);
    for(char* p = value_str; *p != '\0'; p++)
    {
        putchar(*p);
    }
}

/* TODO: implement print_hex */
void print_hex(unsigned int value)
{
    char* value_str = (char*) malloc(sizeof(char) * 32);

    if (value == 0)
    {
        value_str = "0";
    }

    int index = 0;
    while (value > 0)
    {
        int v = value % 16;
        
        char ch;
        if (v < 10)
        {
            ch = '0' + v;
        }
        else
        {
            ch = 'a' + v - 10;
        }
        
        value_str[index] = ch;
        value_str[index + 1] = '\0';
        
        value = value / 16;
        index++;
    }

    reverse_str(value_str);
    for(char* p = value_str; *p != '\0'; p++)
    {
        putchar(*p);
    }
}

/* TODO: implement my_printf */
void my_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    int is_specifier = 0;
    for(char* p = format; *p != '\0'; p++)
    {
        char ch = *p;
        
        if (!is_specifier && ch == '%')
        {
            is_specifier = 1;
            continue;
        }

        if (is_specifier)
        {
            if (ch == 'd')
            {
                int value = va_arg(args, int);
                print_int(value);
            }
            else if(ch == 's')
            {
                const char* str = va_arg(args, const char*);
                if (str == '\0')
                {
                    str = "(null)";
                }
                my_printf(str);
            }
            else if(ch == 'x')
            {
                unsigned int value = va_arg(args, unsigned int);
                print_hex(value);
            }
            else
            {
                putchar(ch);
            }

            is_specifier = 0;
        }
        else
        {
            putchar(ch);
        }
    }

    va_end(args);
}

int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    
    int run_all = strcmp(mode, "int") !=  0 && strcmp(mode, "str") !=  0 && strcmp(mode, "hex") !=  0 && strcmp(mode, "pct") !=  0;
    if (run_all || strcmp(mode, "int") ==  0)
    {
        my_printf("Integer: %d\n", 42);
    }
    if (run_all || strcmp(mode, "str") ==  0)
    {
        my_printf("String: %s\n", "Hello");
    }
    if (run_all || strcmp(mode, "hex") ==  0)
    {
        my_printf("Hex: %x\n", 255);
    }
    if (run_all || strcmp(mode, "pct") ==  0)
    {
        my_printf("Literal %%: %%\n");
    }
    return 0;
}