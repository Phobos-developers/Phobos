# What's New

This page lists the history of changes across stable Phobos releases and also all the stuff that requires modders to change something in their mods to accomodate.

## Migrating

### From vanilla

- SHP debris hardcoded shadows now respect `Shadow=no` tag value, and due to it being the default value they wouldn't have hardcoded shadows anymore by default. Override this by specifying `Shadow=yes` for SHP debris.
- Radiation now has owner by default, which means that radiation kills will affect score and radiation field will respect `Affects...` entries. You can override that with `rulesmd.ini->[SOMEWEAPONTYPE]->Rad.NoOwner=yes` entry.

### From older Phobos versions

#### From 0.2.2.2
- Keys `rulesmd.ini->[SOMEWARHEAD]->PenetratesShield` and `rulesmd.ini->[SOMEWARHEAD]->BreaksShield` have been changed to `Shield.Penetrate` and `Shield.Break`, respectively.

#### From 0.1.1
- Key `rulesmd.ini->[SOMETECHNOTYPE]->Deployed.RememberTarget` is deprecated and can be removed now, the bugfix for `DeployToFire` deployers is now always on.

### For Map Editor (Final Alert 2)

In `FAData.ini`:
```ini
[ParamTypes]
47=Structures,28
53=Play BuildUp,10
54=Use GlobalVar,10
55=Operation,0
56=Variable index,0
57=Lower bound,0
58=Upper bound,0
59=Operate var is global,10
60=Operate var index,0

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
512=Local variable is greater than local variable,48,3,0,0,[LONG DESC],0,1,500,1
513=Local variable is less than local variable,48,3,0,0,[LONG DESC],0,1,501,1
514=Local variable equals to local variable,48,3,0,0,[LONG DESC],0,1,502,1
515=Local variable is greater than or equals to local variable,48,3,0,0,[LONG DESC],0,1,503,1
516=Local variable is less than or equals local variable,48,3,0,0,[LONG DESC],0,1,504,1
517=Local variable and local variable is true,48,3,0,0,[LONG DESC],0,1,505,1
518=Global variable is greater than local variable,48,3,0,0,[LONG DESC],0,1,506,1
519=Global variable is less than local variable,48,3,0,0,[LONG DESC],0,1,507,1
520=Global variable equals to local variable,48,3,0,0,[LONG DESC],0,1,508,1
521=Global variable is greater than or queals to local variable,48,3,0,0,[LONG DESC],0,1,509,1
522=Global variable is less than or equals to local variable,48,3,0,0,[LONG DESC],0,1,510,1
523=Global variable and local variable is true,48,3,0,0,[LONG DESC],0,1,511,1
524=Local variable is greater than global variable,48,35,0,0,[LONG DESC],0,1,500,1
525=Local variable is less than global variable,48,35,0,0,[LONG DESC],0,1,501,1
526=Local variable equals to global variable,48,35,0,0,[LONG DESC],0,1,502,1
527=Local variable is greater than or equals to global variable,48,35,0,0,[LONG DESC],0,1,503,1
528=Local variable is less than or equals global variable,48,35,0,0,[LONG DESC],0,1,504,1
529=Local variable and global variable is true,48,35,0,0,[LONG DESC],0,1,505,1
530=Global variable is greater than global variable,48,35,0,0,[LONG DESC],0,1,506,1
531=Global variable is less than global variable,48,35,0,0,[LONG DESC],0,1,507,1
532=Global variable equals to global variable,48,35,0,0,[LONG DESC],0,1,508,1
533=Global variable is greater than or queals to global variable,48,35,0,0,[LONG DESC],0,1,509,1
534=Global variable is less than or equals to global variable,48,35,0,0,[LONG DESC],0,1,510,1
535=Global variable and global variable is true,48,35,0,0,[LONG DESC],0,1,511,1

[ActionsRA2]
125=Build at...,-10,47,53,0,0,0,1,0,0,[LONG DESC],0,1,125
500=Save game,-4,13,0,0,0,0,0,0,0,[LONG DESC],0,1,500,1
501=Edit variable,0,56,55,6,54,0,0,0,0,[LONG DESC],0,1,501,1
502=Generate random number,0,56,57,58,54,0,0,0,0,[LONG DESC],0,1,502,1
503=Print variable value,0,56,54,0,0,0,0,0,0,[LONG DESC],0,1,503,0
504=Binary operation,0,56,55,60,54,59,0,0,0,[LONG DESC],0,1,504,1

; FOLLOWING STUFFS NEEDS FA2SP.DLL (by secsome)
[ScriptTypeLists]
1=ScriptLocalVariable
2=ScriptGlobalVariable
3=ScriptLocalVariable_Local
4=ScriptLocalVariable_Global
5=ScriptGlobalVariable_Local
6=ScriptGlobalVariable_Global

[ScriptLocalVariable]
HasExtraParam=Yes
BuiltInType=14

[ScriptGlobalVariable]
HasExtraParam=Yes
BuiltInType=5

[ScriptLocalVariable_Local]
HasExtraParam=Yes
ExtraParamType=ScriptExtType_LocalVariables
BuiltInType=14

[ScriptLocalVariable_Global]
HasExtraParam=Yes
ExtraParamType=ScriptExtType_GlobalVariables
BuiltInType=14

[ScriptGlobalVariable_Local]
HasExtraParam=Yes
ExtraParamType=ScriptExtType_LocalVariables
BuiltInType=5

[ScriptGlobalVariable_Global]
HasExtraParam=Yes
ExtraParamType=ScriptExtType_GlobalVariables
BuiltInType=5

[ScriptExtType_LocalVariables]
BuiltInType=14

[ScriptExtType_GlobalVariables]
BuiltInType=5

[ScriptsRA2]   
71=Timed Area Guard,20,0,1,[LONG DESC]
72=Load Onto Transports,0,0,1,[LONG DESC]
73=Wait until ammo is full,0,0,1,[LONG DESC]
500=Local variable set,22,0,1,[LONG DESC]
501=Local variable add,22,0,1,[LONG DESC]
502=Local variable minus,22,0,1,[LONG DESC]
503=Local variable multiply,22,0,1,[LONG DESC]
504=Local variable divide,22,0,1,[LONG DESC]
505=Local variable mod,22,0,1,[LONG DESC]
506=Local variable leftshift,22,0,1,[LONG DESC]
507=Local variable rightshift,22,0,1,[LONG DESC]
508=Local variable reverse,22,0,1,[LONG DESC]
509=Local variable xor,22,0,1,[LONG DESC]
510=Local variable or,22,0,1,[LONG DESC]
511=Local variable and,22,0,1,[LONG DESC]
512=Global variable set,23,0,1,[LONG DESC]
513=Global variable add,23,0,1,[LONG DESC]
514=Global variable minus,23,0,1,[LONG DESC]
515=Global variable multiply,23,0,1,[LONG DESC]
516=Global variable divide,23,0,1,[LONG DESC]
517=Global variable mod,23,0,1,[LONG DESC]
518=Global variable leftshift,23,0,1,[LONG DESC]
519=Global variable rightshift,23,0,1,[LONG DESC]
520=Global variable reverse,23,0,1,[LONG DESC]
521=Global variable xor,23,0,1,[LONG DESC]
522=Global variable or,23,0,1,[LONG DESC]
523=Global variable and,23,0,1,[LONG DESC]
524=Local variable set by local variable,24,0,1,[LONG DESC]
525=Local variable add by local variable,24,0,1,[LONG DESC]
526=Local variable minus by local variable,24,0,1,[LONG DESC]
527=Local variable multiply by local variable,24,0,1,[LONG DESC]
528=Local variable divide by local variable,24,0,1,[LONG DESC]
529=Local variable mod by local variable,24,0,1,[LONG DESC]
530=Local variable leftshift by local variable,24,0,1,[LONG DESC]
531=Local variable rightshift by local variable,24,0,1,[LONG DESC]
532=Local variable reverse by local variable,24,0,1,[LONG DESC]
533=Local variable xor by local variable,24,0,1,[LONG DESC]
534=Local variable or by local variable,24,0,1,[LONG DESC]
535=Local variable and by local variable,24,0,1,[LONG DESC]
536=Global variable set by local variable,25,0,1,[LONG DESC]
537=Global variable add by local variable,25,0,1,[LONG DESC]
538=Global variable minus by local variable,25,0,1,[LONG DESC]
539=Global variable multiply by local variable,25,0,1,[LONG DESC]
540=Global variable divide by local variable,25,0,1,[LONG DESC]
541=Global variable mod by local variable,25,0,1,[LONG DESC]
542=Global variable leftshift by local variable,25,0,1,[LONG DESC]
543=Global variable rightshift by local variable,25,0,1,[LONG DESC]
544=Global variable reverse by local variable,25,0,1,[LONG DESC]
545=Global variable xor by local variable,25,0,1,[LONG DESC]
546=Global variable or by local variable,25,0,1,[LONG DESC]
547=Global variable and by local variable,25,0,1,[LONG DESC]
548=Local variable set by global variable,26,0,1,[LONG DESC]
549=Local variable add by global variable,26,0,1,[LONG DESC]
550=Local variable minus by global variable,26,0,1,[LONG DESC]
551=Local variable multiply by global variable,26,0,1,[LONG DESC]
552=Local variable divide by global variable,26,0,1,[LONG DESC]
553=Local variable mod by global variable,26,0,1,[LONG DESC]
554=Local variable leftshift by global variable,26,0,1,[LONG DESC]
555=Local variable rightshift by global variable,26,0,1,[LONG DESC]
556=Local variable reverse by global variable,26,0,1,[LONG DESC]
557=Local variable xor by global variable,26,0,1,[LONG DESC]
558=Local variable or by global variable,26,0,1,[LONG DESC]
559=Local variable and by global variable,26,0,1,[LONG DESC]
560=Global variable set by global variable,27,0,1,[LONG DESC]
561=Global variable add by global variable,27,0,1,[LONG DESC]
562=Global variable minus by global variable,27,0,1,[LONG DESC]
563=Global variable multiply by global variable,27,0,1,[LONG DESC]
564=Global variable divide by global variable,27,0,1,[LONG DESC]
565=Global variable mod by global variable,27,0,1,[LONG DESC]
566=Global variable leftshift by global variable,27,0,1,[LONG DESC]
567=Global variable rightshift by global variable,27,0,1,[LONG DESC]
568=Global variable reverse by global variable,27,0,1,[LONG DESC]
569=Global variable xor by global variable,27,0,1,[LONG DESC]
570=Global variable or by global variable,27,0,1,[LONG DESC]
571=Global variable and by global variable,27,0,1,[LONG DESC]

[ScriptParams] 
22=Local variables,-1
23=Global variables,-2
24=Local variables,-3
25=Local variables,-4
26=Global variables,-5
27=Global variables,-6
```

