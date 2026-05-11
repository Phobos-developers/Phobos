---
name: 检查钩子 Check Hooks
mode: plan
description: "Checks newly added Syringe hooks (DEFINE_HOOK / DEFINE_HOOK_AGAIN) on the current branch for common errors: insufficient size (< 5 bytes), conflicts with hooks from other engine extensions, instruction boundary misalignment, relative instruction coverage (jumps/calls with relative offsets and RIP-relative addressing), and register/stack variable extraction issues (GET_STACK vs GET_BASE, stack alignment). Uses HookAnalysis.log for conflict detection and IDA MCP for deep instruction analysis."
---

#### Helper Scripts

YOU MUST use these scripts. DO NOT reimplement parsing logic.

| Script | Purpose |
|--------|---------|
| `discover_hooks.py` | Discovers new/modified DEFINE_HOOK / DEFINE_HOOK_AGAIN from git. Two modes: auto-detect (no args) or `--commit <sha/name>`. Supports fuzzy commit name resolution (searches last 30 commits). Outputs JSON with `hooks` array, each having `address`, `size`, `name`, `file`, `returns`. Use `--json-only` for piping. |
| `check_hook_conflicts.py` | Reads a JSON array of new hooks from stdin (or a file argument) and checks them against `HookAnalysis.log` for Problem 0 (size < 5) and Problem 1 (conflicts). Outputs JSON with `errors` and `notes` arrays. |
| `parse_hook_log.py` | Parses `HookAnalysis.log` (GBK encoding) and outputs all existing hooks as JSON. Typically not called directly — used by `check_hook_conflicts.py`. |
| `HookAnalysis.log` | Pre-generated hook analysis report from SyringeIH. Read-only reference. |

#### Before You Begin

YOU MUST create a TodoList with these items before starting any work:

1. Step 0: Discover hooks and build matrix
2. Step 1: Fill P0 & P1 (conflict script)
3. Step 2: Check IDA MCP availability
4. Step 3: Fill P2 (instruction boundaries)
5. Step 4: Fill P3 (variable extraction)
6. Step 5: Fill P4 (relative instructions)
7. Step 6: Output final matrix and details

Mark the first as `in_progress`. Update the TodoList as each step is completed.

ALL-STEPS RULE: YOU MUST output ALL findings without omission. DO NOT cherry-pick or show only the most severe findings. Every error, warning, and note discovered must appear in the output. If you suppress any finding, you are violating this skill.

YOU ARE IN PLAN MODE. Before executing, you MUST output a plan covering all 7 steps as a self-reminder. DO NOT skip planning.

#### The Matrix

This skill is driven by a **Problem Matrix**. The matrix has one row per hook and one column per problem type. Each step fills specific columns. At the end, the complete matrix is the summary.

Cell symbols:
- `✓` — passed (no issues)
- `❌` — error
- `⚠️` — warning
- `ℹ️` — note only (informational, no errors/warnings)
- `—` — skipped (IDA MCP unavailable)
- `?` — not yet checked (should never appear in the final output)

For Problem 3 (which has sub-checks 3a–3d), use the worst severity across all sub-checks as the cell value.

#### Severity classification

All findings are classified into three severity levels. YOU MUST follow this when reporting findings throughout all steps.

| Level | Emoji | Label | Criteria | Examples |
|-------|-------|-------|----------|----------|
| Error | ❌ | ERROR | Causes incorrect behavior at runtime — MUST be fixed | Size < 5, address conflict, instruction misalignment, wrong macro (GET_STACK vs GET_BASE), relative instruction with return 0 |
| Warning | ⚠️ | WARNING | May indicate incorrect behavior — should be reviewed | Type mismatch in GET_STACK, stacked hook (exact overlap with another hook) |
| Note | ℹ️ | NOTE | Informational, not necessarily a bug | All-clear confirmation |

---

## Step 0: Discover hooks and build matrix

### RULES

