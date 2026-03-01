# Copilot Instructions for Phobos

> **Note:** These instructions were authored and tested with **GitHub Copilot in VS Code** (agent mode). Most Phobos contributors use **Visual Studio 2022** as their primary IDE. The build commands, project structure, and code patterns described here apply equally to both environments, but tool-specific details (e.g., Copilot chat integration, terminal behavior) may differ. If you are adapting these instructions for Visual Studio, the main differences are: the VS solution explorer replaces the file tree, the integrated terminal is the Developer Command Prompt rather than PowerShell, and Copilot's inline chat UX varies. Contributions to improve VS-specific guidance are welcome.

## Project Summary

Phobos is a community C++ engine extension for Command & Conquer: Yuri's Revenge. It injects code into the game via [Syringe](https://github.com/Ares-Developers/Syringe) hooks and is designed to complement [Ares](https://github.com/Ares-Developers/Ares). The build output is a 32-bit Windows DLL (`Phobos.dll`). There are no unit tests - correctness is validated by successful compilation and manual in-game testing.

- **Language:** C++20 (`/std:c++20`), Win32/x86 only
- **Build system:** MSBuild via Visual Studio 2022 (MSVC v143 toolset)
- **Solution file:** `Phobos.sln` / `Phobos.vcxproj`
- **Submodule:** `YRpp/` - header-only library describing game binary types (cloned recursively)

## Building

Always ensure the YRpp submodule is initialized before building:
```
git submodule update --init --recursive
```

### Build commands (from repo root)

| Config | Command | Output |
|--------|---------|--------|
| Debug (recommended for dev) | `scripts\build_debug.bat` | `Debug\Phobos.dll` + `.pdb` |
| DevBuild (CI nightly) | `scripts\build_devbuild.bat` | `DevBuild\Phobos.dll` + `.pdb` |
| Release | `scripts\build_release.bat` | `Release\Phobos.dll` + `.pdb` |

These scripts invoke `scripts\run_msbuild.bat`, which locates the VS Developer Command Prompt via `vswhere.exe` (bundled in `scripts/`), then runs `msbuild`. **VS 2022 or VS Build Tools 2022** with the components listed in `.vsconfig` must be installed:
- `Microsoft.VisualStudio.Component.VC.Tools.x86.x64`
- `Microsoft.VisualStudio.Component.Windows10SDK.20348`
- `Microsoft.VisualStudio.Component.VC.ATL`

**In VS Code**, prefer the pre-configured build tasks over running scripts directly unless there are issues. The workspace defines a **"Build Phobos"** task (default build task) that prompts for Debug/DevBuild/Release. Run it via `Ctrl+Shift+B` or the `Tasks: Run Build Task` command. A **"Cleanup build folders"** task is also available.

**In Visual Studio 2022**, most contributors build directly from the IDE using `Build > Build Solution` (`Ctrl+Shift+B`) with the solution configuration dropdown (Debug/DevBuild/Release). The batch scripts are not needed when building from VS.

The build takes roughly 1–3 minutes for a full rebuild. Incremental builds are much faster. To clean:
```
scripts\clean.bat
```

### CI build (GitHub Actions)

The CI workflow (`.github/actions/build-phobos/action.yml`) builds the **DevBuild** config with MSBuild, passing `/p:GitCommit=<sha> /p:GitBranch=<ref>` for version stamping. The agent should replicate the CI as:
```
msbuild /m /p:Configuration=DevBuild Phobos.sln
```

## Tests & Validation

There is **no automated test suite**. Validation is:
1. **Successful compilation** with zero errors (warning level 4, but not treated as errors).
2. **PR CI checks** - the `Pull Request Nightly Build` workflow must pass (builds DevBuild config).
3. **PR doc checker** (`.github/workflows/pr-doc-checker.yml`) - unless the PR has the `No Documentation Needed` label, these files must be modified:
   - `docs/Whats-New.md` (changelog entry)
   - `CREDITS.md` (credit entry; skipped if the `Bugfix` label is set)

Always verify your changes compile by running `scripts\build_debug.bat` before committing.

## Project Layout

