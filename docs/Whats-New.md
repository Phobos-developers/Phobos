# What's New

This page lists the history of changes across stable Phobos releases and also all the stuff that requires modders to change something in their mods to accommodate.

## Migrating

```{hint}
You can use the migration utility (can be found on [Phobos supplementaries repo](https://github.com/Phobos-developers/PhobosSupplementaries)) to apply most of the changes automatically using a corresponding sed script file.
```

### From vanilla

- `PowersUpNAnim` is now used instead of the upgrade building's image file for upgrade animation if set. Note that displaying a damaged version will still require setting `PowerUp1DamagedAnim` explicitly in all cases, as the fallback to upgrade building image does not extend to it, nor would it be safe to add.
- `[CrateRules]` -> `FreeMCV` now controls whether or not player is forced to receive unit from `[General]` -> `BaseUnit` from goodie crate if they own no buildings or any existing `BaseUnit` vehicles and own more than `[CrateRules]` -> `FreeMCV.CreditsThreshold` (defaults to 1500) credits.
- Translucent RLE SHPs will now be drawn using a more precise and performant algorithm that has no green tint and banding. Can be disabled with `rulesmd.ini->[General]->FixTransparencyBlitters=no`.
- Iron Curtain status is now preserved by default when converting between TechnoTypes via `DeploysInto`/`UndeploysInto`. This behavior can be turned off per-TechnoType and global basis using `[SOMETECHNOTYPE]/[CombatDamage]->IronCurtain.KeptOnDeploy=no`.
- The obsolete `[General]` -> `WarpIn` has been enabled for the default anim type when technos are warping in. If you want to restore the vanilla behavior, use the same anim type as `WarpOut`.
- Vehicles with `Crusher=true` + `OmniCrusher=true` / `MovementZone=CrusherAll` were hardcoded to tilt when crushing vehicles / walls respectively. This now obeys `TiltsWhenCrushes` but can be customized individually for these two scenarios using `TiltsWhenCrusher.Vehicles` and `TiltsWhenCrusher.Overlays`, which both default to `TiltsWhenCrushes`.

### From older Phobos versions

#### From post-0.3 devbuilds