- YOU MUST use `discover_hooks.py`. DO NOT read source files to find hooks manually.
- YOU MUST NOT guess which files were modified from commit titles or commit message keywords.
- YOU MUST examine the JSON output of `discover_hooks.py` carefully.
  - If the output has a `hooks` array: the script succeeded. Proceed.
  - If the output has `"action": "resolve"` with a `candidates` array: the script IS working correctly. YOU MUST examine each candidate's `message` field, find the commit that matches the user's description, then re-run with `python discover_hooks.py --commit <sha>`.
  - If the output has `"action": "error"`: read the error message and act accordingly.
- If the script says the commit is not found, YOU MUST run `git fetch upstream` first, then retry. Only ask the user for the correct SHA after fetching fails.
- If no hooks are found, YOU MUST present the warning to the user and STOP. DO NOT proceed to Step 1.
- If `returns` is `"?"`, YOU MUST read the hook function body from the source file to determine the actual return behavior before proceeding to Step 1.

### Two modes

**Mode A — Specify a commit** (user provides a SHA or commit name):
```
python discover_hooks.py --commit <sha>
```

**Mode B — Auto-detect** (user does not specify a commit):
```
python discover_hooks.py
```

### OUTPUT REQUIREMENT

After completing Step 0, YOU MUST build the empty matrix and output it:

```
[Step 0 Complete]
- Mode used: <A/B>
- Hooks discovered: <count> — <names>

Matrix (empty, awaiting checks):
| Hook | Address | P0 (size) | P1 (conflict) | P2 (boundary) | P3 (variable) | P4 (relative) |
|------|---------|-----------|---------------|---------------|---------------|---------------|
| A    | 0x...   | ?         | ?             | ?             | ?             | ?             |
| B    | 0x...   | ?         | ?             | ?             | ?             | ?             |
```

Then mark the TodoList item as complete and move to Step 1. From this point on, the matrix is your progress tracker — refer to it before and after each step.

---

## Step 1: Fill P0 & P1 — Size and conflict checks (scripted)

### RULES

- YOU MUST pipe the discovered hooks JSON directly to `check_hook_conflicts.py`:
  ```
  python discover_hooks.py --json-only | python check_hook_conflicts.py
  ```
  Or with a commit:
  ```
  python discover_hooks.py --commit <sha> --json-only | python check_hook_conflicts.py
  ```
- If `returns` was `"?"` in Step 0 and you determined the actual value: fix it in the JSON before piping, or save the corrected JSON to a temp file and pass it as a file argument.
- YOU MUST examine the JSON output of `check_hook_conflicts.py`. The `errors` array contains issues that need fixing. The `notes` array contains informational items.
- DO NOT silently ignore any error. For each error, present it to the user using the exact format below.

### Error display formats

For each error in the `errors` array, YOU MUST output the corresponding formatted message:

**Problem 0** (type: `"size"`):
> ❌ **Problem 0: Insufficient hook size**
> Hook `HookName` at `0x<addr>` has size `0x<size>` (< 5). The JMP instruction requires at least 5 bytes. Increase the size to cover the full instruction(s) at this address.

**Problem 1 — Partial overlap** (type: `"conflict"`):
> ❌ **Problem 1: Hook address range conflict**
> Hook `NewHookName` at `0x<addr>` (size `0x<size>`, range `[0x<start>, 0x<end>)`) conflicts with existing hook `ExistingHookName` from `<DLL>` at `0x<existing_addr>` (size `0x<existing_size>`, range `[0x<existing_start>, 0x<existing_end>)`).

**Problem 1 — Return address conflict** (type: `"return_conflict"`):
> ❌ **Problem 1: Return address conflict**
> Hook `NewHookName` at `0x<addr>` returns to `0x<ret_addr>`, which falls within existing hook `ExistingHookName` from `<DLL>` covering `[0x<start>, 0x<end>)`.

For each note in the `notes` array:

**Stacked hook** (type: `"stacked"`):
> ⚠️ **Problem 1: Stacked hook — verify intent**
> Hook `NewHookName` at `0x<addr>` (size `0x<size>`) exactly matches existing hook `ExistingHookName` from `<DLL>`. The second hook will execute after the first returns 0. Verify this is intended.

**OK confirmation** (type: `"ok"`):
> ✓ No conflicts for hook `HookName`.

### OUTPUT REQUIREMENT

After completing Step 1, YOU MUST output the summary AND the updated matrix with P0 and P1 columns filled:

```
[Step 1 Complete]
- Hooks checked: <count>
- Problem 0 errors: <count>
- Problem 1 conflicts (overlap): <count>
- Problem 1 conflicts (return): <count>
- Stacked hooks (warning): <count>

Updated matrix (P0, P1 filled):
| Hook | Address | P0 (size) | P1 (conflict) | P2 (boundary) | P3 (variable) | P4 (relative) |
|------|---------|-----------|---------------|---------------|---------------|---------------|
| A    | 0x...   | ✓         | ❌            | ?             | ?             | ?             |
| B    | 0x...   | ❌        | ✓             | ?             | ?             | ?             |
```

Then mark the TodoList item as complete and move to Step 2.

---

## Step 2: Check IDA MCP availability

### RULES

- YOU MUST attempt to connect to IDA MCP by calling `mcp_ida-pro-mcp_server_health`.
- If the health check succeeds AND the loaded IDB is `gamemd.exe`:
  - P2, P3, P4 columns will be filled in Steps 3-5.
- If the health check fails OR `gamemd.exe` is not loaded:
  - YOU MUST mark P2, P3, P4 columns as `—` (skipped) for ALL hooks in the matrix.
  - YOU MUST output the reason.
  - Steps 3-5 will still execute but with no work to do.
  - DO NOT silently skip this step.

### OUTPUT REQUIREMENT

After completing Step 2, YOU MUST output:

```
[Step 2 Complete]
- IDA MCP health check: <pass/fail>
- gamemd.exe loaded: <yes/no>

Updated matrix (P2-P4 status determined):
| Hook | Address | P0 (size) | P1 (conflict) | P2 (boundary) | P3 (variable) | P4 (relative) |
|------|---------|-----------|---------------|---------------|---------------|---------------|
| A    | 0x...   | ✓         | ❌            | ? / —         | ? / —         | ? / —         |
```

If IDA MCP is available, the P2-P4 cells remain `?` (to be filled in Steps 3-5). If unavailable, they are `—`.

Then mark the TodoList item as complete and move to Step 3.

---

## Step 3: Fill P2 — Instruction boundary check

### RULES

- If IDA MCP was unavailable in Step 2, YOU MUST skip the checks below (P2 column is already `—`).
- YOU MUST fill the P2 column for every hook. DO NOT skip any hook.

### Check

For each new hook:
1. Use IDA MCP (`mcp_ida-pro-mcp_disasm`) to verify the hook address is at the start of an x86 instruction.
2. Use IDA MCP to verify that `addr + size` is also at an instruction boundary (the hook covers complete instructions).
3. For fixed return addresses, verify they are at instruction boundaries.

If any check fails:
> ❌ **Problem 2: Instruction boundary issue**
> Hook `HookName` at `0x<addr>` (size `0x<size>`) — <specific issue>. Disassemble the area at this address to find the correct boundaries.

### OUTPUT REQUIREMENT

After completing Step 3, YOU MUST output the per-hook detail blocks AND the updated matrix with P2 column filled:

```
[Step 3 Complete]
- Hooks checked: <count>
- Problem 2 errors: <count>

Details for hooks with P2 findings:

### TEST1 (0x6EC3D0, size 0x5, range [0x6EC3D0, 0x6EC3D5))
6EC3D0  push    ebx             ; 1 byte
6EC3D1  push    ebp             ; 1 byte
6EC3D2  push    esi             ; 1 byte
6EC3D3  mov     esi, [ecx+54h]  ; 3 bytes → covers to 0x6EC3D6

❌ Problem 2: Instruction boundary issue — Hook end address 0x6EC3D5 falls in the middle of `mov esi, [ecx+54h]` (3 bytes, [0x6EC3D3, 0x6EC3D6)). Only the first 2 bytes are copied to the trampoline; the 3rd byte is missing. Increase size to 6.

Updated matrix (P2 filled):
| Hook | Address | P0 | P1 | P2 | P3 | P4 |
|------|---------|----|----|----|----|----|
| A    | 0x...   | ✓  | ❌ | ❌ | ?  | ?  |
| B    | 0x...   | ❌ | ✓  | ✓  | ?  | ?  |
```

DO NOT omit any hook with a finding. If a hook has no P2 issues, skip its detail block.

Then mark the TodoList item as complete and move to Step 4.

---

## Step 4: Fill P3 — Variable extraction and stack access validation

### RULES

- If IDA MCP was unavailable in Step 2, YOU MUST skip the checks below (P3 column is already `—`).
- YOU MUST fill the P3 column for every hook. DO NOT skip any hook.
- P3 has sub-checks 3a–3d. The matrix cell shows the worst severity across all sub-checks.

### Validation

For each new hook, inspect the function body for `GET`, `GET_STACK`, `REF_STACK`, `LEA_STACK`, `GET_BASE` macros and register writes (`R->EAX(value)`, `R->ECX(value)`, `R->STACK(offset, value)`, etc.). Use IDA MCP to decompile or disassemble the code around the hook address and verify the register/stack state matches.

#### 4a — Register extraction (GET)

For `GET(type, var, reg)`:
- Check what `reg` holds at the hook point according to IDA
- If the type declared in GET differs from what IDA suggests, warn the user

#### 4b — Stack variable extraction (GET_STACK / REF_STACK / LEA_STACK)

For `GET_STACK(type, var, offset)` / `REF_STACK(type, var, offset)`:
- DO NOT rely on IDA's offset labels alone — they may reference a virtual frame pointer that is not ESP. `R->Stack` always reads from `captured_ESP`, so you MUST compute the actual ESP at the hook point.
- Verify EBP is a real frame pointer: disassemble the first 5-10 instructions of the function. If EBP is overwritten (e.g. `mov ebp, [esp+...]`) instead of set to `mov ebp, esp`, then EBP is NOT a frame pointer — do NOT use EBP-relative offsets from IDA as ESP offsets.
- Trace ESP from function entry to the hook point. Manually compute the cumulative offset:
  1. Start from `ESP_entry` — the ESP value immediately after the `call` that entered the function
  2. Account for every `push` (subtract 4 per push), `pop` (add 4 per pop), and `sub esp, X` / `add esp, X` between function entry and the hook address
  3. The resulting ESP at the hook point = `ESP_entry + cumulative_offset`
  4. The offset passed to `R->Stack` (after resolving `STACK_OFFSET` macros) is then added to this value
- Resolve `STACK_OFFSET` macros explicitly. `STACK_OFFSET(a, b)` is simply `(a + b)`.
- Map the final address back to the function's parameter list.

If a type mismatch is found:
> ⚠️ **Problem 3b: Variable extraction may be incorrect**
> At `0x<addr>`: `GET_STACK(<type>, <var>, <offset>)` — resolved offset `<computed_offset>` maps to function entry `+<entry_offset>`, expected parameter `<param_name>` of type `<expected_type>` at that position. The declared type `<declared_type>` does not match.

#### 4c — Macro selection error: GET_STACK vs GET_BASE after stack alignment

Check for stack alignment (`and esp, alignment_mask`) in the function prologue. Disassemble the first ~20 instructions of the function. Look for `and esp, <mask>` where `<mask>` is an alignment boundary (e.g. `0FFFFFFF8h` for 8-byte, `0FFFFFFF0h` for 16-byte, `0FFFFFFFCh` for 4-byte, or their equivalents `-8`, `-16`, `-4`). After this instruction, ESP is aligned down to the mask boundary and is no longer at a known offset from `ESP_entry` — `GET_STACK` cannot reliably access function parameters.