```
src/                    All Phobos source code
├── Phobos.cpp/h        Extension bootstrap, command-line parsing, global state
├── Phobos.Ext.cpp      TypeRegistry (MassActions) - register new/extended classes here
├── Phobos.version.h    Version numbers and build metadata macros
├── Phobos.Save.cpp     Save/load game hooks
├── Phobos.INI.cpp      INI inheritance/inclusion logic
├── Commands/           Hotkey command classes (inherit PhobosCommandClass)
├── Ext/                Vanilla class extensions (one subfolder per game class)
│   └── <ClassName>/
│       ├── Body.h/cpp  ExtData class, ExtContainer, ExtMap, serialization, INI hooks
│       └── Hooks*.cpp  Syringe hooks for patching game logic
├── New/
│   ├── Type/           New enumerable type classes (inherit Enumerable<T>)
│   └── Entity/         New in-game entity classes
├── Misc/               Uncategorized hooks and helpers
├── Utilities/          Shared infrastructure (Container.h, Macro.h, Enumerable.h, etc.)
├── Locomotion/         Custom locomotor implementations
└── Blowfish/           Blowfish encryption replacement
YRpp/                   Game binary header definitions (git submodule)
lib/                    Third-party headers (e.g. nameof)
docs/                   Sphinx documentation (Markdown/MyST)
scripts/                Build and setup scripts
.editorconfig           Code style enforcement (tabs, Allman braces, CRLF)
.vsconfig               Required VS components
```

## Key Patterns for Code Changes

### Extending a vanilla game class

Each extension lives in `src/Ext/<ClassName>/` with:
- **`Body.h`**: Declares `<Name>Ext` with `ExtData` (inherits `Extension<T>`) and `ExtContainer`/`ExtMap`.
- **`Body.cpp`**: Implements constructor, `LoadFromINIFile`, serialization (`Serialize`), and common hooks.
- **`Hooks.cpp` / `Hooks.*.cpp`**: `DEFINE_HOOK(address, Name, size)` macros for Syringe code injection.

After creating a new extension class, **always register it** in `src/Phobos.Ext.cpp` inside the `PhobosTypeRegistry` alias (the `using PhobosTypeRegistry = TypeRegistry<...>` declaration).

### Adding a new enumerable type

Create the class in `src/New/Type/`, inheriting `Enumerable<T>` from `src/Utilities/Enumerable.h`. Register it in `PhobosTypeRegistry` in `src/Phobos.Ext.cpp`.

### Adding a new command

Create a class in `src/Commands/` inheriting `PhobosCommandClass`, then register it in `src/Commands/Commands.cpp`.

### Adding new source files

New `.cpp` files must be added to `Phobos.vcxproj` inside the appropriate `<ClCompile>` ItemGroup (and `.h` files in `<ClInclude>`). The project file uses backslash paths relative to repo root.

### Hooks (Syringe code injection)

Hooks are the central mechanism in Phobos. Syringe writes a 5-byte `JMP` into the game binary at a specified address, redirecting execution into a C-exported function in Phobos.dll. The Syringe-specific macros (`DEFINE_HOOK`, `DEFINE_HOOK_AGAIN`) and the `REGISTERS` class are defined in `YRpp/Syringe.h` and `YRpp/Helpers/Macro.h`. The remaining patching macros (`DEFINE_JUMP`, `DEFINE_PATCH`, `DEFINE_FUNCTION_JUMP`, `DEFINE_NAKED_HOOK`, etc.) are **Phobos-specific** and defined in `src/Utilities/Macro.h`.

#### `DEFINE_HOOK(address, HookName, size)`

Declares a Syringe hook **and** opens the hook function body. The generated signature is:

```cpp
extern "C" __declspec(dllexport) DWORD __cdecl HookName(REGISTERS* R)
```

- **`address`** - The hex address in `gamemd.exe` where Syringe inserts its 5-byte `JMP`.
- **`HookName`** - Exported symbol name. Convention: `ClassName_Method_Purpose`.
- **`size`** - Number of **stolen bytes** to preserve. When the hook returns 0, Syringe executes these saved bytes before resuming at `address + size`. This is **not** the size of the jump itself (always 5 bytes); it represents how many bytes of original instructions the hook "covers". If your hook never returns 0 (always jumps elsewhere), size can be 0. Size can be less than 5 (if trailing bytes are NOPs) or greater than 5 (to cover multi-byte instructions that straddle the 5-byte boundary).

#### Hook return values

The return value of a hook function is a `DWORD` that controls where execution resumes:

