# New / Enhanced Logics

This page describes all the engine features that are either new and introduced by Phobos or significantly extended or expanded.

## New types / ingame entities

### Shields

![image](_static/images/technoshield-01.gif)  
*Buildings, Infantries and Vehicles with Shield in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)*

In `rulesmd.ini`:
```ini
[AudioVisual]
Pips.Shield=-1,-1,-1           ; int, frames of pips.shp for Green, Yellow, Red
Pips.Shield.Building=-1,-1,-1  ; int, frames of pips.shp for Green, Yellow, Red

[ShieldTypes]
0=SOMESHIELDTYPE

[SOMESHIELDTYPE]               ; ShieldType name
Strength=0                     ; integer
Armor=none                     ; ArmorType
Powered=false                  ; boolean
AbsorbOverDamage=false         ; boolean
SelfHealing=0.0                ; double, percents or absolute
SelfHealing.Rate=0.0           ; double, ingame minutes
Respawn=0.0                    ; double, percents or absolute
Respawn.Rate=0.0               ; double, ingame minutes
BracketDelta=0                 ; integer - pixels
IdleAnim=                      ; animation
IdleAnim.OfflineAction=Hides   ; AttachedAnimFlag (None, Hides, Temporal, Paused or PausedTemporal)
IdleAnim.TemporalAction=Hides  ; AttachedAnimFlag (None, Hides, Temporal, Paused or PausedTemporal)
BreakAnim=                     ; animation
HitAnim=                       ; animation

[SOMETECHNO]                   ; TechnoType
ShieldType=SOMESHIELDTYPE      ; ShieldType; none by default

[SOMEWARHEAD]                  ; WarheadType
PenetratesShield=false         ; boolean
BreaksShield=false             ; boolean
```
- Now you can have a shield for any TechnoType. It serves as a second health pool with independent `Armor` and `Strength` values.
  - Negative damage will recover shield, unless shield has been broken. If shield isn't full, all negative damage will be absorbed by shield.
  - When the TechnoType with a unbroken shield, `[ShieldType]->Armor` will replace `[TechnoType]->Armor` for game calculation.
- When executing `DeploysInto` or `UndeploysInto`, if both of the TechnoTypes have shields, the transformed unit/building would keep relative shield health (in percents), same as with `Strength`. If one of the TechnoTypes doesn't have shields, it's shield's state on conversion will be preserved until converted back.
  - This also works with Ares' `Convert.*`.
- `Powered` controls whether or not the shield is active when a unit is running low on power or it is affected by EMP.
  - Attention, if TechnoType itself is not `Powered`, then the shield won't be offline when low power. 
- `AbsorbOverDamage` controls whether or not the shield absorbs damage dealt beyond shield's current strength when the shield breaks.
- `SelfHealing` and `Respawn` respect the following settings: 0.0 disables the feature, 1%-100% recovers/respawns the shield strength in percentage, other number recovers/respawns the shield strength directly. Specially, `SelfHealing` with a negative number deducts the shield strength.
  - If you want shield recovers/respawns 1 HP per time, currently you need to set tag value to any number between 1 and 2, like `1.1`.
- `SelfHealing.Rate` and `Respawn.Rate` respect the following settings: 0.0 instantly recovers the shield, other values determine the frequency of shield recovers/respawns in ingame minutes.
- `IdleAnim`, if set, will be played while the shield is intact. This animation is automatically set to loop indefinitely.
  - `Bouncer=yes` animations are not supported at the moment.
- `IdleAnim.OfflineAction` indicates what happens to the animation when the shield is in a low power state.
- `IdleAnim.TemporalAction` indicates what happens to the animation when the shield is attacked by temporal weapons.
- `BreakAnim`, if set, will be played when the shield has been broken.
- `HitAnim`, if set, will be played when the shield is attacked, similar to `WeaponNullifyAnim` for Iron Curtain.
- A TechnoType with a shield will show its shield Strength. An empty shield strength bar will be left after destroyed if it is respawnable.
  - Buildings now use the 5th frame of `pips.shp` to display the shield strength while other units uses the 16th frame by default.
  - `Pips.Shield` can be used to specify which pip frame should be used as shield strength. If only 1 digit set, then it will always display it, or if 3 digits set, it will respect `ConditionYellow` and `ConditionRed`. `Pips.Shield.Building` is used for BuildingTypes.
  - `pipbrd.shp` will use its 4th frame to display an infantry's shield strength and the 3th frame for other units if `pipbrd.shp` has extra 2 frames. And `BracketDelta` can be used as additional `PixelSelectionBracketDelta` for shield strength.
