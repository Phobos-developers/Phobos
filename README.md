![Phobos logo](logo.png)

[![license](https://img.shields.io/github/license/Phobos-developers/Phobos.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)
[![Workflow](https://img.shields.io/github/workflow/status/Phobos-developers/Phobos/Nightly%20Build.svg)](https://github.com/Phobos-developers/Phobos/actions)
[![Github All Releases](https://img.shields.io/github/downloads/Phobos-developers/Phobos/total.svg)](https://github.com/Phobos-developers/Phobos/releases)

**Phobos** is a WIP project providing a set of new features and fixes for Yuri's Revenge based on [modified YRpp](https://github.com/Metadorius/YRpp) and [Syringe](https://github.com/Ares-Developers/Syringe) to allow injecting code. It's meant to accompany [Ares](https://github.com/Ares-Developers/Ares) rather than replace it, thus it won't introduce incompatibilities.

**Current features**
============

- Full-color RGB PCX graphics support
- PCX loading screens of any resolution that are centered (and cropped if bigger than the actual resolution); depends on Ares tag `File.LoadScreen=`
- 3 new warheads:
  - `[Warhead]->TransactMoney=0 (integer - credits)`
    This many credits are added to warhead owner's credits when the warhead detonates. Use a negative number to subtract credits.
  - `[Warhead]->SpySat=no (boolean)`
    Reveals the map to the warhead owner when the warhead detonates.
  - `[Warhead]->BigGap=no (boolean)`
    Reshrouds the map for all opponents when the warhead detonates.
- Save/load support
- Ability to specify custom `gamemd.exe` icon via `-icon` command line argument followed by absolute or relative path to an `*.ico` file (f. ex. `gamemd.exe -icon Resources/clienticon.ico`); currently doesn't work with `CnC-DDraw` as it overrides the icon too
- Disable black dot spawn position markers on map preview (`[LoadingScreen]->DisableEmptySpawnPositions=no (boolean)` in `uimd.ini`)
- SHP debris now has their hardcoded shadows controlled by `Shadow=no` flag
- Building upgrades improve:
  - `[BuildingType]->PowersUp.Owner (list of owner)`
    Building upgrades placeable on ally or enemy buildings, controlled by `PowersUp.Owner=Self,Ally,Enemy` (mix and match the values separated with commas, for example you can   make powerplant upgrade be applicable to allies and yourself by specifying `PowersUp.Owner=Self,Ally` in the INI). Defaults to `Self`
  - `[BuildingType]->PowersUp.Buildings (list of BuildingType)`
    Specifies a list of structures that this update may improve.
- `[TechnoType]->Deployed.RememberTarget=no (boolean)` - makes vehicle-to-building deployer not lose the target on deploy.
- Fixed the bug when the mind control link was broken on vehicle-to-building deployment and it permanently changed owner
- Ability to hide the unstable warning by specifying the build number after `-b=` as a command line arg. (for example, `-b=1` would hide the warning for build 1). **Please, test the features (especially online and edge cases) before disabling it, we can't test everything :)**
- Customizable laser disk radius via `[WeaponType]->DiskLaser.Radius=38.2 (double)` (in voxels). Default value is roughly the default radius used for vanilla saucer.
- Extended sidebar tooltips (only for TechnoTypes for now, WIP)
  - `uimd.ini` flags to control the new tooltips:
    - `[ToolTips]->ExtendedToolTips=no (boolean)` controls whether the extended tooltip or the vanilla tooltip would be drawn at sidebar. Extended tooltips **don't** use `TXT_MONEY_FORMAT_1` and `TXT_MONEY_FORMAT_2`. Instead you can specify cost, power and time (WIP) labels with the next tags
    - `[ToolTips]->CostLabel= (CSF key)` specifies the character or label to display to denote cost in an extended tooltip instead of default `$` sign
    - `[ToolTips]->PowerLabel= (CSF key)` ditto for power generation/consumption instead of default `⚡ U+26A1` sign
    - `[ToolTips]->TimeLabel= (CSF key)` tritto for build time instead of default `⌚ U+231A` sign (WIP)
  - `[TechnoType]->UIDescription= (CSF key)` - description text which is shown in sidebar on hover over a unit, won't show up if not specified
- Win/lose themes (`[Side]->IngameScore.WinTheme= (theme id)` and `[Side]->IngameScore.LoseTheme= (theme id)`)
- Switch hardcoded sidebar button coords to GDI sidebar (`[Side]->Sidebar.GDIPositions= (boolean)`)



Building
--------

Windows:

0. Install Visual Studio with "Desktop development with C++" workload, "C++ Windows XP Support for VS 2017 (v141) tools" individual component and clone this repo recursively.
1. Open the solution file in VS and build it.

Upon build completion place the resulting `Phobos.dll` in your YR directory and launch Syringe targeting your YR executable (usually `gamemd.exe`).


Credits
-------

- Belonit aka Gluk-v48, Metadorius aka Kerbiter - project authors
- misha135n2 - YRpp edits
- tomsons26, CCHyper - all-around help, assistance and guidance in reverse-engineering, YR binary mappings
- Ares developers - creating YRpp, Syringe and Ares without which the project wouldn't exist
  - DCoder - unused deployer fixes that are now included in Phobos
  - AlexB - save/load code
- CCHyper - current project logo
- ZΞPHYɌUS - win/lose themes code


Legal and License
-----
[![LGPL v3](https://www.gnu.org/graphics/lgplv3-147x51.png)](https://opensource.org/licenses/LGPL-3.0)

The Phobos project is an unofficial open-source community collaboration project to extend the Red Alert 2 Yuri's Revenge engine for modding and compatibility purposes.

This project has no direct affiliation with Electronic Arts Inc. Command & Conquer, Command & Conquer Red Alert 2, Command & Conquer Yuri's Revenge are registered trademarks of Electronic Arts Inc. All Rights Reserved.

