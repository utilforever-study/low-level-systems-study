#include "../c-common/array.h"
#include "../c-common/c_scan.h"
#include "../c-common/text.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* C Compilation Model
 * Read the input described in the brief's "Input" section from stdin and
 * print exactly the lines described in its "Output" section.
 * Run checks the sample tests; Submit also runs the hidden ones.
 */

// Basic types and interfaces.
#define SUCCESS 0
#define FAILURE -1

typedef enum {
    TK_Identifier,
    TK_Number,
    TK_StringLiteral,
    TK_CharacterLiteral,
    TK_Punctuation,
    TK_Whitespace
} TokenKind;

// Each token owns its null-terminated text.
typedef struct {
    TokenKind kind;
    char *text;
    size_t length;
    size_t line_number;
    // Original location distinguishes source adjacency from expansion
    // boundaries.
    size_t source_line_number;
    size_t source_offset;
} Token;

C_COMMON_DEFINE_VECTOR(TokenVec, Token, token_vec_push)

typedef enum { MK_Object, MK_Function } MacroKind;

typedef struct {
    char *name;
    MacroKind kind;
    char *params[8];
    size_t param_count;
    TokenVec body;
} Macro;

C_COMMON_DEFINE_VECTOR(MacroTable, Macro, macro_table_push)

typedef struct {
    bool parent_active;
    bool condition_true;
    bool else_seen;
} Conditional;

C_COMMON_DEFINE_VECTOR(ConditionalStack, Conditional, conditional_stack_push)

typedef struct {
    FILE *input;
    // Number of physical newlines consumed so far.
    size_t line_number;
    bool in_block_comment;
} SourceReader;

// A new span starts when retained text resumes on another physical line.
typedef struct {
    size_t offset;
    size_t line_number;
} SourceSpan;

C_COMMON_DEFINE_VECTOR(SourceSpans, SourceSpan, source_spans_push)

// Comments are replaced here; preserve indentation and literal contents.
typedef struct {
    char *text;
    size_t length;
    size_t capacity;
    size_t line_number;
    SourceSpans spans;
} SourceLine;

// Disabled macro pointers are borrowed from the table during expansion.
C_COMMON_DEFINE_VECTOR(ExpansionContext, const Macro *, expansion_context_push)

typedef struct {
    MacroTable macros;
    ConditionalStack conditionals;
    size_t expansion_count;
    FILE *output;
    bool last_output_period;
    bool penultimate_output_period;
} Preprocessor;

// Lifecycle and memory ownership.
void preprocessor_init(Preprocessor *pp, FILE *output) {
    *pp = (Preprocessor){.output = output};
}
void token_vec_free(TokenVec *tokens) {
    for (size_t i = 0; i < tokens->count; ++i)
        free(tokens->items[i].text);
    free(tokens->items);
    *tokens = (TokenVec){0};
}

// A stored macro owns its name, parameter names, and body tokens.
void macro_free(Macro *definition) {
    free(definition->name);
    for (size_t i = 0; i < definition->param_count; ++i)
        free(definition->params[i]);
    token_vec_free(&definition->body);
    *definition = (Macro){0};
}

void preprocessor_destroy(Preprocessor *pp) {
    for (size_t i = 0; i < pp->macros.count; ++i)
        macro_free(&pp->macros.items[i]);
    free(pp->macros.items);
    free(pp->conditionals.items);
    // The output stream is borrowed from the caller.
    *pp = (Preprocessor){0};
}

void source_line_free(SourceLine *line) {
    free(line->text);
    free(line->spans.items);
    *line = (SourceLine){0};
}

static int source_line_reserve(SourceLine *line, size_t required) {
    while (line->capacity < required) {
        char *grown =
            c_array_grow(line->text, &line->capacity, sizeof *line->text);
        if (grown == NULL)
            return FAILURE;
        line->text = grown;
    }
    return SUCCESS;
}

static int source_line_append(SourceLine *line, int c, size_t line_number) {
    if (line->length > SIZE_MAX - 2 ||
        source_line_reserve(line, line->length + 2) == FAILURE)
        return FAILURE;
    if (line->spans.count == 0 ||
        line->spans.items[line->spans.count - 1].line_number != line_number) {
        SourceSpan span = {.offset = line->length, .line_number = line_number};
        if (source_spans_push(&line->spans, span) == FAILURE)
            return FAILURE;
    }
    line->text[line->length++] = (char)c;
    line->text[line->length] = '\0';
    return SUCCESS;
}