- Warheads have new options that interact with shields.
  - `PenetratesShield` allows the warhead ignore the shield and always deal full damage to the TechnoType itself. It also allows targeting the TechnoType as if shield isn't existed.
  - `BreaksShield` allows the warhead to always break shields of TechnoTypes, regardless of the amount of strength the shield has remaining or the damage dealt, assuming it affects the shield's armor type. Residual damage, if there is any, still respects `AbsorbOverDamage`.


### Laser Trails

![Laser Trails](_static/images/lasertrails.gif)  
*Laser trails used in [Rise of the East](https://www.moddb.com/mods/riseoftheeast)*

- Technos and projectiles can now have colorful trails of different transparency, thickness and color, which are drawn via laser drawing code.
- Technos and projectiles can have multiple laser trails. For technos each trail can have custom laser trail type and FLH offset relative to turret and body.

```{warning}
Laser trails are very resource intensive! Due to the game not utilizing GPU having a lot of trails can quickly drop the FPS on even good machines. To reduce that effect:
 - don't put too many laser trails on units and projectiles;
 - make sure you set as high `SegmentLength` value as possible without trails being too jagged;
 - try to keep the length of the trail minimal (can be achieved with smaller `FadeDuration` durations).
```

In `artmd.ini`:
```ini
[LaserTrailTypes]
0=SOMETRAIL

[SOMETRAIL]                 ; LaserTrailType name
IsHouseColor=no             ; boolean
Color=255,0,0               ; integer - Red,Green,Blue
FadeDuration=64             ; integer
Thickness=4                 ; integer
SegmentLength=128           ; integer, minimal length of each trail segment
IgnoreVertical=no           ; boolean, whether the trail won't be drawn on vertical movement
IsIntense=no                ; boolean, whether the laser is "supported" (AKA prism forwarding)

[SOMEPROJECTILE]            ; BulletType Image
LaserTrail.Types=SOMETRAIL  ; list of LaserTrailTypes

[SOMETECHNO]                ; TechnoType Image
LaserTrailN.Type=SOMETRAIL  ; LaserTrailType
LaserTrailN.FLH=0,0,0       ; integer - Forward,Lateral,Height
LaserTrailN.IsOnTurret=no   ; boolean, whether the trail origin is turret
; where N = 0, 1, 2, ...
```

### Custom Radiation Types

![image](_static/images/radtype-01.png)  
*Mixing different radiation types*

- Allows to have custom radiation type for any weapon now. More details on radiation [here](https://www.modenc.renegadeprojects.com/Radiation).

In `rulesmd.ini`
```ini
[RadiationTypes]
0=SOMERADTYPE

[SOMEWEAPON]                    ; WeaponType
RadType=Radiation               ; RadType to use instead
                                ; of default [Radiation]

[SOMERADTYPE]                   ; custom RadType name
RadDurationMultiple=1           ; int
RadApplicationDelay=16          ; int
RadApplicationDelay.Building=0  ; int
RadLevelMax=500                 ; int
RadLevelDelay=90                ; int
RadLightDelay=90                ; int
RadLevelFactor=0.2              ; double
RadLightFactor=0.1              ; double
RadTintFactor=1.0               ; double
RadColor=0,255,0                ; RGB
RadSiteWarhead=RadSite          ; WarheadType
```

## Animations

### Anim-to-Unit

![image](_static/images/animToUnit.gif)  

- Animations can now create (or "convert" to) units when they end.
  - Because anims usually don't have an owner the unit will be created with civilian owner unless you use `DestroyAnim` which was modified to store owner and facing information from the destroyed unit.

In `rulesmd.ini`:
```ini
[SOMEUNIT]                  ; UnitType
DestroyAnim.Random=yes      ; boolean, whether to randomize DestroyAnim
```

In `art.ini`:
```ini
[SOMEANIM]                          ; AnimationType
CreateUnit=                         ; UnitType
CreateUnit.Facing=0                 ; unsigned short, `CreateUnit` facings in range of 0-255
CreateUnit.RandomFacing=yes         ; boolean, `CreateUnit` use random facings
CreateUnit.InheritFacings=no        ; boolean, inherit facing from destroyed unit
CreateUnit.InheritTurretFacings=no  ; boolean, inherit facing from destroyed unit
CreateUnit.RemapAnim=no             ; boolean, whether to remap anim to owner color
CreateUnit.Mission=Guard            ; MissionType
CreateUnit.Owner=Victim             ; owner house kind, Invoker/Killer/Victim/Civilian/Special/Neutral/Random
```


## Buildings

### Extended building upgrades logic

![image](_static/images/powersup.owner-01.png)  
*Upgrading own and allied Power Plants in [CnC: Final War](https://www.moddb.com/mods/cncfinalwar)*

- Building upgrades now can be placed on own buildings, on allied buildings and/or on enemy buildings. These three owners can be specified via a new tag, comma-separated. When upgrade is placed on building, it automatically changes it's owner to match the building's owner.
- One upgrade can now be applied to multiple buildings via a new tag, comma-separated.
  - Currently Ares-introduced build limit for building upgrades doesn't work with this feature. This may change in future.

In `rulesmd.ini`:
```ini
[UPGRADENAME]       ; BuildingType
PowersUp.Owner=Self ; list of owners (Self, Ally and/or Enemy)
PowersUp.Buildings= ; list of BuildingTypes
```

## Vehicles

### Stationary vehicles

- Setting VehicleType `Speed` to 0 now makes game treat them as stationary, behaving in very similar manner to deployed vehicles with `IsSimpleDeployer` set to true. Should not be used on buildable vehicles, as they won't be able to exit factories.

### No Manual Move

- You can now specify whether a TechnoType is unable to receive move command.

```ini
[SOMETECHNO]           ; TechnoType
NoManualMove=no        ; boolean
```

## Technos

### Mind Control enhancement

![image](_static/images/mindcontrol-max-range-01.gif)  
*Mind Control Range Limit used in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)*  
![image](_static/images/mindcontrol-multiple-01.gif)  
*Multiple Mind Control unit auto-releases the first victim in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)*

- Mind controllers now can have the upper limit of the control distance. Tag values greater than 0 will activate this feature.
- Mind controllers with multiple controlling slots can now release the first controlled unit when they have reached the control limit and are ordered to control a new target.
- Allows Warheads to play custom `MindControl.Anim` which defaults to `ControlledAnimationType`.

In `rulesmd.ini`
```ini
[SOMETECHNO]                       ; TechnoType
MindControlRangeLimit=-1.0         ; double
MultiMindControl.ReleaseVictim=no  ; boolean

[SOMEWARHEAD]                            ; Warhead
MindControl.Anim=ControlledAnimationType ; AnimType
```

### Spawn range limit

![image](_static/images/spawnrange-01.gif)  
*Limited pursue range for spawns in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)*