| Return value | Behavior |
|---|---|
| `return 0;` | Execute the stolen bytes, then continue at `address + size`. Use this to run your code *in addition to* the original logic. |
| `return 0x<addr>;` | Jump directly to the given address, **skipping** the stolen bytes. Use this to *replace* or *shortcut* original logic. |
| `return R->Origin() + <offset>;` | Jump to a relative offset from the hook's own address. Useful in `DEFINE_HOOK_AGAIN` where the same handler serves multiple addresses and you need per-address offsets. |

Return addresses are typically stored in an anonymous enum at the top of the hook function:

```cpp
DEFINE_HOOK(0x6FD0B0, TechnoClass_Update_CustomLogic, 0x6)
{
    enum { Continue = 0x6FD0B6, Skip = 0x6FD120 };

    GET(TechnoClass*, pThis, ESI);
    // ... custom logic ...
    return Continue;
}
```

#### `DEFINE_HOOK_AGAIN(address, HookName, size)`

Adds an **additional hook entry point** for the same handler function. Only emits the `hookdecl` metadata (stored in the `.syhks00` PE section) - it does **not** open a function body. Place `DEFINE_HOOK_AGAIN` lines **before** the `DEFINE_HOOK` that opens the body:

```cpp
DEFINE_HOOK_AGAIN(0x43C30A, TechnoClass_Draw_Something, 0xC)
DEFINE_HOOK(0x43BF8B, TechnoClass_Draw_Something, 0x5)
{
    // R->Origin() tells you which address triggered this invocation.
    // Use R->Origin() + offset to return correctly per call site.
    return R->Origin() + 0xF;
}
```

#### Accessing registers and the stack (`REGISTERS* R`)

The `REGISTERS` class (`YRpp/Syringe.h`) provides typed access to all x86 registers at the hook point:

| Macro | Definition | Example |
|---|---|---|
| `GET(type, var, reg)` | `type var = R->reg<type>()` | `GET(TechnoClass*, pThis, ESI)` |
| `GET_STACK(type, var, offset)` | `type var = R->Stack<type>(offset)` | `GET_STACK(int, damage, 0x4)` |
| `LEA_STACK(type, var, offset)` | `type var = R->lea_Stack<type>(offset)` | `LEA_STACK(CoordStruct*, pCoord, 0xC)` - pointer to stack location |
| `REF_STACK(type, var, offset)` | `type var = R->ref_Stack<type>(offset)` | `REF_STACK(CoordStruct, coord, 0xC)` - reference to stack location |
| `GET_BASE(type, var, offset)` | `type var = R->Base<type>(offset)` | `GET_BASE(int, arg0, 0x4)` - reads from EBP-relative (caller frame) |

Stack offsets in `GET_STACK` are relative to ESP at hook entry. Use `STACK_OFFSET(cur_offset, wanted_offset)` for arithmetic when the stack layout is documented at a different point.

You can also write registers back: `R->EAX(value)`, `R->ECX(value)`, etc.

**Note on EBP:** The compiler used to build `gamemd.exe` has a bug that causes EBP to almost never be used as a frame pointer. As a result, `GET_BASE` is rarely useful; prefer `GET_STACK` with ESP-relative offsets instead. Do not assume EBP-based stack frames in the game binary.

#### Choosing where to place a hook

