---
name: 检查钩子 Check Hooks
description: "Checks newly added Syringe hooks (DEFINE_HOOK / DEFINE_HOOK_AGAIN) on the current branch for common errors: insufficient size (< 5 bytes), conflicts with hooks from other engine extensions, instruction boundary misalignment, and register/stack variable extraction issues. Uses HookAnalysis.log for conflict detection and IDA MCP for deep instruction analysis."
---

#### Helper Scripts

All scripts in this directory. The AI MUST use them — do not reimplement parsing logic.

| Script | Purpose |
|--------|---------|
| `discover_hooks.py` | Discovers new/modified DEFINE_HOOK / DEFINE_HOOK_AGAIN from git. Two modes: auto-detect (no args) or `--commit <sha/name>`. Supports fuzzy commit name resolution (searches last 30 commits). Outputs JSON with `hooks` array, each having `address`, `size`, `name`, `file`, `returns`. Use `--json-only` for piping. |
| `check_hook_conflicts.py` | Reads a JSON array of new hooks from stdin (or a file argument) and checks them against `HookAnalysis.log` for Problem 0 (size < 5) and Problem 1 (conflicts). Outputs JSON with `errors` and `notes` arrays. |
| `parse_hook_log.py` | Parses `HookAnalysis.log` (GBK encoding) and outputs all existing hooks as JSON. Typically not called directly — used by `check_hook_conflicts.py`. |
| `HookAnalysis.log` | Pre-generated hook analysis report from SyringeIH. Read-only reference. |

#### Workflow

This skill checks all newly added `DEFINE_HOOK` and `DEFINE_HOOK_AGAIN` macro invocations on the current branch for four classes of problems.

**Step 0: Discover new hooks**

Use `discover_hooks.py` to find which hooks need checking. Two modes:

**Mode A — Specify a commit** (user provides a SHA or commit name):
```
python discover_hooks.py --commit <sha>
```

**IMPORTANT — Terminal reliability:** The script captures all git output internally via `capture_output=True` and sets `GIT_PAGER=cat` as a safety measure. It CANNOT be affected by the terminal's pager/less configuration. If the script runs and produces output that is not what you expected, READ the output carefully — do NOT assume the terminal is broken. Valid outputs include:
  - `"action": "resolve"` + `candidates` array: the script IS working correctly, it just needs you to pick a SHA
  - `"action": "error"`: there is a specific error message explaining what went wrong
  - A `hooks` array with the discovered hooks — the script ran successfully