## Changelog

### 0.3

New:
- LaserTrails initial implementation (by Kerbiter, ChrisLv_CN)
- Anim-to-Unit logic and ability to randomize DestroyAnim (by Otamaa)
- Shield modification warheads (by Starkku)
- Shield BreakWeapon & InitialStrength (by Starkku)
- Initial Strength for TechnoTypes (by Uranusian)
- Re-enable obsolete `JumpjetControls` for TechnoTypes' default Jumpjet properties (by Uranusian)
- Weapon targeting filter (by Uranusian, Starkku)
- Secondary weapon fallback customization (by Starkku)
- Burst-specific FLHs for TechnoTypes (by Starkku)
- Burst delays for weapons (by Starkku)
- AreaFire weapon target customization (by Starkku)
- Auto-firing TechnoType weapons (by Starkku)
- PowerPlant Enhancer (by secsome)
- Unlimited Global / Local Variables (by secsome)
- Adds a "Load Game" button to the retry dialog on mission failure (by secsome)
- Default disguise for individual InfantryTypes (by secsome)
- Quicksave hotkey command (by secsome)
- Save Game Trigger Action (by secsome)
- Numeric Variables (by secsome)
- TechnoType's tooltip would display it's build time now (by secsome) 
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
- Limbo Delivery of buildings (by Morton)
- Ore stage threshold for `HideIfNoOre` (by Otamaa)
- Image reading in art rules for all TechnoTypes (by Morton)
- Attached animation layer customization (by Starkku)
- Jumpjet unit layer deviation customization (by Starkku)
- IsSimpleDeployer deploy direction & animation customizations (by Starkku)
- Customizable projectile gravity (by secsome)
- Gates can now link with walls correctly via `NSGates` or `EWGates` (by Uranusian)
- Per-warhead toggle for decloak of damaged targets (by Starkku)
- `DeployFireWeapon=-1` now allows the deployed infantries using both weapons as undeployed (by Uranusian)
- Power delta (surplus) counter for sidebar (by Morton)

Vanilla fixes:
- Fixed laser drawing code to allow for thicker lasers in house color draw mode (by Kerbiter, ChrisLv_CN)
- Fixed DeathWeapon not detonating properly (by Uranusian)
- Fixed lasers & other effects drawing from wrong offset with weapons that use Burst (by Starkku)
- Fixed buildings with `Naval=yes` ignoring `WaterBound=no` to be forced to place onto water (by Uranusian)
- Fixed temporal weapon crash under certain conditions where stack dump starts with 0051BB7D (by secsome)
- Fixed the bug when retinting map lighting with a map action corrupted light sources (by secsome)
- Fixed the bug that AITriggerTypes do not recognize building upgrades (by Uranusian)
- Fixed the bug when occupied building's `MuzzleFlashX` is drawn on the center of the building when `X` goes past 10 (by Otamaa)

Phobos fixes:
- Fixed shields being able to take damage when the parent TechnoType was under effects of a `Temporal` Warhead (by Starkku)

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
