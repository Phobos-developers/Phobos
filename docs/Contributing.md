# Contributing

Engine modding is a complicated process which is pretty hard to pull off, but there are also easier parts which don't require mastering the art of reverse-engineering or becoming a dank magician in C++.

### Research and reverse-engineering

You can observe how the stuff works by using the engine and note which other stuff influences the behavior, but sooner or later you would want to see the innards of that. This is usually done using such tools as disassemblers/decompilers ([IDA](https://www.hex-rays.com/products/ida/), [Ghidra](https://ghidra-sre.org/)) to decipher what is written in the binary (`gamemd.exe` in case of the binary) and debuggers ([Cheat Engine](https://www.cheatengine.org)'s debugger is pretty good for that) to trace how the binary works.

::: tip Hint
Reverse-engineering is a complex task, but don't be discouraged, if you want to try your hands at it ask us in the Discord channel, we will gladly help 😄
:::

::: info Note
[Assembly language](https://www.cs.virginia.edu/~evans/cs216/guides/x86.html) and C++ knowledge, understanding of computer architecture, memory structure, OOP and compiler theory would certainly help.
:::

### Development

When you found out how the engine works and where you need to extend the logic you'd need to develop the code to achieve what you want. This is done by declaring a _hook_ - some code which would be executed after the program execution reaches the certain address in binary. All the development is done in C++ using [YRpp](https://github.com/Phobos-developers/YRpp) (which provides a way to interact with YR code and inject code using Syringe) and usually [Visual Studio 2017/2019](https://visualstudio.microsoft.com) or newer.

#### Quickstart guide for AI-assisted development

The repository includes a [Copilot instructions file](https://github.com/Phobos-developers/Phobos/blob/develop/.github/copilot-instructions.md) that serves as a quickstart guide for the project - it covers building, project structure, hook patterns, patching macros, YRpp usage and more. It is automatically picked up by GitHub Copilot and similar AI coding agents, but is also a useful read for any new contributor looking to understand the codebase quickly.

We encourage contributors to try AI coding agents (such as GitHub Copilot in agent mode, Cursor, or similar tools) to assist with development tasks - writing hooks, reviewing code, generating documentation, and exploring the disassembly via [IDA Pro MCP](https://github.com/mrexodia/ida-pro-mcp). Agent skills (such as the [IDAPython skill](https://skills.sh/mrexodia/ida-pro-mcp/idapython)) can further extend agent capabilities with domain-specific knowledge. AI agents can significantly speed up routine work.

::: info Note
AI agents are a tool to assist development, but they are not perfect and can make mistakes. Always review and test any code generated or modified by an AI agent to ensure it meets the project's standards and works correctly. **You are responsible for the final code**, not the tool you use to write it.
:::

## Contributing changes to the project

To ensure harmonious coexistence, developers and maintainers should first read our [Project guidelines and policies](Project-guidelines-and-policies.md).

To contribute a feature or some sort of a change you you would need a Git client (I recommend [GitKraken](https://www.gitkraken.com/) personally). Fork, clone the repo, preferably make a new branch, then edit/add the code or whatever you want to contribute. Commit, push, start a pull request, wait for it to get reviewed, or merged.

If you contribute something, please make sure:

- you write documentation for the change;
- you mention the change in the changelog and migration sections in the [what's new page](Whats-New.md);
- you mention your contribution in the [credits page](/CREDITS).

If your change does not fit in standard criteria or too small that it doesn't need the above - add `[Minor]` to your pull request's title, so the CI won't yell at you for no reason.

::: tip Hint
Every pull request push trigger a nightly build for the latest pushed commit, so you can check the build status at the bottom of PR page, press `Show all checks`, go to details of a build run and get the zip containing built DLL and PDB (for your testers, f. ex.), or download a build from an automatically posted comment.
:::

::: info Note
You'd benefit from C++ experience, knowledge of programming patterns, common techniques etc. [Basic assembly knowledge](https://www.cs.virginia.edu/~evans/cs216/guides/x86.html) would help to correctly write the interaction with the memory where you hook at. Basic understanding of Git and GitHub is also needed.
:::

### Testing

This is a job that any modder (and even sometimes player) can do. Look at a new feature or a change, try to think of all possible cases when it can work differently, try to think of any possible logic flaws, edge cases, unforeseen interactions or conditions etc., then test it according to your thoughts. Any bugs should be reported to issues section of this repo, if possible.

::: warning Warning
**General stability** can only be achieved by extensive play-testing of new changes, both offline and online. Most modders have beta testing teams, so please, if you want the extension to be stable - contribute to that by having your testers play with the new features! Also the check-list below can help you identify issues quicker.
:::

#### Testing check-list

- **All possible valid use cases covered**. Try to check all of the valid feature use cases you can think of and verify that they work as intended with the feature.
- **Correct saving and loading**. Most of the additions like new INI tags require storing them in saved object info. Sometimes this is not done correctly, especially on complex stuff (like radiation types). Please, ensure all the improvements work **identically** before and after being saved and loaded (on the same version of Phobos, of course).
- **Interaction with other features**. Try to use the feature chained or interacting with other features from vanilla or other libs (for example, mind control removal warhead initially was crashing when trying to remove mind control from a permanently mind-controlled unit).
- **Overlapping features not working correctly** (including those from third-party libs like Ares, HAres, CnCNet spawner DLL). Think of what features' code could overlap (in a technical sense; means they modify the same code) with what you're currently testing. Due to the nature of the project some features from other libs could happen to not work as expected if they are overlapping (for example, when implementing mass selection filtering Ares' `GroupAs` was initially broken and units using it weren't being type selected properly).
- **Edge cases**. Those are the cases of some specific cases usually induced by some extreme parameter values (for example, vanilla game crashes on zero-size `PreviewPack` instead of not drawing it).
- **Corner cases**. Those are similar to edge cases but are hard to reproduce and are usually induced by a combination of extreme parameter values.

::: info Note
Knowledge on how to mod YR and having an inquisitive mind, being attentive to details would help.
:::

### Writing docs

No explanation needed. If you fully understand how some stuff in Phobos works you can help by writing a detailed description in these docs, or you can just improve the pieces of docs you think are not detailed enough. AI coding agents can also help with writing and improving documentation - see [Quickstart guide for AI-assisted development](#quickstart-guide-for-ai-assisted-development) above.

The docs are written in Markdown (which is dead simple, [learn MD in 60 seconds](https://commonmark.org/help/)). We use [VitePress](https://vitepress.dev/) to build the documentation site and [Read the Docs](https://readthedocs.org/) to host it.

::: tip Hint
You can preview docs locally:

1. `cd docs`
2. `npm ci`
3. `npm run dev`

For a production build, run `npm run build`.

Chinese docs are built from `docs/locale/zh_CN/LC_MESSAGES/*.po` automatically by the VitePress PO locale plugin during `dev` and `build`.
:::

::: tip Hint
You don't need to install Node.js, VitePress and npm packages locally just to see documentation changes - every pull request you make is built and served by Read the Docs automatically. Just like the nightly builds, scroll to the bottom of the PR page, press `Show all checks` and open the details of a Read the Docs build run to see the built documentation.
:::

There are two ways to edit the docs.

- **Edit from your PC**. Pretty much the same like what's described in [contributing changes section](#contributing-changes-to-the-project); the docs are located in the `docs` folder.
- **Edit via online editor**. Navigate to the doc piece that you want to edit, press the button on the top right - and it will take you to the file at GitHub which you would need to edit (look for the pencil icon to the top right). Press it - the fork will be created and you'll edit the docs in your version of the repo (fork). You can commit those changes (preferably to a new branch) and make them into a pull request to main repo.

::: info Note
`README.md`, `CREDITS.md` and `LICENSE.md` are sourced from the repository root by the VitePress root pages plugin. Edit the files in the repository root instead.
:::

::: info Note
OK English grammar and understanding of docs structure would be enough. You would also need a GitHub account.
:::

### Providing media to showcase features

Those would be used in docs and with a link to the respective mod as a bonus for the mod author. Please provide silent demonstration videos as WebM files.

::: info Note
Please, provide screenshots and WebM videos in their natural size and without excess stuff or length.
:::

### Promoting the work

You can always help us by spreading the word about the project among people, whether you're an influential youtuber, a C&C related community leader or just an average player.
