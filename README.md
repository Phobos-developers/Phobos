![Phobos YR Engine Extension](logo.png)

[![Github All Releases](https://img.shields.io/github/downloads/Phobos-developers/Phobos/total.svg)](https://github.com/Phobos-developers/Phobos/releases)
[![Docs status](https://readthedocs.org/projects/phobos/badge/?version=latest)](https://phobos.readthedocs.io/en/latest/?badge=latest)
[![Workflow](https://img.shields.io/github/workflow/status/Phobos-developers/Phobos/Nightly%20Build.svg)](https://github.com/Phobos-developers/Phobos/actions)
[![license](https://img.shields.io/github/license/Phobos-developers/Phobos.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)

# Phobos
...is a community engine extension project providing a set of new features and fixes for Yuri's Revenge based on [modified YRpp](https://github.com/Metadorius/YRpp) and [Syringe](https://github.com/Ares-Developers/Syringe) to allow injecting code. It's meant to accompany [Ares](https://github.com/Ares-Developers/Ares) rather than replace it, thus it won't introduce incompatibilities.

While Phobos is independent of Ares and does NOT require Ares specifically to function, Phobos complements some of the features found in Ares and vice versa.

You can discuss the project at a dedicated [channel on C&C Mod Haven](https://discord.gg/sZeMzz6qVg).

Downloads
---------

You can choose one of the following:
- [Latest stable branch build](https://github.com/Phobos-developers/Phobos/releases/latest) (most bug-free release but very slow on new features)
- [Latest development branch builds](https://github.com/Phobos-developers/Phobos/releases) (a bit less less bug-free releases, devbuilds get new features when they are finished)
- [Latest development branch nightly](https://nightly.link/Phobos-developers/Phobos/blob/develop/.github/workflows/nightly.yml) (added unreleased stuff that will be in next devbuild)
- Individual new feature builds (for testing) can be found in [pull requests](https://github.com/Phobos-developers/Phobos/pulls)

### Note on nightly builds

Last two listed versions are bleeding edge (don't redistribute them outside of testing!) and have build information (commit and branch/tag) in them which is displayed ingame and can't be turned off. You can get a build for development branch (link above) any up-to-date pull request via an automatic bot comment that would appear in it and would contain the most recent successfully compiled version of Phobos for that feature branch. Alternatively, you can get an artifact manually from GitHub Actions runs. You can get an artifact for a specific commit which is built automatically with a GitHub Actions workflow, just press on a green tick, open the workflow, find and download the build artifact. This is limited to authorized users only.

Installation and Usage
----------------------

0. If you don't have Syringe installed into your mod already, you can download it together with the [latest Ares package](https://launchpad.net/ares/+download). To install simply drop `Syringe.exe` into your game folder (where your `gamemd.exe` is located). It's highly recommended to **install Ares** too to get full Phobos feature set, just drop all the files from the archive except documentation folder into your game folder.
1. Obtain a Phobos "package" (official builds can be found on [releases page](https://github.com/Phobos-developers/Phobos/releases); read below to learn how to get nightly builds). You should end up with two files: `Phobos.dll` and `Phobos.pdb`.
2. Place those files in the game folder (where your `gamemd.exe` is located).
3. To launch the game with Phobos (and all other installed Syringe-compatible engine extensions including Ares) you need to execute `Syringe.exe "gamemd.exe" [command line arguments for gamemd.exe]` in command line (omit arguments if you don't need any). `RunAres.bat` from Ares package does the same so you may use that as well.

If you already use Ares in your mod, you just need to drop Phobos files mentioned above in your game folder, Syringe will load Phobos automatically. This also applies to mods using XNA client with Syringe; if your mod doesn't use Syringe and Ares (or you just haven't set up the client) yet we recommend to use [CnCNet client mod base by Starkku](https://github.com/Starkku/cncnet-client-mod-base) which is compatible with Ares and Phobos out of the box.

By default Phobos doesn't do any very noticeable changes except a few bugfixes. To learn how to use Phobos features head over to official documentation.

Documentation
-------------

- [Official docs](https://phobos.readthedocs.io) (also available in [Chinese](https://phobos.readthedocs.io/zh_CN/latest))

You can switch between versions (displays latest develop nightly version by default) in the bottom left corner, as well as download a PDF version.

The documentation is split by a few major categories, each represented with a page on the sidebar. Each page has it's contents grouped into multiple subcategories, be it buildings, technotypes, infantries, superweapons or something else.

### How to read code snippets

```ini
; which section the entries should be in
; can be a freeform name - in this case the comment would explain what it is
; if no comment to be found - then it's a precise name
[SOMENAME]           ; BuildingType
; KeyName=DefaultValue ; accepted type with optional explanation
; if there's nothing to the right of equals sign - the default value is empty/absent
; if the default value is not static - it's written and explained in a comment
UIDescription=<none> ; CSF entry key
```

Building manually
-----------------

0. Install **Visual Studio** (2019 is recommended, 2017 is minimum) with the dependencies listed in `.vsconfig` (it will prompt you to install missing dependences when you open the project, or you can run VS installer and import the config). If you prefer to use **Visual Studio Code** you may install **VS Build Tools** with the stuff from `.vsconfig` instead. You can also don't use any code editor or IDE and build via **command line scripts** included with the project.
1. Clone this repo recursively via your favorite git client (that will also clone YRpp).
2. To build the extension:
   - in Visual Studio: open the solution file in VS and build it (`Debug` build config is recommended);
   - in VSCode: open the project folder and hit `Run Build Task...` (`Ctrl + Shift + B`);
   - barebones: run `scripts/build_debug.bat`.
3. Upon build completion the resulting `Phobos.dll` and `Phobos.pdb` would be placed in the subfolder identical to the name of the buildconfig executed.

Credits
-------

### Developers
- **Belonit (Gluk-v48)** - project author (retired)
- **Kerbiter (Metadorius)** - project co-author, current maintainer ([Patreon](http://patreon.com/kerbiter))
- **Uranusian (Thrifinesma)** - developer, CN community ambassador ([Patreon](https://www.patreon.com/uranusian), [AliPay](http://tiebapic.baidu.com/forum/w%3D580/sign=4b04b953307f9e2f70351d002f31e962/b3f89909b3de9c823bd7f23a7b81800a18d84371.jpg))
- **secsome (SEC-SOME)** - developer ([Patreon](https://www.patreon.com/secsome))
- **Otamaa (Fahroni, BoredEXE)** - developer ([PayPal](https://paypal.me/GeneralOtama))
- **FS-21** - developer
- **Starkku** - developer
- **Morton (MortonPL)** - developer

### Contributions
- **Belonit (Gluk-v48)** - project creation, disable empty spawn positions, custom gamemd icon with Command Line, full-color non-paletted PCX, SpySat, BigGap, TransactMoney, PCX Loading Screen, custom DiskLaser radius, extended tooltips, building upgrades enhancement, hide health bar, Sidebar.GDIPosition, help with CellSpread, Blowfish.dll-related errors fix, zero size map previews, semantic locomotor aliases, shields, input fix
- **Kerbiter (Metadorius)** - SHP debris respect Shadow, building upgrades enhancement, extended tooltips, selection priority filtering, TurretOffset enhancement, customizable ore spawners, select next idle harvester hotkey, interceptor enhancement, zero size map previews, LaserTrails, laser fixes, CI/CD, overhauled Unicode font, docs maintenance, VSCode configs, code style
- **tomsons26** - all-around help, assistance and guidance in reverse-engineering, YR binary mappings
- **CCHyper** - all-around help, current project logo, assistance and guidance in reverse-engineering, YR binary mappings
- **Ares developers** - YRpp and Syringe which are used, save/load, project foundation and generally useful code from Ares, unfinished RadTypes code, prototype deployer fixes
- **ZΞPHYɌUS** - win/lose themes code
- **ayylmao** - help with docs
- **SMxReaver** - help with docs, extensive and thorough testing
- **4SG** - help with docs
- **wiktorderelf** - overhauled Unicode font
- **Uranusian (Thrifinesma)** - Mind Control enhancement, custom warhead splash list, harvesters counter, promoted spawns, shields, death after dead fix, customizeable missing cameo, cameo sorting priority, placement mode responding of tab hotkeys fix, producing progress, custom ore gathering anim, NoManualMove, weapon target house filtering, DeathWeapon fix, re-enable obsolete `JumpjetControls`, AITrigger Building Upgrades recognition, Wall-Gate links, deployed infantry using both weapons, overhauled Unicode font, docs maintenance, CN docs translation
- **secsome (SEC-SOME)** - debug info dump hotkey, refactoring & porting of Ares helper code, introducing more Ares-derived stuff, disguise removal warhead, Mind Control removal warhead, Mind Control enhancement, shields, AnimList.PickRandom, MoveToCell fix, unlimited waypoints, Build At trigger action buildup anim fix, Undeploy building into a unit plays `EVA_NewRallyPointEstablished` fix, custom ore gathering anim, TemporaryClass related crash, Retry dialog on mission failure, Default disguise for individual InfantryTypes, PowerPlant Enhancer, SaveGame Trigger Action, QuickSave command, Numeric variables, Custom gravity for projectiles, Retint map actions bugfix, Sharpnel enhancement, Vanilla map preview reading bugfix
- **Otamaa (Fahroni, BoredEXE)** - help with CellSpread, ported and fixed custom RadType code, togglable ElectricBolt bolts, customizable Chrono Locomotor properties per TechnoClass, DebrisMaximums fixes, Anim-to-Unit, NotHuman anim sequences improvements, Customizable OpenTopped Properties, hooks for ScriptType Actions 92 & 93, ore stage threshold for `HideIfNoOre`, occupied building `MuzzleFlashX` bugfix,`EnemyUIName=` for other TechnoTypes, TerrainType `DestroyAnim` & `DestroySound`, Laser trails for VoxelAnims
- **E1 Elite** - TileSet 255 and above bridge repair fix
- **FS-21** - Dump Object Info enhancements, Powered.KillSpawns, Spawner.LimitRange, ScriptType Actions 71 to 113, MC deployer fixes, help with docs, Automatic Passenger Deletion, Fire SW At Location Trigger Action, Fire SW At Waypoint Trigger Action, Kill Object Automatically, Customize resource storage, Override Uncloaked Underwater attack behavior, AI Aircraft docks fix, Shared Ammo
- **AutoGavy** - interceptor logic, Warhead critical hit logic, Customize resource storage
- **ChrisLv_CN** - interceptor logic, LaserTrails, laser fixes, general assistance (work relicensed under [following permission](images/ChrisLv-relicense.png))
- **Xkein** - general assistance, YRpp edits
- **thomassneddon** - general assistance
- **Starkku** - Warhead shield penetration & breaking, strafing aircraft weapon customization, vehicle DeployFire fixes/improvements, stationary VehicleTypes, Burst logic improvements, TechnoType auto-firing weapons, Secondary weapon fallback customization, weapon target type filtering, AreaFire targeting customization, CreateUnit improvements, Attached animation & jumpjet unit layer customization, IsSimpleDeployer improvements, Shield modification warheads, Warhead decloaking toggle, Warp(In/Out)Weapon, Grinder improvements / additions, Attached animation position customization, Critical hit logic additions, Aircraft & jumpjet speed modifiers fix, Local warhead screen shaking, Vehicle custom palette fix, Weapon owner detachment, Feedback weapon
- **SukaHati (Erzoid)** - Minimum interceptor guard range
- **Morton (MortonPL)** - XDrawOffset, Shield passthrough & absorption, building LimboDelivery, fix for Image in art rules, power delta counter
- **mevitar** - honorary shield tester *triple* award
- **Damfoos** - extensive and thorough testing
- **Dmitry Volkov** - extensive and thorough testing
- **Rise of the East community** - extensive playtesting of in-dev features
- **Chasheen (Chasheenburg)** - CN docs translation

Thanks to everyone who uses Phobos, tests changes and reports bugs! You can show your appreciation and help project by displaying the logo (monochrome version can be found [here](logo-mono.png)) in your client/launcher (make it open Phobos GitHub page for extra fanciness), linking to Phobos repository, contributing or donating to us via the links above.

Legal and License
-----

[![LGPL v3](https://www.gnu.org/graphics/lgplv3-147x51.png)](https://opensource.org/licenses/LGPL-3.0)

The Phobos project is an unofficial open-source community collaboration project to extend the Red Alert 2 Yuri's Revenge engine for modding and compatibility purposes.

This project has no direct affiliation with Electronic Arts Inc. Command & Conquer, Command & Conquer Red Alert 2, Command & Conquer Yuri's Revenge are registered trademarks of Electronic Arts Inc. All Rights Reserved.

