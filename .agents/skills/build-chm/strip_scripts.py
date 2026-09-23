#!/usr/bin/env python3
"""Strip scripts from built htmlhelp topics so the CHM viewer stops throwing
"syntax error" popups.

The CHM viewer renders topics with the legacy MSHTML (IE7 compatibility mode).
Modern JS therefore fails at parse time: Sphinx 7's doctools.js/sidebar.js and
this project's scheme-switcher.js / ini-block-maker.js all use ES6 syntax, and
the RTD theme's inline jQuery init also fails at runtime.  A help file needs no
JavaScript, so every <script> is removed except the ES5-safe html5shiv, which
is copied next to this script and injected into <head> so that <section> and
other HTML5 elements render as blocks.

Usage:
    python strip_scripts.py <build-dir> [--shiv <path-to-html5shiv.min.js>]
"""
from __future__ import annotations

import argparse
import glob
import os
import re
import shutil
import sys

SHIV_REL = "_static/js/html5shiv.min.js"
DEFAULT_SHIV = os.path.join(os.path.dirname(os.path.abspath(__file__)), "html5shiv.min.js")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("build_dir", help="e.g. docs/_build/htmlhelp_zh_CN")
    ap.add_argument("--shiv", default=DEFAULT_SHIV, help="html5shiv.min.js to bundle (default: the one next to this script)")
    args = ap.parse_args()

    build = os.path.abspath(args.build_dir)
    if not os.path.isdir(build):
        print(f"ERROR: not a directory: {build}")
        return 1
    if not os.path.isfile(args.shiv):
        print(f"ERROR: html5shiv not found: {args.shiv}")
        return 1

    os.makedirs(os.path.join(build, "_static", "js"), exist_ok=True)
    shutil.copy(args.shiv, os.path.join(build, "_static", "js", "html5shiv.min.js"))

    files = sorted(glob.glob(os.path.join(build, "*.html")))
    if not files:
        print(f"ERROR: no .html topics in {build}")
        return 1

    rewritten = injected = 0
    for path in files:
        with open(path, encoding="utf-8") as f:
            original = t = f.read()
        # external <script src=...></script>
        t = re.sub(r'<script\b[^>]*\bsrc="[^"]*"[^>]*>\s*</script>\s*', "", t, flags=re.I)
        # inline scripts with a body
        t = re.sub(r"<script\b(?![^>]*\bsrc=)[^>]*>.*?</script>\s*", "", t, flags=re.I | re.S)
        # self-closing leftovers
        t = re.sub(r"<script\b[^>]*/>\s*", "", t, flags=re.I)
        if SHIV_REL not in t:
            t = t.replace("</head>", f'    <script src="{SHIV_REL}"></script>\n</head>', 1)
            injected += 1
        if t != original:
            with open(path, "w", encoding="utf-8", newline="") as f:
                f.write(t)
            rewritten += 1

    # verification pass: nothing but html5shiv may remain
    bad = []
    for path in files:
        with open(path, encoding="utf-8") as f:
            t = f.read()
        for m in re.finditer(r'<script[^>]*src="([^"]+)"', t):
            if "html5shiv" not in m.group(1):
                bad.append((os.path.basename(path), m.group(1)))
        if re.search(r"<script(?![^>]*src)[^>]*>\s*\S", t):
            bad.append((os.path.basename(path), "inline script"))
    print(f"processed {len(files)} topics, rewritten {rewritten}, html5shiv injected into {injected}")
    print("remaining problem scripts:", bad if bad else "none")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