// Reading and tokenizing.
//  read_logical_line: 1 = line, 0 = end of input, -1 = failure.
//  Initialize SourceReader with { .input = stdin }.
//  Initialize SourceLine with {0}; its allocation is reused between reads.
//  A returned line excludes its ending newline and maps retained text to its
//  physical line numbers. Other int operations: 0 = success, -1 = failure.
//  Report recoverable input errors and continue processing.

// Normalize physical line endings before comments and tokens are recognized.
// Otherwise a CR in CRLF becomes part of a macro's replacement list.
static int source_getc(SourceReader *reader) {
    int c = fgetc(reader->input);
    if (c == '\r') {
        int next = fgetc(reader->input);
        if (next != '\n' && next != EOF)
            ungetc(next, reader->input);
        return '\n';
    }
    return c;
}

int read_logical_line(SourceReader *reader, SourceLine *line) {
    line->length = 0;
    line->spans.count = 0;
    line->line_number = reader->line_number + 1;
    if (line->text != NULL)
        line->text[0] = '\0';

    int c = source_getc(reader);
    if (c == EOF)
        return ferror(reader->input) ? FAILURE : 0;
    if (source_line_reserve(line, 1) == FAILURE)
        return FAILURE;
    line->text[0] = '\0';

    int quote = 0;
    bool escaped = false;
    bool block_star = false;
    while (c != EOF) {
        if (c == '\n') {
            ++reader->line_number;
            if (!reader->in_block_comment)
                break;
            block_star = false;
        } else if (reader->in_block_comment) {
            if (block_star && c == '/') {
                reader->in_block_comment = false;
                block_star = false;
            } else {
                block_star = c == '*';
            }
        } else if (quote != 0) {
            if (source_line_append(line, c, reader->line_number + 1) == FAILURE)
                return FAILURE;
            if (escaped)
                escaped = false;
            else if (c == '\\')
                escaped = true;
            else if (c == quote)
                quote = 0;
        } else if (c == '/') {
            int next = source_getc(reader);
            if (next == '*' || next == '/') {
                if (source_line_append(line, ' ', reader->line_number + 1) ==
                    FAILURE)
                    return FAILURE;
                if (next == '*') {
                    reader->in_block_comment = true;
                    block_star = false;
                } else {
                    do {
                        c = source_getc(reader);
                    } while (c != EOF && c != '\n');
                    continue;
                }
            } else {
                if (source_line_append(line, c, reader->line_number + 1) ==
                    FAILURE)
                    return FAILURE;
                c = next;
                continue;
            }
        } else {
            if (source_line_append(line, c, reader->line_number + 1) == FAILURE)
                return FAILURE;
            if (c == '"' || c == '\'')
                quote = c;
        }
        c = source_getc(reader);
    }

    return ferror(reader->input) ? FAILURE : 1;
}

int tokenize(const SourceLine *line, TokenVec *tokens) {
    const char *text = line->text;
    size_t i = 0;
    size_t span = 0;
    size_t line_number = line->line_number;

    while (i < line->length) {
        while (span < line->spans.count && line->spans.items[span].offset <= i)
            line_number = line->spans.items[span++].line_number;
        unsigned char c = (unsigned char)text[i];
        TokenKind kind;
        size_t end;

        if (c == ' ' || c == '\t') {
            kind = TK_Whitespace;
            end = c_scan_whitespace(text, i);
        } else if (c == '"' || c == '\'') {
            kind = c == '"' ? TK_StringLiteral : TK_CharacterLiteral;
            end = c_scan_literal(text, i);
            // Keep malformed quoted text opaque, then recover on the next line.
            // The task defines no diagnostic for a missing closing quote.
            if (end == SIZE_MAX)
                end = line->length;
        } else if (isalpha(c) || c == '_') {
            kind = TK_Identifier;
            end = c_scan_identifier(text, i);
            if (c_literal_prefix(text + i, end - i, text[end])) {
                kind =
                    text[end] == '"' ? TK_StringLiteral : TK_CharacterLiteral;
                end = c_scan_literal(text, end);
                if (end == SIZE_MAX)
                    end = line->length;
            }
        } else if (isdigit(c) || (c == '.' && i + 1 < line->length &&
                                  isdigit((unsigned char)text[i + 1]))) {
            kind = TK_Number;
            end = c_scan_number(text, i);
        } else {
            kind = TK_Punctuation;
            end = c_scan_punctuation(text, i);
        }

        Token t = {.kind = kind,
                   .text = malloc(end - i + 1),
                   .length = end - i,
                   .line_number = line_number,
                   .source_line_number = line->line_number,
                   .source_offset = i};

        if (t.text == NULL)
            return FAILURE;
        memcpy(t.text, text + i, t.length);
        t.text[t.length] = '\0';
        if (token_vec_push(tokens, t) == FAILURE) {
            free(t.text);
            return FAILURE;
        }

        i = end;
    }
    return SUCCESS;
}