If a hook uses `GET_STACK` to access what appears to be a function parameter (offset maps to `ESP_entry + positive_offset`) and the function prologue contains stack alignment:

> ❌ **Problem 3c: GET_STACK used after stack alignment — should use GET_BASE**
> Hook `HookName` at `0x<addr>` uses `GET_STACK` to access what appears to be a function parameter (resolved to entry offset `+<entry_offset>`). The function prologue at `0x<prologue_addr>` contains `and esp, <alignment_mask>` which realigns ESP, making `GET_STACK` unreliable for parameter access. Replace with `GET_BASE(type, name, <entry_offset - 4>)`.

#### 4d — Register writes (R->EAX, R->ECX, etc.)

For register writes like `R->EAX(value)`:
- Disassemble after the hook point to verify the register will be read as expected by the original code
- If the return address is a fixed address (not `R->Origin()`), verify the original code at that address uses the register being set

If all Problem 3 checks pass: "✓ Variable extraction and stack access checks passed."

### OUTPUT REQUIREMENT

After completing Step 4, YOU MUST output the per-hook detail blocks AND the updated matrix with P3 column filled:

```
[Step 4 Complete]
- Hooks checked: <count>
- Problem 3a errors (register extraction): <count>
- Problem 3b errors (stack variable extraction): <count>
- Problem 3c errors (GET_STACK vs GET_BASE): <count>
- Problem 3d errors (register writes): <count>

Details for hooks with P3 findings:

### TEST4 (0x6EB3B8)
GET_STACK(double, number, STACK_OFFSET(0x18, 0x8))

Function: TeamClass::AttackedBy(TeamClass *this, int a2, int a3, FootClass *pAttacker)

⚠️ Problem 3b: Stack variable type may not match — STACK_OFFSET(0x18, 0x8) = 0x20. After `sub esp, 0Ch` + 3 pushes (16 bytes), ESP_entry + 0x20 - 0x10 = 0x10 → maps to parameter a3 (int). Declared type is double. Verify.

Updated matrix (P3 filled):
| Hook | Address | P0 | P1 | P2 | P3 | P4 |
|------|---------|----|----|----|----|----|
| A    | 0x...   | ✓  | ❌ | ❌ | ✓  | ?  |
| B    | 0x...   | ❌ | ✓  | ✓  | ⚠️ | ?  |
```

DO NOT omit any hook with a finding. The matrix cell for P3 shows the worst severity across all sub-checks.

Then mark the TodoList item as complete and move to Step 5.

---

## Step 5: Fill P4 — Relative instruction coverage check

### RULES

- If IDA MCP was unavailable in Step 2, YOU MUST skip the checks below (P4 column is already `—`).
- YOU MUST fill the P4 column for every hook. DO NOT skip any hook.

### Check

For each new hook, disassemble the full address range `[addr, addr + size)` using IDA MCP and check for any relative-offset instructions. These instructions encode their target as `current_address + instruction_length + relative_offset`. When Syringe copies these bytes to a trampoline at a different address, the relative offset points to the wrong location.

Hooks that cover any of the following instructions MUST return a fixed value (not 0):

Relative jump/call instructions:
- `jmp short`, `jmp near`, `jz`, `jnz`, `je`, `jne`, `jg`, `jge`, `jl`, `jle`
- `ja`, `jae`, `jb`, `jbe`, `jo`, `jno`, `js`, `jns`, `jp`, `jnp`, `jpe`, `jpo`
- `jcxz`, `jecxz`, `jrcxz`
- `call` (relative call, i.e. `call rel32`)
- `loop`, `loope`, `loopne`, `loopz`, `loopnz`
- `xbegin`

RIP-relative addressing instructions:
- `mov`, `lea`, `cmp`, `add`, `sub`, `and`, `or`, `xor`, `test`
- `push`, `pop`, `movsxd`, `movzx`, `movsx`
- When any of the above uses RIP-relative addressing mode

