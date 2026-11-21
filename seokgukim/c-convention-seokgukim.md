# C Conventions for seokgukim

These conventions apply to C code written by seokgukim.

> Data: 2026-11-14  
> Author: seokgukim

## Overview

Use PascalCase for types and macro parameters, snake_case for functions and ordinary identifiers, and LLVM-style names for enum members.
Use LLVM-based formatting with 4-space indentation.
Four-space indentation is a project preference that overrides LLVM's usual two-space indentation.

## Naming

- Use PascalCase nouns for types, including struct tags, union tags, enum tags, and typedef names.
- Use PascalCase for macro parameters, such as TypeName, ElementType, and PushName.
- Use snake_case for variables, function parameters, and struct or union members.
- Use snake_case verbs or verb phrases for functions.
- Use a category prefix and an uppercase initial for enum constants, such as TK_Identifier for TokenKind.
- Keep macro names in SCREAMING_SNAKE_CASE, such as DEFINE_VECTOR and SUCCESS.
- Prefix related C functions with their type or module name, such as preprocessor_init and preprocessor_destroy.
- Preserve identifiers supplied by the standard library, external APIs, and exercise interfaces that must remain compatible.

```c
typedef enum {
    TK_Identifier,
    TK_Number
} TokenKind;

typedef struct {
    TokenKind kind;
    char *text;
    size_t line_number;
} Token;

void token_free(Token *item);

#define DEFINE_VECTOR(TypeName, ElementType, PushName) /* ... */
```

## Formatting

- Use four spaces for indentation and no indentation tabs.
- Put opening braces on the same line as the declaration or control statement.
- Omit braces around simple single-statement if, else, and loop bodies.
- Use braces for multi-statement or complex bodies and keep bracing consistent within an if/else chain.
- Aim for an 80-column limit in C source and let clang-format handle wrapping.
- Write pointer declarations as char *text and const Token *item.
- Put spaces around binary operators and after commas, and omit spaces immediately inside parentheses.
- Keep system includes in alphabetical order within their include group.

The c-programming-deep-dive/.clang-format file contains the project formatting settings:

```yaml
BasedOnStyle: LLVM
IndentWidth: 4
ContinuationIndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 80
```

Run this command from c-programming-deep-dive to format the exercise file:

```sh
clang-format -i -style=file c-compilation-model.c
```

clang-format handles layout; naming and brace choices still require review.

## Vim and Neovim

Use the following command in the current C buffer:

```vim
:setlocal shiftwidth=4 tabstop=4 softtabstop=4 expandtab
```

For persistent C settings, add this Vimscript line to .vimrc or init.vim:

```vim
autocmd FileType c setlocal shiftwidth=4 tabstop=4 softtabstop=4 expandtab
```

shiftwidth controls indentation, tabstop controls the display width of tab characters, softtabstop controls editing behavior, and expandtab inserts spaces when pressing Tab.
The editor settings and .clang-format must agree to keep indentation consistent.

## Comments and Documentation

- Explain purpose, ownership, assumptions, and return contracts when they are not clear from the code.
- Use TODO comments to identify unfinished exercise work.
- Keep Markdown documentation ASCII-only.
- Keep each prose sentence on a single physical line without inserting a newline in the middle of the sentence.
- Use blank lines between paragraphs and allow code blocks to follow the formatting rules of their language.

The 80-column source limit does not apply to prose sentences in Markdown documentation.

## Scope

Naming and formatting changes should preserve program behavior.
Handle implementation fixes separately so that unfinished exercises remain easy to review.

## References

- [LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html)
- [Clang-Format Style Options](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)