// Definition parsing and storage. Macro tables own independent copies.
typedef enum { DR_Ok, DR_Invalid, DR_NoMemory } DefineResult;

static bool punctuation(const Token *token, const char *text) {
    return token->kind == TK_Punctuation && strcmp(token->text, text) == 0;
}

static size_t skip_whitespace(const TokenVec *tokens, size_t i) {
    while (i < tokens->count && tokens->items[i].kind == TK_Whitespace)
        ++i;
    return i;
}

static int copy_token(TokenVec *output, const Token *token) {
    Token copy = *token;
    copy.text = c_text_copy(token->text);
    if (copy.text == NULL)
        return FAILURE;
    if (token_vec_push(output, copy) == FAILURE) {
        free(copy.text);
        return FAILURE;
    }
    return SUCCESS;
}

// The input is a borrowed view of the tokens following define.
static DefineResult parse_define_tokens(const TokenVec *tokens, Macro *out) {
    *out = (Macro){0};
    size_t i = skip_whitespace(tokens, 0);
    if (i == tokens->count || tokens->items[i].kind != TK_Identifier)
        return DR_Invalid;
    out->name = c_text_copy(tokens->items[i++].text);
    if (out->name == NULL)
        return DR_NoMemory;

    // Whitespace here makes this an object-like definition.
    if (i < tokens->count && punctuation(&tokens->items[i], "(")) {
        out->kind = MK_Function;
        i = skip_whitespace(tokens, i + 1);
        if (i < tokens->count && punctuation(&tokens->items[i], ")")) {
            ++i;
        } else {
            for (;;) {
                if (i == tokens->count ||
                    tokens->items[i].kind != TK_Identifier ||
                    out->param_count == 8)
                    return DR_Invalid;
                for (size_t p = 0; p < out->param_count; ++p)
                    if (strcmp(out->params[p], tokens->items[i].text) == 0)
                        return DR_Invalid;
                char *param = c_text_copy(tokens->items[i].text);
                if (param == NULL)
                    return DR_NoMemory;
                out->params[out->param_count++] = param;
                i = skip_whitespace(tokens, i + 1);
                if (i < tokens->count && punctuation(&tokens->items[i], ")")) {
                    ++i;
                    break;
                }
                if (i == tokens->count || !punctuation(&tokens->items[i], ","))
                    return DR_Invalid;
                i = skip_whitespace(tokens, i + 1);
            }
        }
    }

    i = skip_whitespace(tokens, i);
    size_t end = tokens->count;
    while (end > i && tokens->items[end - 1].kind == TK_Whitespace)
        --end;
    for (; i < end; ++i)
        if (copy_token(&out->body, &tokens->items[i]) == FAILURE)
            return DR_NoMemory;
    return DR_Ok;
}

DefineResult parse_define(const TokenVec *tokens, Macro *out) {
    DefineResult result = parse_define_tokens(tokens, out);
    if (result != DR_Ok)
        macro_free(out);
    return result;
}

const Macro *macro_find(const MacroTable *table, const char *name) {
    for (size_t i = 0; i < table->count; ++i)
        if (strcmp(table->items[i].name, name) == 0)
            return &table->items[i];
    return NULL;
}

// The caller owns the copy, including partial results on failure.
static int macro_copy(const Macro *definition, Macro *copy) {
    *copy = (Macro){.kind = definition->kind};
    copy->name = c_text_copy(definition->name);
    if (copy->name == NULL)
        return FAILURE;
    for (size_t i = 0; i < definition->param_count; ++i) {
        char *param = c_text_copy(definition->params[i]);
        if (param == NULL)
            return FAILURE;
        copy->params[copy->param_count++] = param;
    }
    for (size_t i = 0; i < definition->body.count; ++i)
        if (copy_token(&copy->body, &definition->body.items[i]) == FAILURE)
            return FAILURE;
    return SUCCESS;
}

