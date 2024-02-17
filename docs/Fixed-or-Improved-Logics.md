# Fixed / Improved Logics

This page describes all ingame logics that are fixed or improved in Phobos without adding anything significant.

## Bugfixes and miscellaneous

- Fixed the bug that GameModeOptions are not correctly saved. For example, `BuildOffAlly` is corrupted after load a save.
- Fixed the bug when reading a map which puts `Preview(Pack)` after `Map` lead to the game fail to draw the preview
- Fixed the bug when retinting map lighting with a map action corrupted light sources.
- Fixed the bug when deploying mindcontrolled vehicle into a building permanently transferred the control to the house which mindcontrolled it.
- Fixed the bug when units are already dead but still in map (for sinking, crashing, dying animation, etc.), they could die again.
- Fixed the bug when cloaked Desolator was unable to fire his deploy weapon.
- Fixed the bug that temporaryed unit cannot be erased correctly and no longer raise an error.
- Fixed `DebrisMaximums` (spawned debris type amounts cannot go beyond specified maximums anymore). Only applied when `DebrisMaximums` values amount is more than 1 for compatibility reasons.
- Fixed building and defense tab hotkeys not enabling the placement mode after `Cannot build here.` triggered and the placement mode cancelled.
- Fixed buildings with `UndeployInto` playing `EVA_NewRallypointEstablished` on undeploying.
- Fixed buildings with `Naval=yes` ignoring `WaterBound=no` to be forced to place onto water.
- Fixed AI Aircraft docks bug when Ares tag `[GlobalControls]` > `AllowParallelAIQueues=no` is set.
- Fixed laser drawing code to allow for thicker lasers in house color draw mode.
- Fixed `DeathWeapon` not detonating properly.
  - Some settings are still ignored like `PreImpactAnim` *(Ares feature)*, this might change in future.
- Fixed the bug when occupied building's `MuzzleFlashX` is drawn on the center of the building when `X` goes past 10.
- Fixed jumpjet units that are `Crashable` not crashing to ground properly if destroyed while being pulled by a `Locomotor` warhead.
- Fixed jumpjet units being unable to turn to the target when firing from a different direction.
- Fixed turreted jumpjet units always facing bottom-right direction when motion stops.
- Fixed jumpjet objects being unable to use `Sensors`.
- Fixed interaction of `UnitAbsorb` & `InfantryAbsorb` with `Grinding` buildings. The keys will now make the building only accept appropriate types of objects.
- Fixed missing 'no enter' cursor for VehicleTypes being unable to enter a `Grinding` building.
- Fixed Engineers being able to enter `Grinding` buildings even when they shouldn't (such as ally building at full HP).
- Allowed usage of TileSet of 255 and above without making NE-SW broken bridges unrepairable.
- Added a "Load Game" button to the retry dialog on mission failure.

![image](_static/images/turretoffset-01.png)
*Side offset voxel turret in Breaking Blue project*

- `TurretOffset` tag for voxel turreted TechnoTypes now accepts FLH (forward, lateral, height) values like `TurretOffset=F,L` or `TurretOffset=F,L,H`, which means turret location can be adjusted in all three axes.
- `InfiniteMindControl` with `Damage=1` can now control more than 1 unit.
- Aircraft with `Fighter` set to false or those using strafing pattern (weapon projectile `ROT` is below 2) now take weapon's `Burst` into accord for all shots instead of just the first one.
- `EMEffect` used for random AnimList pick is now replaced by a new tag `AnimList.PickRandom` with no side effect. (EMEffect=yes on AA inviso projectile deals no damage to units in movement)
- Vehicles using `DeployFire` will now use `DeployFireWeapon` for firing the deploy weapon if explicitly set, if not it behaves like previously (`Primary` if can fire, `Secondary` if not) and respect `FireOnce` setting on weapon and any stop commands issued during firing. If `FireOnce` is set to true the unit won't accept further deploy commands for number of frames that is equal to whichever is smaller between weapon `ROF` and `[Unload]` -> `Rate` times 900.
- Infantry with `DeployFireWeapon=-1` can now fire both weapons (decided by its target), regardless of deployed or not.

