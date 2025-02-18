# Contributing

This page describes ways to help or contribute to Phobos and lists the contributing guidelines that are used in the project.

## Guidelines for contributors

### Project structure

Assuming you've successfully cloned and built the project before getting here, you should end up with the following project structure:
- `src/` - all the project's source code resides here.
  - `Commands/` - source code for new hotkey commands. Every command is a new class that inherits from `PhobosCommandClass` (defined in `Commands.h`) and is defined in a separate file with a few methods and then registered in `Commands.cpp`.
  - `New/` - source code for new ingame classes.
    - `Type/` - new enumerated types (types that are declared with a list section in an INI, for example, radiation types) implemented in the project. Every enumerated type class inherits `Enumerable<T>` (where `T` is an enum. type class) class that is defined in `Enumerable.h`.
    - `Entity/` - classes that represent ingame entities are located here.
  - `Ext/` - source code for vanilla engine class extensions. Each class extension is kept in a separate folder named after vanilla engine class name and contains the following:
    - `Body.h` and `Body.cpp` contain class and method definitions/declarations and common extension hooks. Each extension class must contain the following to work correctly:
      - `ExtData` - extension data class definition which inherits `Extension<T>` from `Container.h` (where `T` is the class that is being extended), which is the actual class that contains new data for vanilla classes;
      - `ExtContainer` - a definition of a special map class to store and look up `ExtData` instances for base class instances which inherits `Container<T>` from `Container.h` (where `T` is the extension data class);
      - `ExtMap` - a static instance of `ExtContainer` map;
      - constructor, destructor, serialization, deserialization and (for appropriate classes) INI reading hooks.
    - `Hooks.cpp` and `Hooks.*.cpp` contain non-common hooks to correctly patch in new custom logics.
  - `ExtraHeaders/` - extra header files to interact with / describe types included in game binary that are not included in YRpp yet.
  - `Misc/` - uncategorized source code, including hooks that don't belong to an extension class.
  - `Utilities/` - common code that is used across the project.
  - `Phobos.cpp`/`Phobos.h` - extension bootstrapping code.
  - `Phobos.Ext.cpp` - contains common processing code new or extended classes. If you define a new or extended class you have to add your new class into `MassActions` global variable type declaration in this file.
- `YRpp/` - contains the header files to interact with / describe types included in game binary and also macros to write hooks using Syringe. Included as a submodule.

### Code styleguide