- Lunar theater tileset parsing unhardcoding is now only applied if `lunarmd.ini` has `[General]` -> `ApplyLunarFixes` set to true.
- `Units.DisableRepairCost` was changed to `Units.UseRepairCost` (note inverted expected value) as it no longer has discrete default value and affects `Hospital=true` buildings, infantry do not have repair cost by default.
- Critical hit animations created by `Crit.AnimOnAffectedTargets=true` Warheads no longer default to `AnimList.PickRandom` if `Crit.AnimList.PickRandom` is not set.
- `SelfHealGainType` value `none` has been changed to `noheal` due to `none` being treated as a blank string and not parsed by the game.
- Affected target enum (`CanTarget`, `Crit.Affects` et al) now considers buildings considered vehicles (`ConsideredVehicle=true` or not set in conjunction with `UndeploysInto` & 1x1 foundation) as units instead of buildings.
- If `CreateUnit.AlwaysSpawnOnGround` is set to false, jumpjet vehicles created will now automatically take off instead of staying on ground. Set to true to force spawn on ground.
- Digital display `Offset` and `Offset.ShieldDelta` Y-axis coordinates now work in inverted fashion (negative goes up, positive goes down) to be consistent with how pixel offsets work elsewhere in the game.
- Phobos Warhead effects combined with `CellSpread` now correctly apply to buildings if any of the foundation cells are hit instead of only the top-left most cell (cell #0).
- `ExtraWarheads.DamageOverrides` now falls back to last listed value if list is shorter than `ExtraWarheads` for all Warhead detonations exceeding the length.
- Air and Top layer contents are no longer sorted, animations in these layers no longer respect `YSortAdjust`. Animations attached to flying units now get their layer updated immediately after parent unit, if they are on same layer they will draw above the parent unit.
- `AnimList.ShowOnZeroDamage` has been renamed to `CreateAnimsOnZeroDamage` to make it more clear it applies to both `AnimList` and splash animations.
- INI inclusion and inheritance are now turned off by default and need to be turned on via command line flags `-Include` and `-Inheritance`.
- `Level=true` projectiles no longer attempt to do reposition against targets that are behind non-water tiles by default. Use `SubjectToLand=true` to re-enable this behaviour.

#### From 0.3

- Phobos-introduced Warhead effects like shield modifiers, critical hits, disguise & mind control removal now require Warhead `Verses` to affect target to apply unless `EffectsRequireVerses` is set to false. Shield armor type is used if target has an active shield that cannot be penetrated by the Warhead.
- `Trajectory=Straight` projectiles can now snap on targets within 0.5 cells from their detonation point, this distance can be customized via `Trajectory.Straight.TargetSnapDistance`.
- `LaunchSW.RealLaunch=false` now checks if firing house has enough credits to satisfy SW's `Money.Amount` in order to be fired.
- `CreateUnit` now creates the units by default at animation's height (even if `CreateUnit.ConsiderPathfinding` is enabled) instead of always at ground level. This behaviour can be restored by setting `CreateUnit.AlwaysSpawnOnGround` to true.
- Phobos-introduced attack scripts now consider potential target's current map zone when evaluating targets. [TargetZoneScanType](Fixed-or-Improved-Logics.md#customizable-target-evaluation-map-zone-check-behaviour) can be used to customize this behaviour.
- `Artillary`, `ICBMLauncher`, `TickTank` or `SensorArray` no longer affect whether or not building is considered as vehicle for AI attack scripts. Use [ConsideredVehicle](Fixed-or-Improved-Logics.md#buildings-considered-as-vehicles) instead on buildings that do not have both `UndeploysInto` set and `Foundation=1x1`.
- `PassengerDeletion.SoylentFriendlies` has been replaced by `PassengerDeletion.SoylentAllowedHouses`. Current default value of `PassengerDeletion.SoylentAllowedHouses=enemies` matches the previous default behaviour with `PassengerDeletion.SoylentFriendlies=false`.
- `Grinding.DisplayRefund` is changed to `DisplayIncome`, `Grinding.DisplayRefund.Houses` is changed to `DisplayIncome.Houses`, `Grinding.DisplayRefund.Offset` is changed to `DisplayIncome.Offset`
- `[JumpjetControls]`->`AllowLayerDeviation` and `JumpjetAllowLayerDeviation` have been deprecated as the animation layering issues have been properly fixed by default now.
- `[JumpjetControls]->TurnToTarget` and `JumpjetTurnToTarget` are obsolete. Jumpjet units who fire `OmniFire=no` weapons **always** turn to targets as other units do.
- Buildings delivered by trigger action 125 will now **always** play buildup anim as long as it exists. `[ParamTypes]->53` is deprecated.
- `Shadow` for debris & meteor animations is changed to `ExtraShadow`.

#### From pre-0.3 devbuilds

- `Trajectory.Speed` is now defined on projectile instead of weapon.
- `Gravity=0` is not supported anymore as it will cause the projectile to fly backwards and be unable to hit the target which is not at the same height. Use `Straight` Trajectory instead. See [here](New-or-Enhanced-Logics.md#projectile-trajectories).
- Automatic self-destruction logic logic has been reimplemented, `Death.NoAmmo`, `Death.Countdown` and `Death.Peaceful` tags have been remade/renamed and require adjustments to function.
- `DetachedFromOwner` on weapons is deprecated. This has been replaced by `AllowDamageOnSelf` on warheads.
- Timed jump script actions now take the time measured in ingame seconds instead of frames. Divide your value by 15 to accommodate to this change.
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

### New user settings in RA2MD.ini

- These are new user setting keys added by various features in Phobos. Most of them can be found in either in [user inteface](User-Interface.md) or [miscellaneous](Miscellanous.md) sections. Search functionality can be used to find them quickly if needed.

```ini
[Phobos]
CampaignDefaultGameSpeed=4       ; integer
ShowBriefing=true                ; boolean
DigitalDisplay.Enable=false      ; boolean
ShowDesignatorRange=false        ; boolean
PrioritySelectionFiltering=true  ; boolean
ShowPlacementPreview=yes         ; boolean
RealTimeTimers=false             ; boolean
RealTimeTimers.Adaptive=false    ; boolean
ShowHarvesterCounter=true        ; boolean
ShowPowerDelta=true              ; boolean
ShowWeedsCounter=true            ; boolean
ToolTipDescriptions=true         ; boolean
ToolTipBlur=false                ; boolean
SaveGameOnScenarioStart=true     ; boolean
HideLightFlashEffects=false      ; boolean
```

### For Map Editor (Final Alert 2)

<details>
  <summary>Click to show</summary>

  In `FAData.ini`:
  ```ini
  [ParamTypes]
  47=Structures,28
  54=Use GlobalVar,10
  55=Operation,0
  56=Variable index,0
  57=Lower bound,0
  58=Upper bound,0
  59=Operate var is global,10
  60=Operate var index,0
  65=Campaign AI Repairable,0
  68=House,1,2
  69=Non-inert,10
  70=AITargetTypes index,0

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
  601=House owns Techno Type,68,46,0,0,[LONG DESC],0,1,601,1
  602=House doesn't own Techno Type,68,46,0,0,[LONG DESC],0,1,602,1
  604=Techno Type Entered Cell,68,46,0,0,[LONG DESC],0,1,604,1
  605=AI Target Type Entered Cell,68,70,0,0,[LONG DESC],0,1,605,1

  [ActionsRA2]
  41=Play animation at a waypoint...,0,25,69,0,0,0,1,0,0,[LONG DESC].,0,1,41
  125=Build at...,-10,47,0,65,0,0,1,0,0,[LONG DESC],0,1,125
  500=Save game,-4,13,0,0,0,0,0,0,0,[LONG DESC],0,1,500,1
  501=Edit variable,0,56,55,6,54,0,0,0,0,[LONG DESC],0,1,501,1
  502=Generate random number,0,56,57,58,54,0,0,0,0,[LONG DESC],0,1,502,1
  503=Print variable value,0,56,54,0,0,0,0,0,0,[LONG DESC],0,1,503,0
  504=Binary operation,0,56,55,60,54,59,0,0,0,[LONG DESC],0,1,504,1
  505=Fire Super Weapon at specified location (Phobos),0,0,20,2,21,22,0,0,0,Launch a Super Weapon from [SuperWeaponTypes] list at a specified location. House=-1 means random target that isn't neutral. House=-2 means the first neutral house. House=-3 means random human target. Coordinate X=-1 means random. Coordinate Y=-1 means random,0,1,505
  506=Fire Super Weapon at specified waypoint (Phobos),0,0,20,2,30,0,0,0,0,Launch a Super Weapon from [SuperWeaponTypes] list at a specified waypoint. House=-1 means random target that isn't neutral. House=-2 means the first neutral house. House=-3 means random human target. Coordinate X=-1 means random. Coordinate Y=-1 means random,0,1,506
  510=Toggle MCV Redeployablility (Phobos),0,0,15,0,0,0,0,0,0, Set MCVRedeploys to the given value,0,1,510

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
  10101=Wait until ammo is full,0,0,1,[LONG DESC]
  10102=Regroup Temporarily Around the Team Leader,20,0,1,[LONG DESC]
  10103=Load Onto Transports,0,0,1,[LONG DESC]
  10104=Chronoshift to Enemy Base,20,0,1,[LONG DESC]
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

### Version TBD (develop branch nightly builds)

<details open>
  <summary>Click to show</summary>

New:
- `Crit.AffectsHouses` for critical hit system (by Starkku)
- Warhead or weapon detonation at superweapon target cell (by Starkku)
- Super Weapons launching other Super Weapons (by Morton)
- Launching Super Weapons on building infiltration (by Morton)
- Building airstrike target eligibility customization (by Starkku)
- IvanBomb detonation & image display centered on buildings (by Starkku)
- Forcing specific weapon against cloaked or disguised targets (by Starkku)
- Customizable ROF random delay (by Starkku)
- Animation with `Tiled=yes` now supports `CustomPalette` (by ststl)
- Toggleable DieSound when grinding (by Trsdy)
- Shields can inherit Techno ArmorType (by Starkku)
- Income money flying-string display when harvesters or slaves are docking to refineries or when spies steal credits (by Trsdy)
- Allow random crates to be generated only on lands (by Trsdy)
- Iron-curtain effects on infantries and organic units (by ststl)
- Custom `SlavesFreeSound` (by TwinkleStar)
- Allows jumpjet to crash without rotation (by TwinkleStar)
- Customizable priority of superweapons timer sorting(by ststl)
- Customizable aircraft spawner spawn delay (by Starkku)
- Customizable Cluster scatter distance (by Starkku)
- Customizable FlakScatter distance (by Starkku)
- Customizable debris & meteor impact and warhead detonation behaviour (by Starkku, Otamaa)
- Custom warhead debris animations (by Starkku)
- Multiple burst shots / burst delay within infantry firing sequence (by Starkku)
- Attached particle system for animations (by Starkku)
- Removal of hardcoded AA & Gattling weapon selection restrictions (by Starkku)
- Projectile `SubjectToLand/Water` (by Starkku)
- Real time timers (by Morton)
- Default campaign game speed override and custom campaign game speed FPS (by Morton)
- Trigger actions that allow/forbid MCV to redeploy in game (by Trsdy)
- `AnimList` on zero damage Warheads toggle via `AnimList.ShowOnZeroDamage` (by Starkku)
- Including INI files and inheriting INI sections (by Morton)
- Additions to automatic passenger deletion (by Starkku)
- Buildings considered as vehicles (by Starkku)
- TechnoType target evaluation map zone check behaviour customization (by Starkku)
- `CanC4=false` building zero damage toggle (by Starkku)
- OpenTopped transport target sharing customization (by Starkku)
- Vanish animation for `AutoDeath.Behavior=vanish` (by Starkku)
- `AAOnly` for projectiles (by Starkku)
- `CreateUnit` improvements & additions (can spawn infantry and aircraft, units spawning in air, spawn animation) (by Starkku)
- Option to center pause menu background (by Starkku)
- LaunchSW.DisplayMoney (by Starkku)
- Disguise logic improvements (by Starkku)
- Custom insignias (by Starkku)
- Upgrade logic to allow altering of SpySat status (by Otamaa)
- Allow `ZShapePointMove` to apply during buildup via `ZShapePointMove.OnBuildup` (by Starkku)
- `UndeploysInto` building selling buildup sequence length customization (by Starkku)
- Allow overriding `Shield.AffectTypes` for each Warhead shield interaction (by Starkku)
- TechnoType conversion warhead & superweapon (by Morton)
- TechnoType conversion on ownership change (by Trsdy)
- Unlimited skirmish colors (by Morton)
- Example custom locomotor that circles around the target (*NOTE: For developer use only*) (by Kerbiter, CCHyper, with help from Otamaa; based on earlier experiment by CnCVK)
- Vehicle voxel turret shadows & body multi-section shadows (by TwinkleStar & Trsdy)
- Crushing tilt and slowdown customization (by Starkku)
- Extra warhead detonations on weapon (by Starkku)
- Chrono sparkle animation display customization and improvements (by Starkku)
- Script action to Chronoshift teams to enemy base (by Starkku)
- Customizable ElectricBolt Arcs (by Fryone, Kerbiter)
- Digital display of HP and SP (by ststl, FlyStar, Saigyouji, JunJacobYoung)
- PipScale pip customizations (size, ammo / spawn / tiberium frames or offsets) (by Starkku)
- Auto-deploy/Deploy block on ammo change (by Fryone)
- `AltPalette` lighting toggle (by Starkku)
- Unhardcoded timer blinking color scheme (by Starkku)
- Customizing shield self-healing timer restart when shield is damaged (by Starkku)
- Customizing minimum & maximum amount of damage shield can take from a single hit (by Starkku)
- `AutoDeath.Technos(Dont)Exist` can optionally track limboed (not physically on map, e.g transports etc) technos (by Starkku)
- Wall overlay `Palette` support (by Starkku)
- Show designator & inhibitor range (by Morton)
- Owner-only sound on unit creation (by Fryone)
- Allow using `Secondary` weapon against walls if `Primary` cannot target them (by Starkku)
- Reloading ammo in transports (by Starkku)
- Dump variables to file on scenario end / hotkey (by Morton)
- "House owns TechnoType" and "House doesn't own TechnoType" trigger events
- Allow toggling `Infantry/UnitsGainSelfHeal` for `MultiplayPassive=true` houses (by Starkku)
- Customizable straight trajectory detonation & snap distance and pass-through option (by Starkku)
- Airstrike & spy plane fixed spawn distance & height (by Starkku)
- Allow enabling application of `Verses` and `PercentAtMax` for negative damage (by Starkku)
- In addition to `PlacementGrid.Translucency`, allow to set the transparency of the grid when PlacementPreview is enabled, using the `PlacementGrid.TranslucencyWithPreview` tag (by Belonit).
- Show briefing screen on singleplayer mission start (by Starkku)
- Allow setting mission par times and related messages in `missionmd.ini` (by Starkku)
- Allow setting default singleplayer map loading screen and briefing offsets (by Starkku)
- Allow toggling whether or not fire particle systems adjust target coordinates when firer rotates (by Starkku)
- `AmbientDamage` warhead & main target ignore customization (by Starkku)
- Flashing Technos on selecting (by Fryone)
- Customizable DropPod properties on a per-InfantryType basis (by Trsdy)
- Projectile return weapon (by Starkku)
- Allow customizing aircraft landing direction per aircraft or per dock (by Starkku)
- Allow animations to play sounds detached from audio event handler (by Starkku)
- Game save option when starting campaigns (by Trsdy)
- Carryall pickup voice (by Starkku)
- Option to have `Grinding.Weapon` require accumulated credits from grinding (by Starkku)
- Re-enable the Veinhole Monster and Weeds from TS (by ZivDero)
- Recreate the weed-charging of SWs like the TS Chemical Missile (by ZivDero)
- Allow to change the speed of gas particles (by ZivDero)
- Allow upgrade animations to use `Powered` & `PoweredLight/Effect/Special` keys (by Starkku)
- Toggle for `Explodes=true` BuildingTypes to not explode during buildup or being sold (by Starkku)
- Toggleable height-based shadow scaling for voxel air units (by Trsdy & Starkku)
- User setting toggles for harvester counter & power delta indicator (by Starkku)
- Shrapnel weapon target filtering toggle (by Starkku)
- Restore functionality of `[CrateRules]` -> `FreeMCV` with customizable credits threshold (by Starkku)
- Allow customizing the number of vehicles required for unit crates to turn into money crates (by Starkku)
- Per-VehicleType reroll chance for `CrateGoodie=true` (by Starkku)
- Warheads spawning powerup crates (by Starkku)
- Custom tint on TechnoTypes (by Starkku)
- Revenge weapon (by Starkku)
- AttachEffect types with new features like custom tint and weapon range modifier (by Starkku)
- Force shield effect sync on deploy & vs. organic targets effect customization to complement the Iron Curtain ones (by Starkku)
- Map trigger action 41 (Play animation at waypoint) now uses additional parameter to determine if animation can play sound, deal damage etc. (by Starkku)
- Allow restricting how many times per frame a single radiation site can damage a building (by Starkku)
- Allow explicitly setting the superweapons AI uses for Chronoshift script actions (by Starkku)
- Allow customizing Aircraft weapon strafing regardless of `ROT` and `Strafing.Shots` values beyond 5 (by Trsdy)
- Allow strafing weapons to deduct ammo per shot instead of per strafing run (by Starkku)
- Allow `CloakVisible=true` laser trails optinally be seen only if unit is detected (by Starkku)
- Customizing whether passengers are kicked out when an aircraft fires (by ststl)
- Shield hit flash (by Starkku)
- Option to scatter `Anim/SplashList` animations around impact coordinates (by Starkku)
- Customizable wake anim (by TwinkleStar)
- Customizable rocker amplitude (by TwinkleStar, Ollerus)
- AI script action to jump back to previous script after picking a random script (by handama)
- Insignias visibility and position adjustments (by Fryone)
- Promotion animation (by Fryone)
- Allow different technos to share build limit in a group (by ststl & Ollerus)
- Map events `604-605` for checking if a specific Techno enters in a cell (by FS-21)
- Waypoint path is drawn for all units under player control or if `[GlobalControls]->DebugPlanningPaths=yes` (by Trsdy)
- `RemoveDisguise` now works on vehicle disguises (by Trsdy)
- Allow anchoring extended tooltips to the left side of the sidebar (by Trsdy)
- Toggle to allow spawned aircraft to attack immediately after being spawned (by Starkku)
- `ZAdjust` for OverlayTypes (by Starkku)
- Allow customizing extra tint intensity for Iron Curtain & Force Shield (by Starkku)
- Option to enable parsing 8-bit RGB values from `[ColorAdd]` instead of RGB565 (by Starkku)
- Customizing height and speed at which subterranean units travel (by Starkku)
- Option for Warhead damage to penetrate Iron Curtain or Force Shield (by Starkku)
- Option for Warhead to remove all shield types at once (by Starkku)
- Allow customizing voxel light source position (by Kerbiter, Morton, based on knowledge of thomassnedon)
- Option to fix voxel light source being offset and incorrectly tilting on slopes (by Kerbiter)
- Allow using waypoints, area guard and attack move with aircraft (by CrimRecya)
- AI superweapon delay timer customization (by Starkku)
- Disabling `MultipleFactory` bonus from specific BuildingType (by Starkku)
- Customizable ChronoSphere teleport delays for units (by Starkku)
- Allowed and disallowed types for `FactoryPlant` (by Starkku)
- Customizable damage & 'crumbling' (destruction) frames for TerrainTypes (by Starkku)
- Custom object palettes for TerrainTypes (by Starkku)
- Forbidding parallel AI queues for specific TechnoTypes (by Starkku)
- Nonprovocative Warheads (by Starkku)
- Buildings considered as destroyable pathfinding obstacles (by Starkku)
- `FireOnce` infantry sequence reset toggle (by Starkku)
- Assign Super Weapon cameo to any sidebar tab (by NetsuNegi)
- Customizing effect of level lighting on air units (by Starkku)
- Reimplemented `Airburst` & `Splits` logic with more customization options (by Starkku)
- Buildings considered as destroyable pathfinding obstacles (by Starkku)
- Animation visibility customization settings (by Starkku)
- Light effect customizations (by Starkku)
- Building unit repair customizations (by Starkku)
- Toggle to disallow buildings from providing build area during buildup (by Starkku)
- Allow customizing which building types provide build area for a building (by Starkku)
- `Scorch` / `Flamer` fire animation customization (by Starkku)
- Warheads parasite removal customization (by Starkku)
- Allow infantry to use land sequences in water (by Starkku)
- `<Player @ X>` can now be used as owner for pre-placed objects on skirmish and multiplayer maps (by Starkku)
- Allow customizing charge turret delays per burst on a weapon (by Starkku)
- Unit `Speed` setting now accepts floating point values (by Starkku)

Vanilla fixes:
- Allow AI to repair structures built from base nodes/trigger action 125/SW delivery in single player missions (by Trsdy)
- Allow usage of `AlternateFLH%d` of vehicles in `OpenTopped` transport. (by Trsdy)
- Improved the statistic distribution of the spawned crates over the visible area of the map. (by Trsdy, based on TwinkleStar's work)
- Teams spawned by trigger action 7,80,107 can use IFV and `OpenTopped` logic normally (by Trsdy)
- Prevented units from retaining previous order after ownership change (by Trsdy)
- Break the mind-control link when capturing a mind-controlled building with an engineer (by Trsdy)
- Fixed BibShape drawing for a couple of frames during buildup for buildings with long buildup animations (by Starkku)
- Cloaked & disguised objects displaying to observers (by Starkku)
- Cloaked objects from allies displaying to player in single player missions (by Trsdy)
- Skip `NaturalParticleSystem` displaying from in-map pre-placed structures (by Trsdy)
- Made sure that `Suicide=yes` weapon does kill the firer (by Trsdy)
- Made sure that vxl units being flipped over get killed instead of rotating up and down (by Trsdy)
- Allow jumpjet units to visually tilt or be flipped over on the ground even if `TiltCrashJumpjet=no` (by Trsdy)
- Fixed the range for number of debris spawned by Warhead to use MaxDebris instead of MaxDebris - 1 (by Starkku)
- Fixed `LandTargeting=1` not preventing from targeting TerrainTypes (trees etc.) on land (by Starkku)
- Fixed `NavalTargeting=6` not preventing from targeting empty water cells or TerrainTypes (trees etc.) on water (by Starkku)
- Fixed `NavalTargeting=7` and/or `LandTargeting=2` resulting in still targeting TerrainTypes (trees etc.) on land with `Primary` weapon (by Starkku)
- Fixed an issue that causes objects in layers outside ground layer to not be sorted correctly (caused issues with animation and jumpjet layering for an instance) (by Starkku)
- Restored `EVA_StructureSold` for buildings with `UndeploysInto` (by Trsdy)
- Allow MCV to redeploy in campaigns (by Trsdy)
- Allow buildings with `UndeploysInto` to be sold if `Unsellable=no` but `ConstructionYard=no` (by Trsdy)
- Fixed infantry without `C4=true` being killed in water if paradropped, chronoshifted etc. even if they can normally enter water (by Starkku)
- Fixed `WaterBound=true` buildings with `UndeploysInto` not correctly setting the location for the vehicle to move into when undeployed (by Starkku)
- Allow more than 5 `AlternateFLH` entries for units (by ststl)
- Buildings with `CanC4=false` will no longer take 1 point of positive damage if hit by negative damage (by Starkku)
- Buildings with primary weapon that has `AG=false` projectile now have attack cursor when selected (by Starkku)
- Transports with `OpenTopped=true` and weapon that has `Burst` above 1 and passengers firing out no longer have the passenger firing offset shift lateral position based on burst index (by Starkku)
- Light tint created by a building is now able to be removed after loading the game (by Trsdy)
- Prevented crashing jumpjet units from firing (by Trsdy)
- Fixed disguised infantry not using custom palette for drawing the disguise when needed (by Starkku)
- Reenabled the obsolete `[General] WarpIn` as default anim type when units are warping in (by Trsdy)
- Fixed permanent health bar display for units targeted by temporal weapons upon mouse hover (by Trsdy)
- Buildings with superweapons no longer display `SuperAnimThree` at beginning of match if pre-placed on the map (by Starkku)
- AI players can now build `Naval=true` and `Naval=false` vehicles concurrently like human players do (by Starkku)
- Fixed the bug when jumpjets were snapping into facing bottom-right when starting movement (by Kerbiter)
- Suppressed the BuildingCaptured EVA events when capturing a building considered as a vehicle (by Trsdy)
- Objects with `Palette` set now have their color tint adjusted accordingly by superweapons, map retint actions etc. if they belong to a house using any color scheme instead of only those from the first half of `[Colors]` list (by Starkku)
- Animations using `AltPalette` are now remapped to their owner's color scheme instead of first listed color scheme and no longer draw over shroud (by Starkku)
- Fixed `DeployToFire` not considering building placement rules for `DeploysInto` buildings and as a result not working properly with `WaterBound` buildings (by Starkku)
- Fixed `DeployToFire` not recalculating firer's position on land if it cannot currently deploy (by Starkku)
- `Arcing=true` projectile elevation inaccuracy can now be fixed by setting `Arcing.AllowElevationInaccuracy=false` (by Starkku)
- Fixed position and layer of info tip and reveal production cameo on selected building (by Belonit)
- Fixed `TurretOffset` to be supported for SHP vehicles (by TwinkleStar)
- `Powered`/`PoweredSpecial` buildings' powered anims will update as usual when being captured by enemies (by Trsdy)
- Fixed a glitch related to incorrect target setting for missiles (by Belonit)
- Skipped parsing `[Header]` section of compaign maps which led to occasional crashes on Linux (by Trsdy)
- Fixed units' turret rotation and jumpjet wobble under EMP (by Trsdy)
- Fixed `AmbientDamage` when used with `IsRailgun=yes` being cut off by elevation changes (by Starkku)
- Fixed railgun and fire particles being cut off by elevation changes (by Starkku)
- Fixed teleport units' frozen-still timer being reset after load game (by Trsdy)
- Fixed teleport units being unable to visually tilt on slopes (by Trsdy)
- Fixed teleport and drill units being unable to be visually flipped (by Trsdy)
- Aircraft docking on buildings now respect `[AudioVisual]`->`PoseDir` as the default setting and do not always land facing north or in case of pre-placed buildings, the building's direction (by Starkku)
- Spawned aircraft now align with the spawner's facing when landing (by Starkku)
- Fixed infantries attempted to entering buildings when waypointing together with engineer/agent/occupier/etc (by Trsdy)
- Fixed jumpjet crash speed when crashing onto buildings (by NetsuNegi)
- Fixed a desync potentially caused by displaying of cursor over selected `DeploysInto` units (by Starkku)
- Skipped drawing the rally point line when undeploying a factory (by Trsdy)
- Tint effects are now correctly applied to SHP vehicles and all types of aircraft as well as building animations regardless of their position (by Starkku)
- Iron Curtained / Force Shielded objects now always use the correct tint color (by Starkku)
- Objects in invalid map coordinates are no longer used for starting view and AI base center calculations (by Starkku)
- Units & buildings with `DecloakToFire=false` weapons can now cloak while targeting & reloading (by Starkku)
- Units with `Sensors=true` will no longer reveal ally buildings (by Starkku)
- Air units are now reliably included by target scan with large range and Warhead detonation by large `CellSpread` (by Starkku)
- Weapons with `AA=true` Projectile can now correctly fire at air units when both firer and target are over a bridge (by Starkku)
- Fixed disguised units not using the correct palette if target has custom palette (by NetsuNegi)
- Building upgrades now consistently use building's `PowerUpN` animation settings corresponding to the upgrade's `PowersUpToLevel` where possible (by Starkku)
- Subterranean units are no longer allowed to perform deploy functions like firing weapons or `IsSimpleDeployer` while burrowed or burrowing, they will instead emerge first like they do for transport unloading (by Starkku)
- Fixed `Temporal=true` Warheads potentially crashing game if used to attack `Slaved=true` infantry (by Starkku)
- Fixed some locomotors (Tunnel, Walk, Mech) getting stuck when moving too fast (by NetsuNegi)
- Animations with `MakeInfantry` and `UseNormalLight=false` that are drawn in unit palette will now have cell lighting changes applied on them (by Starkku)
- Fixed Nuke & Dominator Level lighting not applying to AircraftTypes (by Starkku)
- Removed the 0 damage effect from `InfDeath=9` warheads to in-air infantries (by Trsdy)
- Projectiles created from `AirburstWeapon` now remember their WeaponType and can apply radiation etc. (by Starkku)
- Fixed damaged aircraft not repairing on `UnitReload=true` docks unless they land on the dock first (by Starkku)
- Certain global tileset indices (`ShorePieces`, `WaterSet`, `CliffSet`, `WaterCliffs`, `WaterBridge`, `BridgeSet` and `WoodBridgeSet`) can now be toggled to be parsed for lunar theater (by Starkku)
- Fixed infantry `SecondaryFire` / `SecondaryProne` sequences being displayed in water instead of `WetAttack` (by Starkku)
- Fixed objects with ally target and `AttackFriendlies=true` having their target reset every frame, particularly AI-owned buildings (by Starkku)
- Follower vehicle index for preplaced vehicles in maps is now explicitly constrained to `[Units]` list in map files and is no longer thrown off by vehicles that could not be created or created vehicles having other vehicles as initial passengers (by Starkku)
- Drive/Jumpjet/Ship/Teleport locomotor did not power on when it is un-piggybacked bugfix (by tyuah8)
- Fix `Stop` command not working so well in some cases (by CrimRecya)
- Subterranean movement now benefits from speed multipliers from all sources such as veterancy, AttachEffect etc. (by Starkku)

Phobos fixes:
- Fixed a few errors of calling for superweapon launch by `LaunchSW` or building infiltration (by Trsdy)
- Add `ImmuneToCrit` for shields (by Trsdy)
- Reimplemented the bugfix for jumpjet units' facing when firing, discard the inappropriate `JumpjetTurnToTarget` tag (by Trsdy)
- `Gunner=true` transports now correctly change turret if a passenger is removed by `PassengerDeletion` (by Starkku)
- `PassengerDeletion.Soylent` now correctly calculates refund value if removed passenger has no explicitly set `Soylent` value (by Starkku)
- Superweapon `Detonate.Weapon` & `Detonate.Warhead` now use the firing house to deal damage and apply Phobos warhead effects even if no firing building is found (by Starkku)
- `CreateUnit` now uses civilian house as owner instead if the intended owner house has been defeated (this is in-line with how `MakeInfantry` works) (by Starkku)
- `IsHouseColor` laser trails on techno now correctly change color when it changes owner (by Trsdy)
- Fixed `Layer.UseObjectLayer=true` to work correctly for all cases where object changes layer (by Starkku)
- Fixed `DetonateOnAllMapObjects.RequireVerses` not considering shield armor types (by Starkku)
- Fixed new Phobos script actions not picking team leader correctly based on `LeadershipRating` (by Starkku)
- Fixed an issue with `Gunner=true` vehicles not correctly using the first passenger's mode with multiple passengers inside (by Starkku)
- Used `MindControl.Anim` for buildings deployed from mind-controlled vehicles (by Trsdy)
- Optimized extension class implementation, should improve performance all around (by Otamaa & Starkku)
- Fixed `Interceptor` not resetting target if the intercepted projectile changes type to non-interceptable one afterwards (by Starkku)
- Fixed `PlacementPreview` setting for BuildingTypes not being parsed from INI (by Starkku)
- Fixed Phobos animation additions that support `CreateUnit.Owner` not also checking `MakeInfantryOwner` (by Starkku)
- Fixed `AutoDeath` to consider all conditions for objects in limbo (by Starkku)
- Shields will no longer take damage if the parent techno has `Immune=true` or has `TypeImmune=true` and the damage comes from instance of same TechnoType owned by same house (by Starkku)
- Fixed interceptors causing multiplayer games to desync (by Starkku)
- Optimized performance for map trigger retint action light source fix (by Starkku)
- Fixed a number of issues with Warhead Shield respawn / self heal rate modifiers like timers getting reset unnecessarily, the timer being adjusted wrong after the Warhead effect runs out etc. (by Starkku)
- Fixed a problem with disguise visibility logic that could cause game to crash on loading a map (by Starkku)
- Fixed owned `LimboDelivery` buildings not being saved correctly in savegames (by Starkku)
- Fixed a typo in weapon selector code causing issues with `NoAmmoWeapon` and related checks (by Starkku)
- Fixed `DetonateOnAllMapObjects` behaving erratically or potentially crashing if it destroys buildings using Ares' advanced rubble (by Starkku)
- Fixed game crashing on loading save games if the saved game state had active radiation sites (by Starkku)
- Fixed a desync error caused by air/top layer sorting (by Starkku)
- Fixed heal / repair weapons being unable to remove parasites from shielded targets if they were unable to heal / repair the parent unit (by Starkku)
- Fixed `Inviso=true` interceptor projectiles applying damage on interceptable, armor type-having projectiles twice (by Starkku)
- Fixed `AutoDeath` causing crashes when used to kill a parasite unit inside an another unit (by Starkku)
- Phobos Warhead effects combined with `CellSpread` now correctly apply to buildings if any of the foundation cells are hit (by Starkku)
- Phobos Warhead effects on zero-`CellSpread` Warheads no longer apply to target if projectile detonates prematurely, far-away from target (by Starkku)
- Fixed radiation site damage not taking the radiation level reduction into accord (by Starkku)
- Correctly update laser trail position while techno is cloaked even if trail is not drawn (by Starkku)
- Fixed `Shield.Respawn.Amount` not defaulting to shield type default if not set (by Starkku)
- Fixed frame by frame hotkey description to read `TXT_FRAME_BY_FRAME_DESC` instead of `TXT_DISPLAY_DAMAGE_DESC` (by DeathFishAtEase)
- Buildings considered vehicles (`ConsideredVehicle=true` or not set in conjunction with `UndeploysInto` & 1x1 foundation) are now considered units by affected target enum checks (by Starkku)
- Fixed Phobos Warhead effects not reliably being applied on damage area as opposed to full weapon-based Warhead detonation (by Starkku)
- Fix `LimboKill` not working reliably (by CrimRecya)
- Fixed `SelfHealGainType=none` not working (changed to `noheal`) (by Starkku)
- Fixed AircraftTypes gaining self-healing from `UnitsGainSelfHeal` by default (while not displaying the pip) when they should not (by Starkku)
- Fixed `LaunchSW.IgnoreInhibitors` and `SW.Next.IgnoreInhibitors` overriding corresponding `IgnoreDesignators` settings (by Ollerus)

Fixes / interactions with other extensions:
- Weapons fired by EMPulse superweapons *(Ares feature)* now fully respect the firing building's FLH.
- Weapons fired by EMPulse superweapons *(Ares feature)* now respect `Floater` and Phobos-added `Gravity` setting (by Starkku)
- `IsSimpleDeployer` units with Hover locomotor and `DeployToLand` no longer get stuck after deploying or play their move sound indefinitely (by Starkku)
- All forms of type conversion (including Ares') now correctly update the warp-in delay if unit with teleport `Locomotor` was converted while the delay was active (by Starkku)
- All forms of type conversion (including Ares') now correctly update `MoveSound` if a moving unit has their type changed (by Starkku)
- All forms of type conversion (including Ares') now correctly update `OpenTopped` state of passengers in transport that is converted (by Starkku)
- Fixed an issue introduced by Ares that caused `Grinding=true` building `ActiveAnim` to be incorrectly restored while `SpecialAnim` was playing and the building was sold, erased or destroyed (by Starkku)
- Appended Ares' `SW.Shots` usage to extended tooltips (by Trsdy)
- Fixed Ares' Abductor weapon leaves permanent placement stats when abducting moving vehicles (by Trsdy)
- Suppressed Ares' swizzle warning when parsing `Tags` and `TaskForces` (by Trsdy)
- Fixed Academy *(Ares feature)* not working on the initial payloads *(Ares feature)* of vehicles built from a war factory (by Trsdy, supersedes Aephiex impl.)
- Fixed Ares' InitialPayload not being created for vehicles spawned by trigger actions (by Trsdy)
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
- New condition for automatic self-destruction logic when TechnoTypes exist/don't exist (by FlyStar)
- Skirmish AI "sell all buildings and set all technos to hunt" behavior dehardcode (by TaranDahl/航味麻酱)
- Skirmish AI "gather when MCV deploy" behavior dehardcode (by TaranDahl/航味麻酱)

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
- Fixed techno-extdata update after type conversion (by Trsdy)
- Fixed Phobos Warhead effects (crits, new shield modifiers etc.) considering sinking units valid targets (by Starkku)
- Fixed an issue where `FireOnce=yes` deploy weapons on vehicles would still fire multiple times if deploy command is issued repeatedly or when not idle (by Starkku)
- Fixed a game crash when checking BuildLimit if Phobos is running without Ares (by Belonit)
- Corrected the misinterpretation in the definition of `DiskLaser.Radius` (by Trsdy)
- Fixed GlobalVariables failed working among scenarios (by Trsdy)

Fixes / interactions with other extensions:
- Weapons fired by EMPulse superweapons *(Ares feature)* without `EMPulse.TargetSelf=true` can now create radiation (by Starkku)

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
