#include "../c-common/array.h"
#include "../c-common/c_scan.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Binary Inspection with objdump and nm
 * Classify the brief's restricted file-scope declarations for x86-64 Linux.
 * Sizes and section choices follow the exercise, without invoking a compiler.
 */

typedef enum { PR_Ok, PR_Invalid, PR_NoMemory } ParseResult;
typedef enum {
    TK_End,
    TK_Identifier,
    TK_Number,
    TK_String,
    TK_Character,
    TK_Punctuation,
    TK_Invalid
} TokenKind;

// Token text is borrowed from the current physical line.
typedef struct {
    TokenKind kind;
    const char *text;
    size_t length;
    size_t end;
} Token;

typedef struct {
    const char *text;
    size_t offset;
    ParseResult result;
} Cursor;

typedef struct {
    char *text;
    size_t length;
    size_t capacity;
} Line;

typedef enum {
    BT_Char,
    BT_Short,
    BT_Int,
    BT_Long,
    BT_Float,
    BT_Double,
    BT_Void
} BaseType;
typedef enum { SC_None, SC_Static, SC_Extern } StorageClass;
typedef enum {
    S_Text,
    S_Rodata,
    S_Strings,
    S_Data,
    S_Relro,
    S_Bss,
    S_Undefined,
    S_Count
} Section;

typedef struct {
    BaseType base;
    bool base_const;
    size_t pointer_depth;
    bool pointer_const; // Qualifier on the outermost (last written) '*'.
} Type;

// Decoded bytes exclude the implicit terminator and may contain embedded NULs.
typedef struct {
    unsigned char *bytes;
    size_t length;
} Literal;

C_COMMON_DEFINE_VECTOR(LiteralVec, Literal, literal_vec_push)

typedef struct {
    bool present;
    bool any_nonzero;
    bool contains_address;
    bool array_string;
    size_t element_count;
    size_t string_length;
} InitializerInfo;

// A declaration owns its name and pending literals until it is committed.
typedef struct {
    char *name;
    Type type;
    StorageClass storage;
    bool is_array;
    size_t array_length; // Zero means an inferred bound while parsing.
    bool is_function;
    bool is_definition;
    size_t size;
    InitializerInfo init;
    LiteralVec literals;
} Declaration;

typedef struct {
    char *name;
    Section section;
    bool is_static;
    size_t size;
} Symbol;

C_COMMON_DEFINE_VECTOR(SymbolVec, Symbol, symbol_vec_push)

typedef struct {
    SymbolVec symbols;
    LiteralVec literals;
} Inspection;

static void literals_free(LiteralVec *literals) {
    for (size_t i = 0; i < literals->count; ++i)
        free(literals->items[i].bytes);
    free(literals->items);
    *literals = (LiteralVec){0};
}

static void declaration_free(Declaration *decl) {
    free(decl->name);
    literals_free(&decl->literals);
    *decl = (Declaration){0};
}

static void inspection_free(Inspection *inspection) {
    for (size_t i = 0; i < inspection->symbols.count; ++i)
        free(inspection->symbols.items[i].name);
    free(inspection->symbols.items);
    literals_free(&inspection->literals);
}

// Physical lines, with CRLF and CR normalized; 1 = line, 0 = EOF, -1 = failure.
// Unlike the preprocessor reader, this does not join lines or strip comments.
static int read_line(FILE *input, Line *line) {
    line->length = 0;
    int c;
    while ((c = fgetc(input)) != EOF) {
        if (c == '\r' || c == '\n') {
            if (c == '\r') {
                int next = fgetc(input);
                if (next != '\n' && next != EOF)
                    ungetc(next, input);
            }
            break;
        }
        if (line->length > SIZE_MAX - 2)
            return -1;
        if (line->length + 1 >= line->capacity) {
            char *grown = c_array_grow(line->text, &line->capacity, 1);
            if (grown == NULL)
                return -1;
            line->text = grown;
        }
        line->text[line->length++] = (char)c;
    }
    if (ferror(input))
        return -1;
    if (c == EOF && line->length == 0)
        return 0;
    if (line->text == NULL) {
        line->text = c_array_grow(NULL, &line->capacity, 1);
        if (line->text == NULL)
            return -1;
    }
    line->text[line->length] = '\0';
    return 1;
}

