#ifndef SEOKGUKIM_C_COMMON_C_SCAN_H
#define SEOKGUKIM_C_COMMON_C_SCAN_H

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Lexical boundaries only: these helpers do not validate declarations,
// numeric values, suffixes, or escape sequences. start must be in bounds.
// For identifiers, numbers, literals, and punctuation, the caller must
// first recognize the token start; literal start points at its quote.
// Whitespace means horizontal spaces and tabs on an already-read line.
// Scanners return the index after their token. Input is null-terminated.
// c_scan_literal returns SIZE_MAX if the closing quote is missing.

static inline size_t c_scan_whitespace(const char *text, size_t start) {
    while (text[start] == ' ' || text[start] == '\t')
        ++start;
    return start;
}

static inline size_t c_scan_identifier(const char *text, size_t start) {
    while (isalnum((unsigned char)text[start]) || text[start] == '_')
        ++start;
    return start;
}

static inline size_t c_scan_number(const char *text, size_t start) {
    // The caller has checked the first digit or the . followed by a digit.
    // Consume a preprocessing number without validating its numeric value.

    ++start;
    while (text[start] != '\0') {
        unsigned char c = (unsigned char)text[start];
        if (isalnum(c) || c == '_' || c == '.') {
            ++start;
        } else if ((c == '+' || c == '-') &&
                   (text[start - 1] == 'e' || text[start - 1] == 'E' ||
                    text[start - 1] == 'p' || text[start - 1] == 'P')) {
            ++start;
        } else {
            break;
        }
    }
    return start;
}

static inline size_t c_scan_literal(const char *text, size_t start) {
    char quote = text[start++];
    while (text[start] != '\0') {
        if (text[start] == quote)
            return start + 1;
        if (text[start] == '\\') {
            ++start;
            if (text[start] == '\0')
                return SIZE_MAX;
        }
        ++start;
    }
    return SIZE_MAX;
}

// C11 encoding prefixes are part of the literal's preprocessing token.
static inline bool c_literal_prefix(const char *text, size_t length,
                                    char quote) {
    if (quote != '"' && quote != '\'')
        return false;
    if (length == 1)
        return text[0] == 'L' || text[0] == 'u' || text[0] == 'U';
    return quote == '"' && length == 2 && text[0] == 'u' && text[1] == '8';
}

// Longest punctuators come first, including digraphs and token-pasting tokens.
static inline size_t c_scan_punctuation(const char *text, size_t start) {
    static const char *const punctuators[] = {
        "%:%:", ">>=", "<<=", "...", "->", "++", "--", "<<", ">>", "<=",
        ">=",   "==",  "!=",  "&&",  "||", "*=", "/=", "%=", "+=", "-=",
        "&=",   "^=",  "|=",  "##",  "<:", ":>", "<%", "%>", "%:"};
    for (size_t i = 0; i < sizeof punctuators / sizeof *punctuators; ++i) {
        size_t length = strlen(punctuators[i]);
        if (strncmp(text + start, punctuators[i], length) == 0)
            return start + length;
    }
    return start + 1;
}

#endif /* SEOKGUKIM_C_COMMON_C_SCAN_H */