int macro_define(MacroTable *table, const Macro *definition) {
    Macro copy;
    if (macro_copy(definition, &copy) == FAILURE) {
        macro_free(&copy);
        return FAILURE;
    }
    for (size_t i = 0; i < table->count; ++i) {
        if (strcmp(table->items[i].name, copy.name) == 0) {
            macro_free(&table->items[i]);
            table->items[i] = copy;
            return SUCCESS;
        }
    }
    if (macro_table_push(table, copy) == SUCCESS)
        return SUCCESS;
    macro_free(&copy);
    return FAILURE;
}

void macro_undef(MacroTable *table, const char *name) {
    for (size_t i = 0; i < table->count; ++i) {
        if (strcmp(table->items[i].name, name) == 0) {
            macro_free(&table->items[i]);
            memmove(table->items + i, table->items + i + 1,
                    (table->count - i - 1) * sizeof *table->items);
            table->items[--table->count] = (Macro){0};
            return;
        }
    }
}

static bool preprocessor_active(const Preprocessor *pp) {
    if (pp->conditionals.count == 0)
        return true;
    const Conditional *top =
        &pp->conditionals.items[pp->conditionals.count - 1];
    return top->parent_active && top->condition_true;
}

static int diagnostic(Preprocessor *pp, size_t line_number, const char *format,
                      ...) {
    if (fprintf(pp->output, "line %zu: error: ", line_number) < 0)
        return FAILURE;
    va_list args;
    va_start(args, format);
    int result = vfprintf(pp->output, format, args);
    va_end(args);
    return result < 0 || fputc('\n', pp->output) == EOF ? FAILURE : SUCCESS;
}

static int write_raw_tokens(Preprocessor *pp, const TokenVec *tokens) {
    for (size_t i = 0; i < tokens->count; ++i) {
        const Token *token = &tokens->items[i];
        if (fwrite(token->text, 1, token->length, pp->output) != token->length)
            return FAILURE;
    }
    return fputc('\n', pp->output) == EOF ? FAILURE : SUCCESS;
}

static int directive_error(Preprocessor *pp, const TokenVec *tokens,
                           const char *message) {
    size_t first = skip_whitespace(tokens, 0);
    if (diagnostic(pp, tokens->items[first].line_number, "%s", message) ==
        FAILURE)
        return FAILURE;
    return write_raw_tokens(pp, tokens);
}

// The caller has identified # as the first non-whitespace token.
int handle_directive(Preprocessor *pp, const TokenVec *tokens) {
    size_t keyword = skip_whitespace(tokens, skip_whitespace(tokens, 0) + 1);
    if (keyword == tokens->count)
        return preprocessor_active(pp) ? write_raw_tokens(pp, tokens) : SUCCESS;
    const char *name = tokens->items[keyword].text;
    size_t argument = skip_whitespace(tokens, keyword + 1);

    if (strcmp(name, "ifdef") == 0 || strcmp(name, "ifndef") == 0) {
        if (argument == tokens->count ||
            tokens->items[argument].kind != TK_Identifier)
            return directive_error(pp, tokens,
                                   strcmp(name, "ifdef") == 0
                                       ? "#ifdef needs a macro name"
                                       : "#ifndef needs a macro name");
        bool defined =
            macro_find(&pp->macros, tokens->items[argument].text) != NULL;
        Conditional frame = {
            .parent_active = preprocessor_active(pp),
            .condition_true = strcmp(name, "ifdef") == 0 ? defined : !defined};
        return conditional_stack_push(&pp->conditionals, frame);
    }
    if (strcmp(name, "else") == 0) {
        if (pp->conditionals.count == 0)
            return directive_error(pp, tokens, "#else without #if");
        Conditional *top = &pp->conditionals.items[pp->conditionals.count - 1];
        if (top->else_seen)
            return directive_error(pp, tokens, "#else after #else");
        top->else_seen = true;
        top->condition_true = !top->condition_true;
        return SUCCESS;
    }
    if (strcmp(name, "endif") == 0) {
        if (pp->conditionals.count == 0)
            return directive_error(pp, tokens, "#endif without #if");
        --pp->conditionals.count;
        return SUCCESS;
    }
    if (!preprocessor_active(pp))
        return SUCCESS;
    if (strcmp(name, "define") == 0) {
        if (argument == tokens->count ||
            tokens->items[argument].kind != TK_Identifier)
            return directive_error(pp, tokens, "macro name missing");
        TokenVec rest = {.items = tokens->items + keyword + 1,
                         .count = tokens->count - keyword - 1};
        Macro definition;
        DefineResult parsed = parse_define(&rest, &definition);
        if (parsed == DR_NoMemory)
            return FAILURE;
        if (parsed == DR_Invalid) {
            if (diagnostic(pp, tokens->items[argument].line_number,
                           "bad parameter list for %s",
                           tokens->items[argument].text) == FAILURE)
                return FAILURE;
            return write_raw_tokens(pp, tokens);
        }
        int result = macro_define(&pp->macros, &definition);
        macro_free(&definition);
        return result;
    }
    if (strcmp(name, "undef") == 0) {
        if (argument == tokens->count ||
            tokens->items[argument].kind != TK_Identifier)
            return directive_error(pp, tokens, "macro name missing");
        macro_undef(&pp->macros, tokens->items[argument].text);
        return SUCCESS;
    }
    return write_raw_tokens(pp, tokens);
}

