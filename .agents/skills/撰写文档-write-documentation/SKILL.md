---
name: 撰写文档 Write Documentation
description: "### Write Documentation\n\nThis command is invoked when the code changes for a feature or bugfix are complete on a branch (branched from `develop`) and documentation is the only remaining task. The AI should discover what changed, categorize it, and write the appropriate documentation entries."
---

#### Workflow

**Step 0: Determine the author**

Use `git log develop...HEAD --format="%an <%ae>"` to list commits on the current branch that are not in `develop`. If the branch has extra commits, use the author name(s) from those commits as the user's identity for `CREDITS.md` and `Whats-New.md`. If there are no extra commits or the author cannot be determined from commit information, ask the user directly for their identity before proceeding.

**Step 1: Discover what changed**

Run `git diff develop...HEAD --stat` to see which files were modified. Then inspect the diffs in detail using `git diff develop...HEAD` (or `git diff develop...HEAD -- <specific files>`) to understand the nature of each change. Key things to identify:
- New/changed INI keys (tags) — look for `.Read(exINI, ...)` calls in `LoadFromINIFile` / `LoadBeforeTypeData` etc.
- New/changed hooks — look for `DEFINE_HOOK`, `DEFINE_JUMP`, `DEFINE_PATCH` macros.
- New types/classes — look for new files in `src/New/` or new `ExtData` classes.
- Whether the change is a new feature, an enhancement of existing behavior, a bugfix, a UI change, an AI/scripting change, or something miscellaneous.

**Step 2: Categorize the change**

Determine which doc pages need updating:

| Change type | Where to document |
|-------------|------------------|
| New feature (brand new functionality) | New `### ` section in the appropriate primary doc page, inserted in **dictionary (alphabetical) order** among existing sibling `### ` headings |
| Enhancement of existing game behavior that adds new INI tags | New `### ` section in the appropriate primary doc page, likewise in alphabetical order |
| Enhancement/bugfix of existing game behavior with **no new INI tags** | A bullet point under `## Bugfixes and miscellaneous` in `docs/Fixed-or-Improved-Logics.md` |
| User interface change (new hotkey, display, sidebar, tooltip) | `docs/User-Interface.md` |
| AI, scripting, trigger, or mapping change | `docs/AI-Scripting-and-Mapping.md` |
| Uncategorized change | `docs/Miscellanous.md` |

Primary doc page mapping:
- Most game logic features → `docs/New-or-Enhanced-Logics.md`
  - Within this file, `## ` headings are categories (e.g., `## Buildings`, `## Projectiles`, `## Warheads`). Place new `### ` sections under the appropriate category, sorted alphabetically.

Additionally, every change requires:
- **`docs/Whats-New.md`** — changelog entry.
- **`CREDITS.md`** — credit the author.

**Step 3: Draft the documentation text — user review required before writing**

Before writing any documentation, draft the descriptive text and present it to the user for review. Use the following format for the draft:

```
### <Feature Name>

- <Brief meaning of the feature.>
  - (tab-indented) <INI key> controls/determines/is used for <explanation>.
```

Two styles for the first bullet point:
- **New feature** (brand new functionality):
  ```
  - Now you can <do something>. (新功能)
  ```
- **Enhancement of existing behavior** (patches/improves vanilla logic):
  ```
  - In vanilla, <describe the problem>. Now you can <describe the fix/enhancement>.
  ```

Wait for the user to approve the drafted text before proceeding to write it into the actual doc files.

**Step 4: Write the main documentation**

After user approval, write the approved text into the appropriate doc file. For sections in `docs/New-or-Enhanced-Logics.md` or similar, insert the new `### ` section in alphabetical order among existing sibling headings under the correct `## ` category. Look at existing sibling `### ` headings to determine the correct insertion point.

Continue with the INI code block following existing conventions from the [README's "How to read code snippets" section](../../README.md#how-to-read-code-snippets):

````markdown
; which section the entries should be in
; can be a freeform name - in this case the comment would explain what it is
; if no comment to be found - then it's a precise name
[SOMENAME]           ; BuildingType
; KeyName=DefaultValue ; accepted type with optional explanation
; if there's nothing to the right of equals sign - the default value is empty/absent
; if these keys have had their value set, they can only be set to their default
; unset state again by setting the value to <default>, <none> or none
; for list of values only <default> clears the entire list
; if the default value is not static - it's written and explained in a comment
UIDescription=<none> ; CSF entry key
```
````

Key rules for INI documentation:
- Use ` ```ini ` fenced code blocks.
- Section header comment format: `[SOMENAME]` followed by spaces then `; ObjectType` (e.g., `; BuildingType`, `; TechnoType`, `; WarheadType`, `; SuperWeaponType`).
- For global sections use the literal section name: `[General]`, `[AudioVisual]`, `[CombatDamage]`, `[Radiation]`, `[AI]`, etc.
- Key name, equals sign, default value (or blank if empty), spaces, semicolon, type description.
- Boolean types are documented as `; boolean`.
- Integer types as `; integer`.
- Floating point types as `; double` or `; float`.
- Pointer types as `; AnimType` (just the game class name, not full C++ type).
- List types as `; list of TechnoType` etc.
- If the INI key name contains dots, it maps naturally (e.g. `KeyName.SubKey` stays as-is in docs).

When there are many related keys, group them logically within the same section and INI block. Put them in the order they appear in the INI section.

**Step 5: Chinese translation**

After the English documentation has been written and confirmed, produce a Chinese translation of the new section(s). Present the Chinese text to the user for review and confirmation. Update the corresponding `.po` files in `docs/locale/zh_CN/LC_MESSAGES/` after the user approves the translation.

**Step 6: Write `docs/Whats-New.md` entry**

Whats-New entries go under the `### Version TBD (develop branch nightly builds)` section. Use the author identity determined in Step 0. The entry should be a bullet point with a feature description matching the phrasing used in other entries:

```markdown
- <feature description matching the phrasing used in other entries> (by <AuthorName>)
```

If new INI keys are user-facing settings in `RA2MD.INI`, also add them under the `### New user settings in RA2MD.INI` section:
```ini
[Phobos]
NewKeyName=true                  ; boolean
```

If INI keys were renamed, add entries under `### Changed tags` using the format seen in existing entries:
```markdown
- `[Section] -> OldName` -> `[Section] -> NewName`
```

**Step 7: Write `CREDITS.md` entry**

Use the author identity determined in Step 0. Find the author's section in `CREDITS.md`. If the author doesn't have a section yet, create one. Add a bullet describing the contribution:

```markdown
- **AuthorName (GitHubUsername)**:
  - Feature description matching the phrasing used in other entries
```

Do not skip the CREDITS entry unless the user explicitly says to skip it.

**Step 8: Review and confirm**

After writing all entries, present a summary to the user showing which files were modified and what was added.