- The spawned units will abort the infinite pursuit if the enemy is out of range.
`Spawner.ExtraLimitRange` adds extra pursuit range to the spawned units.

In `rulesmd.ini`:
```ini
[SOMETECHNO]              ; TechnoType
Spawner.LimitRange=no     ; boolean
Spawner.ExtraLimitRange=0 ; integer
```

### Promoted Spawns

![image](_static/images/promotedspawns-01.gif)  
*Promoted Spawns in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)*

- The spawned units will promote as their owner's veterancy.

In `rulesmd.ini`:
```ini
[SOMETECHNO]              ; TechnoType
Promote.IncludeSpawns=no  ; boolean
```

## Weapons

### Strafing aircraft weapon customization

![image](_static/images/strafing-01.gif)  
*Strafing aircraft weapon customization in [Project Phantom](https://www.moddb.com/mods/project-phantom)*

- Some of the behavior of strafing aircraft weapons (weapon projectile has `ROT` below 2) can now be customized.
  - `Strafing.Shots` controls the number of times the weapon is fired during a single strafe run. `Ammo` is only deducted at the end of the strafe run, regardless of the number of shots fired. Valid values range from 1 to 5, any values smaller or larger are effectively treated same as either 1 or 5, respectively. Defaults to 5.
  - `Strafing.SimulateBurst` controls whether or not the shots fired during strafing simulate behavior of `Burst`, allowing for alternating firing offset. Only takes effect if weapon has `Burst` set to 1 or undefined. Defaults to false.

In `rulesmd.ini`:
```ini
[SOMEWEAPON]        ; WeaponType
Strafing.Shots=5             ; integer
Strafing.SimulateBurst=false ; bool
```

### Radiation enhancements

- Radiation now has owner by default, so any rad-kills will be scored. This behavior can be reverted by a corresponding tag.
  - `AffectsAllies`, `AffectsOwner` and `AffectsEnemies` on `RadSiteWarhead` are respected.
  - Currently the rad maker doesn't gain experience from kills, this may change in future.
- Radiation is now able to deal damage to Buildings. To enable set `RadApplicationDelay.Building` value more than 0.

In `rulesmd.ini`:
```ini
[SOMEWEAPON]    ; WeaponType
Rad.NoOwner=no  ; boolean
```

## Warheads

```{hint}
All new warheads can be used with CellSpread and Ares' GenericWarhead superweapon where applicable.
```

### Generate credits on impact

![image](_static/images/hackerfinallyworks-01.gif)  
*`TransactMoney` used in [Rise of the East](https://www.moddb.com/mods/riseoftheeast) mod*

- Warheads can now give credits to its owner at impact.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]   ; Warhead
TransactMoney=0 ; integer - credits added or subtracted
```

### Reveal map for owner on impact

![image](_static/images/revealwarhead-01.gif)  
*`SpySat=yes` on `[NUKE]` warhead reveals the map when nuclear missile detonates*

- Warheads can now reveal the entire map on impact.
- Reveal only applies to the owner of the warhead.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD] ; Warhead
SpySat=no     ; boolean
```

### Shroud map for enemies on impact

- Warheads can now shroud the entire map on impact.
- Shroud only applies to enemies of the warhead owner.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD] ; Warhead
BigGap=no     ; boolean
```

### Remove disguise on impact

- Warheads can now remove disguise of spies.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]                      ; Warhead
RemoveDisguise=no                  ; boolean
```

