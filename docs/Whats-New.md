# What's New

This page lists the history of changes across stable Phobos releases and also all the stuff that requires modders to change something in their mods to accomodate.

## Migrating

### From vanilla

- SHP debris hardcoded shadows now respect `Shadow=no` tag value, and due to it being the default value they wouldn't have hardcoded shadows anymore by default. Override this by specifying `Shadow=yes` for SHP debris.
- Radiation now has owner by default, which means that radiation kills will affect score and radiation field will respect `Affects...` entries. You can override that with `rulesmd.ini->[SOMEWEAPONTYPE]->Rad.NoOwner=yes` entry.

### From older Phobos versions

- Key `rulesmd.ini->[SOMETECHNOTYPE]->Deployed.RememberTarget` is deprecated and can be removed now, the bugfix for `DeployToFire` deployers is now always on.

### For Map Editor (Final Alert 2)

In `FAData.ini`:
```ini
[ParamTypes]
47=Structures,28
53=Play BuildUp,10

[EventsRA2]
500=Local variable is greater than,48,6,0,0,[LONG DESC],0,1,500,1
501=Local variable is less than,48,6,0,0,[LONG DESC],0,1,501,1
502=Local variable equals to,48,6,0,0,[LONG DESC],0,1,502,1
503=Local variable is greater than or equals to,48,6,0,0,[LONG DESC],0,1,503,1
504=Local variable is less than or equals,48,6,0,0,[LONG DESC],0,1,504,1
505=Local variable and X is true,48,6,0,0,[LONG DESC],0,1,505,1
506=Global variable is greater than,48,6,0,0,[LONG DESC],0,1,506,1
507=Global variable is less than,48,6,0,0,[LONG DESC],0,1,507,1
508=Global variable equals to,48,6,0,0,[LONG DESC],0,1,508,1
509=Global variable is greater than or queals to,48,6,0,0,[LONG DESC],0,1,509,1
510=Global variable is less than or equals to,48,6,0,0,[LONG DESC],0,1,510,1
511=Global variable and X is true,48,6,0,0,[LONG DESC],0,1,511,1

[ActionsRA2]
125=Build at...,-10,47,53,0,0,0,1,0,0,[LONG DESC],0,1,125
500=Save game,-4,13,0,0,0,0,0,0,0,[LONG DESC],0,1,500,1
501=Edit variable,0,3,21,22,23,24,0,0,0,[LONG DESC],0,1,501,1

[ScriptsRA2]   ; NEEDS FA2EXT.DLL (by AlexB) or FA2SP.DLL (by secsome)
71=Timed Area Guard,4,0,1,[LONG DESC]         ; FA2Ext.dll only
71=Timed Area Guard,20,0,1,[LONG DESC]        ; FA2sp.dll only
72=Load Onto Transports,0,0,1,[LONG DESC]
73=Wait until ammo is full,0,0,1,[LONG DESC]
```

## Changelog

### 0.3

New:
- LaserTrails initial implementation (by Kerbiter, ChrisLv_CN)
- Anim-to-Unit logic and ability to randomize DestroyAnim (by Otamaa)
- Initial Strength for TechnoTypes (by Uranusian)
- Re-enable obsolete `JumpjetControls` for TechnoTypes' default Jumpjet properties (by Uranusian)
- Weapon targeting filter (by Uranusian)
- Burst-specific FLHs for TechnoTypes (by Starkku)
- Burst delays for weapons (by Starkku)
- PowerPlant Enhancer (by secsome)
- Unlimited Global / Local Variables (by secsome)
- Adds a "Load Game" button to the retry dialog on mission failure (by secsome)
- Default disguise for individual InfantryTypes (by secsome)
- Quicksave hotkey command (by secsome)
- Save Game Trigger Action (by secsome)
- Numeric Variables (by secsome)
- Allow `NotHuman=yes` infantry to use random `Death` anim sequence (by Otamaa)
- Ability for warheads to trigger specific `NotHuman=yes` infantry `Death` anim sequence (by Otamaa)
- XDrawOffset for animations (by Morton)
- Customizable OpenTopped properties (by Otamaa)
- Automatic Passenger Deletion (by FS-21)
- Script Action 74 to 81 and 84 to 91 for new AI attacks (by FS-21)
- Script Actions 82 & 83 for modifying AI Trigger Current Weight (by FS-21)
- Script Action 92 for waiting & repeat the same new AI attack if no target was found (by FS-21)
- Script Action 93 that modifies the Team's Trigger Weight when ends the new attack action (by FS-21)
- Script Action 94 for picking a random script from a list (by FS-21)
- Script Action 95 to 98 for new AI movements towards certain objects (by FS-21)
- Script Action 111 that un-register Team success, is just the opposite effect of Action 49 (by FS-21)
- Script Action 112 to regroup temporarily around the Team Leader (by FS-21)
- ObjectInfo now shows current Target and AI Trigger data (by FS-21)
- Shield absorption and passthrough customization (by Morton)

Vanilla fixes:
- Fixed laser drawing code to allow for thicker lasers in house color draw mode (by Kerbiter, ChrisLv_CN)
- Fixed DeathWeapon not detonating properly (by Uranusian)
- Fixed lasers & other effects drawing from wrong offset with weapons that use Burst (by Starkku)
- Fixed buildings with `Naval=yes` ignoring `WaterBound=no` to be forced to place onto water (by Uranusian)
- Fixed temporal weapon crash under certain conditions where stack dump starts with 0051BB7D (by secsome)

Phobos fixes:
- TBA

### 0.2.2.2

