#!/usr/bin/env python3
"""Idempotently patch sphinxcontrib-htmlhelp inside a virtualenv for CHM builds.

Two patches, both required for a healthy Chinese CHM:

  A. update_page_context: write HTML topics as UTF-8.  By default the builder
     picks the codepage from an LCID table (zh_CN -> cp936) while the Sphinx
     themes hardcode <meta charset="utf-8">; the mismatch garbles titles and
     full-text indexing in the CHM viewer.
  B. build_toc_file: resolve toctrees with includehidden=True.  Sphinx resolves
     a hidden toctree to nothing when includehidden is False
     (sphinx/environment/adapters/toctree.py), which produces an empty .hhc
     and a blank Contents pane in the CHM viewer.

Usage:
    python patch_sphinxcontrib_htmlhelp.py <path-to-site-packages/sphinxcontrib/htmlhelp/__init__.py>

Exit code 0 means the file is patched (or was already patched).
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ENCODING_TOKEN = "ctx['encoding'] = 'utf-8'"
INCLUDEHIDDEN_CALL_RE = re.compile(
    r"get_and_resolve_doctree\([^)]*includehidden\s*=\s*True", re.S
)


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__)
        return 2

    target = Path(sys.argv[1])
    if not target.is_file():
        print(f"ERROR: not a file: {target}")
        print("Get the exact path with:")
        print('  <venv>\\Scripts\\python.exe -c "import sphinxcontrib.htmlhelp as m; print(m.__file__)"')
        return 1

    text = target.read_text(encoding="utf-8")

    # --- Patch A: topic encoding -------------------------------------------------
    if ENCODING_TOKEN in text:
        print("[patch A] already applied (ctx['encoding'] = 'utf-8')")
    else:
        new_text, n = re.subn(
            r"ctx\['encoding'\] = self\.encoding",
            ENCODING_TOKEN,
            text,
            count=1,
        )
        if n == 0:
            print("ERROR: patch A pattern 'ctx[\\'encoding\\'] = self.encoding' not found.")
            print("The vendored sphinxcontrib-htmlhelp differs from the expected layout;")
            print("inspect sphinxcontrib/htmlhelp/__init__.py::update_page_context manually.")
            return 1
        text = new_text
        print("[patch A] applied: HTML topics will be written as UTF-8")

    # --- Patch B: includehidden for TOC ------------------------------------------
    if INCLUDEHIDDEN_CALL_RE.search(text):
        print("[patch B] already applied (get_and_resolve_doctree call has includehidden=True)")
    else:
        new_text, n = re.subn(
            r"get_and_resolve_doctree\(\s*self\.config\.master_doc,\s*self,\s*prune_toctrees=False\s*\)",
            "get_and_resolve_doctree(self.config.master_doc, self, prune_toctrees=False, includehidden=True)",
            text,
            count=1,
            flags=re.S,
        )
        if n == 0:
            print("ERROR: patch B pattern (build_toc_file's get_and_resolve_doctree call) not found.")
            print("The vendored sphinxcontrib-htmlhelp differs from the expected layout;")
            print("inspect sphinxcontrib/htmlhelp/__init__.py::build_toc_file manually.")
            return 1
        text = new_text
        print("[patch B] applied: hidden toctrees are included in the CHM Contents pane")

    target.write_text(text, encoding="utf-8")
    print(f"OK: patched {target}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
