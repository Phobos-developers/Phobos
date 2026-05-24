#!/usr/bin/env python3
"""Check new hooks against existing hooks in HookAnalysis.txt for conflicts.

Usage:
  python check_hook_conflicts.py <new_hooks_json>

new_hooks_json is a JSON file (or '-' for stdin) containing an array of new hook objects:
  [{"address": "0x46BDD9", "size": 5, "name": "MyHook", "returns": "0x46BDE0"}]

"returns" can be:
  - "0" or missing/empty → returns 0 (resolves to hook address, safe, no return-check needed)
  - "0x..." → fixed return address to check
  - "R->Origin() + N" → relative return (hook address + N) to check

Output is JSON with conflict results.
"""

import json
import os
import sys
import re

# Ensure the script's own directory is in sys.path so that
# 'import parse_hook_log' works regardless of the working directory.
_script_dir = os.path.dirname(os.path.abspath(__file__))
if _script_dir not in sys.path:
    sys.path.insert(0, _script_dir)

def parse_existing_hooks(script_dir):
    """Parse existing hooks from HookAnalysis.txt or ares_3.0p1_hooks.cpp.

    Tries HookAnalysis.txt first (faster, richer metadata).
    Falls back to ares_3.0p1_hooks.cpp if HookAnalysis.txt does not exist.
    """
    log_path = os.path.join(script_dir, 'HookAnalysis.txt')
    if os.path.exists(log_path):
        import parse_hook_log
        return parse_hook_log.parse_hook_log(log_path)

    cpp_path = os.path.join(script_dir, 'ares_3.0p1_hooks.cpp')
    if os.path.exists(cpp_path):
        import parse_hooks_cpp
        return parse_hooks_cpp.parse_hooks_cpp(cpp_path)

    print(json.dumps({
        'errors': [],
        'notes': [{
            'problem': 'Setup',
            'type': 'error',
            'message': (
                "Neither HookAnalysis.txt nor ares_3.0p1_hooks.cpp found "
                f"in {script_dir}. Cannot perform conflict check."
            )
        }]
    }, indent=2, ensure_ascii=False))
    sys.exit(1)


def check_hooks(new_hooks, existing_hooks):
    results = []
    errors = []
    notes = []

    for nh in new_hooks:
        addr = nh['address']
        addr_int = int(addr, 16)
        size = nh['size']
        name = nh.get('name', '<unknown>')
        ret = nh.get('returns', '0')  # returns "0", "0x...", or "R->Origin() + N"
        range_start = addr_int
        range_end = addr_int + size

        # Resolve return address
        ret_addr = None
        if ret == '0' or not ret:
            ret_addr = None  # means safe, no return-check needed
        elif re.match(r'^0x[0-9A-Fa-f]+$', ret):
            ret_addr = int(ret, 16)
        else:
            m = re.match(r'R->Origin\(\)\s*\+\s*(\d+)', ret)
            if m:
                ret_addr = addr_int + int(m.group(1))

        # Check Problem 0: size >= 5
        if size < 5:
            errors.append({
                'problem': 'Problem 0',
                'hook': name,
                'address': addr,
                'message': f"Hook '{name}' at {addr} has size {size} (< 5). The JMP instruction requires at least 5 bytes."
            })

        # Check Problem 1: conflicts
        found_conflict = False
        for eh in existing_hooks:
            e_range_start = eh['range_start']
            e_range_end = eh['range_end']
            e_addr = eh['address']

            # Check address range overlap
            if range_start < e_range_end and range_end > e_range_start:
                if range_start == e_range_start and range_end == e_range_end:
                    # Exact overlap - stacked hooks, not an error
                    notes.append({
                        'problem': 'Problem 1',
                        'hook': name,
                        'address': addr,
                        'size': size,
                        'existing_hook': eh['name'],
                        'existing_dll': eh['dll'],
                        'existing_address': e_addr,
                        'existing_size': eh['size'],
                        'type': 'stacked',
                        'message': (
                            f"Hook '{name}' at {addr} (size {size}) exactly matches "
                            f"existing hook '{eh['name']}' from {eh['dll']}. "
                            f"This is a stacked hook — the second will execute after the first returns 0. "
                            f"Verify this is intended."
                        )
                    })
                else:
                    # Partial overlap - conflict
                    errors.append({
                        'problem': 'Problem 1',
                        'hook': name,
                        'address': addr,
                        'size': size,
                        'range': f"[0x{range_start:08X}, 0x{range_end:08X})",
                        'existing_hook': eh['name'],
                        'existing_dll': eh['dll'],
                        'existing_address': e_addr,
                        'existing_size': eh['size'],
                        'existing_range': f"[0x{e_range_start:08X}, 0x{e_range_end:08X})",
                        'type': 'conflict',
                        'message': (
                            f"Hook '{name}' at {addr} (size {size}, range "
                            f"[0x{range_start:08X}, 0x{range_end:08X})) conflicts with "
                            f"existing hook '{eh['name']}' from {eh['dll']} at "
                            f"{e_addr} (size {eh['size']}, range "
                            f"[0x{e_range_start:08X}, 0x{e_range_end:08X})). "
                            f"The address ranges overlap."
                        )
                    })
                found_conflict = True

            # Check return address
            if ret_addr is not None and ret_addr != addr_int:
                if e_range_start <= ret_addr < e_range_end:
                    errors.append({
                        'problem': 'Problem 1',
                        'hook': name,
                        'address': addr,
                        'returns': ret,
                        'return_addr': f"0x{ret_addr:08X}",
                        'existing_hook': eh['name'],
                        'existing_dll': eh['dll'],
                        'existing_range': f"[0x{e_range_start:08X}, 0x{e_range_end:08X})",
                        'type': 'return_conflict',
                        'message': (
                            f"Hook '{name}' at {addr} returns to 0x{ret_addr:08X}, "
                            f"which falls within existing hook '{eh['name']}' from {eh['dll']} "
                            f"covering [0x{e_range_start:08X}, 0x{e_range_end:08X})."
                        )
                    })

        if not found_conflict:
            notes.append({
                'problem': 'Problem 1',
                'hook': name,
                'address': addr,
                'type': 'ok',
                'message': f"No conflicts detected for hook '{name}' at {addr}."
            })

    return {'errors': errors, 'notes': notes}


def main():
    if len(sys.argv) < 2:
        new_hooks = json.load(sys.stdin)
    else:
        new_hooks_input = sys.argv[1]
        if new_hooks_input == '-':
            new_hooks = json.load(sys.stdin)
        else:
            with open(new_hooks_input, 'r', encoding='utf-8') as f:
                new_hooks = json.load(f)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    existing_hooks = parse_existing_hooks(script_dir)

    results = check_hooks(new_hooks, existing_hooks)
    json.dump(results, sys.stdout, indent=2, ensure_ascii=False)


if __name__ == '__main__':
    main()