![image](_static/images/remember-target-after-deploying-01.gif)
*Nod arty keeping target on attack order in [C&C: Reloaded](https://www.moddb.com/mods/cncreloaded/)*

- Vehicle to building deployers now keep their target when deploying with `DeployToFire`.
- Effects like lasers are no longer drawn from wrong firing offset on weapons that use Burst.
- Animations can now be offset on the X axis with `XDrawOffset`.
- `IsSimpleDeployer` units now only play `DeploySound` and `UndeploySound` once, when done with (un)deploying instead of repeating it over duration of turning and/or `DeployingAnim`.
- AITrigger can now recognize Building Upgrades as legal condition.
- `EWGates` and `NSGates` now will link walls like `xxGateOne` and `xxGateTwo` do.
- Fixed the bug when occupied building's `MuzzleFlashX` is drawn on the center of the building when `X` goes past 10.
- Fixed jumpjet units that are `Crashable` not crashing to ground properly if destroyed while being pulled by a `Locomotor` warhead.
- Fixed interaction of `UnitAbsorb` & `InfantryAbsorb` with `Grinding` buildings. The keys will now make the building only accept appropriate types of objects.
- Fixed missing 'no enter' cursor for VehicleTypes being unable to enter a `Grinding` building.
- Fixed Engineers being able to enter `Grinding` buildings even when they shouldn't (such as ally building at full HP).
- Aircraft & jumpjet units are now affected by speed modifiers such as `SpeedAircraft/Infantry/UnitsMult` on `Countries`, `VeteranSpeed` and Crates / AttachEffect (Ares feature).
- Both voxel and SHP vehicle units should now correctly respect custom palette set through `Palette`.
- Weapons fired by EMPulse superweapons without `EMPulse.TargetSelf=true` *(Ares feature)* can now create radiation.
- Setting `RadarInvisible` to true on TerrainTypes now hides them from minimap display.
- Mind control indicator animations will now correctly restore on mind controlled objects when uncloaked.
- Animations from Warhead `AnimList` & `SplashList` etc. as well as animations created through map trigger `41 Play Anim At` now have the appropriate house set as owner of the animation by default.
- Nuke carrier & payload weapons now respect `Bright` setting on the weapons always when appropriate (previously only payload did and only if Superweapon had `Nuke.SiloLaunch=false` *(Ares feature)*).
- Self-healing pips from `InfantryGainSelfHeal` & `UnitsGainSelfHeal` now respect unit's `PixelSelectionBracketDelta` like health bar pips do.
- Buildings using `SelfHealing` will now correctly revert to undamaged graphics if their health is restored back by self-healing.
- Allow use of `Foundation=0x0` on TerrainTypes without crashing for similar results as for buildings.
- Projectiles now remember the house of the firer even if the firer is destroyed before the projectile detonates. Does not currently apply to some Ares-introduced Warhead effects like EMP.
- `OpenTopped` transports now take `OpenTransportWeapon` setting of passengers into consideration when determining weapon range used for threat scanning and approaching targets.
- Trailer animations now inherit the owner of the object (animation, projectile or aircraft) they are attached to.
- Buildings now correctly use laser parameters set for Secondary weapons instead of reading them from Primary weapon.
- Fixed an issue that caused vehicles killed by damage dealt by a known house but without a known source TechnoType (f.ex animation warhead damage) to not be recorded as killed correctly and thus not spring map trigger events etc.

![Waving trees](_static/images/tree-shake.gif)
*Animated trees used in [Ion Shock](https://www.moddb.com/mods/tiberian-war-ionshock)*

- `IsAnimated`, `AnimationRate` and `AnimationProbability` now work on TerrainTypes without `SpawnsTiberium` set to true. Note that this might impact performance.
- Fixed transports recursively put into each other not having a correct killer set after second transport when being killed by something.

![image](_static/images/translucency-fix.png)
*Example gradient SHP drawing with 75% translucency, before and after*

- Translucent RLE SHPs will now be drawn using a more precise and performant algorithm that has no green tint and banding. Can be disabled with `rulesmd.ini->[General]->FixTransparencyBlitters=no`.
  - Only applies to Z-aware drawing mode for now.
- Fixed projectiles with `Inviso=true` suffering from potential inaccuracy problems if combined with `Airburst=yes` or Warhead with `EMEffect=true`.
- Fixed the bug when `MakeInfantry` logic on BombClass resulted in `Neutral` side infantry.
- Fixed railgun particles being drawn to wrong coordinate against buildings with non-default `TargetCoordOffset` or when force-firing on bridges.
- Fixed building `TargetCoordOffset` not being taken into accord for several things like fire angle calculations and target lines.
- Fix a glitch related to incorrect target setting for missiles.
- Fix [EIP 00529A14](https://modenc.renegadeprojects.com/Internal_Error/YR#eip_00529A14) when attempting to read `[Header]` section of campaign maps.
- Fixed teleport units' (for example CLEG) frozen-still timer being cleared after load game.

## Fixes / interactions with other extensions

- Fixed an issue introduced by Ares that caused `Grinding=true` building `ActiveAnim` to be incorrectly restored while `SpecialAnim` was playing and the building was sold, erased or destroyed.

## Animations

### Animation weapon and damage settings

- `Weapon` can be set to a WeaponType, to create a projectile and immediately detonate it instead of simply dealing `Damage` by `Warhead`. This allows weapon effects to be applied.
- `Damage.Delay` determines delay between two applications of `Damage`. Requires `Damage` to be set to 1.0 or above. Value of 0 disables the delay. Keep in mind that this is measured in animation frames, not game frames. Depending on `Rate`, animation may or may not advance animation frames on every game frame.
- `Damage.DealtByInvoker`, if set to true, makes any `Damage` dealt to be considered as coming from the animation's invoker (f.ex, firer of the weapon if it is Warhead `AnimList/SplashList` animation, the destroyed vehicle if it is `DestroyAnim` animation or the object the animation is attached to). If invoker has died or does not exist, the house the invoker belonged to is still used to deal damage and apply Phobos-introduced Warhead effects. Does not affect which house the `Damage` dealt by `Warhead` is dealt by.
- `Damage.ApplyOncePerLoop`, if set to true, makes `Damage` be dealt only once per animation loop (on single loop animations, only once, period) instead of on every frame or intervals defined by `Damage.Delay`. The frame on which it is dealt is determined by `Damage.Delay`, defaulting to after the first animation frame.

In `artmd.ini`:
```ini
[SOMEANIM]                     ; AnimationType
Weapon=                        ; WeaponType
Damage.Delay=0                 ; integer, animation frames
Damage.DealtByInvoker=false    ; boolean
Damage.ApplyOncePerLoop=false  ; boolean
```

```{note}
`Weapon` and `Damage.Delay`, beyond the other additions, should function similarly to the equivalent features introduced by Ares and take precedence over them if Phobos is used together with Ares.
```

### Attached animation position customization

- You can now customize whether or not animations attached to objects are centered at the object's actual center rather than the bottom of their top-leftmost cell (cell #0).

In `artmd.ini`:
```ini
[SOMEANIM]                       ; AnimationType
UseCenterCoordsIfAttached=false  ; boolean
```

### Customizable debris & meteor
- `ExtraShadow` can be set to false to disable the display of shadows on the ground.

In `artmd.ini`:
```ini
[SOMEANIM]                    ; AnimationType
ExtraShadow=true              ; boolean
```

### Layer on animations attached to objects

- You can now customize whether or not animations attached to objects follow the object's layer or respect their own `Layer` setting. If this is unset, attached animations use `ground` layer.

In `artmd.ini`:
```ini
[SOMEANIM]             ; AnimationType
Layer.UseObjectLayer=  ; boolean
```

### Ore stage threshold for `HideIfNoOre`

- You can now customize which growth stage should an ore/tiberium cell have to have animation with `HideIfNoOre` displayed. Cells with growth stage less than specified value won't allow the anim to display.

In `artmd.ini`:
```ini
[SOMEANIM]               ; AnimationType
HideIfNoOre.Threshold=0  ; integer, minimal ore growth stage
```

## Buildings

### Customizable & new grinder properties

![image](_static/images/grinding.gif)
*Using ally grinder, restricting to vehicles only and refund display ([Project Phantom](https://www.moddb.com/mods/project-phantom))*

- You can now customize which types of objects a building with `Grinding` set can grind as well as the grinding sound.
  - `Grinding.AllowAllies` changes whether or not to allow units to enter allies' buildings.
  - `Grinding.AllowOwner` changes whether or not to allow units to enter your own buildings.
  - `Grinding.AllowTypes` can be used to define InfantryTypes and VehicleTypes that can be grinded by the building. Listing any will disable grinding for all types except those listed.
  - `Grinding.DisallowTypes` can be used to exclude InfantryTypes or VehicleTypes from being able to enter the grinder building.
  - `Grinding.Sound` is a sound played by when object is grinded by the building. If not set, defaults to `[AudioVisual]`->`EnterGrinderSound`.
  - `Grinding.Weapon` is a weapon fired at the building & by the building when it grinds an object. Will only be fired if at least weapon's `ROF` amount of frames have passed since it was last fired.
  - `Grinding.DisplayRefund` can be set to display the amount of credits acquired upon grinding on the building. Multiple refunded objects within a short period of time have their refund amounts coalesced into single display.
    - `Grinding.DisplayRefund.Houses` determines which houses can see the credits display.
    - `Grinding.DisplayRefund.Offset` is additional pixel offset for the center of the credits display, by default (0,0) at building's center.

In `rulesmd.ini`:
```ini
[SOMEBUILDING]                     ; BuildingType
Grinding.AllowAllies=false         ; boolean
Grinding.AllowOwner=true           ; boolean
Grinding.AllowTypes=               ; List of InfantryTypes / VehicleTypes
Grinding.DisallowTypes=            ; List of InfantryTypes / VehicleTypes
Grinding.Sound=                    ; Sound
Grinding.Weapon=                   ; WeaponType
Grinding.DisplayRefund=false       ; boolean
Grinding.DisplayRefund.Houses=All  ; Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
Grinding.DisplayRefund.Offset=0,0  ; X,Y, pixels relative to default
```

## Projectiles

### Customizable projectile gravity

-  You can now specify individual projectile gravity.
    - Setting `Gravity=0` is not recommended as it will cause the projectile to fly backwards and be unable to hit the target which is not at the same height. We suggest to use `Straight` Trajectory instead. See [here](New-or-Enhanced-Logics.md#projectile-trajectories).

In `rulesmd.ini`:
```ini
[SOMEPROJECTILE]        ; Projectile
Gravity=6.0             ; floating point value
```

## Technos

### Building-provided self-healing customization

- It is now possible to set a global cap for the effects of `InfantryGainSelfHeal` and `UnitsGainSelfHeal` by setting `InfantryGainSelfHealCap` & `UnitsGainSelfHealCap` under `[General]`, respectively.
- It is also possible to change the pip frames displayed from `pips.shp` individually for infantry, units and buildings by setting the frames for infantry & unit self-healing on `Pips.SelfHeal.Infantry/Units/Buildings` under `[AudioVisual]`, respectively.
  - `Pips.SelfHeal.Infantry/Units/Buildings.Offset` can be used to customize the pixel offsets for the displayed pips, individually for infantry, units and buildings.
- Whether or not a TechnoType benefits from effects of `InfantryGainSelfHeal` or `UnitsGainSelfHeal` buildings or neither can now be controlled by setting `SelfHealGainType`.
  - If `SelfHealGainType` is not set, InfantryTypes and VehicleTypes with `Organic` set to true gain self-healing from `InfantryGainSelfHeal`, other VehicleTypes from `UnitsGainSelfHeal` and AircraftTypes & BuildingTypes never gain self-healing.

In `rulesmd.ini`:
```ini
[General]
InfantryGainSelfHealCap=               ; integer, maximum amount of InfantryGainSelfHeal that can be in effect at once, must be 1 or higher
UnitsGainSelfHealCap=                  ; integer, maximum amount of UnitsGainSelfHeal that can be in effect at once, must be 1 or higher

[AudioVisual]
Pips.SelfHeal.Infantry=13,20           ; integer, frames of pips.shp for infantry & unit-self healing pips, respectively
Pips.SelfHeal.Units=13,20              ; integer, frames of pips.shp for infantry & unit-self healing pips, respectively
Pips.SelfHeal.Buildings=13,20          ; integer, frames of pips.shp for infantry & unit-self healing pips, respectively
Pips.SelfHeal.Infantry.Offset=25,-35   ; X,Y, pixels relative to default
Pips.SelfHeal.Units.Offset=33,-32      ; X,Y, pixels relative to default
Pips.SelfHeal.Buildings.Offset=15,10   ; X,Y, pixels relative to default

[SOMETECHNO]                           ; TechnoType
SelfHealGainType=                      ; Self-Heal Gain Type Enumeration (none|infantry|units)
```

### Customizable harvester ore gathering animation

![image](_static/images/oregath.gif)
*Custom ore gathering anims in [Project Phantom](https://www.moddb.com/mods/project-phantom)*

- You can now specify which anim should be drawn when a harvester of specified type is gathering specified type of ore.

In `rulesmd.ini`:
```ini
[SOMETECHNO]                     ; TechnoType
OreGathering.Anims=              ; list of animations
OreGathering.FramesPerDir=15     ; list of integers
OreGathering.Tiberiums=0         ; list of Tiberium IDs
```

### Customizable Teleport/Chrono Locomotor settings per TechnoType

![image](_static/images/cust-Chrono.gif)
*Chrono Legionnaire and Ronco using different teleportation settings in [YR: New War](https://www.moddb.com/mods/yuris-revenge-new-war)*

- You can now specify Teleport/Chrono Locomotor settings per TechnoType to override default rules values. Unfilled values default to values in `[General]`.
- Only applicable to Techno that have Teleport/Chrono Locomotor attached.

In `rulesmd.ini`:
```ini
[SOMETECHNO]            ; TechnoType
WarpOut=                ; Anim (played when Techno warping out)
WarpIn=                 ; Anim (played when Techno warping in)
WarpAway=               ; Anim (played when Techno chronowarped by chronosphere)
ChronoTrigger=          ; boolean, if yes then delay varies by distance, if no it is a constant
ChronoDistanceFactor=   ; integer, amount to divide the distance to destination by to get the warped out delay
ChronoMinimumDelay=     ; integer, the minimum delay for teleporting, no matter how short the distance
ChronoRangeMinimum=     ; integer, can be used to set a small range within which the delay is constant
ChronoDelay=            ; integer, delay after teleport for chronosphere
```

### Customizable unit image in art

- `Image` tag in art INI is no longer limited to AnimationTypes and BuildingTypes, and can be applied to all TechnoTypes (InfantryTypes, VehicleTypes, AircraftTypes, BuildingTypes).
- The tag specifies **only** the file name (without extension) of the asset that replaces TechnoType's graphics. If the name in `Image` is also an entry in the art INI, **no tags will be read from it**.
- **By default this feature is disabled** to remain compatible with YR. To use this feature, enable it in rules with `ArtImageSwap=true`.
- This feature supports SHP images for InfantryTypes, SHP and VXL images for VehicleTypes and VXL images for AircraftTypes.

In `rulesmd.ini`:
```ini
[General]
ArtImageSwap=false  ; disabled by default
```

In `artmd.ini`:
```ini
[SOMETECHNO]
Image=              ; name of the file that will be used as image, without extension
```

### Customize resource storage

- Now Ares `Storage` feature can set which Tiberium type from `[Tiberiums]` list should be used for storing resources in structures with `Refinery.UseStorage=yes` and `Storage` > 0.
- This tag can not be used without Ares.

In `rulesmd.ini`:
```ini
[General]
Storage.TiberiumIndex=-1  ; integer, [Tiberiums] list index
```

### Exploding unit passenger killing customization

- By default `Explodes=true` TechnoTypes have all of their passengers killed when they are destroyed. This behaviour can now be disabled by setting `Explodes.KillPassengers=false`.

In `rulesmd.ini`:
```ini
[SOMETECHNO]                 ; TechnoType
Explodes.KillPassengers=true ; boolean
```

### Jumpjet unit layer deviation customization

- Allows turning on or off jumpjet unit behaviour where they fluctuate between `air` and `top` layers based on whether or not their current altitude is equal / below or above `JumpjetHeight` or `[JumpjetControls] -> CruiseHeight` if former is not set on TechnoType. If disabled, airborne jumpjet units exist only in `air` layer. `JumpjetAllowLayerDeviation` defaults to value of `[JumpjetControls] -> AllowLayerDeviation` if not specified.

In `rulesmd.ini`:
```ini
[JumpjetControls]
AllowLayerDeviation=true         ; boolean

[SOMETECHNO]                     ; TechnoType
JumpjetAllowLayerDeviation=true  ; boolean
```

### Jumpjet turning to target

![image](_static/images/jumpjet-turning.gif)
*Jumpjet turning to target applied in [Robot Storm X](https://www.moddb.com/mods/cc-robot-storm-x)*

- Allows jumpjet units to face towards the target when firing from different directions. Set `[JumpjetControls] -> TurnToTarget=yes` to enable it for all jumpjet locomotor units. This behavior can be overriden by setting `[UnitType] -> JumpjetTurnToTarget` for specific units.
- This behavior does not apply to `TurretSpins=yes` units for obvious reasons.

In `rulesmd.ini`:
```ini
[JumpjetControls]
TurnToTarget=false     ; boolean

[SOMEUNITTYPE]         ; UnitType with jumpjet locomotor
JumpjetTurnToTarget=   ; boolean, override the tag in JumpjetControls
```


### Kill spawns on low power

- `Powered=yes` structures that spawns aircraft like Aircrafts Carriers will stop targeting the enemy if low power.
- Spawned aircrafts self-destruct if they are flying.

In `rulesmd.ini`:
```ini
[SOMESTRUCTURE]          ; BuildingType
Powered.KillSpawns=false ; boolean
```

### Re-enable obsolete [JumpjetControls]

- Re-enable obsolete [JumpjetControls], the keys in it will be as the default value of jumpjet units.
  - Moreover, added two tags for missing ones.

In `rulesmd.ini`:
```ini
[JumpjetControls]
Crash=5.0        ; floating point value
NoWobbles=false  ; boolean
```

```{note}
`CruiseHeight` is for `JumpjetHeight`, `WobblesPerSecond` is for `JumpjetWobbles`, `WobbleDeviation` is for `JumpjetDeviation`, and `Acceleration` is for `JumpjetAccel`. All other corresponding keys just simply have no Jumpjet prefix.
```

### Forbid parallel AI queues

- You can now set if specific types of factories do not have AI production cloning issue instead of Ares' indiscriminate behavior of `AllowParallelAIQueues=no`.
  - If `AllowParallelAIQueues=no` (*Ares feature*) is set, the tags have no effect.

In `rulesmd.ini`
```ini
[GlobalControls]
AllowParallelAIQueues=yes           ; must be set yes/true unless you don't use Ares
ForbidParallelAIQueues.Infantry=no  ; boolean
ForbidParallelAIQueues.Vehicle=no   ; boolean
ForbidParallelAIQueues.Navy=no      ; boolean
ForbidParallelAIQueues.Aircraft=no  ; boolean
ForbidParallelAIQueues.Building=no  ; boolean
```

## Terrains

### Customizable ore spawners

![image](_static/images/ore-01.png)
*Different ore spawners in [Rise of the East](https://www.moddb.com/mods/riseoftheeast) mod*

- You can now specify which type of ore certain TerrainType would generate.
- It's also now possible to specify a range value for an ore generation area different compared to standard 3x3 rectangle. Ore will be uniformly distributed across all affected cells in a spread range.
- You can specify which ore growth stage will be spawned and how much cells will be filled with ore per ore generation animation. Corresponding tags accept either a single integer value or two comma-separated values to allow randomized growth stages from the range (inclusive).

In `rulesmd.ini`:
```ini
[SOMETERRAINTYPE]             ; TerrainType
SpawnsTiberium.Type=0         ; tiberium/ore type index
SpawnsTiberium.Range=1        ; integer, radius in cells
SpawnsTiberium.GrowthStage=3  ; single int / comma-sep. range
SpawnsTiberium.CellsPerAnim=1 ; single int / comma-sep. range
```

### Minimap color customization

- TerrainTypes can now be made to display on minimap with different colors by setting `MinimapColor`.

In `rulesmd.ini`:
```ini
[SOMETERRAINTYPE]  ; TerrainType
MinimapColor=      ; integer - Red,Green,Blue
```

### Passable & buildable-upon TerrainTypes

- TerrainTypes can now be made passable or buildable upon by setting `IsPassable` or `CanBeBuiltOn`, respectively.
  - Movement cursor is displayed on `IsPassable` TerrainTypes unless force-firing.
  - `CanBeBuiltOn=true` terrain objects are removed when building is placed on them.

In `rulesmd.ini`:
```ini
[SOMETERRAINTYPE]   ; TerrainType
IsPassable=false    ; boolean
CanBeBuiltOn=false  ; boolean
```

## Tiberiums (ores)

### Minimap color customization

- Ore can now be made to display on minimap with different colors by setting `MinimapColor` on Tiberiums.

In `rulesmd.ini`:
```ini
[SOMEORE]      ; Tiberium
MinimapColor=  ; integer - Red,Green,Blue
```

## Vehicles

### Destroy animations

- `DestroyAnim` has been extended to work with VehicleTypes, with option to pick random animation if `DestroyAnim.Random` is set to true. These animations store owner and facing information for use with [CreateUnit logic](New-or-Enhanced-Logics.md#anim-to-unit).

In `rulesmd.ini`:
```ini
[SOMEVEHICLE]                          ; VehicleType
DestroyAnim=                           ; list of animations
DestroyAnim.Random=true                ; boolean
```

### IsSimpleDeployer vehicle deploy animation / direction customization

- `DeployingAnim.AllowAnyDirection` if set, disables any direction constraints for deployers with `DeployingAnim` set. Only works for ground units.
- `DeployingAnim.KeepUnitVisible` determines if the unit is hidden while the animation is playing.
- `DeployingAnim.ReverseForUndeploy` controls whether or not the animation is played in reverse for undeploying.
- `DeployingAnim.UseUnitDrawer` controls whether or not the animation is displayed in the unit's palette and team colours or regular animation palette, including a potential custom palette.

In `rulesmd.ini`:
```ini
[SOMEVEHICLE]                          ; VehicleType
DeployingAnim.AllowAnyDirection=false  ; boolean
DeployingAnim.KeepUnitVisible=false    ; boolean
DeployingAnim.ReverseForUndeploy=true  ; boolean
DeployingAnim.UseUnitDrawer=true       ; boolean
```

### Stationary vehicles

- Setting VehicleType `Speed` to 0 now makes game treat them as stationary, behaving in very similar manner to deployed vehicles with `IsSimpleDeployer` set to true. Should not be used on buildable vehicles, as they won't be able to exit factories.

### Preserve Iron Curtain status on type conversion

![image](_static/images/preserve-ic.gif)
*Bugfix in action*

- Iron Curtain status is now preserved by default when converting between TechnoTypes via `DeploysInto`/`UndeploysInto`.
  - This behavior can be turned off per-TechnoType and global basis.
  - `IronCurtain.Modifier` is re-applied upon type conversion.

In `rulesmd.ini`:
```ini
[CombatDamage]
IronCurtain.KeptOnDeploy=yes ; boolean

[SOMETECHNO]                 ; VehicleType with DeploysInto or BuildingType with UndeploysInto
IronCurtain.KeptOnDeploy=    ; boolean, default to [CombatDamage]->IronCurtain.KeptOnDeploy
```

## Warheads

### Allowing damage dealt to firer

- You can now allow warhead to deal damage (and apply damage-adjacent effects such as `KillDriver` and `DisableWeapons/Sonar/Flash.Duration` *(Ares features)*) on the object that is considered as the firer of the Warhead even if it does not have `DamageSelf=true`.
  - Note that effect of `Psychedelic=true`, despite being tied to damage will still fail to apply on the firer as it does not affect any objects belonging to same house as the firer, including itself.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]            ; WarheadType
AllowDamageOnSelf=false  ; boolean
```

### Customizing decloak on damaging targets

- You can now specify whether or not the warhead decloaks objects that are damaged by the warhead.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]               ; WarheadType
DecloakDamagedTargets=true  ; boolean
```

### Restricting screen shaking to current view

- You can now specify whether or not the warhead can only shake screen (`ShakeX/Ylo/hi`) if it is detonated while visible on current screen view.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]       ; WarheadType
ShakeIsLocal=false  ; boolean
```

## Weapons

### Customizable disk laser radius

![image](_static/images/disklaser-radius-values-01.gif)
- You can now set disk laser animation radius using a new tag.

In `rulesmd.ini`:
```ini
[SOMEWEAPON]          ; WeaponType
DiskLaser.Radius=38.2 ; floating point value
                      ; 38.2 is roughly the default saucer disk radius
```

### Single-color lasers

![image](_static/images/issinglecolor.gif)
*Comparison of `IsSingleColor=yes` lasers with higher thickness to regular ones* ([RA2: Reboot](https://www.moddb.com/mods/reboot))

- You can now set laser to draw using only `LaserInnerColor` by setting `IsSingleColor`, in same manner as `IsHouseColor` lasers do using player's team color. These lasers respect laser thickness. Note that this is not available on prism support weapons.

In `rulesmd.ini`:
```ini
[SOMEWEAPON]         ; WeaponType
IsSingleColor=false  ; boolean
```

### Toggle-able ElectricBolt visuals

![image](_static/images/ebolt.gif)
*EBolt customization utilized for different Tesla bolt weapon usage* ([RA2: Reboot](https://www.moddb.com/mods/reboot))


- You can now specify individual ElectricBolt bolts you want to disable. Note that this is only a visual change.

In `rulesmd.ini`:
```ini
[SOMEWEAPONTYPE]       ; WeaponType
IsElectricBolt=true    ; an ElectricBolt Weapon, vanilla tag
Bolt.Disable1=false    ; boolean
Bolt.Disable2=false    ; boolean
Bolt.Disable3=false    ; boolean
```

## RadialIndicator visibility

In vanilla game, a structure's radial indicator can be drawn only when it belongs to the player. Now it can also be visible to observer.
On top of that, you can specify its visibility from other houses.

In `rulesmd.ini`:
```ini
[AudioVisual]
RadialIndicatorVisibility=allies  ; list of Affected House Enumeration (owner/self | allies/ally | enemies/enemy | all)
```
