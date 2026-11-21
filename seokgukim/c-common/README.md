# Shared C helpers

Small C11 header-only helpers extracted from `c-programming-deep-dive/c-compilation-model.c`.
Include only the modules an exercise needs; no separate linker arguments are required.
Functions use `static inline` so multiple source files can include the headers safely.

| Header     | Interface                                                                                                             |
| ---------- | --------------------------------------------------------------------------------------------------------------------- |
| `array.h`  | `c_array_grow`, `C_COMMON_DEFINE_VECTOR(TypeName, ElementType, PushName)`                                             |
| `text.h`   | `c_text_copy`                                                                                                         |
| `c_scan.h` | `c_scan_whitespace`, `c_scan_identifier`, `c_scan_number`, `c_scan_literal`, `c_scan_punctuation`, `c_literal_prefix` |

```c
#include "../c-common/array.h"
#include "../c-common/c_scan.h"
#include "../c-common/text.h"
```

Initialize vectors with `{0}`; push returns 0 on success and -1 on allocation failure.
Vectors copy element values and do not free nested allocations.
Free owned elements before freeing the vector's `items` allocation.
`c_array_grow` requires a nonzero element size and leaves the old allocation and capacity unchanged on failure; assign its result to a temporary pointer first.
`c_text_copy` returns an owned allocation or NULL and expects a NUL-terminated string.

Scanners expect NUL-terminated input and return the offset immediately after a token.
The caller must recognize the token's first character and pass an in-bounds offset; `c_scan_literal` starts at the quote, after any encoding prefix.
`c_scan_literal` returns `SIZE_MAX` for an unclosed literal.
`c_scan_whitespace` consumes only spaces and tabs.
`c_scan_number` finds preprocessing-number boundaries; it does not validate numeric syntax or determine a value.
The scanners do not decode escapes or determine decoded string lengths.

Source reading, token ownership, macro expansion, diagnostics, and declaration classification belong to each exercise.
In particular, the compilation model's comment-aware logical-line reader is not a physical-line reader for binary inspection.

## Local build and online judge submission

From `c-programming-deep-dive`:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Werror c-compilation-model.c -o /tmp/c-compilation-model
python3 ../c-common/bundle.py c-compilation-model.c -o oj/c-compilation-model.c
python3 ../c-common/bundle.py binary-inspection-with-objdump-and-nm.c -o oj/binary-inspection-with-objdump-and-nm.c
```

Paste the generated file from `oj/` into the judge's editor.
The bundle inserts the shared header code in place of local includes and retains standard C includes and header guards.
It has no dependency on this directory at compile time.
Regenerate after editing a source or header; generated files should not be edited directly.
The binary-inspection implementation and its validation are documented in [../c-programming-deep-dive/binary-inspection-notes.md](../c-programming-deep-dive/binary-inspection-notes.md).

The bundler uses Python 3.9+ and supports this project's unconditional, single-line quoted includes of `c-common` headers.
Do not place include-like lines inside multiline comments or literals, use conditional local includes, or use it as a general C preprocessor.
Unrecognized include layouts are outside its supported input convention.
