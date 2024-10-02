![Phobos YR Engine Extension](logo.png)

[![Github All Releases](https://img.shields.io/github/downloads/Phobos-developers/Phobos/total.svg)](https://github.com/Phobos-developers/Phobos/releases)
[![Docs status](https://readthedocs.org/projects/phobos/badge/?version=latest)](https://phobos.readthedocs.io/en/latest/?badge=latest)
[![Workflow](https://img.shields.io/github/actions/workflow/status/Phobos-developers/Phobos/nightly.yml?branch=develop)](https://github.com/Phobos-developers/Phobos/actions)
[![EditorConfig](https://github.com/Phobos-developers/Phobos/workflows/EditorConfig/badge.svg)](https://github.com/Phobos-developers/Phobos/actions?query=workflow%3AEditorConfig)
[![license](https://img.shields.io/github/license/Phobos-developers/Phobos.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)

> **Warning**
> The project is currently not maintained actively enough and thus we are looking for active maintainers at the moment. Please message us [in Discord channel](https://discord.gg/sZeMzz6qVg) (or PM Kerbiter directly).

# Phobos
...is a community engine extension project providing a set of new features and fixes for Yuri's Revenge based on [modified YRpp](https://github.com/Metadorius/YRpp) and [Syringe](https://github.com/Ares-Developers/Syringe) to allow injecting code. It's meant to accompany [Ares](https://github.com/Ares-Developers/Ares) rather than replace it, thus it won't introduce incompatibilities.

While Phobos is independent of Ares and does NOT require Ares specifically to function, Phobos complements some of the features found in Ares and vice versa.

You can discuss the project at a dedicated [channel on C&C Mod Haven](https://discord.gg/sZeMzz6qVg).

Downloads
---------

You can choose one of the following:
- [Latest stable branch build](https://github.com/Phobos-developers/Phobos/releases/latest) (most bug-free release but very slow on new features)
- [Latest development branch builds](https://github.com/Phobos-developers/Phobos/releases) (a bit less bug-free releases, devbuilds get new features when they are finished)
- [Latest development branch nightly](https://nightly.link/Phobos-developers/Phobos/blob/develop/.github/workflows/nightly.yml) (added unreleased features that will be in next devbuild)
- Individual new feature nightly builds for testing can be found in [pull requests](https://github.com/Phobos-developers/Phobos/pulls)

### Note on nightly builds

Last two listed versions are bleeding edge (don't redistribute them outside of testing!) and have build information (commit and branch/tag) in them which is displayed ingame and can't be turned off. You can get a build for development branch (link above) any up-to-date pull request via an automatic bot comment that would appear in it and would contain the most recent successfully compiled version of Phobos for that feature branch. Please note that the build is  produced *only if the PR has no merge conflicts*. Alternatively, you can get an artifact manually from GitHub Actions runs. You can get an artifact for a specific commit which is built automatically with a GitHub Actions workflow, just press on a green tick, open the workflow, find and download the build artifact. This is limited to authorized users only.

Installation and Usage
----------------------

0. If you don't have Syringe installed into your mod already, you can download it together with the [latest Ares package](https://launchpad.net/ares/+download). To install simply drop `Syringe.exe` into your game folder (where your `gamemd.exe` is located). It's highly recommended to **install Ares** too to get full Phobos feature set, just drop all the files from the archive except documentation folder into your game folder.
1. Obtain a Phobos "package" (official builds can be found on [releases page](https://github.com/Phobos-developers/Phobos/releases); read below to learn how to get nightly builds). You should end up with two files: `Phobos.dll` and `Phobos.pdb`.
2. Place those files in the game folder (where your `gamemd.exe` is located).
3. To launch the game with Phobos (and all other installed Syringe-compatible engine extensions including Ares) you need to execute `Syringe.exe "gamemd.exe" [command line arguments for gamemd.exe]` in command line (omit arguments if you don't need any). `RunAres.bat` from Ares package does the same so you may use that as well.

If you already use Ares in your mod, you just need to drop Phobos files mentioned above in your game folder, Syringe will load Phobos automatically. This also applies to mods using XNA client with Syringe; if your mod doesn't use Syringe and Ares (or you just haven't set up the client) yet we recommend to use [CnCNet client mod base by Starkku](https://github.com/Starkku/cncnet-client-mod-base) which is compatible with Ares and Phobos out of the box.

Additional files and tools that you may need are located at [Phobos supplementaries repo](https://github.com/Phobos-developers/PhobosSupplementaries).

By default Phobos doesn't do any very noticeable changes except a few bugfixes. To learn how to use Phobos features head over to official documentation.

Documentation
-------------

- [Official docs](https://phobos.readthedocs.io) (also available in [Chinese](https://phobos.readthedocs.io/zh_CN/latest))

You can switch between versions (displays latest develop nightly version by default) in the bottom left corner, as well as download a PDF version.

The documentation is split by a few major categories, each represented with a page on the sidebar. Each page has its contents grouped into multiple subcategories, be it buildings, technotypes, infantries, superweapons or something else.

### How to read code snippets

```ini
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

Building manually
-----------------

0. Install **Visual Studio** (2022 is recommended, 2019 is minimum) with the dependencies listed in `.vsconfig` (it will prompt you to install missing dependences when you open the project, or you can run VS installer and import the config). If you prefer to use **Visual Studio Code** you may install **VS Build Tools** with the dependencies from `.vsconfig` instead. Not using a code editor or IDE and building via **command line scripts** included with the project is also an option.
1. Clone this repo recursively via your favorite git client (that will also clone YRpp).
2. To build the extension:
   - in Visual Studio: open the solution file in VS and build it (`Debug` build config is recommended);
   - in VSCode: open the project folder and hit `Run Build Task...` (`Ctrl + Shift + B`);
   - barebones: run `scripts/build_debug.bat`.
3. Upon build completion the resulting `Phobos.dll` and `Phobos.pdb` would be placed in the subfolder identical to the name of the buildconfig executed.

Credits
-------

### Developers
- **Belonit (Gluk-v48)** - project author
- **Kerbiter (Metadorius)** - project co-author, BDFL, maintainer (semi-active for the time being; [Patreon](https://www.patreon.com/kerbiter), PM me for PayPal to avoid fees)
- **Starkku** - co-maintainer, developer ([Patreon](https://www.patreon.com/Starkku))
- **Uranusian (Thrifinesma)** - retired developer, CN community ambassador
- **secsome (SEC-SOME)** - developer
- **Otamaa (Fahroni, BoredEXE)** - developer ([PayPal](https://paypal.me/GeneralOtama))
- **FS-21** - developer
- **Morton (MortonPL)** - co-maintainer, developer
- **Trsdy (chaserli)** - co-maintainer, developer

For all contributions see [full credits list](CREDITS.md).

Thanks to everyone who uses Phobos, tests changes and reports bugs! You can show your appreciation and help project by displaying the logo (monochrome version can be found [here](https://github.com/Phobos-developers/Phobos/blob/develop/logo-mono.png)) in your client/launcher (make it open Phobos GitHub page for extra fanciness), linking to Phobos repository, contributing or donating to us via the links above.

Legal and License
-----

[![LGPL v3](https://www.gnu.org/graphics/lgplv3-147x51.png)](https://opensource.org/licenses/LGPL-3.0)

The Phobos project is an unofficial open-source community collaboration project to extend the Red Alert 2 Yuri's Revenge engine for modding and compatibility purposes.

As a modification, the project complies with [EA C&C modding guidelines](https://www.ea.com/games/command-and-conquer/command-and-conquer-remastered/modding-faq); should there be conflict between the project's license and modding guidelines - the rules imposed by guidelines shall take precedence (for example, the project should not be commercial or used to make money).

This project has no direct affiliation with Electronic Arts Inc. Command & Conquer, Command & Conquer Red Alert 2, Command & Conquer Yuri's Revenge are registered trademarks of Electronic Arts Inc. All Rights Reserved.
