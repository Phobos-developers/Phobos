#!/usr/bin/env python3
"""Parse ares_3.0p1_hooks.cpp and output all hooks as JSON.

This produces the same dict structure as parse_hook_log.parse_hook_log(),
so it can be used interchangeably by check_hook_conflicts.py.

Usage:
  python parse_hooks_cpp.py [filepath]
  (defaults to ares_3.0p1_hooks.cpp in the same directory)
"""
import re
import json
import sys
import os


HOOK_RE = re.compile(
    r'DEFINE_HOOK(?:AGAIN)?\s*\(\s*'
    r'(0x[0-9A-Fa-f]+)\s*,\s*'
    r'(\w+)\s*,\s*'
    r'(0x[0-9A-Fa-f]+|\d+)\s*'
    r'\)'
)


def parse_hooks_cpp(filepath):
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        lines = f.readlines()

    hooks = []
    seen = set()

    for line in lines:
        stripped = line.strip()
        if not stripped or stripped.startswith('//'):
            continue

        comment_pos = stripped.find('//')
        code_part = stripped[:comment_pos] if comment_pos >= 0 else stripped

        m = HOOK_RE.search(code_part)
        if m:
            addr_str = m.group(1)
            name = m.group(2)
            size_raw = m.group(3)

            addr = int(addr_str, 16)
            size = int(size_raw, 16) if size_raw.startswith(('0x', '0X')) else int(size_raw)

            key = (addr, name)
            if key in seen:
                hook = _make_hook(addr, name, 'Ares 3.0p1', size, 0)
                hooks.append(hook)
                continue

            seen.add(key)
            hook = _make_hook(addr, name, 'Ares 3.0p1', size, 0)
            hooks.append(hook)

    return hooks


def _make_hook(addr, name, dll, size, priority):
    return {
        'address': f'0x{addr:08X}',
        'address_int': addr,
        'name': name,
        'dll': dll,
        'size': size,
        'priority': priority,
        'range_start': addr,
        'range_end': addr + size,
    }


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    filepath = os.path.join(script_dir, 'ares_3.0p1_hooks.cpp')
    if len(sys.argv) > 1:
        filepath = sys.argv[1]

    hooks = parse_hooks_cpp(filepath)

    output = []
    for h in hooks:
        output.append({
            'address': h['address'],
            'name': h['name'],
            'dll': h['dll'],
            'size': h['size'],
            'range': f"[{h['address']}, 0x{h['range_end']:08X})",
        })
    json.dump(output, sys.stdout, indent=2, ensure_ascii=False)


if __name__ == '__main__':
    main()