If the script appears to "fail" (you ran it but didn't get hooks), first check what it actually output, then check this document for what to do next.

The script first tries `git show <sha>` directly (git natively resolves partial SHAs). If that fails (e.g., the user provided a non-SHA name like "Country"), the script outputs the last 30 commits as a `candidates` list. The AI MUST:

1. Examine the `candidates` array in the JSON output. Find the commit(s) whose `message` field matches the user's description.
2. Pick the most relevant SHA and re-run with it directly: `python discover_hooks.py --commit <sha>`
3. If multiple candidates match closely, present them to the user and ask which one.
4. **Do NOT fall back to manually reading source files to find hooks.** The script is the only correct way to determine which hooks were added by a commit.

**CRITICAL — Never guess which files were modified based on the commit title:** If you cannot get the script to run successfully (e.g., the script says the commit is not found), do NOT try to infer the relevant source file from the commit title or commit message keywords. Instead:
  - First try the commit SHA directly: `python discover_hooks.py --commit <sha>`
  - If the commit is not in the local repo, run `git fetch upstream` first, then try again
  - Only if all git/npm approaches fail should you consult the user for the correct SHA

**Mode B — Auto-detect** (user does not specify a commit):
```
python discover_hooks.py
```

The script automatically determines the diff range with this priority:
1. Uncommitted changes (including new/untracked source files) + unpushed commits on current branch → checks both
2. Only unpushed commits → checks those
3. Neither unpushed nor uncommitted → falls back to `develop...HEAD` (tries `origin/develop` if `develop` doesn't exist locally)
4. If none of the above work → tries to find the branch fork point via `git merge-base`
5. If all attempts fail → reports no changes found

The output is a JSON object with a `hooks` array. Each hook has: `address`, `name`, `size`, `file`, `returns`. The `returns` field is auto-detected from the diff context: `"0"`, `"0x<hex>"`, `"R->Origin() + N"`, or `"?"` if undetermined.

If no hooks are found, the script reports a warning. Present this to the user and stop.

**Important:** If `returns` is `"?"`, the AI MUST read the hook function body from the source file to determine the actual return behavior before proceeding.

**Step 1: Problem 0 & 1 — Size and conflict checks (scripted)**

Pipe the discovered hooks directly to the conflict checker:
```
python discover_hooks.py --json-only | python check_hook_conflicts.py
```

Or with a commit:
```
python discover_hooks.py --commit <sha> --json-only | python check_hook_conflicts.py
```

If `returns` was `"?"`, first fix it in the JSON, or save the corrected JSON to a temp file and pass it as an argument instead of piping.

The script:
- Reports **Problem 0** errors for any hook with `size < 5`
- Reports **Problem 1** conflicts: partial address range overlaps and return address overlaps
- Notes exact overlaps (stacked hooks) as informational — not errors

Interpret the JSON output. The `errors` array contains issues that need fixing. The `notes` array contains informational items (stacked hooks, OK confirmations, etc.).

For each error, present it to the user clearly:

**Problem 0** (from script output):
> ❌ **Problem 0: Insufficient hook size**
> Hook `HookName` at `0x<addr>` has size `0x<size>` (< 5). The JMP instruction requires at least 5 bytes. Increase the size to cover the full instruction(s) at this address.

**Problem 1 — Stacked hook** (from `notes` with `type: "stacked"`):
> ℹ️ **Problem 1: Stacked hook (not an error, verify intent)**
> Hook `NewHookName` at `0x<addr>` (size `0x<size>`) exactly matches existing hook `ExistingHookName` from `<DLL>`. The second hook will execute after the first returns 0. Verify this is intended.

**Problem 1 — Partial overlap** (from `errors` with `type: "conflict"`):
> ❌ **Problem 1: Hook address range conflict**
> Hook `NewHookName` at `0x<addr>` (size `0x<size>`, range `[0x<start>, 0x<end>)`) conflicts with existing hook `ExistingHookName` from `<DLL>` at `0x<existing_addr>` (size `0x<existing_size>`, range `[0x<existing_start>, 0x<existing_end>)`).

**Problem 1 — Return address conflict** (from `errors` with `type: "return_conflict"`):
> ❌ **Problem 1: Return address conflict**
> Hook `NewHookName` at `0x<addr>` returns to `0x<ret_addr>`, which falls within existing hook `ExistingHookName` from `<DLL>` covering `[0x<start>, 0x<end>)`.

If no conflicts were found for a hook, the script outputs a note with `type: "ok"`.

**Step 2: Problem 2 & 3 — Instruction boundary and variable validation via IDA MCP**

Attempt to connect to the IDA MCP server. Check if `gamemd.exe` is the loaded IDB.

**If IDA MCP is not available or gamemd.exe is not loaded:**

> ⚠️ **IDA MCP server is not available.** Skipping Problem 2 (instruction boundary) and Problem 3 (variable extraction) checks. Connect the IDA MCP server with gamemd.exe loaded for full validation.

Skip Step 2 entirely.

**If IDA MCP is available:**

**Problem 2 — Instruction boundary check:**

For each new hook:
1. Use the IDA MCP to verify the hook address is at the start of an x86 instruction.
2. Use the IDA MCP to verify that `addr + size` is also at an instruction boundary (the hook covers complete instructions).
3. For fixed return addresses, verify they are at instruction boundaries.

If any check fails:
> ❌ **Problem 2: Instruction boundary issue**
> Hook `HookName` at `0x<addr>` (size `0x<size>`) — <specific issue, e.g. "address is in the middle of an instruction" or "size does not end at an instruction boundary" or "return address 0x<ret> is not at an instruction start">. Disassemble the area at this address to find the correct boundaries.

**Problem 3 — Variable extraction validation:**

For each new hook, inspect the function body for `GET`, `GET_STACK`, `REF_STACK`, `LEA_STACK` macros and register writes (`R->EAX(value)`, `R->ECX(value)`, `R->STACK(offset, value)`, etc.). Use IDA MCP to decompile or disassemble the code around the hook address and verify the register/stack state matches.

For `GET(type, var, reg)`:
- Check what `reg` holds at the hook point according to IDA
- If the type declared in GET differs from what IDA suggests, warn the user

For `GET_STACK(type, var, offset)` / `REF_STACK(type, var, offset)`:
- Check the stack layout at the hook point per IDA
- If the offset suggests a different type or value, warn the user

If a mismatch is found:
> ⚠️ **Problem 3: Variable extraction may be incorrect**
> At `0x<addr>`: `GET(<type>, <var>, <reg>)` — `<reg>` appears to hold `<actual_type>` based on IDA analysis. Verify the register assignment at this address.

If all Problem 3 checks pass: "✓ Variable extraction checks passed."

**Step 3: Summary**

After all checks, print a summary listing all checked hooks and any problems found, grouped by severity (❌ errors first, then ⚠️ warnings, then ℹ️ notes). If no problems were found at all: "✅ All checks passed. No issues found with the new hooks."