Check the `returns` field from Step 0. If the hook returns `"0"` but covers any of these instructions:
> ❌ **Problem 4: Hook covers relative-offset instruction but returns 0**
> Hook `HookName` at `0x<addr>` covers instruction `<mnemonic>` at `0x<instruction_addr>` which uses relative addressing (encoded relative offset `<encoded_value>` → target `<computed_target>`). This hook MUST return a fixed value (e.g. `return 0x<fixed_addr>` or `return R->Origin() + <offset>`), not `return 0`.

If no relative-offset instructions are found, or the hook already returns a fixed value: "✓ No relative instruction issues found."

### OUTPUT REQUIREMENT

After completing Step 5, YOU MUST output the per-hook detail blocks AND the updated matrix with P4 column filled:

```
[Step 5 Complete]
- Hooks checked: <count>
- Problem 4 errors (relative instruction): <count>

Details for hooks with P4 findings:

### TEST7 (0x4D56B1, size 0x6, range [0x4D56B1, 0x4D56B7))
4D56B1  jz      loc_4D5A42      ; 6 bytes (0F 84 8B 03 00 00) — near conditional jump, 32-bit relative offset

❌ Problem 4: Hook covers relative-offset instruction but returns 0 — The hooked `jz` encodes a 32-bit relative offset (0x38B → target = 0x4D56B7 + 0x38B = 0x4D5A42). When copied to a trampoline, the offset points to the wrong address. MUST return a fixed address (e.g. `return R->Origin() + 6`), not `return 0`.

Updated matrix (P4 filled — matrix now complete):
| Hook | Address | P0 | P1 | P2 | P3 | P4 |
|------|---------|----|----|----|----|----|
| A    | 0x...   | ✓  | ❌ | ❌ | ✓  | ✓  |
| B    | 0x...   | ❌ | ✓  | ✓  | ⚠️ | ❌ |
```

DO NOT omit any hook with a finding.

Then mark the TodoList item as complete and move to Step 6.

---

## Step 6: Output complete matrix and details

### RULES

- YOU MUST output the final complete matrix (no `?` cells should remain).
- YOU MUST include ALL hooks discovered in Step 0.
- YOU MUST include ALL findings in the Issues by hook section. DO NOT omit any hook that has non-✓ findings. DO NOT show only the most severe issues — every error, warning, and note must appear.
- DO NOT create a "✓ No-problem hooks" or "✓ 无问题钩子" section. If a hook passed everything, simply omit it from the Issues section.
- Hooks in the table MUST be in the same order as discovered in Step 0.

### OUTPUT REQUIREMENT

YOU MUST output the final summary:

```
========================================
  Check Hooks Summary
========================================
Total hooks checked: <N>

| Hook | Address | P0 | P1 | P2 | P3 | P4 |
|------|---------|----|----|----|----|----|
| A    | 0x...   | ✓  | ❌ | ❌ | ✓  | ✓  |
| B    | 0x...   | ❌ | ✓  | ✓  | ⚠️ | ❌ |

### Issues by hook

### HookA (0x...)
- **P1 — Conflict with ExistingHook:** overlaps [0x..., 0x...)
- **P2 — Instruction boundary:** <details>

### HookB (0x...)
- **P0 — Size < 5:** size is 0x3, needs at least 5 bytes
- **P3b — Variable type mismatch:** declared <type>, IDA suggests <other_type>
- **P4 — Relative instruction:** covers `call rel32` at 0x... but returns 0
```

If no problems were found at all:
```
========================================
  Check Hooks Summary
========================================
Total hooks checked: <N>

| Hook | Address | P0 | P1 | P2 | P3 | P4 |
|------|---------|----|----|----|----|----|
| ...  | ...     | ✓  | ✓  | ✓  | ✓  | ✓  |

✅ All checks passed. No issues found with the new hooks.
========================================
```

Then mark the TodoList item as complete.