### Break Mind Control on impact

- Warheads can now break mind control (doesn't apply to perma-MC-ed objects).

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]                        ; Warhead
RemoveMindControl=no                 ; boolean
```

### Critical damage chance

- Warheads can now apply additional chance-based damage (known as "critical" damage) with the ability to customize chance, damage, affected targets, and animations of critical strike.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]       ; Warhead
Crit.Chance=0.0     ; float, chance on [0.0-1.0] scale
Crit.ExtraDamage=0  ; integer, extra damage
Crit.Affects=all    ; list of "affects" flags (same as SWType's)
Crit.AnimList=      ; list of animations

[SOMETECHNO]     ; TechnoType
ImmuneToCrit=no  ; boolean
```

### Custom 'SplashList' on Warheads

![image](_static/images/splashlist-01.gif)  
- Allows Warheads to play custom water splash animations. See vanilla's [Conventional](https://www.modenc.renegadeprojects.com/Conventional) system here.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]            ; Warhead
SplashList=<none>        ; list of animations to play
SplashList.PickRandom=no ; play a random animation from the list? boolean, defaults to no
```

## Projectiles

### Projectile interception logic

![image](_static/images/projectile-interception-01.gif)  
*Interception logic used in [Tiberium Crisis](https://www.moddb.com/mods/tiberium-crisis) mod*

- Projectiles can now be made targetable by certain TechnoTypes. Interceptor TechnoType's projectile must be `Inviso=yes` and `AA=yes` in order for it to work properly and the projectile must be used in a primary Weapon.
  - `Interceptor.GuardRange` is maximum range of the unit to intercept projectile. The unit weapon range will limit the unit interception range though.
  - `Interceptor.EliteGuardRange` value is used if the unit veterancy is Elite.
  - `Interceptor.MinimumGuardRange` is the minimum range of the unit to intercept projectile. Any projectile under this range will not be intercepted.
  - `Interceptor.EliteMinimumGuardRange` value is used if the unit veterancy is Elite.

In `rulesmd.ini`:
```ini
[SOMETECHNO]                            ; TechnoType
Interceptor=no                          ; boolean
Interceptor.GuardRange=0.0              ; double
Interceptor.EliteGuardRange=0.0         ; double
Interceptor.MinimumGuardRange=0.0       ; double
Interceptor.EliteMinimumGuardRange=0.0  ; double

