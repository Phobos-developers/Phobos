# What's New

This page lists the history of changes across stable Phobos releases and also all the stuff that requires modders to change something in their mods to accomodate.

## Migrating

### From vanilla

- SHP debris hardcoded shadows now respect `Shadow=no` tag value, and due to it being the default value they wouldn't have hardcoded shadows anymore by default. Override this by specifying `Shadow=yes` for SHP debris.
- Radiation now has owner by default, which means that radiation kills will affect score and radiation field will respect `Affects...` tags. You can override that with `rulesmd.ini->[SOMEWEAPONTYPE]->Rad.NoOwner=yes` tag value.

### From older Phobos versions

- Tag `rulesmd.ini->[SOMETECHNOTYPE]->Deployed.RememberTarget` is deprecated and can be removed now, the bugfix for `DeployToFire` deployers is now always on.
## Changelog

### 0.2

New:
- Shield logic for TechnoTypes (by Uranusian, secsome) with warhead additions (by Starkku)
- Custom Radiation Types (by AlexB, Otamaa, Belonit, Uranusian)
- New ScriptType actions `71 Timed Area Guard`, `72 Load Onto Transports`, `73 Wait until ammo is full` (by FS-21)
- Ore drills now have customizable ore type, range, ore growth stage and amount of cells generated (by Kerbiter)
- Basic projectile interception logic (by AutoGavy, Kerbiter, Erzoid/SukaHati)
- Customizable harvester active/total counter next to credits counter (by Uranusian)
- Select Next Idle Harvester hotkey command (by Kerbiter)
- Dump Object Info hotkey command (by secsome, FS-21)
- Remove Disguise and Remove Mind Control warhead effects (by secsome)
- Custom per-warhead SplashLists (by Uranusian)
- Chance-based critical damage system on warheads (by AutoGavy)
- Optional mind control range limit (by Uranusian)
- Multiple mind controllers can now release units on overload (by Uranusian, secsome)
- Spawns now can be killed on low power and have limited pursuing range (by FS-21)
- Spawns can now have the same exp. level as owner techno (by Uranusian)
- `TurretOffset` now accepts `F,L,H` and `F,L` values instead of just `F` value (by Kerbiter)
- ElectricBolt visuals can now be disabled (by Otamaa)
- Semantic locomotor aliases for modder convenience (by Belonit)
- Ability to specify amount of shots for strafing aircraft and burst simulation (by Starkku)
- Customizeable Teleport/Chrono Locomotor properties per TechnoClass (by Otamaa)

Vanilla fixes:
- Map previews with zero size won't crash the game anymore (by Kerbiter, Belonit)
- Tileset 255+ bridge fix (by E1 Elite)
- Fixed fatal errors when `Blowfish.dll` couldn't be registered in the system properly due to missing admin rights (by Belonit)
- Fix to take Burst into account for aircraft weapon shots beyond the first one (by Starkku)
- Fixed the bug when units are already dead but still in map (for sinking, crashing, dying animation, etc.), they could die again (by Uranusian)
- Fixed the bug when cloaked Desolator uneable to fire his deploy weapon (by Otamaa)

Phobos fixes:
- Properly rewritten a fix for mind-controlled vehicles deploying into buildings (by FS-21)
- Properly rewritten `DeployToFire` fix, tag `Deployed.RememberTarget` is deprecated, now always on (by Kerbiter)

### 0.1.1

- Fixed an occasional crash when selecting units with a selection box

### 0.1

New:
- Full-color PCX graphics support (by Belonit)
- Support for PCX loading screens of any size (by Belonit)
- Extended sidebar tooltips with descriptions, recharge time and power consumption/generation (by Kerbiter, Belonit)
- Selection priority filtering for box selection (by Kerbiter)
- Shroud, reveal and money transact warheads (by Belonit)
- Custom game icon command line arg (by Belonit)
- Ability to disable black spawn position dots on map preview (by Belonit)
- Ability to specify applicable building owner for building upgrades (by Kerbiter)
- Customizable disk laser radius (by Belonit, Kerbiter)
- Ability to switch to GDI sidebar layout for any side (by Belonit)

Vanilla fixes:
- Deploying mind-controlled TechnoTypes won't make them permanently mind-controlled anymore (unfinished fix by DCoder)
- SHP debris hardcoded shadows now respect `Shadow=no` tag value (by Kerbiter)
- `DeployToFire` vehicles won't lose target on deploy anymore (unfinished fix by DCoder)
- Fixed QWER hotkey tab switching not hiding the displayed tooltip as it should (by Belonit)
- Sidebar tooltips now can go over sidebar bounds (by Belonit)
- Lifted stupidly small limit for tooltip character amount (by Belonit)
