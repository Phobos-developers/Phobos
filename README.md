![Phobos logo](logo.png)

**Phobos** is a WIP project providing a set of new features and fixes for Yuri's Revenge based on [modified YRpp](https://github.com/Metadorius/YRpp) and [Syringe](https://github.com/Ares-Developers/Syringe) to allow injecting code. It's meant to accompany [Ares](https://github.com/Ares-Developers/Ares) rather than replace it, thus it won't introduce incompatibilities.

**Current features**
============

- Full-color RGB PCX graphics support
- PCX loading screens of any resolution that are centered (and cropped if bigger than the actual resolution); depends on Ares tag `File.LoadScreen=`
- 3 new warheads
  - `[Warhead]->TransactMoney= (integer - credits)`
    This many credits are added to warhead owner's credits when the warhead detonates. Use a negative number to subtract credits. Defaults to `0`
  - `[Warhead]->SpySat= (boolean)`
    Reveals the map to the warhead owner when the warhead detonates. Defaults to `no`
  - `[Warhead]->BigGap= (boolean)`
    Reshrouds the map for all opponents when the warhead detonates. Defaults to `no`
- Save/load support
- Ability to specify custom `gamemd.exe` icon via `-icon` command line argument followed by absolute or relative path to an `*.ico` file (f. ex. `gamemd.exe -icon Resources/clienticon.ico`); currently doesn't work with `CnC-DDraw` as it overrides the icon too
- Disable black dot spawn position markers on map preview (`[LoadingScreen]->DisableEmptySpawnPosition= (boolean)` in `uimd.ini`)
- SHP debris now has their hardcoded shadows controlled by `Shadow` flag (defaults to `no`)
- Building upgrades placeable on ally or enemy buildings, controlled by `PowersUp.Owner=Self,Ally,Enemy` (mix and match the values separated with commas, for example you can make powerplant upgrade be applicable to allies and yourself by specifying `PowersUp.Owner=Self,Ally` in the INI). Defaults to `Self`
- `Deployed.RememberTarget= (boolean)` - makes vehicle-to-building deployer not lose the target on deploy. Defaults to `no`
- Fixed the bug when the mind control link was broken on vehicle-to-building deployment and it permanently changed owner
- Ability to hide the unstable warning by specifying the build number after `-b=` as a command line arg. (for example, `-b=1` would hide the warning for build 1). **Please, test the features (especially online and edge cases) before disabling it, we can't test everything :)**



Building
--------

Windows:

0. Install Visual Studio with "Desktop development with C++" workload, "C++ Windows XP Support for VS 2017 (v141) tools" individual component and clone this repo recursively.
1. Open the solution file in VS and build it.

Upon build completion place the resulting `Phobos.dll` in your YR directory and launch Syringe targeting your YR executable (usually `gamemd.exe`).


Credits
-------

- Belonit aka Gluk-v48, Metadorius aka Kerbiter, misha135n2 - collaborators
- TheAssemblyArmada (tomsons26 especially) - all-around help, assistance and guidance in reverse-engineering
- Ares developers - creating YRpp, Syringe and Ares without which the project wouldn't exist
  - DCoder - unused deployer fixes that are now included in Phobos
  - AlexB - save/load code
- CCHyper - current project logo