[SOMEPROJECTILE] ; Projectile
Interceptable=no ; boolean
```

## Script actions

### `71` Timed Area Guard

- Puts the TaskForce into Area Guard Mode for the given units of time. Unlike the original timed Guard script (`5,n`) that just stays in place doing a basic guard operation the "Area Guard" action has a more active role attacking nearby invaders or defending units that needs protection.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=71,n            ; integer, time in ingame seconds
```

### `72` Load Onto Transports

- If the TaskForce contains unit(s) that can be carried by the transports of the same TaskForce then this action will make the units enter the transports. In Single player missions the next action must be "Wait until fully loaded" (`43,0`) or the script will not continue.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=72,0
```

### `73` Wait until ammo is full

- If the TaskForce contains unit(s) that use ammo then the the script will not continue until all these units have fully refilled the ammo.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=73,0
```

### `74-81`, `84-91` New Attack Action

- These Actions instructs the TeamType to use the TaskForce to approach and attack the target specified by the second parameter. Look at the tables below for the possible Actions (first parameter value) and Arguments (the second parameter value).

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=74-81 or 84-91,n
```

| *Action* | *Argument*   | *Repeats* | *Target Priority* | *Description*                                 |
| :------: | :----------: | :-------: | :---------------: | :-------------------------------------------: |
74         | Target Type# | Yes | Closer, higher threat |  |
75         | Target Type# | Yes | Farther, higher threat |  |
76         | Target Type# | Yes | Closer |  |
77         | Target Type# | Yes | Farther |  |
78         | Target Type# | No | Closer, higher threat | Ends when a team member kill the designated target |
79         | Target Type# | No | Farther, higher threat | Ends when a team member kill the designated target |
80         | Target Type# | No | Closer | Ends when a team member kill the designated target |
81         | Target Type# | No | Farther | Ends when a team member kill the designated target |
84         | [AITargetType] index# | Yes | Closer, higher threat |  |
85         | [AITargetType] index# | Yes | Farther, higher threat |  |
86         | [AITargetType] index# | Yes | Closer |  |
87         | [AITargetType] index# | Yes | Farther |  |
88         | [AITargetType] index# | No | Closer, higher threat | Ends when a team member kill the designated target |
89         | [AITargetType] index# | No | Farther, higher threat | Ends when a team member kill the designated target |
90         | [AITargetType] index# | No | Closer | Ends when a team member kill the designated target |
91         | [AITargetType] index# | No | Farther | Ends when a team member kill the designated target |

Note: New Attack actions scripts (74, 75, 78 ,79, 84, 85, 88 & 89) that are focused in target threat use `TargetSpecialThreatCoefficientDefault` and `EnemyHouseThreatBonus` tags from Rulesmd.ini.

The second parameter with a 0-based index for the *[AITargetType]* section specifies the list of possible *[VehicleTypes]*, *[AircraftTypes]*, *[InfantryTypes]* and *[BuildingTypes]* that can be evaluated. The new *[AITargetType]* section must be declared in RulesMD.ini for making this script work:

In `rulesmd.ini`:
```ini
[AITargetType]  ; List of Technos
0=Techno N,Techno N+1, Techno N+1,... Techno N+M
1= ...
2= ...
...
```

The following values are the *Target Type#* which can be used as second parameter of the new attack script actions:

| *Value* | *Target Type*     | *Description*                                 |
| :-----: | :---------------: | :-------------------------------------------: |
1         | Anything          |	Any enemy *[VehicleTypes]*, *[AircraftTypes]*, *[InfantryTypes]* and *[BuildingTypes]* |
2         | Structures        |	Any enemy *[BuildingTypes]* without `Artillary=yes`, `TickTank=yes`, `ICBMLauncher=yes` or `SensorArray=yes` |
3         | Ore Miners        |	Any enemy *[VehicleTypes]* with `Harvester=yes` or `ResourceGatherer=yes`, *[BuildingTypes]* with `ResourceGatherer=yes` |
4         | Infantry          |	Any enemy *[InfantryTypes]* |
5         | Vehicles          |	Any enemy *[VehicleTypes]*, *[AircraftTypes]*, *[BuildingTypes]* with `Artillary=yes`, `TickTank=yes`, `ICBMLauncher=yes` & `SensorArray=yes` |
6         | Factories         |	Any enemy *[BuildingTypes]* with a Factory= setting |
7         | Base Defenses     |	Any enemy *[BuildingTypes]* with `IsBaseDefense=yes` |
8         | House Threats     |	Any object that targets anything of the Team's House or any enemy that is near to the Team Leader |
9         | Power Plants      |	Any enemy *[BuildingTypes]* with positive `Power=` values |
10        | Occupied          |	Any *[BuildingTypes]* with garrisoned infantry |
11        | Tech Buildings    |	Any *[BuildingTypes]* with `Unsellable=yes`, `Capturable=yes`, negative `TechLevel=` values or appears in `[AI]>NeutralTechBuildings=` list |
12        |	Refinery          |	Any enemy *[BuildingTypes]* with `Refinery=yes` or `ResourceGatherer=yes`, *[VehicleTypes]* with `ResourceGatherer=yes` & `Harvester=no` (i.e. Slave Miner) |
13        |	Mind Controller   |	Anything *[VehicleTypes]*, *[AircraftTypes]*, *[InfantryTypes]* and *[BuildingTypes]* with `MindControl=yes` in the weapons Warheads |
14        |	Air Units         |	Any enemy *[AircraftTypes]*, flying *[VehicleTypes]* or *[InfantryTypes]* |
15        |	Naval             |	Any enemy *[BuildingTypes]* and *[VehicleTypes]* with a `Naval=yes`, any enemy *[VehicleTypes]*, *[AircraftTypes]*, *[InfantryTypes]* in a water cell |
16        |	Disruptors        |	Any enemy objects with positive `InhibitorRange=` values, positive `RadarJamRadius=` values, `CloakGenerator=yes` or `GapGenerator=yes` |
17        |	Ground Vehicles   |	Any enemy *[VehicleTypes]* without `Naval=yes`, landed *[AircraftTypes]*, Deployed vehicles into *[BuildingTypes]* |
18        |	Economy           |	Any enemy *[VehicleTypes]* with `Harvester=yes` or `ResourceGatherer=yes`, *[BuildingTypes]* with `Refinery=yes`, `ResourceGatherer=yes` or `OrePurifier=yes` |
19        |	Infantry Factory  |	Any enemy *[BuildingTypes]* with `Factory=InfantryType` |
20        |	Vehicle Factory   |	Any enemy *[BuildingTypes]* with `Factory=UnitType` |
21        |	Aircraft Factory  |	Any enemy *[BuildingTypes]* with `Factory=AircraftType` |
22        |	Radar             |	Any enemy *[BuildingTypes]* with `Radar=yes` or `SpySat=yes` |
23        |	Tech Lab          |	Any enemy *[BuildingTypes]* in `[AI]>BuildTech=` list |
24        |	Naval Factory     |	Any enemy *[BuildingTypes]* with `Naval=yes` and `Factory=UnitType` |
25        |	Super Weapon      |	Any enemy *[BuildingTypes]* with `SuperWeapon=`, `SuperWeapon2=` or `SuperWeapons=` |
26        |	Construction Yard |	Any enemy *[BuildingTypes]* with `ConstructionYard=yes` and `Factory=BuildingType` |
27        |	Neutrals          |	Any neutral object (Civilian) |
28        |	Generators        |	Any enemy *[BuildingTypes]* with `CloakGenerator=yes` or `GapGenerator=yes` |
29        |	Radar Jammer      |	Any enemy objects with positive `RadarJamRadius=` values |
30        |	Inhibitors        |	Any enemy objects with positive `InhibitorRange=` values |
31        |	Naval Units       |	Any enemy *[VehicleTypes]* with a `Naval=yes` or any enemy *[VehicleTypes]*, *[AircraftTypes]*, *[InfantryTypes]* in a water cell |
32        | Mobile Units      |	Anything *[VehicleTypes]*, *[AircraftTypes]* and *[InfantryTypes]* |
33        |	Capturable        |	Any *[BuildingTypes]* with `Capturable=yes` or any *[BuildingTypes]* with `BridgeRepairHut=yes` and `Repairable=yes` |

### `82` Decrease AI Trigger Current Weight

- When executed this decreases the current Weight of the AI Trigger.The current Weight will never surprass the Minimum Weight and Maximum Weight limits of the AI Trigger. Take note that all TeamTypes of the same AI Trigger will update sooner or later the AI Trigger Current Weight. The second parameter is a positive value.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=82,n
```