Phobos fixes:
- Fixed shield type info not saving properly (by Uranusian)
- Fixed extended building upgrades logic not properly interacting with Ares' BuildLimit check (by Uranusian)
- Fix more random crashes for Cameo Priority (by Uranusian)
- Fix aircraft weapons causing game freeze when burst index was not correctly reset after firing (by Starkku)

### 0.2.2.1

Phobos fixes:
- Fixed random crashes about CameoPriority (by Uranusian)
- Fixed trigger action 125 not functioning properly (by Uranusian)
- Fixed area warhead detonation not falling back to firer house (by Otamaa)
- RadSite hook adjustment for `FootClass` to support Ares `RadImmune`; also various fixes to radiation / desolators (by Otamaa)
- Fixed `Crit.Affects` not functioning properly (by Uranusian)
- Fixed improper upgrade owner transfer which resulted in built ally / enemy building upgrades keeping the player who built them alive (by Kerbiter)

### 0.2.2

New:
- Customizable producing progress "bars" like CnC:Remastered did (by Uranusian)
- Customizable cameo sorting priority (by Uranusian)
- Customizable harvester ore gathering animation (by secsome, Uranusian)
- Allow making technos unable to be issued with movement order (by Uranusian)

Vanilla fixes:
- Fixed non-IME keyboard input to be working correctly for languages / keyboard layouts that use character ranges other than Basic Latin and Latin-1 Supplement (by Belonit)

Phobos fixes:
- Fixed the critical damage logic not functioning properly (by Uranusian)
- Fixed the bug when executing the stop command game crashes (by Uranusian)

### 0.2.1.1

Phobos fixes:
- Fixed occasional crashes introduced by `Speed=0` stationary vehicles code (by Starkku)

### 0.2.1

New:
- Setting VehicleType `Speed` to 0 now makes game treat them as stationary (by Starkku)

Vanilla fixes:
- Fixed the bug when after a failed placement the building/defence tab hotkeys won't trigger placement mode again (by Uranusian)
- Fixed the bug when building with `UndeployInto` plays `EVA_NewRallypointEstablished` while undeploying (by secsome)

Phobos fixes:
- Fixed the bug when trigger action `125 Build At...` wasn't actually producing a building when the target cells were occupied (by secsome)

### 0.2

New:
- Shield logic for TechnoTypes (by Uranusian, secsome, Belonit) with warhead additions (by Starkku)
- Custom Radiation Types (by AlexB, Otamaa, Belonit, Uranusian)
- New ScriptType actions `71 Timed Area Guard`, `72 Load Onto Transports`, `73 Wait until ammo is full` (by FS-21)
- Ore drills now have customizable ore type, range, ore growth stage and amount of cells generated (by Kerbiter)
- Basic projectile interception logic (by AutoGavy, ChrisLv_CN, Kerbiter, Erzoid/SukaHati)
- Customizable harvester active/total counter next to credits counter (by Uranusian)
- Select Next Idle Harvester hotkey command (by Kerbiter)
- Dump Object Info hotkey command (by secsome, FS-21)
- Remove Disguise and Remove Mind Control warhead effects (by secsome)
- Custom per-warhead SplashLists (by Uranusian)
- `AnimList.PickRandom` used to randomize `AnimList` with no side effects (by secsome)
- Chance-based critical damage system on warheads (by AutoGavy)
- Optional mind control range limit (by Uranusian)
- Multiple mind controllers can now release units on overload (by Uranusian, secsome)
- Spawns now can be killed on low power and have limited pursuing range (by FS-21)
- Spawns can now have the same exp. level as owner techno (by Uranusian)
- `TurretOffset` now accepts `F,L,H` and `F,L` values instead of just `F` value (by Kerbiter)
- ElectricBolt arc visuals can now be disabled per-arc (by Otamaa)
- Semantic locomotor aliases for modder convenience (by Belonit)
- Ability to specify amount of shots for strafing aircraft and burst simulation (by Starkku)
- Customizeable Teleport/Chrono Locomotor properties per TechnoType (by Otamaa)
- Maximum waypoints amount increased from 702 to 2147483647 (by secsome)
- Customizeable Missing Cameo file (by Uranusian)

Vanilla fixes:
- Map previews with zero size won't crash the game anymore (by Kerbiter, Belonit)
- Tileset 255+ bridge fix (by E1 Elite)
- Fixed fatal errors when `Blowfish.dll` couldn't be registered in the system properly due to missing admin rights (by Belonit)
- Fix to take Burst into account for aircraft weapon shots beyond the first one (by Starkku)
- Fixed the bug when units are already dead but still in map (for sinking, crashing, dying animation, etc.), they could die again (by Uranusian)
- Fixed the bug when cloaked Desolator was unable to fire his deploy weapon (by Otamaa)
- Fixed the bug when `InfiniteMindControl` with `Damage=1` will auto-release the victim to control new one (by Uranusian)
- Fixed the bug that script action `Move to cell` was still using leftover cell calculations from previous games (by secsome)
- Fixed the bug when trigger action `125 Build At...` didn't play buildup anim (by secsome)
- Fixed `DebrisMaximums` (spawned debris type amounts cannot go beyond specified maximums anymore) (by Otamaa)
- Fixes to `DeployFire` logic (`DeployFireWeapon`, `FireOnce`, stop command now work properly) (by Starkku)

Phobos fixes:
- Properly rewritten a fix for mind-controlled vehicles deploying into buildings (by FS-21)
- Properly rewritten `DeployToFire` fix, tag `Deployed.RememberTarget` is deprecated, now always on (by Kerbiter)
- New warheads now work with Ares' `GenericWarhead` superweapon (by Belonit)

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
