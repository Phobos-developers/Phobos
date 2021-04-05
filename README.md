# ![Phobos YR Engine Extension](logo.png)

[![Github All Releases](https://img.shields.io/github/downloads/Phobos-developers/Phobos/total.svg)](https://github.com/Phobos-developers/Phobos/releases)
[![Docs status](https://readthedocs.org/projects/phobos/badge/?version=latest)](https://phobos.readthedocs.io/en/latest/?badge=latest)
[![Workflow](https://img.shields.io/github/workflow/status/Phobos-developers/Phobos/Nightly%20Build.svg)](https://github.com/Phobos-developers/Phobos/actions)
[![license](https://img.shields.io/github/license/Phobos-developers/Phobos.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)

**Phobos** is a WIP community project providing a set of new features and fixes for Yuri's Revenge based on [modified YRpp](https://github.com/Metadorius/YRpp) and [Syringe](https://github.com/Ares-Developers/Syringe) to allow injecting code. It's meant to accompany [Ares](https://github.com/Ares-Developers/Ares) rather than replace it, thus it won't introduce incompatibilities.

For now you can discuss the project at a dedicated [channel on C&C Mod Haven](https://discord.gg/sZeMzz6qVg) (which is my C&C modding server).


Building and Usage (Windows)
----------------------------

0. Install Visual Studio with "Desktop development with C++" workload and "C++ Windows XP Support for VS 2017 (v141) tools" individual component and clone this repo recursively (that will also clone YRpp).
1. Open the solution file in VS and build it (`DevBuild` build config is recommended).
2. Upon build completion place the resulting `Phobos.dll` from folder named identical to the used build config in your YR directory and launch Syringe targeting your YR executable (usually `gamemd.exe`).

You can also get a nightly build for those who want to help testing Phobos features as soon as they are done. Those versions are bleeding edge (don't redistribute them outside of testing!) and have build information (commit and branch/tag) in them which is displayed ingame and can't be turned off. There are two ways get a nightly build.
- **Get an artifact via nightly.link**. This is a service that allows guests to download automatic builds from GitHub. You can get a build for the latest successful (marked with a green tick) `develop` branch commit via [this link](https://nightly.link/Phobos-developers/Phobos/blob/develop/.github/workflows/nightly.yml), or get a build for any up-to-date pull request via an automatic comment that would appear in it. 
- **Get an artifact manually from GitHub Actions runs**. You can get an artifact for a specific commit which is built automatically with a GitHub Actions workflow, just press on a green tick, open the workflow, find and download the build artifact. This is limited to authorized users only.

Documentation
-------------

The documentation can be found at [here @ Read the Docs](https://phobos.readthedocs.io) and is split by a few major categories (similiar to Ares docs), each represented with a page on the sidebar. Each page has it's contents grouped into multiple subcategories, be it buildings, technotypes, infantry, superweapons or something else.

You can switch between versions in the bottom left corner, as well as download a PDF version.

How to read code snippets
-------------------------

```ini
; which section the params should be in
; can be a freeform name - in this case the comment would explain what it is
; if no comment to be found - then it's a precise name
[SOMENAME] ; BuildingType
; KeyName=DefaultValue ; accepted type with optional explanation
; if there's nothing to the right of equals sign - the default value is empty/absent
UIDescription=<none> ; CSF entry key
```

Credits
-------

- Belonit (Gluk-v48), Metadorius (Kerbiter) - project authors
- misha135n2, Xkein - YRpp edits
- tomsons26, CCHyper - all-around help, assistance and guidance in reverse-engineering, YR binary mappings
- Ares developers - YRpp and Syringe which are used, save/load, project foundation and generally useful code from Ares, unfinished RadTypes code 
- DCoder - unused deployer fixes that are now included in Phobos
- CCHyper - current project logo
- ZΞPHYɌUS - win/lose themes code
- ayylmao, SMxReaver, 4SG, FS-21, Thrifinesma (Uranusian), SEC-SOME (secsome) - help with docs
- wiktorderelf, Metadorius (Kerbiter), Thrifinesma (Uranusian) - overhauled Unicode font
- Thrifinesma (Uranusian) - Mind Control range limit, custom warhead splash list, harvesters counter, promoted spawns, shields, general help
- SEC-SOME (secsome) - debug info dump hotkey, refactoring & porting of Ares helper code, introducing more Ares-derived stuff, disguise removal warhead, mind control removal warhead, shields, general help
- Otamaa (BoredEXE) - help with CellSpread, ported and fixed custom RadType code
- E1 Elite - TileSet 255 and above bridge repair fix
- FS-21 - Dump Object Info enhancements, Powered.KillSpawns, Spawner.LimitRange, ScriptType Actions 71 & 72, MC deployer fixes, help with docs
- AutoGavy - interceptor logic, warhead critical damage system
- Xkein - general assistance, YRpp edits
- thomassneddon - general assistance

Thanks to everyone who uses Phobos, tests changes and reports bugs! You can show your appreciation and help project by displaying the logo (monochrome version can be found [here](https://github.com/Phobos-developers/Phobos/logo-mono.png)) in your client/launcher, contributing or donating to us via links on the right and the `Sponsor` button on top of the repo.

Legal and License
-----
[![LGPL v3](https://www.gnu.org/graphics/lgplv3-147x51.png)](https://opensource.org/licenses/LGPL-3.0)

The Phobos project is an unofficial open-source community collaboration project to extend the Red Alert 2 Yuri's Revenge engine for modding and compatibility purposes.

This project has no direct affiliation with Electronic Arts Inc. Command & Conquer, Command & Conquer Red Alert 2, Command & Conquer Yuri's Revenge are registered trademarks of Electronic Arts Inc. All Rights Reserved.