// A replacement carries its disabled macros along when it is rescanned next to
// the remaining input. The pointers stay valid for the duration of a line.

typedef struct {
    Token token;
    ExpansionContext disabled;
} ExpansionToken;

C_COMMON_DEFINE_VECTOR(ExpansionTokens, ExpansionToken, expansion_tokens_push)

static bool macro_disabled(const ExpansionContext *context,
                           const Macro *macro) {
    for (size_t i = 0; i < context->count; ++i)
        if (context->items[i] == macro)
            return true;
    return false;
}

static int disable_macro(ExpansionContext *context, const Macro *macro) {
    return macro_disabled(context, macro)
               ? SUCCESS
               : expansion_context_push(context, macro);
}

static int merge_disabled(ExpansionContext *target,
                          const ExpansionContext *source) {
    for (size_t i = 0; i < source->count; ++i)
        if (disable_macro(target, source->items[i]) == FAILURE)
            return FAILURE;
    return SUCCESS;
}

static void expansion_token_free(ExpansionToken *token) {
    free(token->token.text);
    free(token->disabled.items);
    *token = (ExpansionToken){0};
}

static void expansion_tokens_free(ExpansionTokens *tokens) {
    for (size_t i = 0; i < tokens->count; ++i)
        expansion_token_free(&tokens->items[i]);
    free(tokens->items);
    *tokens = (ExpansionTokens){0};
}

static int expansion_append(ExpansionTokens *output, const Token *token,
                            const ExpansionContext *disabled) {
    ExpansionToken copy = {.token = *token};
    copy.token.text = c_text_copy(token->text);
    if (copy.token.text == NULL)
        return FAILURE;
    if (merge_disabled(&copy.disabled, disabled) == FAILURE ||
        expansion_tokens_push(output, copy) == FAILURE) {
        expansion_token_free(&copy);
        return FAILURE;
    }
    return SUCCESS;
}

static size_t expansion_skip_whitespace(const ExpansionTokens *tokens,
                                        size_t i) {
    while (i < tokens->count && tokens->items[i].token.kind == TK_Whitespace)
        ++i;
    return i;
}

typedef struct {
    size_t start, end;
} ArgumentRange;

// Return the closing parenthesis, or count if this line has no matching one.
static size_t collect_arguments(const ExpansionTokens *tokens, size_t open,
                                size_t param_count, ArgumentRange ranges[8],
                                size_t *argument_count) {
    size_t start = open + 1;
    size_t depth = 0;
    *argument_count = 0;
    for (size_t i = start; i < tokens->count; ++i) {
        const Token *token = &tokens->items[i].token;
        bool close = punctuation(token, ")") && depth == 0;
        if (close || (punctuation(token, ",") && depth == 0)) {
            size_t first = expansion_skip_whitespace(tokens, start);
            size_t end = i;
            while (end > first &&
                   tokens->items[end - 1].token.kind == TK_Whitespace)
                --end;
            // F() is one empty argument for a macro with parameters.
            if (!close || *argument_count != 0 || first < end ||
                param_count != 0) {
                if (*argument_count < 8)
                    ranges[*argument_count] = (ArgumentRange){first, end};
                ++*argument_count;
            }
            if (close)
                return i;
            start = i + 1;
        } else if (punctuation(token, "(")) {
            ++depth;
        } else if (punctuation(token, ")")) {
            --depth;
        }
    }
    return tokens->count;
}