We have established a couple of code style rules to keep things consistent. Some of the rules are enforced in `.editorconfig`, where applicable, so you can autoformat the code by pressing `Ctrl + K, D` hotkey chord in Visual studio. Still, it is advised to manually check the style before submitting the code.
- We use tabs instead of spaces to indent code.
- Curly braces are always to be placed on a new line ([Allman indentation style](https://en.wikipedia.org/wiki/Indentation_style#Allman_style)). One of the reasons for this is to clearly separate the end of the code block head and body in case of multiline bodies:
```cpp
if (SomeReallyLongCondition()
    || ThatSplitsIntoMultipleLines())
{
    DoSomethingHere();
    DoSomethingMore();
}
```
- Braceless code block bodies should be made only when both code block head and body are single line,  statements split into multiple lines and nested braceless blocks are not allowed within braceless blocks:
```cpp
// OK
if (Something())
    DoSomething();

// OK
if (SomeReallyLongCondition()
    || ThatSplitsIntoMultipleLines())
{
    DoSomething();
}

// OK
if (SomeCondition())
{
    if (SomeOtherCondition())
        DoSomething();
}

// OK
if (SomeCondition())
{
    return VeryLongExpression()
        || ThatSplitsIntoMultipleLines();
}
```
- Only empty curly brace blocks may be left on the same line for both opening and closing braces (if appropriate).
- If you use if-else you should either have all of the code blocks braced or braceless to keep things consistent.
- Big conditions which span multiple lines and are hard to read otherwise should be split into smaller logical parts to improve readability:
```cpp
// Not OK
if (This() && That() && AlsoThat()
    || (OrOtherwiseThis && OtherwiseThat && WhateverElse))
{
    DoSomething();
}

// OK
bool firstCondition = This() && That() && AlsoThat();
bool secondCondition = OrOtherwiseThis && OtherwiseThat && WhateverElse;

if (firstCondition || secondCondition)
    DoSomething();
```
- Code should have empty lines to make it easier to read. Use an empty line to split code into logical parts. It's mandatory to have empty lines to separate:
  - `return` statements (except when there is only one line of code except that statement);
  - local variable assignments that are used in the further code (you shouldn't put an empty line after one-line local variable assignments that are used only in the following code block though);
  - code blocks (braceless or not) or anything using code blocks (function or hook definitions, classes, namespaces etc.);
  - hook register input/output.
```cpp
// OK
auto localVar = Something();
if (SomeConditionUsing(localVar))
    ...

// OK
auto localVar = Something();
auto anotherLocalVar = OtherSomething();

if (SomeConditionUsing(localVar, anotherLocalVar))
    ...

// OK
auto localVar = Something();

if (SomeConditionUsing(localVar))
    ...

if (SomeOtherConditionUsing(localVar))
    ...

localVar = OtherSomething();

// OK
if (SomeCondition())
{
    Code();
    OtherCode();

    return;
}

// OK
if (SomeCondition())
{
    SmallCode();
    return;
}

```
- `auto` may be used to hide an unnecessary type declaration if it doesn't make the code harder to read. `auto` may not be used on primitive types.
- A space must be put between braces of empty curly brace blocks.
- To have less Git merge conflicts member initializer lists and other list-like syntax structures used in frequently modified places should be split per-item with item separation characters (commas, for example) placed *after newline character*:
```cpp
ExtData(TerrainTypeClass* OwnerObject) : Extension<TerrainTypeClass>(OwnerObject)
    , SpawnsTiberium_Type(0)
    , SpawnsTiberium_Range(1)
    , SpawnsTiberium_GrowthStage({ 3, 0 })
    , SpawnsTiberium_CellsPerAnim({ 1, 0 })
{ }
```
- Local variables and function/method args are named in the `camelCase` (using a `p` prefix to denote pointer type for every pointer nesting level) and a descriptive name, like `pTechnoType` for a local `TechnoTypeClass*` variable.
- Classes, namespaces, class fields and members are always written in `PascalCase`.
- Class fields that can be set via INI tags should be named exactly like ini tags with dots replaced with underscores.
- Pointer type declarations always have pointer sign `*` attached to the type declaration.
- Non-static class extension methods faked by declaring a static method with `pThis` as a first argument are only to be placed in the extension class for the class instance of which `pThis` is.
  - If it's crucial to fake `__thiscall` you may use `__fastcall` and use `void*` or `void* _` as a second argument to discard value passed through `EDX` register. Such methods are to be used for call replacement.
- Hooks have to be named using a following scheme: `HookedFunction_HookPurpose`, or `ClassName_HookedMethod_HookPurpose`. Defined-again hooks are exempt from this scheme due to impossibility to define different names for the same hook.
- Return addresses should use anonymous enums to make it clear what address means what, if applicable. The enum has to be placed right at the function start and include all addresses that are used in this hook:
```cpp
DEFINE_HOOK(0x48381D, CellClass_SpreadTiberium_CellSpread, 0x6)
{
    enum { SpreadReturn = 0x4838CA, NoSpreadReturn = 0x4838B0 };

    ...
}
```
- Even if the hook doesn't use `return 0x0` to execute the overriden instructions, you still have to write correct hook size (last parameter of `DEFINE_HOOK` macro) to reduce potential issues if the person editing this hook decides to use `return 0x0`.
- New ingame "entity" classes are to be named with `Class` postfix (like `RadTypeClass`). Extension classes are to be named with `Ext` postfix instead (like `RadTypeExt`).
- Do not pollute the namespace.
- Avoid introducing unnecessary macros if they can be replaced by equivalent `constexpr` or `__forceinline` functions.

```{note}
The styleguide is not exhaustive and may be adjusted in the future.
```

### Git branching model

Couple of notes regarding the Git practices. We use [git-flow](https://nvie.com/posts/a-successful-git-branching-model/)-like workflow:
  - `master` is for stable releases, can have hotfixes pushed to it or branched off like a feature branch with the requirement of version increment and `master` being merged into `develop` after that;
  - `develop` is the main development branch;
  - `feature/`-prefixed branches (sometimes the prefix may be different if appropriate, like for big fixes or changes) are so called "feature branches" - those are branched off `develop` for every new feature to be introduced into it and then merged back. We use squash merge to merge them back in case of smaller branches and sometimes merge commit in case the branch is so big it would be viable to keep it as is.
  - `hotfix/`-prefixed branches may be used in a same manner as `feature/`, but with `master` branch, with a requirement of `master` being merged into `develop` after `hotfix/` branch was squash merged into `master`.
  - `release/`-prefixed branches are branched off `develop` when a new stable release is slated to allow working on features for a next release and stability improvements for this release. Those are merged with a merge commit into `master` and `develop` with a stable version number increase, after which the stable version is released.
- When you're working with your local & remote branches use **fast-forward** pulls to get the changes from remote branch to local, **don't merge remote branch into local and vice versa**, this creates junk commits and makes things unsquashable.

These commands will do the following for all repositories on your PC:
1) remove the automatic merge upon pull and replace it with a rebase;
2) highlight changes consisting of moving existing lines to another location with a different color.

```bash
git config --global pull.rebase true
git config --global branch.autoSetupRebase always
git config --global diff.colorMoved zebra
```

### Working with YRpp via submodules

Often when working on Phobos and/or researching the YR engine you'll need to implement corrections for YRpp. Generally the corrections need to be submitted to [YRpp repository](https://github.com/Phobos-developers/YRpp) and can be done separately from the actual features in Phobos, but frequently the improvements are to be submitted as a part of Phobos contribution process. To submit improvements to YRpp you have to create a branch in YRpp, then you can push it and submit a pull request to YRpp repository.

When you clone Phobos recursively - you also clone YRpp as a submodule. Basically submodules are just nested repositories. You can open it like any other repository, so the changes can be synchronized to Phobos and you don't need to rename stuff by hand.

The suggested workflow is as follows:
1. In your IDE of choice rename fields and functions using symbol renaming feature (`Rename...` feature in Visual Studio (regular or Code), `[F2]` by default), then you will have two "levels" of changes displayed in your Git client:
   - for Phobos repository - changes in the Phobos code (as regular changes) and changes to YRpp (as one submodule change)
   - for YRpp repository - changes to the field names and function names in YRpp as regular changes.
2. Create a branch in YRpp repository (create a fork of it if you didn't yet), commit and push the changes and submit it as a pull request. After pushing it you have two options in Phobos repository:
   - wait until it's accepted, then checkout YRpp at the newest commit, then commit and push - this will save you having to commit and push multiple times, but you won't be able to get a nightly build for people to test;
   - don't wait for YRpp changes to be merged, commit and push right after you pushed the YRpp changes to your YRpp branch - you will have an up-to-date build on Phobos pull request this way. Note that you must do this only after you committed to and pushed your YRpp branch, otherwise the build system won't know what are the changes as they are not exposed to the world, only available to you locally.
3. After the YRpp pull request gets accepted you will need to switch to the latest commit that was merged (you do that in the submodule), verify that it compiles like normal, and then commit and push it to your Phobos branch that you made for your pull request.

## Ways to help

Engine modding is a complicated process which is pretty hard to pull off, but there are also easier parts which don't require mastering the art of reverse-engineering or becoming a dank magician in C++.

### Research and reverse-engineering

You can observe how the stuff works by using the engine and note which other stuff influences the behavior, but sooner or later you would want to see the innards of that. This is usually done using such tools as disassemblers/decompilers ([IDA](https://www.hex-rays.com/products/ida/), [Ghidra](https://ghidra-sre.org/)) to decipher what is written in the binary (`gamemd.exe` in case of the binary) and debuggers ([Cheat Engine](https://www.cheatengine.org)'s debugger is pretty good for that) to trace how the binary works.

```{hint}
Reverse-engineering is a complex task, but don't be discouraged, if you want to try your hands at it ask us in the Discord channel, we will gladly help 😄
```

```{note}
[Assembly language](https://www.cs.virginia.edu/~evans/cs216/guides/x86.html) and C++ knowledge, understanding of computer architecture, memory structure, OOP and compiler theory would certainly help.
```

### Development

When you found out how the engine works and where you need to extend the logic you'd need to develop the code to achieve what you want. This is done by declaring a *hook* - some code which would be executed after the program execution reaches the certain address in binary. All the development is done in C++ using [YRpp](https://github.com/Phobos-developers/YRpp) (which provides a way to interact with YR code and inject code using Syringe) and usually [Visual Studio 2017/2019](https://visualstudio.microsoft.com) or newer.

(contributing-changes-to-the-project)=
#### Contributing changes to the project

To contribute a feature or some sort of a change you you would need a Git client (I recommend [GitKraken](https://www.gitkraken.com/) personally). Fork, clone the repo, preferably make a new branch, then edit/add the code or whatever you want to contribute. Commit, push, start a pull request, wait for it to get reviewed, or merged.

If you contribute something, please make sure:
- you write documentation for the change;
- you mention the change in the changelog and migration sections in the [what's new page](Whats-New.md);
- you mention your contribution in the [credits page](CREDITS.md).

If your change does not fit in standard criteria or too small that it doesn't need the above - add `[Minor]` to your pull request's title, so the CI won't yell at you for no reason.

```{hint}
Every pull request push trigger a nightly build for the latest pushed commit, so you can check the build status at the bottom of PR page, press `Show all checks`, go to details of a build run and get the zip containing built DLL and PDB (for your testers, f. ex.), or download a build from an automatically posted comment.
```

```{note}
You'd benefit from C++ experience, knowledge of programming patterns, common techniques etc. [Basic assembly knowledge](https://www.cs.virginia.edu/~evans/cs216/guides/x86.html) would help to correctly write the interaction with the memory where you hook at. Basic understanding of Git and GitHub is also needed.
```

### Testing

This is a job that any modder (and even sometimes player) can do. Look at a new feature or a change, try to think of all possible cases when it can work differently, try to think of any possible logic flaws, edge cases, unforeseen interactions or conditions etc., then test it according to your thoughts. Any bugs should be reported to issues section of this repo, if possible.

```{warning}
**General stability** can only be achieved by extensive play-testing of new changes, both offline and online. Most modders have beta testing teams, so please, if you want the extension to be stable - contribute to that by having your testers play with the new features! Also the check-list below can help you identify issues quicker.
```

#### Testing check-list

- **All possible valid use cases covered**. Try to check all of the valid feature use cases you can think of and verify that they work as intended with the feature.
- **Correct saving and loading**. Most of the additions like new INI tags require storing them in saved object info. Sometimes this is not done correctly, especially on complex stuff (like radiation types). Please, ensure all the improvements work __identically__ before and after being saved and loaded (on the same version of Phobos, of course).
- **Interaction with other features**. Try to use the feature chained or interacting with other features from vanilla or other libs (for example, mind control removal warhead initially was crashing when trying to remove mind control from a permanently mind-controlled unit).
- **Overlapping features not working correctly** (including those from third-party libs like Ares, HAres, CnCNet spawner DLL). Think of what features' code could overlap (in a technical sense; means they modify the same code) with what you're currently testing. Due to the nature of the project some features from other libs could happen to not work as expected if they are overlapping (for example, when implementing mass selection filtering Ares' `GroupAs` was initially broken and units using it weren't being type selected properly).
- **Edge cases**. Those are the cases of some specific cases usually induced by some extreme  parameter values (for example, vanilla game crashes on zero-size `PreviewPack` instead of not drawing it).
- **Corner cases**. Those are similar to edge cases but are hard to reproduce and are usually induced by a combination of extreme parameter values.

```{note}
Knowledge on how to mod YR and having an inquisitive mind, being attentive to details would help.
```

### Writing docs

No explanation needed. If you fully understand how some stuff in Phobos works you can help by writing a detailed description in these docs, or you can just improve the pieces of docs you think are not detailed enough.

The docs are written in Markdown (which is dead simple, [learn MD in 60 seconds](https://commonmark.org/help/); if you need help on extended syntax have a look at [MyST parser reference](https://myst-parser.readthedocs.io/)). We use [Sphinx](https://sphinx-doc.org/) to build docs, [Read the Docs](https://readthedocs.io/) to host.

```{hint}
You don't need to install Python, Sphinx and modules to see changes - every pull request you make is being built and served by Read the Docs automatically. Just like the nightly builds, scroll to the bottom, press `Show all checks` and see the built documentation in the details of a build run.
```

There are two ways to edit the docs.
- **Edit from your PC**. Pretty much the same like what's described in [contributing changes section](contributing-changes-to-the-project); the docs are located in the `docs` folder.
- **Edit via online editor**. Navigate to the doc piece that you want to edit, press the button on the top right - and it will take you to the file at GitHub which you would need to edit (look for the pencil icon to the top right). Press it - the fork will be created and you'll edit the docs in your version of the repo (fork). You can commit those changes (preferably to a new branch) and make them into a pull request to main repo.

```{note}
OK English grammar and understanding of docs structure would be enough. You would also need a GitHub account.
```

### Providing media to showcase features

Those would be used in docs and with a link to the respective mod as a bonus for the mod author. To record GIFs you can use such apps as, for example, [GifCam](http://blog.bahraniapps.com/gifcam/).

```{note}
Please, provide screenshots, GIFs and videos in their natural size and without excess stuff or length.
```

### Promoting the work

You can always help us by spreading the word about the project among people, whether you're an influential youtuber, a C&C related community leader or just an average player.
