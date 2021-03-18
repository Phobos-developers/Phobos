# Contributing

This page describes how to help or contribute to Phobos and all the different ways to do so.

Engine modding is a complicated process which is pretty hard to pull off, but there are also easier parts which don't require mastering the art of reverse-engineering or becoming a dank magician in C++.

## Research

You can observe how the stuff works by using the engine and note which other stuff infliuences the behavior, but sooner or later you would want to see the innards of that. This is usually done using such tools as disassemblers/decompilers ([IDA](https://www.hex-rays.com/products/ida/), [Ghidra](https://ghidra-sre.org/)) to decipher what is written in the binary (`gamemd.exe` in case of the binary) and debuggers ([Cheat Engine](https://www.cheatengine.org)'s debugger is pretty good for that) to trace how the binary works.

:::{note}
Assembly language and C++ knowledge, understanding of computer architecture, memory structure, OOP and compiler theory would certainly help.
:::

## Code development

When you found out how the engine works and where you need to extend the logic you'd need to develop the code to achieve what you want. This is done by declaring a *hook* - some code which would be executed after the program execution reaches the certain address in binary. All the development is done in C++ using [YRpp](https://github.com/Phobos-developers/YRpp) (which provides a way to interact with YR code and inject code using Syringe) and usually [Visual Studio 2017/2019](https://visualstudio.microsoft.com) or newer.

:::{note}
You'd benefit from C++ experience, knowledge of programming patterns, common techniques etc. Basic assembly knowledge would help to correctly write the interaction with the memory where you hook at. Basic understanding of Git and GitHub is also needed.
:::

## Testing

This is a job that any modder can do. Look at a new feature or a change, try to think of all possible cases when it can work differently, try to think of any possible logic flaws, edge cases, unforeseen interactions or conditions etc., then test it according to your thoughts. Any bugs should be reported to issues section of this repo, if possible.

:::{warning}
**General stability** can only be achieved by extensive playtesting of new changes, both offline and online. Most modders have beta testing teams, so please, if you want the extension to be stable - contribute to that by having your testers play with the new features! Also the check-list below can help you identify issues quicker.
:::

### Testing check-list

- **All possible valid use cases covered**. Try check all of the valid feature use cases you can think of and verify that they work as intended with the feature.
- **Correct saving and loading**. Most of the additions like new INI tags require storing them in saved object info. Sometimes this is not done correctly, especially on complex stuff (like radiation types). Please, ensure all the improvements work __identically__ before and after being saved and loaded (on the same version of Phobos, of course).
- **Interaction with other features**. Try to use the feature chained or interacting with other features from vanilla or other libs (for example, mind control removal warhead initially was crashing when trying to remove mind control from a permanently MC'ed unit).
- **Overlapping features not working correctly** (including those from third-party libs like Ares, HAres, CnCNet spawner DLL). Think of what features' code could overlap (in a technical sense; means they modify the same code) with what you're currently testing. Due to the nature of the project some features from other libs could happen to not work as expected if they are overlapping (for example, when implementing mass selection filtering Ares' `GroupAs` was initially broken and units using it weren't being type selected properly).
- **Edge cases**. Those are the cases of some specific cases usually induced by some extreme  parameter values (for example, vanilla game crashes on zero-size `PreviewPack` instead of not drawing it).
- **Corner cases**. Those are similiar to edge cases but are hard to reproduce and are usually induced by a combination of extreme parameter values.

:::{note}
Knowledge on how to mod YR is the only requirement to do this, but having an inquisitive mind, being attentive to details would also help.
:::

## Writing docs

No explanation needed. If you fully understand how some stuff in Phobos works you can help by writing a detailed description in this wiki, or you can just improve the pieces of wiki you think are not detailed enough.

:::{note}
OK English grammar and understanding of docs structure would be enough.
:::

## Providing media to showcase features

Those would be used in docs and with a link to the respective mod as a bonus for the mod author. To record gifs you can use such apps as, for example, [GifCam](http://blog.bahraniapps.com/gifcam/).

:::{note}
Please, provide screenshots, gifs and videos in their natural size and without excess stuff or length.
:::
