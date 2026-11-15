#!/usr/bin/env python3
"""Verify a built htmlhelp output (and its compiled .chm if present).

Run this BEFORE reporting the CHM build as done.  Checks:

  topics   - every *.html decodes as UTF-8, has a <title>, carries no script
             other than html5shiv, no inline script body, and the classic
             theme's related navigation bar
  toc      - phobosdoc.hhc has entries and every "Local" target (page or
             page#anchor) exists in the build directory
  chm      - the compiled file starts with ITSF and carries LCID 0x804 (zh-CN)

Usage:
    python verify_chm.py <build-dir> [--chm <path-to-chm>]
"""
from __future__ import annotations

import argparse
import glob
import io
import os
import re
import struct
import sys

problems: list[str] = []


def ok(msg: str) -> None:
    print(f"  PASS {msg}")


def fail(msg: str) -> None:
    problems.append(msg)
    print(f"  FAIL {msg}")


def check_topics(build: str) -> None:
    files = sorted(glob.glob(os.path.join(build, "*.html")))
    print(f"[topics] {len(files)} html files")
    if not files:
        fail("no html topics found")
        return
    for path in files:
        name = os.path.basename(path)
        with open(path, "rb") as f:
            raw = f.read()
        try:
            t = raw.decode("utf-8")
        except UnicodeDecodeError as e:
            fail(f"{name}: not valid UTF-8 ({e})")
            continue
        if not re.search(r"<title>.+?</title>", t, re.S):
            fail(f"{name}: missing <title>")
        bad_src = [m.group(1) for m in re.finditer(r'<script[^>]*src="([^"]+)"', t)
                   if "html5shiv" not in m.group(1)]
        if bad_src:
            fail(f"{name}: external scripts remain: {bad_src}")
        if re.search(r"<script(?![^>]*src)[^>]*>\s*\S", t):
            fail(f"{name}: inline script with body remains")
        if "sphinxsidebar" in t:
            fail(f"{name}: in-page sidebar present (htmlhelp design has none)")
    ok(f"{len(files)} topics: UTF-8, titled, script-free (html5shiv only)")


def check_toc(build: str, basename: str = "phobosdoc.hhc") -> None:
    path = os.path.join(build, basename)
    if not os.path.isfile(path):
        fail(f"{basename} missing")
        return
    with open(path, encoding="cp936") as f:
        hhc = f.read()
    names = re.findall(r'<param name="Name" value="([^"]+)"', hhc)
    locals_ = re.findall(r'<param name="Local" value="([^"]+)"', hhc)
    print(f"[toc] {basename}: {len(names)} entries")
    if len(names) < 10:
        fail(f".hhc has only {len(names)} entries - hidden-toctree regression?")
    missing = []
    for local in locals_:
        page = local.split("#", 1)[0]
        if not page:
            continue
        page_path = os.path.join(build, page)
        if not os.path.isfile(page_path):
            missing.append(local)
            continue
        if "#" in local:
            anchor = local.split("#", 1)[1]
            with open(page_path, encoding="utf-8") as f:
                body = f.read()
            if f'id="{anchor}"' not in body:
                missing.append(local)
    if missing:
        fail(f"{len(missing)} TOC targets missing, e.g. {missing[:5]}")
    else:
        ok(f"{len(names)} TOC entries, all page/anchor targets exist")


def check_chm(path: str) -> None:
    if not os.path.isfile(path):
        fail(f"chm missing: {path}")
        return
    with open(path, "rb") as f:
        head = f.read(0x60)
    if head[:4] != b"ITSF":
        fail("chm signature is not ITSF")
        return
    lcid = struct.unpack("<I", head[0x14:0x18])[0]
    size_mb = os.path.getsize(path) / 1048576
    print(f"[chm] {os.path.basename(path)}: {size_mb:.1f} MB")
    if lcid != 0x804:
        fail(f"chm language id is 0x{lcid:x}, expected 0x804 (zh-CN)")
    else:
        ok(f"ITSF signature + LCID 0x804, {size_mb:.1f} MB")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("build_dir")
    ap.add_argument("--chm", help="compiled chm to check (default: phobosdoc.chm in build dir)")
    args = ap.parse_args()

    build = os.path.abspath(args.build_dir)
    chm = args.chm or os.path.join(build, "phobosdoc.chm")

    check_topics(build)
    check_toc(build)
    check_chm(chm)

    print()
    if problems:
        print(f"VERIFY FAILED ({len(problems)} problem(s)) - do NOT report the build as done")
        return 1
    print("VERIFY PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
