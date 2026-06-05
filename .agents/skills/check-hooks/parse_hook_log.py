#!/usr/bin/env python3
"""Parse HookAnalysis.txt and output JSON with all hooks."""
import re
import json
import sys
import os

def parse_hook_log(filepath):
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        lines = f.readlines()

    hooks = []
    current_addr = None

    # Format A: separate address line, hook has 来自"<dll>"
    addr_re = re.compile(r'在\s+([0-9A-Fa-f]+)\s*[：:]')
    hook_re_a = re.compile(
        r'钩子"(.+?)，相对于""，来自"(.+?)"，长度(\d+)，优先级\s+(\d+)，次优先级\s+""'
    )
    # Format B: no separate address line, hook has 位于<addr>
    hook_re_b = re.compile(
        r'钩子"(.+?)，相对于""，位于([0-9A-Fa-f]+)，长度(\d+)，优先级\s+(\d+)，次优先级\s+""'
    )

    for line in lines:
        line = line.strip()
        if not line:
            continue

        # Try address line (Format A style)
        m = addr_re.match(line)
        if m:
            current_addr = int(m.group(1), 16)
            continue

        # Try Format A hook: 来自"<dll>"
        m = hook_re_a.match(line)
        if m and current_addr is not None:
            name = m.group(1)
            dll = m.group(2)
            size = int(m.group(3))
            priority = int(m.group(4))
            hooks.append(_make_hook(current_addr, name, dll, size, priority))
            continue

        # Try Format B hook: 位于<addr> (no preceding address line needed)
        m = hook_re_b.match(line)
        if m:
            addr = int(m.group(2), 16)
            name = m.group(1)
            dll = '(unknown)'
            size = int(m.group(3))
            priority = int(m.group(4))
            hooks.append(_make_hook(addr, name, dll, size, priority))

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
    filepath = os.path.join(script_dir, 'HookAnalysis.txt')
    hooks = parse_hook_log(filepath)

    # Output as JSON
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
