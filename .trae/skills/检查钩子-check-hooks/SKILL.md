---
name: 检查钩子 Check Hooks
description: "Checks newly added Syringe hooks (DEFINE_HOOK / DEFINE_HOOK_AGAIN) on the current branch for common errors: insufficient size (< 5 bytes), conflicts with hooks from other engine extensions, instruction boundary misalignment, relative instruction coverage (jumps/calls with relative offsets and RIP-relative addressing), and register/stack variable extraction issues (GET_STACK vs GET_BASE, stack alignment). Uses HookAnalysis.log for conflict detection and IDA MCP for deep instruction analysis."
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

This skill checks all newly added `DEFINE_HOOK` and `DEFINE_HOOK_AGAIN` macro invocations on the current branch for the following problem classes.

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

**Problem 2b — Relative instruction coverage check:**

For each new hook, disassemble the full address range `[addr, addr + size)` and check for any relative-offset instructions. These instructions encode their target as `current_address + instruction_length + relative_offset`. When Syringe copies these bytes to a trampoline at a different address, the relative offset points to the wrong location, causing control flow to jump/call to an unintended address.

Hooks that cover any of the following instructions **MUST return a fixed value (not 0)** — returning 0 would execute the trampolined original code and trigger the bug:

Relative jump/call instructions:
- `jmp short`, `jmp near`, `jz`, `jnz`, `je`, `jne`, `jg`, `jge`, `jl`, `jle`
- `ja`, `jae`, `jb`, `jbe`, `jo`, `jno`, `js`, `jns`, `jp`, `jnp`, `jpe`, `jpo`
- `jcxz`, `jecxz`, `jrcxz`
- `call` (relative call, i.e. `call rel32`)
- `loop`, `loope`, `loopne`, `loopz`, `loopnz`
- `xbegin`

RIP-relative addressing instructions (x64 style, but also relevant for x86 with `[eip+disp32]`):
- `mov`, `lea`, `cmp`, `add`, `sub`, `and`, `or`, `xor`, `test`
- `push`, `pop`, `movsxd`, `movzx`, `movsx`
- When any of the above uses RIP-relative addressing mode

Use IDA MCP to disassemble each instruction in the hook range and check for these patterns. Check the `returns` field from Step 0. If the hook returns `"0"` but covers any of these instructions, report:

> ❌ **Problem 2b: Hook covers relative-offset instruction but returns 0**
> Hook `HookName` at `0x<addr>` covers instruction `<mnemonic>` at `0x<instruction_addr>` which uses relative addressing (encoded relative offset `<encoded_value>` → target `<computed_target>`). When this instruction is copied to a trampoline, the relative offset will point to the wrong address. This hook MUST return a fixed value (e.g. `return 0x<fixed_addr>` or `return R->Origin() + <offset>`), not `return 0`.

If no relative-offset instructions are found, or the hook already returns a fixed value: "✓ No relative instruction issues found."

**Problem 3 — Variable extraction validation:**

For each new hook, inspect the function body for `GET`, `GET_STACK`, `REF_STACK`, `LEA_STACK` macros and register writes (`R->EAX(value)`, `R->ECX(value)`, `R->STACK(offset, value)`, etc.). Use IDA MCP to decompile or disassemble the code around the hook address and verify the register/stack state matches.

For `GET(type, var, reg)`:
- Check what `reg` holds at the hook point according to IDA
- If the type declared in GET differs from what IDA suggests, warn the user

For `GET_STACK(type, var, offset)` / `REF_STACK(type, var, offset)`:
- **Do NOT rely on IDA's offset labels alone** — they may reference a virtual frame pointer that is not ESP. `R->Stack` always reads from `captured_ESP`, so you MUST compute the actual ESP at the hook point.
- **Verify EBP is a real frame pointer:** Disassemble the first 5-10 instructions of the function. If EBP is overwritten (e.g. `mov ebp, [esp+...]`) instead of set to `mov ebp, esp`, then EBP is NOT a frame pointer — do NOT use EBP-relative offsets from IDA as ESP offsets.
- **Trace ESP from function entry to the hook point.** Manually compute the cumulative offset:

  1. Start from `ESP_entry` — the ESP value immediately after the `call` that entered the function (i.e. the stack pointer at function entry, with the return address already pushed by `call`)
  2. Account for every `push` (subtract 4 per push), `pop` (add 4 per pop), and `sub esp, X` / `add esp, X` between function entry and the hook address
  3. The resulting ESP at the hook point = `ESP_entry + cumulative_offset` (where cumulative_offset is negative for pushes/subs, positive for pops/adds)
  4. The offset passed to `R->Stack` (after resolving `STACK_OFFSET` macros) is then added to this value

- **Resolve `STACK_OFFSET` macros explicitly.** `STACK_OFFSET(a, b)` is simply `(a + b)` — it is pure addition. Do NOT assign special semantics to the parameter names `cur_offset` or `wanted_offset`. Compute the numeric result before using it.
- **Map the final address back to the function's parameter list.** Once you have `captured_ESP + resolved_offset`, subtract `ESP_entry` to get the offset from the function entry point. This tells you which parameter slot or local variable the macro is accessing. Cross-reference with the function's signature (parameters start at `ESP_entry + 0x04` for the first param after the return address).

- **Check for stack alignment (`and esp, alignment_mask`) in the function prologue.** Disassemble the first ~20 instructions of the function. Look for `and esp, <mask>` where `<mask>` is an alignment boundary (e.g. `0FFFFFFF8h` for 8-byte, `0FFFFFFF0h` for 16-byte, `0FFFFFFFCh` for 4-byte, or their equivalents `-8`, `-16`, `-4`). After this instruction, ESP is aligned down to the mask boundary and is no longer at a known offset from `ESP_entry` — `GET_STACK` cannot reliably access function parameters. In this case, the hook MUST use `GET_BASE(type, name, offset)` instead of `GET_STACK(type, name, STACK_OFFSET(...))` for any parameter access. If a hook uses `GET_STACK` to access a parameter (offset maps to `ESP_entry + positive_offset`) and the function prologue contains stack alignment, report:

> ❌ **Problem 3: GET_STACK used after stack alignment — should use GET_BASE**
> Hook `HookName` at `0x<addr>` uses `GET_STACK` to access what appears to be a function parameter (resolved to entry offset `+<entry_offset>`). The function prologue at `0x<prologue_addr>` contains `and esp, <alignment_mask>` which realigns ESP, making `GET_STACK` unreliable for parameter access. Replace with `GET_BASE(type, name, <entry_offset - 4>)` (the offset from EBP after a standard `push ebp; mov ebp, esp` frame, adjusting for the alignment loss).

If a mismatch is found:
> ⚠️ **Problem 3: Variable extraction may be incorrect**
> At `0x<addr>`: `GET_STACK(<type>, <var>, <offset>)` — resolved offset `<computed_offset>` maps to function entry `+<entry_offset>`, expected parameter `<param_name>` of type `<expected_type>` at that position. The declared type `<declared_type>` does not match.

For register writes like `R->EAX(value)`:
- Disassemble after the hook point to verify the register will be read as expected by the original code
- If the return address is a fixed address (not `R->Origin()`), verify the original code at that address uses the register being set

If all Problem 3 checks pass: "✓ Variable extraction checks passed."

**Step 3: Summary**

After all checks, print a summary listing all checked hooks and any problems found, grouped by severity (❌ errors first, then ⚠️ warnings, then ℹ️ notes). If no problems were found at all: "✅ All checks passed. No issues found with the new hooks."