### `83` Increase AI Trigger Current Weight

- When executed this increases the current Weight of the AI Trigger.The current Weight will never surprass the Minimum Weight and Maximum Weight limits of the AI Trigger. Take note that all TeamTypes of the same AI Trigger will update sooner or later the AI Trigger Current Weight. The second parameter is a positive value.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=83,n
```

### `92` Wait If No Target Found

- When executed before a new Attack ScriptType Actions like `74-81` and `84-91` the TeamType will remember that must wait 1 second if no target was selected. The second parameter is a positive value that specifies how much retries the Attack will do when no target was found before new Attack ScriptType Action is discarded & the script execution jumps to the next line. The value `0` means infinite retries.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=92,n            ; integer n=0
```

### `93` Team's Trigger Weight Reward

- When executed before a new Attack ScriptType Actions like `74-81` and `84-91` the TeamType will remember that must be rewarded increasing the current Weight of the AI Trigger when the TeamType Target was killed by any of the Team members. The current Weight will never surprass the Minimum Weight and Maximum Weight limits of the AI Trigger. The second parameter is a positive value.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=93,n            ; integer n=0
```

### `94` Pick A Random Script

- When executed this action picks a random Script Type and replaces the current script by the new picked. The second parameter is a 0-based index from the new section *[AIScriptsList]* explained below.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=94,n
```

