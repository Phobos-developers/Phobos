#!/usr/bin/env python3
"""Discover new or modified DEFINE_HOOK / DEFINE_HOOK_AGAIN for checking.

Modes:
  python discover_hooks.py --commit <sha>      Check hooks in a specific commit
  python discover_hooks.py                      Auto-detect what to check

Auto-detect priority:
  1. Uncommitted changes (working tree vs HEAD) + unpushed commits
     on the current branch (vs upstream)
  2. If nothing above, check from the branch point off develop,
     i.e. commits in HEAD that are not in develop.

Outputs JSON array to stdout with hooks and their source locations.
"""

import subprocess
import sys
import os
import re
import json


# 首先查找并切换到 git 仓库根目录
def find_git_root():
    """Find the git repository root by searching upwards from current directory."""
    current_dir = os.path.abspath(os.getcwd())
    while True:
        if os.path.isdir(os.path.join(current_dir, '.git')):
            return current_dir
        parent = os.path.dirname(current_dir)
        if parent == current_dir:  # 到达根目录仍未找到
            return None
        current_dir = parent

# 切换到 git 根目录
git_root = find_git_root()
if git_root:
    os.chdir(git_root)


NO_PAGER_ENV = None


def _no_pager_env():
    global NO_PAGER_ENV
    if NO_PAGER_ENV is None:
        NO_PAGER_ENV = os.environ.copy()
        NO_PAGER_ENV['GIT_PAGER'] = 'cat'
        NO_PAGER_ENV['PAGER'] = 'cat'
    return NO_PAGER_ENV


def run_git(args):
    result = subprocess.run(
        ['git'] + args,
        capture_output=True,
        text=True,
        encoding='utf-8',
        errors='replace',
        env=_no_pager_env()
    )
    return result.stdout.strip()


def has_uncommitted():
    out = run_git(['diff', 'HEAD', '--name-only'])
    if out:
        return True
    # Also check untracked source files — git diff HEAD doesn't show new files
    untracked = run_git(['ls-files', '--others', '--exclude-standard', '--', '*.cpp', '*.h'])
    return len(untracked) > 0


def get_upstream():
    """Return upstream branch name, or empty string if not set."""
    out = run_git(['rev-parse', '--abbrev-ref', '@{u}'])
    # If no upstream, git prints an error to stderr and empty stdout
    if out and 'fatal' not in out:
        return out
    return ''


def has_unpushed():
    upstream = get_upstream()
    if not upstream:
        return False
    out = run_git(['log', f'{upstream}..HEAD', '--oneline'])
    return len(out) > 0


def get_unpushed_commits():
    upstream = get_upstream()
    if not upstream:
        return None
    out = run_git(['log', f'{upstream}..HEAD', '--format=%H'])
    if out:
        return out.splitlines()
    return None


def get_develop_diff_commits():
    """Get commits on HEAD that are not in develop.
    Tries 'develop' first, then 'origin/develop' as fallback."""
    for candidate in ('develop', 'origin/develop'):
        out = run_git(['log', f'{candidate}..HEAD', '--format=%H'])
        if out:
            return out.splitlines(), candidate
    return None, None


def get_diff_from_range(commit_range):
    """Run git diff and return the text."""
    args = ['diff', commit_range, '--', '*.cpp', '*.h']
    return run_git(args)


def _include_untracked_content(diff_text):
    """Append untracked source file content as diff additions.
    This allows new files (not yet tracked by git) to be parsed for hooks."""
    untracked = run_git(['ls-files', '--others', '--exclude-standard', '--', '*.cpp', '*.h'])
    if not untracked:
        return diff_text
    result = [diff_text] if diff_text else []
    for file_path in untracked.splitlines():
        try:
            with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
                content = f.read()
            result.append(f'+++ b/{file_path}')
            for line in content.split('\n'):
                result.append(f'+{line}')
        except OSError:
            pass
    return '\n'.join(result)


def get_show_commit(commit):
    """Run git show for a single commit, restricted to source files.
    Returns (diff_text, error) where error is None on success."""
    result = subprocess.run(
        ['git', 'show', commit, '--', '*.cpp', '*.h'],
        capture_output=True,
        text=True,
        encoding='utf-8',
        errors='replace',
        env=_no_pager_env()
    )
    if result.returncode != 0:
        return '', result.stderr.strip()
    return result.stdout, None


def get_recent_commits(count=30):
    """Return the last N commits as a list of {sha, message}."""
    log_lines = run_git(['log', f'-{count}', '--format=%H %s']).splitlines()
    result = []
    for line in log_lines:
        line = line.strip()
        if not line:
            continue
        parts = line.split(' ', 1)
        if len(parts) >= 2:
            result.append({'sha': parts[0], 'message': parts[1]})
        elif parts:
            result.append({'sha': parts[0], 'message': ''})
    return result


