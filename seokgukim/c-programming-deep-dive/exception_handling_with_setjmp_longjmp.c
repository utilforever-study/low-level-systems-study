#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Incomplete starter — implement the required behavior.
 * Stdin modes (if any): file, calc, success, nested
 * Match the sample test output exactly. Hidden tests use the other inputs
 * listed above.
 */

/* Core types from the exercise (keep these) */
typedef enum {
    NO_ERROR = 0,
    FILE_ERROR,
    CALCULATION_ERROR,
    NESTED_ERROR
} Exception;

static jmp_buf exception_env;

/* setjmp selects case 0 initially and the thrown code after a jump. */
#define TRY                                                                    \
    do {                                                                       \
        switch (setjmp(exception_env)) {                                       \
        case NO_ERROR:
#define CATCH(exception)                                                       \
    break;                                                                     \
    case (exception):
#define FINALLY                                                                \
    break;                                                                     \
    }
#define ETRY                                                                   \
    }                                                                          \
    while (0)
#define THROW(exception) longjmp(exception_env, (exception))

/* TODO: implement read_file */
void read_file(const char *name) {
    if (name[0] == '\0') {
        THROW(FILE_ERROR);
    }
}

/* TODO: implement calculate */
int calculate(int x) {
    if (x == 0) {
        THROW(CALCULATION_ERROR);
    }
    return 100 / x;
}

/* TODO: implement nested_demo */
void nested_demo(void) {
    jmp_buf inner_env;

    if (setjmp(inner_env) == 0) {
        longjmp(inner_env, NESTED_ERROR);
    }
    puts("Nested exception handled");
}

/* TODO: implement run_file_error */
void run_file_error(void) {
    TRY {
        read_file("");
        puts("File read successfully");
    }
    CATCH(FILE_ERROR) { puts("Caught file error"); }
    FINALLY { puts("Cleanup code"); }
    ETRY;
}

/* TODO: implement run_calc_error */
void run_calc_error(void) {
    TRY { printf("Success! Result: %d\n", calculate(0)); }
    CATCH(CALCULATION_ERROR) { puts("Caught calculation error"); }
    FINALLY { puts("Cleanup code"); }
    ETRY;
}

/* TODO: implement run_success */
void run_success(void) {
    TRY { printf("Success! Result: %d\n", calculate(10)); }
    FINALLY { puts("Cleanup code"); }
    ETRY;
}

int main(void) {
    char mode[64] = "";
    if (scanf("%63s", mode) != 1) {
        mode[0] = '\0';
    }
    /* TODO: handle mode "file" (sample) and other modes */
    if (strcmp(mode, "file") == 0) {
        run_file_error();
    } else if (strcmp(mode, "calc") == 0) {
        run_calc_error();
    } else if (strcmp(mode, "success") == 0) {
        run_success();
    } else if (strcmp(mode, "nested") == 0) {
        nested_demo();
    } else {
        run_file_error();
        run_calc_error();
        run_success();
        nested_demo();
    }
    (void)mode;
    return 0;
}