static int expand_work(Preprocessor *pp, ExpansionTokens *work);

static int expand_arguments(Preprocessor *pp, const ExpansionTokens *work,
                            const Macro *macro, const ArgumentRange ranges[8],
                            ExpansionTokens args[8]) {
    // Argument prescan happens before disabling the called macro, so
    // MAX(MAX(1, 2), 3) expands both calls.

    if (macro->kind == MK_Object)
        return SUCCESS;
    for (size_t p = 0; p < macro->param_count; ++p) {
        for (size_t i = ranges[p].start; i < ranges[p].end; ++i)
            if (expansion_append(&args[p], &work->items[i].token,
                                 &work->items[i].disabled) == FAILURE)
                return FAILURE;
        if (expand_work(pp, &args[p]) == FAILURE)
            return FAILURE;
    }
    return SUCCESS;
}

static int substitute_body(const ExpansionTokens *work, size_t name,
                           const Macro *macro, const ExpansionTokens args[8],
                           const ExpansionContext *disabled,
                           ExpansionTokens *replacement) {
    for (size_t i = 0; i < macro->body.count; ++i) {
        const Token *body = &macro->body.items[i];
        size_t param = macro->param_count;
        if (body->kind == TK_Identifier)
            for (size_t p = 0; p < macro->param_count; ++p)
                if (strcmp(body->text, macro->params[p]) == 0) {
                    param = p;
                    break;
                }
        if (param < macro->param_count) {
            // Empty replacements can expose whitespace at either edge during
            // argument prescan. It does not belong to the substituted argument.
            size_t first = expansion_skip_whitespace(&args[param], 0);
            size_t end = args[param].count;
            while (end > first &&
                   args[param].items[end - 1].token.kind == TK_Whitespace)
                --end;
            for (size_t a = first; a < end; ++a) {
                const ExpansionToken *arg = &args[param].items[a];
                if (expansion_append(replacement, &arg->token,
                                     &arg->disabled) == FAILURE)
                    return FAILURE;
                if (merge_disabled(
                        &replacement->items[replacement->count - 1].disabled,
                        disabled) == FAILURE)
                    return FAILURE;
            }
        } else {
            Token token = *body;
            token.line_number = work->items[name].token.line_number;
            if (expansion_append(replacement, &token, disabled) == FAILURE)
                return FAILURE;
        }
    }
    return SUCCESS;
}

static int build_replacement(Preprocessor *pp, const ExpansionTokens *work,
                             size_t name, size_t close, const Macro *macro,
                             const ArgumentRange ranges[8],
                             ExpansionTokens *replacement) {
    ExpansionTokens args[8] = {{0}};
    ExpansionContext disabled = {0};
    int result = expand_arguments(pp, work, macro, ranges, args);
    const ExpansionContext *invocation = &work->items[name].disabled;
    for (size_t i = 0; result == SUCCESS && i < invocation->count; ++i) {
        const Macro *blocked = invocation->items[i];
        // A function call can cross the end of an earlier replacement.
        // Only disables shared by its name and closing parenthesis survive.
        if (macro->kind == MK_Object ||
            macro_disabled(&work->items[close].disabled, blocked))
            result = disable_macro(&disabled, blocked);
    }
    if (result == SUCCESS)
        result = disable_macro(&disabled, macro);
    if (result == SUCCESS)
        result =
            substitute_body(work, name, macro, args, &disabled, replacement);
    for (size_t p = 0; p < 8; ++p)
        expansion_tokens_free(&args[p]);
    free(disabled.items);
    return result;
}

// Move a replacement into the work list; it will be scanned at start again..
static int splice_replacement(ExpansionTokens *work, size_t start, size_t end,
                              ExpansionTokens *replacement) {
    size_t remaining = work->count - (end - start);
    if (replacement->count > SIZE_MAX - remaining)
        return FAILURE;
    size_t count = remaining + replacement->count;
    while (work->capacity < count) {
        void *grown =
            c_array_grow(work->items, &work->capacity, sizeof *work->items);
        if (grown == NULL)
            return FAILURE;
        work->items = grown;
    }
    for (size_t i = start; i < end; ++i)
        expansion_token_free(&work->items[i]);
    memmove(work->items + start + replacement->count, work->items + end,
            (work->count - end) * sizeof *work->items);
    if (replacement->count != 0)
        memcpy(work->items + start, replacement->items,
               replacement->count * sizeof *replacement->items);
    work->count = count;
    free(replacement->items);
    *replacement = (ExpansionTokens){0};
    return SUCCESS;
}