static Token cursor_peek(const Cursor *cursor) {
    const char *text = cursor->text;
    size_t start = cursor->offset;
    while (isspace((unsigned char)text[start]))
        ++start;
    unsigned char c = (unsigned char)text[start];
    size_t end = start;
    TokenKind kind;
    if (c == '\0') {
        kind = TK_End;
    } else if (isalpha(c) || c == '_') {
        kind = TK_Identifier;
        end = c_scan_identifier(text, start);
    } else if (isdigit(c) ||
               (c == '.' && isdigit((unsigned char)text[start + 1]))) {
        kind = TK_Number;
        end = c_scan_number(text, start);
    } else if (c == '"' || c == '\'') {
        kind = c == '"' ? TK_String : TK_Character;
        end = c_scan_literal(text, start);
        if (end == SIZE_MAX) {
            kind = TK_Invalid;
            end = start + strlen(text + start);
        }
    } else {
        kind = TK_Punctuation;
        end = c_scan_punctuation(text, start);
    }
    return (Token){kind, text + start, end - start, end};
}

static bool token_is(Token token, const char *text) {
    return token.length == strlen(text) &&
           memcmp(token.text, text, token.length) == 0;
}

static bool cursor_accept(Cursor *cursor, const char *text) {
    Token token = cursor_peek(cursor);
    if (!token_is(token, text))
        return false;
    cursor->offset = token.end;
    return true;
}

static bool cursor_invalid(Cursor *cursor) {
    if (cursor->result == PR_Ok)
        cursor->result = PR_Invalid;
    return false;
}

static bool cursor_expect(Cursor *cursor, const char *text) {
    return cursor_accept(cursor, text) || cursor_invalid(cursor);
}

static bool token_is_name(Token token) {
    static const char *const keywords[] = {
        "auto",       "break",     "case",           "char",
        "const",      "continue",  "default",        "do",
        "double",     "else",      "enum",           "extern",
        "float",      "for",       "goto",           "if",
        "inline",     "int",       "long",           "register",
        "restrict",   "return",    "short",          "signed",
        "sizeof",     "static",    "struct",         "switch",
        "typedef",    "union",     "unsigned",       "void",
        "volatile",   "while",     "_Alignas",       "_Alignof",
        "_Atomic",    "_Bool",     "_Complex",       "_Generic",
        "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local"};
    if (token.kind != TK_Identifier)
        return false;
    for (size_t i = 0; i < sizeof keywords / sizeof *keywords; ++i)
        if (token_is(token, keywords[i]))
            return false;
    return true;
}

static bool parse_type(Cursor *cursor, Declaration *decl) {
    static const char *const names[] = {"char",  "short",  "int", "long",
                                        "float", "double", "void"};
    if (cursor_accept(cursor, "static"))
        decl->storage = SC_Static;
    else if (cursor_accept(cursor, "extern"))
        decl->storage = SC_Extern;
    decl->type.base_const = cursor_accept(cursor, "const");
    bool sign =
        cursor_accept(cursor, "unsigned") || cursor_accept(cursor, "signed");
    Token token = cursor_peek(cursor);
    size_t i;
    for (i = 0; i < sizeof names / sizeof *names; ++i) {
        if (token_is(token, names[i])) {
            decl->type.base = (BaseType)i;
            cursor->offset = token.end;
            break;
        }
    }
    if (i == sizeof names / sizeof *names) {
        if (!sign)
            return cursor_invalid(cursor);
        decl->type.base = BT_Int;
    }
    if (sign && decl->type.base > BT_Long)
        return cursor_invalid(cursor);
    if (cursor_accept(cursor, "const"))
        decl->type.base_const = true;
    while (cursor_accept(cursor, "*")) {
        ++decl->type.pointer_depth;
        decl->type.pointer_const = cursor_accept(cursor, "const");
    }
    return true;
}