def resolve_commit(candidate):
    """Try to resolve a commit reference.
    Returns (sha, message, needs_selection, candidates).
    If git can resolve directly, returns that commit.
    Otherwise returns the last 30 commits for AI to pick from and present to user.
    """
    # Try direct resolution first (git handles partial SHAs natively)
    result = subprocess.run(
        ['git', 'rev-parse', '--verify', f'{candidate}^{{commit}}'],
        capture_output=True, text=True, encoding='utf-8', errors='replace',
        env=_no_pager_env()
    )
    if result.returncode == 0:
        sha = result.stdout.strip()
        msg = run_git(['log', '-1', '--format=%s', sha])
        return sha, msg, False, []

    # Can't resolve — return recent commits for AI to pick from
    recent = get_recent_commits()
    if recent:
        return None, '', True, recent
    return None, '', False, []


HOOK_RE = re.compile(
    r'DEFINE_HOOK(?:AGAIN)?\s*\(\s*'
    r'(0x[0-9A-Fa-f]+)\s*,\s*'
    r'(\w+)\s*,\s*'
    r'(0x[0-9A-Fa-f]+|\d+)'
)

RETURN_0_RE = re.compile(r'\breturn\s+0\s*;')
RETURN_HEX_RE = re.compile(r'\breturn\s+(0x[0-9A-Fa-f]+)\s*;')
RETURN_ORIGIN_RE = re.compile(r'\breturn\s+R->Origin\(\)\s*\+\s*(0x[0-9A-Fa-f]+|\d+)\s*;')
ENUM_VAL_RE = re.compile(r'(Continue|Skip)\s*=\s*(0x[0-9A-Fa-f]+)')
ENUM_RET_RE = re.compile(r'\breturn\s+(Continue|Skip)\s*;')


def parse_hooks_from_diff(diff_text):
    """Parse DEFINE_HOOK/DEFINE_HOOK_AGAIN from unified diff."""
    hooks = []
    current_file = ''
    lines = diff_text.split('\n')

    for i, line in enumerate(lines):
        # Track file
        file_match = re.match(r'^\+\+\+\s+b/(.*)', line)
        if file_match:
            current_file = file_match.group(1)
            continue

        # Only process added lines
        if not line.startswith('+'):
            continue
        # Skip the +++ line itself
        if line.startswith('+++'):
            continue

        stripped = line[1:]  # Remove the '+' prefix

        m = HOOK_RE.search(stripped)
        if m:
            address = m.group(1)
            name = m.group(2)
            size_raw = m.group(3)
            # size can be hex or decimal
            if size_raw.startswith('0x') or size_raw.startswith('0X'):
                size = int(size_raw, 16)
            else:
                size = int(size_raw)

            hooks.append({
                'address': address,
                'name': name,
                'size': size,
                'file': current_file,
                'diff_line': i,
            })

    return hooks


def analyze_return_behavior(hooks, diff_text):
    """For each hook, try to determine return behavior from the diff context."""
    lines = diff_text.split('\n')
    # Map diff lines to hook entries
    hook_by_diff_line = {h['diff_line']: h for h in hooks}

    for i, line in enumerate(lines):
        if i not in hook_by_diff_line:
            continue
        h = hook_by_diff_line[i]

        # Look forward in the diff for return statements (within ~150 lines)
        # Track brace depth to handle nested blocks (if/else/for etc.)
        brace_depth = 0
        for j in range(i + 1, min(i + 150, len(lines))):
            future = lines[j]
            if not future.startswith('+') or future.startswith('+++'):
                continue
            content = future[1:]

            # Count braces to track depth into the hook body
            open_count = content.count('{')
            close_count = content.count('}')
            brace_depth += open_count - close_count
            if brace_depth <= 0:
                break  # Exited the hook body

            # Check for enum definitions
            enum_m = ENUM_VAL_RE.search(content)
            if enum_m:
                label = enum_m.group(1)
                val = enum_m.group(2)
                h.setdefault('enum_map', {})[label] = val

            # Check return statements
            ret0 = RETURN_0_RE.search(content)
            if ret0:
                h['returns'] = '0'
                break

            ret_hex = RETURN_HEX_RE.search(content)
            if ret_hex:
                ret_addr = ret_hex.group(1)
                # If we have enum mapping, resolve it
                enum_map = h.get('enum_map', {})
                for label, val in enum_map.items():
                    if ret_addr == label:
                        ret_addr = val
                        break
                h['returns'] = ret_addr
                break

            ret_origin = RETURN_ORIGIN_RE.search(content)
            if ret_origin:
                offset = ret_origin.group(1)
                if offset.startswith('0x') or offset.startswith('0X'):
                    offset_int = int(offset, 16)
                else:
                    offset_int = int(offset)
                h['returns'] = f'R->Origin() + {offset_int}'
                break

            # Check enum-based return
            enum_ret = ENUM_RET_RE.search(content)
            if enum_ret:
                label = enum_ret.group(1)
                enum_map = h.get('enum_map', {})
                if label in enum_map:
                    h['returns'] = enum_map[label]
                else:
                    h['returns'] = f'enum:{label}'
                break

        if 'returns' not in h:
            h['returns'] = '?'  # Could not determine from diff alone


