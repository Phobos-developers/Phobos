#!/usr/bin/env python3
"""Remove binary assets from the [FILES] section of a Sphinx-generated .hhp.

hhc.exe parses every [FILES] entry as HTML; binary files (png/gif/jpg/eot/ttf/
woff/woff2) make it emit bogus HHC3004 warnings and eventually crash with
0xC0000005 (STATUS_ACCESS_VIOLATION).  Images and fonts referenced from the
topics/CSS are pulled into the CHM automatically, so dropping the lines is safe.

The .hhp is encoded in the LCID-mapped codepage (cp936 for zh_CN) because hhc
reads it as ANSI: this script round-trips the file in that encoding.

Usage:
    python filter_hhp.py <path-to-phobosdoc.hhp> [--encoding cp936]
"""
from __future__ import annotations

import argparse
import os
import re
import sys

BINARY_RE = re.compile(r"\.(eot|gif|png|jpg|jpeg|ico|ttf|woff2?)\s*$", re.I)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("hhp", help="path to the generated .hhp project file")
    ap.add_argument("--encoding", default="cp936",
                    help="encoding of the .hhp (default cp936, hhc reads ANSI)")
    args = ap.parse_args()

    path = os.path.abspath(args.hhp)
    if not os.path.isfile(path):
        print(f"ERROR: not a file: {path}")
        return 1

    with open(path, encoding=args.encoding) as f:
        lines = f.read().splitlines()

    kept = [ln for ln in lines if not BINARY_RE.search(ln)]
    removed = len(lines) - len(kept)
    if removed == 0:
        print("nothing to filter; .hhp already clean")
        return 0

    with open(path, "w", encoding=args.encoding, newline="\r\n") as f:
        f.write("\n".join(kept) + "\n")

    exts = sorted({m.group(1).lower() for ln in lines
                   for m in [BINARY_RE.search(ln)] if m})
    print(f"removed {removed} binary entries ({', '.join(exts)}); [FILES] now {len(kept)} lines")
    return 0


if __name__ == "__main__":
    sys.exit(main())