// Parameters and function bodies are opaque, but delimiters and quoted text
// must close on this physical line. Comments inside them do not affect nesting.
static bool skip_group(Cursor *cursor, char open, char close) {
    char opening[] = {open, '\0'};
    if (!cursor_expect(cursor, opening))
        return false;
    size_t depth = 1;
    const char *text = cursor->text;
    size_t i = cursor->offset;
    while (text[i] != '\0') {
        if (text[i] == '"' || text[i] == '\'') {
            i = c_scan_literal(text, i);
            if (i == SIZE_MAX)
                return cursor_invalid(cursor);
            continue;
        }
        if (text[i] == '/' && text[i + 1] == '/')
            return cursor_invalid(cursor);
        if (text[i] == '/' && text[i + 1] == '*') {
            const char *end = strstr(text + i + 2, "*/");
            if (end == NULL)
                return cursor_invalid(cursor);
            i = (size_t)(end - text) + 2;
            continue;
        }
        if (text[i] == open)
            ++depth;
        else if (text[i] == close && --depth == 0) {
            cursor->offset = i + 1;
            return true;
        }
        ++i;
    }
    return cursor_invalid(cursor);
}

// Numeric syntax is validated without converting to a machine integer or
// float: classification needs only whether the significand is zero.
static int digit_value(unsigned char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static size_t number_digits(Token token, size_t *i, int base, bool *nonzero) {
    size_t begin = *i;
    while (*i < token.length) {
        int digit = digit_value((unsigned char)token.text[*i]);
        if (digit < 0 || digit >= base)
            break;
        *nonzero |= digit != 0;
        ++*i;
    }
    return *i - begin;
}

static bool number_nonzero(Token token, bool *nonzero) {
    size_t i = 0;
    bool hex = token.length >= 2 && token.text[0] == '0' &&
               (token.text[1] == 'x' || token.text[1] == 'X');
    if (hex)
        i = 2;
    size_t digits_start = i;
    size_t digits = number_digits(token, &i, hex ? 16 : 10, nonzero);
    bool real = i < token.length && token.text[i] == '.';
    if (real) {
        ++i;
        digits += number_digits(token, &i, hex ? 16 : 10, nonzero);
    }
    if (digits == 0)
        return false;
    bool exponent = i < token.length &&
                    (hex ? (token.text[i] == 'p' || token.text[i] == 'P')
                         : (token.text[i] == 'e' || token.text[i] == 'E'));
    if (exponent) {
        real = true;
        ++i;
        if (i < token.length && (token.text[i] == '+' || token.text[i] == '-'))
            ++i;
        bool ignored = false;
        if (number_digits(token, &i, 10, &ignored) == 0)
            return false;
    }
    if (hex && real && !exponent)
        return false;
    if (real) {
        if (i < token.length && strchr("fFlL", token.text[i]) != NULL)
            ++i;
        return i == token.length;
    }
    // A leading-zero integer is octal, even though decimal digits were scanned.
    if (!hex && token.text[0] == '0')
        for (size_t j = digits_start; j < i; ++j)
            if (token.text[j] > '7')
                return false;
    bool seen_unsigned = false;
    bool seen_long = false;
    while (i < token.length) {
        char c = token.text[i++];
        if ((c == 'u' || c == 'U') && !seen_unsigned) {
            seen_unsigned = true;
        } else if ((c == 'l' || c == 'L') && !seen_long) {
            seen_long = true;
            if (i < token.length && token.text[i] == c)
                ++i;
        } else {
            return false;
        }
    }
    return true;
}

static bool decode_literal(Cursor *cursor, Token token, Literal *literal) {
    literal->bytes = malloc(token.length);
    if (literal->bytes == NULL) {
        cursor->result = PR_NoMemory;
        return false;
    }
    size_t end = token.length - 1;
    for (size_t i = 1; i < end; ++i) {
        unsigned int c = (unsigned char)token.text[i];
        if (c == '\\') {
            if (++i == end)
                return cursor_invalid(cursor);
            c = (unsigned char)token.text[i];
            if (c >= '0' && c <= '7') {
                c -= '0';
                for (size_t n = 1;
                     n < 3 && i + 1 < end && token.text[i + 1] >= '0' &&
                     token.text[i + 1] <= '7';
                     ++n)
                    c = c * 8 + (unsigned int)(token.text[++i] - '0');
            } else if (c == 'x') {
                c = 0;
                size_t start = i;
                while (i + 1 < end &&
                       digit_value((unsigned char)token.text[i + 1]) >= 0)
                    c = (c * 16 + (unsigned int)digit_value(
                                      (unsigned char)token.text[++i])) &
                        255;
                if (i == start)
                    return cursor_invalid(cursor);
            } else {
                switch (c) {
                case 'a':
                    c = '\a';
                    break;
                case 'b':
                    c = '\b';
                    break;
                case 'f':
                    c = '\f';
                    break;
                case 'n':
                    c = '\n';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 't':
                    c = '\t';
                    break;
                case 'v':
                    c = '\v';
                    break;
                case '\\':
                case '\'':
                case '"':
                case '?':
                    break;
                default:
                    return cursor_invalid(cursor);
                }
            }
        }
        literal->bytes[literal->length++] = (unsigned char)c;
    }
    return true;
}

static bool parse_element(Cursor *cursor, Declaration *decl) {
    InitializerInfo *init = &decl->init;
    Token token = cursor_peek(cursor);
    if (token_is(token, "+") || token_is(token, "-")) {
        cursor->offset = token.end;
        token = cursor_peek(cursor);
        if (token.kind != TK_Number)
            return cursor_invalid(cursor);
    }
    if (token.kind == TK_Number) {
        bool nonzero = false;
        if (!number_nonzero(token, &nonzero))
            return cursor_invalid(cursor);
        init->any_nonzero |= nonzero;
        cursor->offset = token.end;
    } else if (token.kind == TK_Character || token.kind == TK_String) {
        Literal literal = {0};
        if (!decode_literal(cursor, token, &literal)) {
            free(literal.bytes);
            return false;
        }
        bool copied = token.kind == TK_String && decl->is_array &&
                      decl->type.base == BT_Char &&
                      decl->type.pointer_depth == 0;
        if ((token.kind == TK_Character && literal.length != 1) ||
            (token.kind == TK_String && !copied &&
             decl->type.pointer_depth == 0)) {
            free(literal.bytes);
            return cursor_invalid(cursor);
        }
        for (size_t i = 0; i < literal.length; ++i)
            init->any_nonzero |= literal.bytes[i] != 0;
        if (copied) {
            init->array_string = true;
            init->string_length = literal.length;
        }
        if (token.kind == TK_String && !copied) {
            init->contains_address = true;
            init->any_nonzero = true; // Even an empty string has an address.
            if (literal_vec_push(&decl->literals, literal) < 0) {
                free(literal.bytes);
                cursor->result = PR_NoMemory;
                return false;
            }
        } else {
            free(literal.bytes);
        }
        cursor->offset = token.end;
    } else if (cursor_accept(cursor, "&")) {
        token = cursor_peek(cursor);
        if (decl->type.pointer_depth == 0 || !token_is_name(token))
            return cursor_invalid(cursor);
        cursor->offset = token.end;
        init->contains_address = true;
        init->any_nonzero = true;
    } else {
        return cursor_invalid(cursor);
    }
    ++init->element_count;
    return true;
}

static bool parse_initializer(Cursor *cursor, Declaration *decl) {
    if (!cursor_accept(cursor, "="))
        return true;
    decl->init.present = true;
    bool list = cursor_accept(cursor, "{");
    if (!list && decl->is_array &&
        !(decl->type.base == BT_Char && decl->type.pointer_depth == 0 &&
          cursor_peek(cursor).kind == TK_String))
        return cursor_invalid(cursor);
    do {
        if (!parse_element(cursor, decl))
            return false;
        if (!list || !cursor_accept(cursor, ","))
            break;
        if (token_is(cursor_peek(cursor), "}"))
            break;
    } while (true);
    if (list && !cursor_expect(cursor, "}"))
        return false;
    if ((!decl->is_array || decl->init.array_string) &&
        decl->init.element_count != 1)
        return cursor_invalid(cursor);
    return true;
}

static bool parse_declaration(Cursor *cursor, Declaration *decl) {
    if (!parse_type(cursor, decl))
        return false;
    Token name = cursor_peek(cursor);
    if (!token_is_name(name))
        return cursor_invalid(cursor);
    decl->name = malloc(name.length + 1);
    if (decl->name == NULL) {
        cursor->result = PR_NoMemory;
        return false;
    }
    memcpy(decl->name, name.text, name.length);
    decl->name[name.length] = '\0';
    cursor->offset = name.end;

    if (token_is(cursor_peek(cursor), "(")) {
        decl->is_function = true;
        if (!skip_group(cursor, '(', ')'))
            return false;
        if (cursor_accept(cursor, ";")) {
            decl->is_definition = false;
        } else {
            if (!skip_group(cursor, '{', '}'))
                return false;
            decl->is_definition = true;
        }
        return cursor_peek(cursor).kind == TK_End || cursor_invalid(cursor);
    }
    if (decl->type.base == BT_Void && decl->type.pointer_depth == 0)
        return cursor_invalid(cursor);
    decl->is_array = cursor_accept(cursor, "[");
    if (decl->is_array) {
        Token bound = cursor_peek(cursor);
        if (!token_is(bound, "]")) {
            if (bound.kind != TK_Number)
                return cursor_invalid(cursor);
            for (size_t i = 0; i < bound.length; ++i) {
                if (!isdigit((unsigned char)bound.text[i]))
                    return cursor_invalid(cursor);
                decl->array_length =
                    decl->array_length * 10 + (size_t)(bound.text[i] - '0');
                if (decl->array_length > 100000)
                    return cursor_invalid(cursor);
            }
            if (decl->array_length == 0)
                return cursor_invalid(cursor);
            cursor->offset = bound.end;
        }
        if (!cursor_expect(cursor, "]"))
            return false;
    }
    if (!parse_initializer(cursor, decl) || !cursor_expect(cursor, ";") ||
        cursor_peek(cursor).kind != TK_End)
        return cursor_invalid(cursor);
    if (decl->is_array) {
        if (decl->array_length == 0) {
            if (!decl->init.present)
                return cursor_invalid(cursor);
            decl->array_length = decl->init.array_string
                                     ? decl->init.string_length + 1
                                     : decl->init.element_count;
            if (decl->array_length == 0 || decl->array_length > 100000)
                return cursor_invalid(cursor);
        }
        // C permits an explicit char bound to omit the terminating NUL.
        size_t required = decl->init.array_string ? decl->init.string_length
                                                  : decl->init.element_count;
        if (required > decl->array_length)
            return cursor_invalid(cursor);
    }
    static const size_t sizes[] = {1, 2, 4, 8, 4, 8, 0};
    size_t element_size = decl->type.pointer_depth ? 8 : sizes[decl->type.base];
    decl->size = element_size * (decl->is_array ? decl->array_length : 1);
    decl->is_definition = decl->storage != SC_Extern || decl->init.present;
    return true;
}

static Section classify(const Declaration *decl) {
    if (!decl->is_definition)
        return S_Undefined;
    if (decl->is_function)
        return S_Text;
    bool top_const = decl->type.pointer_depth ? decl->type.pointer_const
                                              : decl->type.base_const;
    if (top_const)
        return decl->init.contains_address ? S_Relro : S_Rodata;
    return decl->init.any_nonzero ? S_Data : S_Bss;
}

// Returns 0 on success, 1 for a redefinition, and -1 for allocation failure.
// Syntax validation is complete before any symbols or pooled literals change.
static int inspection_commit(Inspection *inspection, Declaration *decl) {
    size_t index;
    for (index = 0; index < inspection->symbols.count; ++index)
        if (strcmp(inspection->symbols.items[index].name, decl->name) == 0)
            break;
    if (index < inspection->symbols.count) {
        if (!decl->is_definition)
            return 0;
        if (inspection->symbols.items[index].section != S_Undefined)
            return 1;
    }
    for (size_t i = 0; i < decl->literals.count; ++i) {
        Literal *literal = &decl->literals.items[i];
        size_t j;
        for (j = 0; j < inspection->literals.count; ++j) {
            Literal *pooled = &inspection->literals.items[j];
            if (literal->length == pooled->length &&
                memcmp(literal->bytes, pooled->bytes, literal->length) == 0)
                break;
        }
        if (j == inspection->literals.count) {
            if (literal_vec_push(&inspection->literals, *literal) < 0)
                return -1;
            literal->bytes = NULL; // Ownership transferred to the global pool.
        }
    }
    Symbol symbol = {decl->name, classify(decl), decl->storage == SC_Static,
                     decl->size};
    if (index < inspection->symbols.count) {
        free(inspection->symbols.items[index].name);
        inspection->symbols.items[index] = symbol;
    } else if (symbol_vec_push(&inspection->symbols, symbol) < 0) {
        return -1;
    }
    decl->name = NULL;
    return 0;
}

static int compare_symbols(const void *left, const void *right) {
    return strcmp(((const Symbol *)left)->name, ((const Symbol *)right)->name);
}

static bool inspection_print(Inspection *inspection, FILE *output) {
    static const char *const sections[] = {
        ".text",        ".rodata", ".rodata.str1.1", ".data",
        ".data.rel.ro", ".bss",    "undefined"};
    static const char letters[] = {'T', 'R', 'R', 'D', 'D', 'B', 'U'};
    size_t counts[S_Count] = {0};
    size_t bytes[S_Count] = {0};
    if (inspection->symbols.count > 1)
        qsort(inspection->symbols.items, inspection->symbols.count,
              sizeof *inspection->symbols.items, compare_symbols);
    for (size_t i = 0; i < inspection->symbols.count; ++i) {
        const Symbol *symbol = &inspection->symbols.items[i];
        Section section = symbol->section;
        char letter = letters[section];
        if (symbol->is_static && section != S_Undefined)
            letter = (char)tolower((unsigned char)letter);
        ++counts[section];
        bytes[section] += symbol->size;
        if (fprintf(output, "%c %s", letter, symbol->name) < 0)
            return false;
        if (section != S_Undefined &&
            fprintf(output, " %s", sections[section]) < 0)
            return false;
        if (section != S_Undefined && section != S_Text &&
            fprintf(output, " %zu", symbol->size) < 0)
            return false;
        if (fputc('\n', output) == EOF)
            return false;
    }
    counts[S_Strings] = inspection->literals.count;
    for (size_t i = 0; i < inspection->literals.count; ++i)
        bytes[S_Strings] += inspection->literals.items[i].length + 1;
    if (fputs("--\n", output) == EOF)
        return false;
    for (Section section = S_Text; section < S_Count; ++section) {
        size_t count = counts[section];
        if (count == 0)
            continue;
        const char *noun = section == S_Text        ? "function"
                           : section == S_Strings   ? "literal"
                           : section == S_Undefined ? "symbol"
                                                    : "object";
        if (fprintf(output, "%s: %zu %s%s", sections[section], count, noun,
                    count == 1 ? "" : "s") < 0)
            return false;
        if (section != S_Text && section != S_Undefined &&
            fprintf(output, ", %zu bytes", bytes[section]) < 0)
            return false;
        if (fputc('\n', output) == EOF)
            return false;
    }
    return true;
}

int main(void) {
    Inspection inspection = {0};
    Line line = {0};
    size_t line_number = 0;
    int status;
    while ((status = read_line(stdin, &line)) > 0) {
        ++line_number;
        const char *text = line.text;
        while (isspace((unsigned char)*text))
            ++text;
        if (*text == '\0' || (text[0] == '/' && text[1] == '/'))
            continue;
        Cursor cursor = {.text = line.text};
        Declaration decl = {0};
        bool parsed = parse_declaration(&cursor, &decl);
        if (cursor.result == PR_NoMemory) {
            status = -1;
        } else if (!parsed) {
            if (printf("line %zu: cannot parse: %s\n", line_number, line.text) <
                0)
                status = -1;
        } else {
            int committed = inspection_commit(&inspection, &decl);
            if (committed < 0)
                status = -1;
            else if (committed > 0 &&
                     printf("line %zu: error: redefinition of %s\n",
                            line_number, decl.name) < 0)
                status = -1;
        }
        declaration_free(&decl);
        if (status < 0)
            break;
    }
    bool ok = status == 0 && inspection_print(&inspection, stdout);
    if (fflush(stdout) == EOF)
        ok = false;
    free(line.text);
    inspection_free(&inspection);
    if (!ok)
        fputs("binary inspection: allocation or I/O failure\n", stderr);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
