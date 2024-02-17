# What's New

This page lists the history of changes across stable Phobos releases and also all the stuff that requires modders to change something in their mods to accomodate.

## Migrating

```{hint}
You can use the migration utility (can be found on [Phobos supplementaries repo](https://github.com/Phobos-developers/PhobosSupplementaries)) to apply most of the changes automatically using a corresponding sed script file.
```

### From vanilla

- Translucent RLE SHPs will now be drawn using a more precise and performant algorithm that has no green tint and banding. Can be disabled with `rulesmd.ini->[General]->FixTransparencyBlitters=no`.
- Iron Curtain status is now preserved by default when converting between TechnoTypes via `DeploysInto`/`UndeploysInto`. This behavior can be turned off per-TechnoType and global basis using `[SOMETECHNOTYPE]/[CombatDamage]->IronCurtain.KeptOnDeploy=no`.

### From older Phobos versions

#### From 0.3

- `Shadow` for debris & meteor animations is changed to `ExtraShadow`.

#### From pre-0.3 devbuilds

- `Trajectory.Speed` is now defined on projectile instead of weapon.
- `Gravity=0` is not supported anymore as it will cause the projectile to fly backwards and be unable to hit the target which is not at the same height. Use `Straight` Trajectory instead. See [here](New-or-Enhanced-Logics.md#projectile-trajectories).
- Automatic self-destruction logic logic has been reimplemented, `Death.NoAmmo`, `Death.Countdown` and `Death.Peaceful` tags have been remade/renamed and require adjustments to function.
- `DetachedFromOwner` on weapons is deprecated. This has been replaced by `AllowDamageOnSelf` on warheads.
- Timed jump script actions now take the time measured in ingame seconds instead of frames. Divide your value by 15 to accomodate to this change.
- [Placement Preview](User-Interface.md#placement-preview) logic has been adjusted, `BuildingPlacementPreview.DefaultTranslucentLevel`, `BuildingPlacementGrid.TranslucentLevel`, `PlacementPreview.Show`, `PlacementPreview.TranslucentLevel` and `ShowBuildingPlacementPreview` tags have been remade/renamed and require adjustments to function. In addition, you must explicitly enable this feature by specifying `[AudioVisual]->PlacementPreview=yes`.
- Existing script actions were renumbered, please use the migration utility to change the numbers to the correct ones.
- `DiskLaser.Radius` values were misinterpreted by a factor of 1/2π. The default radius is now 240, please multiply your customized radii by 2π.

#### From 0.2.2.2

- Keys `rulesmd.ini->[SOMEWARHEAD]->PenetratesShield` and `rulesmd.ini->[SOMEWARHEAD]->BreaksShield` have been changed to `Shield.Penetrate` and `Shield.Break`, respectively.
- `Rad.NoOwner` on weapons is deprecated. This has been replaced by `RadHasOwner` key on radiation types itself. It also defaults to no, so radiation once again has no owner house by default.
- `RadApplicationDelay` and `RadApplicationDelay.Building` on custom radiation types are now only used if `[Radiation]` -> `UseGlobalRadApplicationDelay` is explicitly set to false, otherwise values from `[Radiation]` are used.
- Existing script actions were renumbered, please use the migration utility to change the numbers to the correct ones.

#### From 0.1.1

- Key `rulesmd.ini->[SOMETECHNOTYPE]->Deployed.RememberTarget` is deprecated and can be removed now, the bugfix for `DeployToFire` deployers is now always on.

### For Map Editor (Final Alert 2)

<details>
  <summary>Click to show</summary>

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
  600=Shield of the attached object is broken,0,0,0,0,[LONG DESC],0,1,600,1

  [ActionsRA2]
  125=Build at...,-10,47,53,0,0,0,1,0,0,[LONG DESC],0,1,125
  500=Save game,-4,13,0,0,0,0,0,0,0,[LONG DESC],0,1,500,1
  501=Edit variable,0,56,55,6,54,0,0,0,0,[LONG DESC],0,1,501,1
  502=Generate random number,0,56,57,58,54,0,0,0,0,[LONG DESC],0,1,502,1
  503=Print variable value,0,56,54,0,0,0,0,0,0,[LONG DESC],0,1,503,0
  504=Binary operation,0,56,55,60,54,59,0,0,0,[LONG DESC],0,1,504,1
  505=Fire Super Weapon at specified location (Phobos),0,0,20,2,21,22,0,0,0,Launch a Super Weapon from [SuperWeaponTypes] list at a specified location. House=-1 means random target that isn't neutral. House=-2 means the first neutral house. House=-3 means random human target. Coordinate X=-1 means random. Coordinate Y=-1 means random,0,1,505
  506=Fire Super Weapon at specified waypoint (Phobos),0,0,20,2,30,0,0,0,0,Launch a Super Weapon from [SuperWeaponTypes] list at a specified waypoint. House=-1 means random target that isn't neutral. House=-2 means the first neutral house. House=-3 means random human target. Coordinate X=-1 means random. Coordinate Y=-1 means random,0,1,506

  ; FOLLOWING ENTRIES REQUIRE FA2SP.DLL (by secsome)
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
  10100=Timed Area Guard,20,0,1,[LONG DESC]
  10103=Load Onto Transports,0,0,1,[LONG DESC]
  10101=Wait until ammo is full,0,0,1,[LONG DESC]
  18000=Local variable set,22,0,1,[LONG DESC]
  18001=Local variable add,22,0,1,[LONG DESC]
  18002=Local variable minus,22,0,1,[LONG DESC]
  18003=Local variable multiply,22,0,1,[LONG DESC]
  18004=Local variable divide,22,0,1,[LONG DESC]
  18005=Local variable mod,22,0,1,[LONG DESC]
  18006=Local variable leftshift,22,0,1,[LONG DESC]
  18007=Local variable rightshift,22,0,1,[LONG DESC]
  18008=Local variable reverse,22,0,1,[LONG DESC]
  18009=Local variable xor,22,0,1,[LONG DESC]
  18010=Local variable or,22,0,1,[LONG DESC]
  18011=Local variable and,22,0,1,[LONG DESC]
  18012=Global variable set,23,0,1,[LONG DESC]
  18013=Global variable add,23,0,1,[LONG DESC]
  18014=Global variable minus,23,0,1,[LONG DESC]
  18015=Global variable multiply,23,0,1,[LONG DESC]
  18016=Global variable divide,23,0,1,[LONG DESC]
  18017=Global variable mod,23,0,1,[LONG DESC]
  18018=Global variable leftshift,23,0,1,[LONG DESC]
  18019=Global variable rightshift,23,0,1,[LONG DESC]
  18020=Global variable reverse,23,0,1,[LONG DESC]
  18021=Global variable xor,23,0,1,[LONG DESC]
  18022=Global variable or,23,0,1,[LONG DESC]
  18023=Global variable and,23,0,1,[LONG DESC]
  18024=Local variable set by local variable,24,0,1,[LONG DESC]
  18025=Local variable add by local variable,24,0,1,[LONG DESC]
  18026=Local variable minus by local variable,24,0,1,[LONG DESC]
  18027=Local variable multiply by local variable,24,0,1,[LONG DESC]
  18028=Local variable divide by local variable,24,0,1,[LONG DESC]
  18029=Local variable mod by local variable,24,0,1,[LONG DESC]
  18030=Local variable leftshift by local variable,24,0,1,[LONG DESC]
  18031=Local variable rightshift by local variable,24,0,1,[LONG DESC]
  18032=Local variable reverse by local variable,24,0,1,[LONG DESC]
  18033=Local variable xor by local variable,24,0,1,[LONG DESC]
  18034=Local variable or by local variable,24,0,1,[LONG DESC]
  18035=Local variable and by local variable,24,0,1,[LONG DESC]
  18036=Global variable set by local variable,25,0,1,[LONG DESC]
  18037=Global variable add by local variable,25,0,1,[LONG DESC]
  18038=Global variable minus by local variable,25,0,1,[LONG DESC]
  18039=Global variable multiply by local variable,25,0,1,[LONG DESC]
  18040=Global variable divide by local variable,25,0,1,[LONG DESC]
  18041=Global variable mod by local variable,25,0,1,[LONG DESC]
  18042=Global variable leftshift by local variable,25,0,1,[LONG DESC]
  18043=Global variable rightshift by local variable,25,0,1,[LONG DESC]
  18044=Global variable reverse by local variable,25,0,1,[LONG DESC]
  18045=Global variable xor by local variable,25,0,1,[LONG DESC]
  18046=Global variable or by local variable,25,0,1,[LONG DESC]
  18047=Global variable and by local variable,25,0,1,[LONG DESC]
  18048=Local variable set by global variable,26,0,1,[LONG DESC]
  18049=Local variable add by global variable,26,0,1,[LONG DESC]
  18050=Local variable minus by global variable,26,0,1,[LONG DESC]
  18051=Local variable multiply by global variable,26,0,1,[LONG DESC]
  18052=Local variable divide by global variable,26,0,1,[LONG DESC]
  18053=Local variable mod by global variable,26,0,1,[LONG DESC]
  18054=Local variable leftshift by global variable,26,0,1,[LONG DESC]
  18055=Local variable rightshift by global variable,26,0,1,[LONG DESC]
  18056=Local variable reverse by global variable,26,0,1,[LONG DESC]
  18057=Local variable xor by global variable,26,0,1,[LONG DESC]
  18058=Local variable or by global variable,26,0,1,[LONG DESC]
  18059=Local variable and by global variable,26,0,1,[LONG DESC]
  18060=Global variable set by global variable,27,0,1,[LONG DESC]
  18061=Global variable add by global variable,27,0,1,[LONG DESC]
  18062=Global variable minus by global variable,27,0,1,[LONG DESC]
  18063=Global variable multiply by global variable,27,0,1,[LONG DESC]
  18064=Global variable divide by global variable,27,0,1,[LONG DESC]
  18065=Global variable mod by global variable,27,0,1,[LONG DESC]
  18066=Global variable leftshift by global variable,27,0,1,[LONG DESC]
  18067=Global variable rightshift by global variable,27,0,1,[LONG DESC]
  18068=Global variable reverse by global variable,27,0,1,[LONG DESC]
  18069=Global variable xor by global variable,27,0,1,[LONG DESC]
  18070=Global variable or by global variable,27,0,1,[LONG DESC]
  18071=Global variable and by global variable,27,0,1,[LONG DESC]

  [ScriptParams]
  22=Local variables,-1
  23=Global variables,-2
  24=Local variables,-3
  25=Local variables,-4
  26=Global variables,-5
  27=Global variables,-6
  ```
</details>

## Changelog

### 0.3.1.0

<details open>
  <summary>Click to show</summary>

New:
- Option to center pause menu background (by Starkku)
- In addition to `PlacementGrid.Translucency`, allow to set the transparency of the grid when PlacementPreview is enabled, using the `PlacementGrid.TranslucencyWithPreview` tag (by Belonit).

Phobos fixes:
- Fixed `Interceptor` not resetting target if the intercepted projectile changes type to non-interceptable one afterwards (by Starkku)
- Fixed a potential crash caused by a faulty hook in weapon selection code (by Starkku)
- Fixed `PlacementPreview` setting for BuildingTypes not being parsed from INI (by Starkku)
- Optimized performance for map trigger retint action light source fix (by Starkku)
- Fixed owned `LimboDelivery` buildings not being saved correctly in savegames (by Starkku)
- Fixed a typo in weapon selector code causing issues with `NoAmmoWeapon` and related checks (by Starkku)

Vanilla fixes:
- Fixed position and layer of info tip and reveal production cameo on selected building (by Belonit)
- Fixed a glitch related to incorrect target setting for missiles (by Belonit)
- Skipped parsing `[Header]` section of compaign maps which led to occasional crashes on Linux (by Trsdy)
- Fixed teleport units' frozen-still timer being reset after load game (by Trsdy)

Fixes / interactions with other extensions:
- Fixed an issue introduced by Ares that caused `Grinding=true` building `ActiveAnim` to be incorrectly restored while `SpecialAnim` was playing and the building was sold, erased or destroyed (by Starkku)
</details>

### 0.3.0.1

<details>
  <summary>Click to show</summary>

New:
- Additional sync logging in case of desync errors occuring (by Starkku)

Phobos fixes:
- `AutoDeath` support for objects in limbo (by Trsdy)
- Buildings sold by `AutoDeath` no longer play a click sound effect (by Trsdy)
- Fixed shield animation being hidden while underground or in tunnels fix not working correctly (by Starkku)
- Restore the `MindClearedSound` when deploying a mind-controlled unit into a building loses the mind-control (by Trsdy)
- Fixed `RadSiteWarhead.Detonate` not detonating precisely on the affected object (thus requiring `CellSpread`) (by Starkku)
- Fixed script action 10103 'Load Into Transports' unintentionally skipping next action (by FS-21)
- Changed mission retry dialog button order to better match old order people are used to (by Trsdy)
- Allow PowerPlant Enhancer to be affected by EMP (by Trsdy)
- Animation `Weapon` with `Damage.DealtByInvoker=true` now uses the invoker's house to deal damage and apply Phobos warhead effects even if invoker is dead when weapon is fired (by Starkku)
- Fixed a crash when trying to create radiation outside map bounds (by Otamaa)
- Fixed new AI attack scripts not allowing zero damage weapons to pick targets (by Starkku)
- Fixed floating point value parsing precision to match the game (by Starkku)
- Power output / drain should now correctly be applied for buildings created via `LimboDelivery` in campaigns (by Starkku)
- Fixed shield health bar showing empty bar when shield is still on very low health instead of depleted (by Starkku)
- Fixed `CanTarget` not considering objects on bridges when checking if cell is empty (by Starkku)
- Fixed vehicle deploy weapons not working if the unit is cloaked and weapon has `DecloakToFire=true` (by NetsuNegi & Starkku)
- Fixed `IsAnimated` terrain not updating correctly in all circumstances (by Starkku)
- Fixed `CreateUnit` interaction with bridges (spawning under when shouldn't etc) (by Starkku)
- `CanTarget` now considers bridges as land like game's normal weapon selection does (by Starkku)
- `AreaFire.Target` now takes cells with bridges into consideration depending on firer's elevation (by Starkku)
</details>

### 0.3

<details>
  <summary>Click to show</summary>

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
- Save Game trigger action (by secsome)
- Numeric Variables (by secsome)
- TechnoType's tooltip would display it's build time now (by secsome)
- Customizable tooltip background color and opacity (by secsome)
- FrameByFrame & FrameStep hotkey command (by secsome)
- Allow `NotHuman=yes` infantry to use random `Death` anim sequence (by Otamaa)
- Ability for warheads to trigger specific `NotHuman=yes` infantry `Death` anim sequence (by Otamaa)
- XDrawOffset for animations (by Morton)
- Customizable OpenTopped properties (by Otamaa)
- Automatic Passenger Deletion (by FS-21)
- Script actions for new AI attacks (by FS-21)
- Script actions for modifying AI Trigger Current Weight (by FS-21)
- Script action for waiting & repeat the same new AI attack if no target was found (by FS-21)
- Script action that modifies the Team's Trigger Weight when ends the new attack action (by FS-21)
- Script action for picking a random script from a list (by FS-21)
- Script action for new AI movements towards certain objects (by FS-21)
- Script action that modify target distance in the new move actions (by FS-21)
- Script action that modify how ends the new move actions (by FS-21)
- Script action that un-register Team success (by FS-21)
- Script action to regroup temporarily around the Team Leader (by FS-21)
- Script action to randomly skip next action (by FS-21)
- Script action for timed script action jumps (by FS-21)
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
- Added Production and Money to Dump Object Info command (by FS-21)
- `EnemyUIName=` Now also works for other TechnoTypes (by Otamaa)
- `DestroyAnim` & `DestroySound` for TerrainTypes (by Otamaa)
- Weapons fired on warping in / out (by Starkku)
- `Storage.TiberiumIndex` for customizing resource storage in structures (by FS-21)
- Grinder improvements & customizations (by Starkku)
- Attached animation position customization (by Starkku)
- Trigger Action 505 for Firing SW at specified location (by FS-21)
- Trigger Action 506 for Firing SW at waypoint (by FS-21)
- New behaviors for objects' self-destruction under certain conditions (by Trsdy & FS-21)
- Slaves' ownership decision when corresponding slave miner is destroyed (by Trsdy)
- Customize buildings' selling sound and EVA voice (by Trsdy)
- `ForceWeapon.Naval.Decloacked` for overriding uncloaked underwater attack behavior (by FS-21)
- Shrapnel enhancement (by secsome)
- Shared Ammo for transports to passengers (by FS-21)
- Additional critical hit logic customizations (by Starkku)
- Laser trails for VoxelAnims (by Otamaa)
- Local warhead screen shaking (by Starkku)
- Feedback weapon (by Starkku)
- TerrainType & ore minimap color customization (by Starkku)
- Single-color weapon lasers (by Starkku)
- Customizable projectile trajectory (by secsome)
- Display damage numbers debug hotkey command (by Starkku)
- Toggleable display of TransactMoney amounts (by Starkku)
- Building-provided self-healing customization (by Starkku)
- Building placement preview (by Otamaa & Belonit)
- Passable & buildable-upon TerrainTypes (by Starkku)
- Toggle for passengers to automatically change owner if transport owner changes (by Starkku)
- Superweapon launch on warhead detonation (by Trsdy)
- Preserve IronCurtain status upon DeploysInto/UndeploysInto (by Trsdy)
- Correct owner house for Warhead Anim/SplashList & Play Animation trigger animations (by Starkku)
- Customizable FLH When Infantry Is Crouched Or Deployed (by FS-21)
- Enhanced projectile interception logic, including projectile strength & armor types (by Starkku)
- Initial Strength for Cloned Infantry (by FS-21)
- OpenTopped transport rangefinding & deactivated state customizations (by Starkku)
- Forbidding parallel AI queues by type (by NetsuNegi & Trsdy)
- Animation damage / weapon improvements (by Starkku)
- Warhead self-damaging toggle (by Starkku)
- Trailer animations inheriting owner (by Starkku)
- Warhead detonation on all objects on map (by Starkku)
- Implemented support for PCX images for campaign loading screen (by FlyStar)
- Implemented support for PCX images for observer loading screen (by Uranusian)
- Animated (non-tiberium spawning) TerrainTypes (by Starkku)
- Toggleable passenger killing for Explodes=true units (by Starkku)

Vanilla fixes:
- Fixed laser drawing code to allow for thicker lasers in house color draw mode (by Kerbiter, ChrisLv_CN)
- Fixed DeathWeapon not detonating properly (by Uranusian)
- Fixed lasers & other effects drawing from wrong offset with weapons that use Burst (by Starkku)
- Fixed buildings with `Naval=yes` ignoring `WaterBound=no` to be forced to place onto water (by Uranusian)
- Fixed temporal weapon crash under certain conditions where stack dump starts with 0051BB7D (by secsome)
- Fixed the bug when retinting map lighting with a map action corrupted light sources (by secsome)
- Fixed the bug when reading a map which puts `Preview(Pack)` after `Map` lead to the game fail to draw the preview (by secsome)
- Fixed the bug that GameModeOptions are not correctly saved (by secsome)
- Fixed the bug that AITriggerTypes do not recognize building upgrades (by Uranusian)
- Fixed AI Aircraft docks bug when Ares tag `[GlobalControls]` > `AllowParallelAIQueues=no` is set (by FS-21)
- Fixed the bug when occupied building's `MuzzleFlashX` is drawn on the center of the building when `X` goes past 10 (by Otamaa)
- Fixed jumpjet units that are `Crashable` not crashing to ground properly if destroyed while being pulled by a `Locomotor` warhead (by Starkku)
- Fixed aircraft & jumpjet units not being affected by speed modifiers (by Starkku)
- Fixed vehicles (both voxel & SHP) to fully respect `Palette` (by Starkku)
- Fixed mind control indicator animations not reappearing on mind controlled objects that are cloaked and then uncloaked (by Starkku)
- Fixed Nuke carrier and payload weapons not respecting `Bright` setting on weapon (by Starkku)
- Fixed buildings not reverting to undamaged graphics when HP was restored above `[AudioVisual]`->`ConditionYellow` via `SelfHealing` (by Starkku)
- Fixed jumpjet units being unable to turn to the target when firing from a different direction (by Trsdy)
- Fixed turreted jumpjet units always facing bottom-right direction when stop moving (by Trsdy)
- Fixed jumpjet objects being unable to detect cloaked objects beneath (by Trsdy)
- Anim owner is now set for warhead AnimList/SplashList anims and Play Anim at Waypoint trigger animations (by Starkku)
- Fixed AI script action Deploy getting stuck with vehicles with `DeploysInto` if there was no space to deploy at initial location (by Starkku)
- Fixed `Foundation=0x0` causing crashes if used on TerrainTypes.
- Projectiles now remember the house of the firer even if the firer is destroyed before the projectile detonates. Does not currently apply to Ares-introduced Warhead effects (by Starkku)
- Buildings now correctly use laser parameters set for Secondary weapons instead of reading them from Primary weapon (by Starkku)
- Fixed an issue that caused vehicles killed by damage dealt by a known house but without a known source TechnoType (f.ex animation warhead damage) to not be recorded as killed correctly and thus not spring map trigger events etc. (by Starkku)
- Translucent RLE SHPs will now be drawn using a more precise and performant algorithm that has no green tint and banding (only applies to Z-aware drawing mode for now) (by Apollo)
- Fixed transports recursively put into each other not having a correct killer set after second transport when being killed by something (by Kerbiter)
- Fixed projectiles with `Inviso=true` suffering from potential inaccuracy problems if combined with `Airburst=yes` or Warhead with `EMEffect=true` (by Starkku)
- Fixed the bug when `MakeInfantry` logic on BombClass resulted in `Neutral` side infantry (by Otamaa)
- Fixed railgun particles being drawn to wrong coordinate against buildings with non-default `TargetCoordOffset` or when force-firing on bridges (by Starkku)
- Fixed building `TargetCoordOffset` not being taken into accord for several things like fire angle calculations and target lines (by Starkku)
- Allowed observers to see a selected building's radial indicator (by Trsdy)

Phobos fixes:
- Fixed shields being able to take damage when the parent TechnoType was under effects of a `Temporal` Warhead (by Starkku)
- Improved shield behavior for forced damage (by Uranusian)
- Fixed SplashList animations playing when a unit is hit on a bridge over water (by Uranusian)
- Fixed shielded objects not decloaking if shield takes damage (by Starkku)
- Fixed critical hit animation playing even if no critical hits were dealt due to `Crit.Affects` or `ImmuneToCrit` settings (by Starkku)
- Fixed `RemoveDisguise` not working on `PermaDisguise` infantry (by Starkku)
- Fixed single-color laser (IsHouseColor, IsSingleColor, LaserTrails) glow falloff to match the vanilla appearance (by Starkku)
- Fixed a potential cause of crashes concerning shield animations (such in conjunction with cloaking) (by Starkku)
- Fixed interceptors intercepting projectiles fired by friendly objects if the said object died after firing the projectile (by Starkku)
- Fixed interceptor weapons with `Inviso=true` projectiles detonating the projectile at wrong coordinates (by Starkku)
- Fixed some possible configuration reading issues when using Phobos with patches that rename `uimd.ini` (by Belonit)
- Fixed a game crash when using the Map Snapshot command (by Otamaa)
- Fixed issue with incorrect input in edit dialog element when using IME (by Belonit)
- Fixed an issue where tooltip text could be clipped by tooltip rectangle border if using `MaxWidth` > 0 (by Starkku)
- Fixed projectiles with `Trajectory=Straight` suffering from potential inaccuracy problems if combined with `Airburst=yes` or Warhead with `EMEffect=true` (by Starkku)
- Minor performance optimization related to shields (by Trsdy)
- Fixed teleporting miners (Chrono Miner) considered to be idle by harvester counter, improved related game performance (by Trsdy)
- Fixed negative damage weapons considering shield health when evaluating targets even if Warhead had `Shield.Penetrate` set to true (by Starkku)
- Fixed engineers considering shield health instead of building health when determining if they can repair or capture a building (by Starkku)
- Fixed shield animations (`IdleAnim`, `BreakAnim` and `HitAnim`) showing up even if the object shield is attached to is currently underground (by Starkku)
- Fixed shields not being removed from sinking units until they have fully finished sinking (by Starkku)
- Fixed Phobos Warhead effects (crits, new shield modifiers etc.) considering sinking units valid targets (by Starkku)
- Fixed an issue where `FireOnce=yes` deploy weapons on vehicles would still fire multiple times if deploy command is issued repeatedly or when not idle (by Starkku)
- Fixed techno-extdata update after type conversion (by Trsdy)
- Fixed a game crash when checking BuildLimit if Phobos is running without Ares (by Belonit)
- Corrected the misinterpretation in the definition of `DiskLaser.Radius` (by Trsdy)

Non-DLL:
- Implemented a tool (sed wrapper) to semi-automatically upgrade INIs to use latest Phobos tags (by Kerbiter)
</details>


### 0.2.2.2

<details>
  <summary>Click to show</summary>

Phobos fixes:
- Fixed shield type info not saving properly (by Uranusian)
- Fixed extended building upgrades logic not properly interacting with Ares' BuildLimit check (by Uranusian)
- Fix more random crashes for Cameo Priority (by Uranusian)
- Fix aircraft weapons causing game freeze when burst index was not correctly reset after firing (by Starkku)
</details>


### 0.2.2.1

<details>
  <summary>Click to show</summary>

Phobos fixes:
- Fixed random crashes about CameoPriority (by Uranusian)
- Fixed trigger action 125 not functioning properly (by Uranusian)
- Fixed area warhead detonation not falling back to firer house (by Otamaa)
- RadSite hook adjustment for `FootClass` to support Ares `RadImmune`; also various fixes to radiation / desolators (by Otamaa)
- Fixed `Crit.Affects` not functioning properly (by Uranusian)
- Fixed improper upgrade owner transfer which resulted in built ally / enemy building upgrades keeping the player who built them alive (by Kerbiter)
</details>


### 0.2.2

<details>
  <summary>Click to show</summary>

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
</details>


### 0.2.1.1

<details>
  <summary>Click to show</summary>

Phobos fixes:
- Fixed occasional crashes introduced by `Speed=0` stationary vehicles code (by Starkku)
</details>


### 0.2.1

<details>
  <summary>Click to show</summary>

New:
- Setting VehicleType `Speed` to 0 now makes game treat them as stationary (by Starkku)

Vanilla fixes:
- Fixed the bug when after a failed placement the building/defence tab hotkeys won't trigger placement mode again (by Uranusian)
- Fixed the bug when building with `UndeployInto` plays `EVA_NewRallypointEstablished` while undeploying (by secsome)

Phobos fixes:
- Fixed the bug when trigger action `125 Build At...` wasn't actually producing a building when the target cells were occupied (by secsome)
</details>


### 0.2

<details>
  <summary>Click to show</summary>

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
</details>


### 0.1.1

<details>
  <summary>Click to show</summary>

Phobos fixes:
- Fixed an occasional crash when selecting units with a selection box
</details>


### 0.1

<details>
<summary>Click to show</summary>

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
</details>
