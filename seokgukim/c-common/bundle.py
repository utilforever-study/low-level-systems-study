#!/usr/bin/env python3
"""Inline this library's headers to produce a single-file OJ submission."""

import argparse
from pathlib import Path
import re
import sys


LIBRARY = Path(__file__).resolve().parent
# Project convention: local includes are unconditional, one per line, outside
# comments. System includes remain intact for the judge's standard library.
LOCAL_INCLUDE = re.compile(r'^\s*#\s*include\s+"([^"\n]+)"\s*$')


def bundle(source):
    """Return source with c-common includes expanded; retain header guards."""
    active = set()

    def expand(path):
        path = path.resolve()
        if path in active:
            raise ValueError(f"cyclic local include: {path}")
        active.add(path)
        result = []
        for line in path.read_text().splitlines(keepends=True):
            include = LOCAL_INCLUDE.fullmatch(line)
            if include is None:
                result.append(line)
                continue
            header = (path.parent / include[1]).resolve()
            if not header.is_relative_to(LIBRARY) or header.suffix != '.h':
                raise ValueError(f"local include is outside c-common headers: {header}")
            label = header.relative_to(LIBRARY).as_posix()
            # Clang normally exempts unused static inline functions in headers.
            # Preserve that behavior when their definitions move into main.c.
            result.extend((f'/* Begin c-common/{label}. */\n',
                           '#if defined(__clang__)\n'
                           '#pragma clang diagnostic push\n'
                           '#pragma clang diagnostic ignored "-Wunused-function"\n'
                           '#endif\n',
                           expand(header).rstrip('\n') + '\n',
                           '#if defined(__clang__)\n'
                           '#pragma clang diagnostic pop\n'
                           '#endif\n',
                           f'/* End c-common/{label}. */\n'))
        active.remove(path)
        return ''.join(result)

    return (f'/* Generated from {source.name} by c-common/bundle.py. */\n'
            '/* Edit the original source and headers, then regenerate. */\n\n'
            + expand(source))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('-o', '--output', type=Path,
                        help='write to this file instead of stdout')
    args = parser.parse_args()
    try:
        if args.output is not None:
            output = args.output.resolve()
            if output == args.source.resolve() or output.is_relative_to(LIBRARY):
                raise ValueError('output must not overwrite the source or shared library')
        result = bundle(args.source)
        if args.output is None:
            sys.stdout.write(result)
        else:
            args.output.write_text(result)
    except (OSError, ValueError) as error:
        parser.exit(1, f'bundle: {error}\n')


if __name__ == '__main__':
    main()