def get_fork_point():
    """Find the fork point of the current branch using various strategies.
    Returns (base_ref, description) or (None, message) on failure."""
    # Strategy 1: fork-point from origin/develop
    for candidate in ('origin/develop', 'develop'):
        out = run_git(['merge-base', '--fork-point', candidate, 'HEAD'])
        if out:
            try:
                int(out, 16)
                return out, candidate
            except ValueError:
                pass
    # Strategy 2: plain merge-base
    for candidate in ('origin/develop', 'develop'):
        out = run_git(['merge-base', candidate, 'HEAD'])
        if out:
            try:
                int(out, 16)
                return out, candidate
            except ValueError:
                pass
    return None, 'no fork point found'


def determine_diff_range():
    """Determine the git diff range for auto-detect mode.

    Returns (diff_args, description) where diff_args is list for git diff
    and description is a human-readable explanation.
    """
    # Priority 1: uncommitted changes (including untracked source files)
    if has_uncommitted():
        # Check if there are also unpushed commits
        unpushed = get_unpushed_commits()
        if unpushed:
            upstream = get_upstream() or 'develop'
            return ([f'{upstream}...HEAD'], f'{upstream}...HEAD', f'unpushed (vs {upstream}) + uncommitted changes')
        else:
            return (['HEAD'], 'HEAD', 'uncommitted changes')

    # Priority 1b: unpushed only (no uncommitted)
    unpushed = get_unpushed_commits()
    if unpushed:
        upstream = get_upstream() or 'develop'
        return ([f'{upstream}...HEAD'], f'{upstream}...HEAD', f'unpushed commits (vs {upstream})')

    # Priority 2: nothing unpushed/uncommitted, check commits ahead of develop
    dev_commits, dev_ref = get_develop_diff_commits()
    if dev_commits:
        return ([f'{dev_ref}...HEAD'], f'{dev_ref}...HEAD', f'commits ahead of {dev_ref}')

    # Priority 3: try to find the fork point — useful when neither develop nor upstream exists
    fork_base, fork_desc = get_fork_point()
    if fork_base:
        return ([f'{fork_base}...HEAD'], f'{fork_base}...HEAD', f'fork point from {fork_desc}')

    return ([], '', 'no changes found')


def main():
    import argparse
    parser = argparse.ArgumentParser(description='Discover new hooks for checking.')
    parser.add_argument('--commit', '-c', help='Check hooks in a specific commit (SHA or ref)')
    parser.add_argument('--json-only', action='store_true', help='Output only the JSON array, no metadata')
    args = parser.parse_args()

    if args.commit:
        # First try direct git show
        diff_text, error = get_show_commit(args.commit)
        if error:
            # Failed — try to resolve
            sha, msg, ambiguous, candidates = resolve_commit(args.commit)
            if ambiguous:
                # Multiple matches — output for user selection
                result = {
                    'mode': 'commit',
                    'action': 'resolve',
                    'input': args.commit,
                    'candidates': candidates,
                    'message': f"Ambiguous commit '{args.commit}'. Please pick one:"
                }
                print(json.dumps(result, indent=2, ensure_ascii=False))
                return
            elif sha:
                diff_text, error = get_show_commit(sha)
                if error:
                    print(json.dumps({
                        'mode': 'commit',
                        'action': 'error',
                        'message': f"Cannot show commit {sha}: {error}"
                    }, indent=2, ensure_ascii=False))
                    return
                desc = f'commit {sha} ({msg})'
            else:
                print(json.dumps({
                    'mode': 'commit',
                    'action': 'error',
                    'message': f"Cannot resolve '{args.commit}'. No matching commit found in the last 30."
                }, indent=2, ensure_ascii=False))
                return
        else:
            desc = f'commit {args.commit}'
    else:
        diff_args, range_desc, desc = determine_diff_range()
        if not diff_args:
            result = {
                'mode': 'auto',
                'description': desc,
                'hooks': [],
                'warning': 'No changes to check. No new hooks found.'
            }
            print(json.dumps(result, indent=2, ensure_ascii=False))
            return
        diff_text = get_diff_from_range(' '.join(diff_args))
        diff_text = _include_untracked_content(diff_text)

    hooks = parse_hooks_from_diff(diff_text)
    analyze_return_behavior(hooks, diff_text)

    # Clean up internal fields before output
    output_hooks = []
    for h in hooks:
        out = {
            'address': h['address'],
            'name': h['name'],
            'size': h['size'],
            'file': h['file'],
            'returns': h.get('returns', '?'),
        }
        output_hooks.append(out)

    if args.json_only:
        print(json.dumps(output_hooks, indent=2, ensure_ascii=False))
    else:
        result = {
            'mode': 'commit' if args.commit else 'auto',
            'description': desc,
            'count': len(output_hooks),
            'hooks': output_hooks,
        }
        if not output_hooks:
            result['warning'] = 'No DEFINE_HOOK or DEFINE_HOOK_AGAIN found in the diff.'
        print(json.dumps(result, indent=2, ensure_ascii=False))


if __name__ == '__main__':
    main()