The second parameter is a 0-based index for the *[AIScriptsList]* section that specifies the list of possible *[VehicleTypes]*, *[AircraftTypes]*, *[InfantryTypes]* and *[BuildingTypes]* that can be evaluated. The new *[AIScriptsList]* section must be declared in RulesMD.ini for making this script work:

In `rulesmd.ini`:
```ini
[AIScriptsList]  ; List of Technos
0=Techno N,Techno N+1, Techno N+1,... Techno N+M
1= ...
2= ...
...
```

### `95-102` Moving Team to techno location

- These Actions instructs the TeamType to use the TaskForce to approach the target specified by the second parameter. Look at the tables below for the possible Actions (first parameter value).

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=95-102,n
```

| *Action* | *Argument*    | Target Owner | *Target Priority* |
| :------: | :-----------: | :----------: | :---------------: |
95         | Target Type# | Enemy | Closer, higher threat |
96         | Target Type# | Enemy | Farther, higher threat |
97         | Target Type# | Friendly | Closer |
98         | Target Type# | Friendly | Farther |
99         | [AITargetType] index# | Enemy | Closer, higher threat |
100        | [AITargetType] index# | Enemy | Closer, higher threat |
101        | [AITargetType] index# | Friendly | Closer |
102        | [AITargetType] index# | Friendly | Farther |