Hook addresses must be determined by **disassembling `gamemd.exe`** (e.g., in IDA Pro or Ghidra). Using the [IDA Pro MCP server](https://github.com/mrexodia/ida-pro-mcp) is **highly recommended** - it lets Copilot query the disassembly directly (decompile functions, look up addresses, read bytes, find cross-references, etc.). Install the companion [IDAPython skill](https://skills.sh/mrexodia/ida-pro-mcp/idapython) for writing IDA scripts. If no tools for decompiling or disassembling are available in the current session (search for tools matching `decompile|disasm|ida`), suggest the user install the [IDA Pro MCP server](https://github.com/mrexodia/ida-pro-mcp) and the [IDAPython skill](https://skills.sh/mrexodia/ida-pro-mcp/idapython), as they are essential for effective hook development in this project. Key rules:

1. **Instruction boundary** - The address must be the start of an x86 instruction. Hooking mid-instruction corrupts the code.
2. **Size alignment** - `size` must cover complete instructions. Look at the disassembly to see which instructions span the 5-byte overwrite region and set size to the end of the last overlapping instruction.
3. **Register/stack state** - Choose a point where the registers/stack contain the data you need. Determine which registers hold which values by reading the disassembly - do not assume a fixed calling convention or register assignment without verifying it at the specific address.
4. **No EIP-relative stolen bytes** - Syringe does **not** fix up EIP-relative operands in stolen bytes. If you use `return 0` and the stolen instructions contain EIP-relative addressing (e.g., `CALL rel32`, `JMP rel32`, `Jcc rel8/rel32`, `LEA reg, [EIP+disp]`, maybe others), they will execute with a wrong target because they run from Syringe's buffer, not from their original location. Either avoid hooking at such instructions when you need `return 0`, or always return a nonzero address to skip the stolen bytes entirely.
5. **Stack pointer is read-only** - Syringe currently does not support stack depth/pointer modifications inside hooks. Do not use `push`/`pop` or inline assembly that changes real ESP - it will corrupt the stack. Changes via `R->ESP()` are also ignored for the time being (Syringe version with fixes for that to be released in future). The hook must return at exactly the same stack depth it was entered with.
6. **Control flow** - Hook at or before a branch if you need to influence a conditional. Hook after if you only need to observe the result.
7. **Avoid conflicts** - Check that no other hook (in Phobos or Ares) already occupies overlapping bytes.

#### Static patching macros (Phobos-specific)

These macros are **not** part of Syringe - they are Phobos infrastructure defined in `src/Utilities/Macro.h`. They emit patch data into the `.patch` PE section, applied at DLL load time by `Patch::ApplyStatic()`:

| Macro | Purpose | Example |
|---|---|---|
| `DEFINE_JUMP(type, offset, target)` | Writes a jump/call/vtable entry. `type` is one of: `LJMP` (E9 JMP), `CALL` (E8 CALL), `CALL6` (FF 15 CALL + NOP), `VTABLE` (raw pointer). | `DEFINE_JUMP(LJMP, 0x6BB596, 0x6BB5A3)` |
| `DEFINE_PATCH(offset, ...)` | Writes raw bytes at `offset`. | `DEFINE_PATCH(0x6BB596, 0x90, 0x90)` |
| `DEFINE_FUNCTION_JUMP(type, offset, func)` | Like `DEFINE_JUMP` but targets a C++ function pointer. | `DEFINE_FUNCTION_JUMP(CALL, 0x48A3A0, MyFunc)` |
| `DEFINE_NAKED_HOOK(hook, funcname)` | Combines LJMP patch + naked (no-prolog) function declaration. For raw assembly hooks. | See `src/Phobos.cpp` for an example. |

#### Dynamic (runtime) patching (Phobos-specific)

The `Patch` class (`src/Utilities/Patch.h`) is Phobos infrastructure providing runtime patching via `VirtualProtect` + `memcpy`:

```cpp
Patch::Apply_LJMP(0x6BB596, 0x6BB5A3);  // E9 JMP
Patch::Apply_CALL(0x48A3A0, &MyFunc);    // E8 CALL
Patch::Apply_CALL6(0x48A3A0, &MyFunc);   // FF 15 CALL + NOP (6 bytes)
Patch::Apply_VTABLE(0x7E2280, &MyFunc);  // Overwrite vtable pointer
Patch::Apply_RAW(0x6BB596, { 0x90, 0x90 }); // Raw bytes
```

Use dynamic patches when the patch decision depends on runtime state (e.g., config toggles). Prefer static macros for unconditional patches.

### Serialization

All new data fields in `ExtData` classes that persist across frames **must** be serialized in the `Serialize` method using `PhobosStreamReader`/`PhobosStreamWriter` via the `.Process()` calls. Save/load correctness is critical for multiplayer sync.

### Using YRpp (engine type definitions)

YRpp (`YRpp/` submodule) is the header-only library that describes `gamemd.exe` binary types - classes, structs, enums, virtual function tables, and global variables. Before writing any hook that accesses game objects, you must look up (or add) the relevant definitions in YRpp.

#### Finding types and functions

1. **Start from the disassembly** - Identify the class, vtable, or global address in your IDA/Ghidra database (IDB).
2. **Search YRpp by address or vtable index** - Names in YRpp may not match your IDB's labels. Rely on **vtable slot positions** and **hex addresses** rather than symbol names when cross-referencing.
3. **Statics vs. structs** - The IDB may show individual global variables where YRpp has a struct/class, or vice versa. Check both representations.
4. **Use YRpp macros** - YRpp provides macros for calling virtual functions, accessing global instances, etc. Use them instead of raw pointer arithmetic:
   - Virtual function calls via vtable wrappers declared in the class headers.
   - `DEFINE_JUMP(VTABLE, ...)` to override a vtable entry.
   - Global instances accessed through static members or `DECL_ACCESS` macros in YRpp.

#### Key YRpp patterns

**Class declarations** - Game classes use `class NOVTABLE ClassName : public BaseClass` (suppresses vtable generation since these types are never instantiated directly by Phobos).

**Virtual functions** - Declared as standard C++ `virtual` methods with stub bodies (`R0` returns 0, `RX` for void, `RT(type)` for typed returns) or `JMP_THIS(address)` when the game implementation address is known. Normal C++ virtual dispatch is used - there is no custom vtable machinery.

**Non-virtual member functions** - Use `JMP_THIS(address)` (for `__thiscall`) or `JMP_STD(address)` (for `__stdcall`/static) in the function body to jump directly into game code at the specified address. These are defined in `YRpp/ASMMacros.h`.

**Global variables and arrays** - Accessed via compile-time reference macros from `YRpp/Helpers/CompileTime.h`:

| Macro | Purpose | Example |
|---|---|---|
| `DEFINE_REFERENCE(type, name, addr)` | Typed reference to a game global | `DEFINE_REFERENCE(DynamicVectorClass<BuildingClass*>, Array, 0xA8EB40u)` |
| `DEFINE_POINTER(type, name, addr)` | Typed pointer (for string constants etc.) | `DEFINE_POINTER(const char, ClassName, 0x...)` |
| `DEFINE_ARRAY_REFERENCE(type, dims, name, addr)` | Multi-dimensional array global | - |

**Binary layout helpers** - `DECLARE_PROPERTY(type, name)` embeds a member without invoking its constructor (matching game binary layout). `PROTECTED_PROPERTY(type, name)` marks padding/unknown bytes.

**Type-safe casting** (since RTTI is disabled) - `specific_cast<T*>(pAbstract)` checks `WhatAmI() == T::AbsID` for concrete types; `generic_cast<T*>(pAbstract)` checks `AbstractFlags` for abstract base types like `TechnoClass` or `FootClass`. Both are in `YRpp/Helpers/Cast.h`.

**Memory allocation** - Game objects that exist in the game's memory (not DLL-exclusive) must be allocated/freed with `GameCreate<T>(args...)`/`GameDelete(ptr)` (from `YRpp/Memory.h`) to use the game's heap. DLL-exclusive objects (e.g., `ExtData` classes) can use normal `new`/`delete`.

#### Adding or correcting definitions

When the type or function you need is missing or incorrect in YRpp, add or fix it directly in the `YRpp/` directory. YRpp is a nested git repo (submodule), so changes are committed and submitted separately:

1. **Make changes** inside `YRpp/` - rename fields, add structs, fix prototypes. Your IDE's rename-symbol feature will update both Phobos source and YRpp headers.
2. **Commit & push** the YRpp changes on a branch in your YRpp fork, then open a PR against [Phobos-developers/YRpp](https://github.com/Phobos-developers/YRpp).
3. **In Phobos**, the submodule pointer now references your YRpp branch commit. You can either:
   - Wait for the YRpp PR to merge, then update the submodule to the merged commit and push.
   - Push immediately (pointing at your YRpp branch) so CI can build - update the submodule pointer again after the YRpp PR merges.
4. After the YRpp PR is merged, check out the merge commit in `YRpp/`, verify Phobos compiles, and commit the updated submodule pointer.

**Important:** Always push your YRpp branch *before* pushing the Phobos commit that references it, otherwise CI cannot resolve the submodule.

## Code Style (enforced by .editorconfig)

- **Tabs** for indentation (size 4).
- **Allman brace style** - opening brace on new line.
- **CRLF** line endings for all source files (`.sh` files use LF).
- Local variables: `camelCase` with `p` prefix per pointer level (e.g., `pTechnoType`).
- Classes, namespaces, fields: `PascalCase`.
- INI-mapped fields: named exactly like the INI tag with dots replaced by underscores.
- Member initializer lists: one item per line, comma **after** newline.
- C++ exceptions are disabled (`/EHsc` is off); do not use `try`/`catch`/`throw`.
- RTTI is disabled; do not use `dynamic_cast`.

## Documentation

The docs live in `docs/` and are built with [Sphinx](https://sphinx-doc.org/) + [MyST parser](https://myst-parser.readthedocs.io/) (Markdown with Sphinx directives). They are hosted on [Read the Docs](https://readthedocs.io/).

### Syntax and formatting

Doc pages use **MyST-flavored Markdown** (`.md`), not reStructuredText. Key MyST features used in this project:
- **Directives**: `` ```{directive} `` blocks (e.g., `{hint}`, `{note}`, `{warning}`, `{toctree}`, `{include}`).
- **Colon fences**: `:::{directive}` / `:::` as an alternative to backtick fences.
- **Dollar math**: `$inline$` and `$$block$$` via `dollarmath` and `amsmath` extensions.
- **Heading anchors**: auto-generated up to depth 3 (`myst_heading_anchors = 3`).
- **Cross-references**: `(target-label)=` above a heading, then `` {ref}`target-label` `` to link.
- **sphinx-design components**: tabs (`{tab-set}`/`{tab-item}`), cards (`{card}`), grids (`{grid}`), badges (`` {bdg-primary}`text` ``), and icons (`` {octicon}`icon-name` ``).

INI code snippets should use ` ```ini ` fenced blocks. See existing doc pages for the standard format of documenting INI keys (key name, default value, accepted types, explanatory comments).

### Building docs locally

```
pip install -r docs/requirements.txt
scripts\build_docs.bat
```

Output goes to `docs/_build/html/`. Pull requests are automatically built and served by Read the Docs - check the PR status checks for a preview link.

### Translations

The project uses Sphinx internationalization with `.po` files. Currently only **zh_CN** (Chinese) is maintained. The translation workflow:

1. **Regenerate `.pot` templates and update `.po` files**:
   ```
   scripts\build_docs_locale.bat
   ```
   This runs `sphinx-build -b gettext` then `sphinx-intl update -p ./locale -l zh_CN`.

2. **Edit translations** in `docs/locale/zh_CN/LC_MESSAGES/*.po` - each `.po` file corresponds to a doc page. Key translation conventions: translate "AI agent" as **智能体** (not 代理, which means "proxy").

3. **Build localized docs** with `sphinx-build -D language=zh_CN`.

When editing English source docs, be aware that changes will invalidate corresponding `.po` entries (marked fuzzy), so translators will need to update them.

### English quality and style

Many contributors are non-native English speakers, so the existing documentation may contain grammar issues, awkward phrasing, or unclear wording. When writing or editing docs:

- **Write clear, concise English.** Use short sentences and active voice. Avoid jargon where a simpler word works.
- **Do not use em dashes in docs or comments** (`—`); use a hyphen-minus (`-`) instead.
- **Do not copy the style of poorly-worded sections.** Look for well-written sections as style examples, or improve the wording as you go.
- **If a section is badly worded**, improve it - don't just add to it. When unclear in English, cross-reference the Chinese translation (`docs/locale/zh_CN/LC_MESSAGES/`) which may express the intent more clearly, since some features were documented by Chinese-speaking contributors first.
- **Be consistent** with existing terminology (e.g., "TechnoType", "Warhead", "attached effect") and INI key formatting conventions used throughout the docs.

## PR Checklist

For non-trivial changes (unless labeled `No Documentation Needed`):
1. Update `docs/Whats-New.md` with a changelog entry.
2. Update `CREDITS.md` with your contribution.
3. Update relevant documentation pages in `docs/`.

Use `[Minor]` in the PR title for small changes that don't need documentation updates.

## Trust These Instructions

Trust the information here and proceed directly with implementation. Only search the codebase if these instructions are incomplete or produce errors. The build scripts, project structure, and patterns described above have been validated against the current `develop` branch at commit `a67278ed95c9cdc611e659d677bd4c918a887d16`.

**IMPORTANT:** if there are discrepancies between the codebase and these instructions, and especially if the user encounters issues or misunderstandings when you follow these instructions, verify the accuracy of these instructions against the current codebase. If you find any inaccuracies, update this file to correct them. **You are responsible for keeping these instructions accurate.** You may run a diff against the commit above to check for changes since validation. Use those diffs to account for changes in the codebase.

If your changes affect anything described in this file (project structure, build process, patterns, macros, etc.), **strongly consider updating this instructions file** to keep it accurate.

When selecting a model for Copilot, prefer **Claude** models for this repository.