// An erroneous call can itself be an argument. Preserve the whole call during
// later rescans, including macros inside it, without repeating its diagnostic.

static int preserve_call(Preprocessor *pp, ExpansionTokens *work, size_t start,
                         size_t end) {
    for (size_t i = start; i < end; ++i)
        for (size_t m = 0; m < pp->macros.count; ++m)
            if (disable_macro(&work->items[i].disabled, &pp->macros.items[m]) ==
                FAILURE)
                return FAILURE;
    return SUCCESS;
}

static int expand_work(Preprocessor *pp, ExpansionTokens *work) {
    size_t i = 0;
    while (i < work->count) {
        const ExpansionToken *token = &work->items[i];
        const Macro *macro = token->token.kind == TK_Identifier
                                 ? macro_find(&pp->macros, token->token.text)
                                 : NULL;
        if (macro == NULL || macro_disabled(&token->disabled, macro)) {
            ++i;
            continue;
        }
        ArgumentRange ranges[8] = {{0}};
        size_t end = i + 1;
        size_t close = i;
        if (macro->kind == MK_Function) {
            size_t open = expansion_skip_whitespace(work, i + 1);
            if (open == work->count ||
                !punctuation(&work->items[open].token, "(")) {
                ++i;
                continue;
            }
            size_t count;
            close = collect_arguments(work, open, macro->param_count, ranges,
                                      &count);
            if (close == work->count) {
                if (diagnostic(pp, token->token.line_number,
                               "unterminated argument list for %s",
                               macro->name) == FAILURE)
                    return FAILURE;
                if (preserve_call(pp, work, i, work->count) == FAILURE)
                    return FAILURE;
                break;
            }
            if (count != macro->param_count) {
                if (diagnostic(pp, token->token.line_number,
                               "%s expects %zu argument%s, got %zu",
                               macro->name, macro->param_count,
                               macro->param_count > 1 ? "s" : "",
                               count) == FAILURE)
                    return FAILURE;
                if (preserve_call(pp, work, i, close + 1) == FAILURE)
                    return FAILURE;
                i = close + 1;
                continue;
            }
            end = close + 1;
        }
        ExpansionTokens replacement = {0};
        if (build_replacement(pp, work, i, close, macro, ranges,
                              &replacement) == FAILURE ||
            splice_replacement(work, i, end, &replacement) == FAILURE) {
            expansion_tokens_free(&replacement);
            return FAILURE;
        }
        ++pp->expansion_count;
    }
    return SUCCESS;
}

int expand_tokens(Preprocessor *pp, const TokenVec *input,
                  ExpansionContext *context, TokenVec *output) {
    ExpansionTokens work = {0};
    int result = SUCCESS;
    for (size_t i = 0; result == SUCCESS && i < input->count; ++i)
        result = expansion_append(&work, &input->items[i], context);
    if (result == SUCCESS)
        result = expand_work(pp, &work);
    for (size_t i = 0; result == SUCCESS && i < work.count; ++i)
        result = copy_token(output, &work.items[i].token);
    expansion_tokens_free(&work);
    return result;
}

// Check only the boundaries affected by macro substitution.
static bool tokens_need_space(const Token *left, const Token *right,
                              bool before_left_is_period) {
    if (left->source_line_number == right->source_line_number &&
        left->source_offset + left->length == right->source_offset)
        return false;
    unsigned char first = (unsigned char)right->text[0];
    if (left->kind == TK_Identifier)
        return isalnum(first) || first == '_' ||
               c_literal_prefix(left->text, left->length, (char)first);
    if (left->kind == TK_Number)
        return isalnum(first) || first == '_' || first == '.' || first == '+' ||
               first == '-';
    if (punctuation(left, "."))
        return isdigit(first) || (first == '.' && before_left_is_period);
    if (punctuation(left, "/") &&
        (right->text[0] == '*' || right->text[0] == '/'))
        return true;
    // Clang treats # and its digraph alike when separating printed tokens.
    if ((punctuation(left, "#") && (first == '@' || first == '%')) ||
        (punctuation(left, "%:") && (first == '%' || first == '#')))
        return true;
    if (left->kind == TK_Punctuation && right->kind == TK_Punctuation) {
        /* The longest punctuator, %:%:, contains four characters. */
        char joined[9];
        memcpy(joined, left->text, left->length);
        memcpy(joined + left->length, right->text, right->length + 1);
        return c_scan_punctuation(joined, 0) != left->length;
    }
    return false;
}

int write_tokens(Preprocessor *pp, const TokenVec *tokens,
                 const Token *indentation) {
    size_t first = skip_whitespace(tokens, 0);
    if (first == tokens->count)
        return SUCCESS;
    size_t end = tokens->count;
    while (end > first && tokens->items[end - 1].kind == TK_Whitespace)
        --end;

    // Deleting a macro can expose internal whitespace at the start. Preserve
    // the source indentation, and otherwise collapse that whitespace once.
    // A leading expanded # needs a space so it cannot look like a directive.
    if (indentation != NULL) {
        if (fwrite(indentation->text, 1, indentation->length, pp->output) !=
            indentation->length)
            return FAILURE;
    } else if ((first != 0 || punctuation(&tokens->items[first], "#") ||
                punctuation(&tokens->items[first], "%:")) &&
               fputc(' ', pp->output) == EOF)
        return FAILURE;
    const Token *previous = NULL;
    bool space = false;
    for (size_t i = first; i < end; ++i) {
        const Token *token = &tokens->items[i];
        if (token->kind == TK_Whitespace) {
            space = true;
            continue;
        }
        if (!space && previous != NULL)
            space = tokens_need_space(previous, token,
                                      pp->penultimate_output_period);
        if (space && fputc(' ', pp->output) == EOF)
            return FAILURE;
        if (fwrite(token->text, 1, token->length, pp->output) != token->length)
            return FAILURE;
        previous = token;
        pp->penultimate_output_period = pp->last_output_period;
        pp->last_output_period = punctuation(token, ".");
        space = false;
    }
    return fputc('\n', pp->output) == EOF ? FAILURE : SUCCESS;
}

int preprocessor_finish(Preprocessor *pp, const SourceReader *reader) {
    if (reader->in_block_comment &&
        fputs("error: unterminated comment at end of input\n", pp->output) ==
            EOF)
        return FAILURE;
    if (pp->conditionals.count != 0 &&
        fprintf(pp->output,
                "error: %zu unterminated conditional%s at end of input\n",
                pp->conditionals.count,
                pp->conditionals.count > 1 ? "s" : "") < 0)
        return FAILURE;
    if (fprintf(pp->output, "-- %zu macro%s defined, %zu expansion%s\n",
                pp->macros.count, pp->macros.count == 1 ? "" : "s",
                pp->expansion_count, pp->expansion_count == 1 ? "" : "s") < 0)
        return FAILURE;
    return fflush(pp->output) == EOF ? FAILURE : SUCCESS;
}

static int process_line(Preprocessor *pp, const SourceLine *line) {
    TokenVec tokens = {0};
    TokenVec expanded = {0};
    ExpansionContext context = {0};
    int result = tokenize(line, &tokens);
    if (result == SUCCESS) {
        size_t first = skip_whitespace(&tokens, 0);
        if (first < tokens.count && tokens.items[first].text[0] == '#') {
            if (punctuation(&tokens.items[first], "#"))
                result = handle_directive(pp, &tokens);
            else if (preprocessor_active(pp))
                result = write_raw_tokens(pp, &tokens);
        } else if (preprocessor_active(pp)) {
            const Token *indentation = first != 0 ? &tokens.items[0] : NULL;
            result = expand_tokens(pp, &tokens, &context, &expanded);
            if (result == SUCCESS)
                result = write_tokens(pp, &expanded, indentation);
        }
    }
    free(context.items);
    token_vec_free(&expanded);
    token_vec_free(&tokens);
    return result;
}

int preprocessor_run(Preprocessor *pp, FILE *input) {
    SourceReader reader = {.input = input};
    SourceLine line = {0};
    int status;
    while ((status = read_logical_line(&reader, &line)) == 1) {
        if (process_line(pp, &line) == FAILURE) {
            source_line_free(&line);
            return FAILURE;
        }
    }
    source_line_free(&line);
    return status == FAILURE ? FAILURE : preprocessor_finish(pp, &reader);
}

int main(void) {
    Preprocessor pp;
    preprocessor_init(&pp, stdout);
    int result = preprocessor_run(&pp, stdin);
    preprocessor_destroy(&pp);
    return result == SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}
