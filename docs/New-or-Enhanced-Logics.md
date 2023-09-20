# New / Enhanced Logics

This page describes all the engine features that are either new and introduced by Phobos or significantly extended or expanded.

## New types / ingame entities

### Custom Radiation Types

![image](_static/images/radtype-01.png)
*Mixing different radiation types*

- Any weapon can now have a custom radiation type. More details on radiation [here](https://www.modenc.renegadeprojects.com/Radiation).
- There are several new properties available to all radiation types.
  - `RadApplicationDelay.Building` can be set to value higher than 0 to allow radiation to damage buildings.
  - `RadSiteWarhead.Detonate` can be set to make `RadSiteWarhead` detonate on affected objects rather than only be used to dealt direct damage. This enables most Warhead effects, display of animations etc.
  - `RadHasOwner`, if set to true, makes damage dealt by the radiation count as having been dealt by the house that fired the projectile that created the radiation field. This means that Warhead controls such as `AffectsAllies` will be respected and any units killed will count towards that player's destroyed units count.
  - `RadHasInvoker`, if set to true, makes the damage dealt by the radiation count as having been dealt by the TechnoType (the 'invoker') that fired the projectile that created the radiation field. In addition to the effects of `RadHasOwner`, this will also grant experience from units killed by the radiation to the invoker. Note that if the invoker dies at any point during the radiation's lifetime it continues to behave as if not having an invoker.
- By default `UseGlobalRadApplicationDelay` is set to true. This makes game always use `RadApplicationDelay` and `RadApplicationDelay.Building` from `[Radiation]` rather than specific radiation types. This is a performance-optimizing measure that should be disabled if a radiation type declares different application delay.

In `rulesmd.ini`:
```ini
[RadiationTypes]
0=SOMERADTYPE

[Radiation]
UseGlobalRadApplicationDelay=true  ; boolean

[SOMEWEAPON]                       ; WeaponType
RadType=Radiation                  ; RadType to use instead of default of [Radiation]

[SOMERADTYPE]                      ; RadType
RadDurationMultiple=1              ; integer
RadApplicationDelay=16             ; integer
RadApplicationDelay.Building=0     ; integer
RadLevelMax=500                    ; integer
RadLevelDelay=90                   ; integer
RadLightDelay=90                   ; integer
RadLevelFactor=0.2                 ; floating point value
RadLightFactor=0.1                 ; floating point value
RadTintFactor=1.0                  ; floating point value
RadColor=0,255,0                   ; integer - Red,Green,Blue
RadSiteWarhead=RadSite             ; WarheadType
RadSiteWarhead.Detonate=false      ; boolean
RadHasOwner=false                  ; boolean
RadHasInvoker=false                ; boolean
```

### Laser Trails

![Laser Trails](_static/images/lasertrails.gif)
*Laser trails used in [Rise of the East](https://www.moddb.com/mods/riseoftheeast)*

- Technos, Projectiles, and VoxelAnims can now have colorful trails of different transparency, thickness and color, which are drawn via laser drawing code.
- Technos, Projectiles, and VoxelAnims can have multiple laser trails. For technos each trail can have custom laser trail type and FLH offset relative to turret and body.

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

[SOMETRAIL]                   ; LaserTrailType name
IsHouseColor=false            ; boolean
Color=255,0,0                 ; integer - Red,Green,Blue
FadeDuration=64               ; integer
Thickness=4                   ; integer
SegmentLength=128             ; integer, minimal length of each trail segment
IgnoreVertical=false          ; boolean, whether the trail won't be drawn on vertical movement
IsIntense=false               ; boolean, whether the laser is "supported" (AKA prism forwarding)
CloakVisible=false            ; boolean, whether the laser is visible when the attached unit is cloaked

[SOMEPROJECTILE]              ; BulletType Image
LaserTrail.Types=SOMETRAIL    ; list of LaserTrailTypes

[SOMETECHNO]                  ; TechnoType Image
LaserTrailN.Type=SOMETRAIL    ; LaserTrailType
LaserTrailN.FLH=0,0,0         ; integer - Forward,Lateral,Height
LaserTrailN.IsOnTurret=false  ; boolean, whether the trail origin is turret
; where N = 0, 1, 2, ...
```

In `rulesmd.ini`:
```ini
[SOMEVOXELANIM]             ; VoxelAnim
LaserTrail.Types=SOMETRAIL  ; list of LaserTrailTypes
```

### Shields

![image](_static/images/technoshield-01.gif)
*Buildings, Infantries and Vehicles with Shield in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)*

In `rulesmd.ini`:
```ini
[AudioVisual]
Pips.Shield=-1,-1,-1                        ; integer, frames of pips.shp (zero-based) for Green, Yellow, Red
Pips.Shield.Building=-1,-1,-1               ; integer, frames of pips.shp (zero-based) for Green, Yellow, Red
Pips.Shield.Background=PIPBRD.SHP           ; filename - including the .shp/.pcx extension
Pips.Shield.Building.Empty=0                ; integer, frame of pips.shp (zero-based) for empty building pip

[ShieldTypes]
0=SOMESHIELDTYPE

[SOMESHIELDTYPE]                            ; ShieldType name
Strength=0                                  ; integer
InitialStrength=0                           ; integer
Armor=none                                  ; ArmorType
InheritArmorFromTechno=false                ; boolean
Powered=false                               ; boolean
AbsorbOverDamage=false                      ; boolean
SelfHealing=0.0                             ; double, percents or absolute
SelfHealing.Rate=0.0                        ; double, ingame minutes
SelfHealing.RestartInCombat=true            ; boolean
SelfHealing.RestartInCombatDelay=0          ; integer, game frames
Respawn=0.0                                 ; double, percents or absolute
Respawn.Rate=0.0                            ; double, ingame minutes
BracketDelta=0                              ; integer - pixels
Pips=-1,-1,-1                               ; integer, frames of pips.shp (zero-based) for Green, Yellow, Red
Pips.Building=-1,-1,-1                      ; integer, frames of pips.shp (zero-based) for Green, Yellow, Red
Pips.Background=                            ; filename - including the .shp/.pcx extension
Pips.Building.Empty=                        ; integer, frame of pips.shp (zero-based) for empty building pip
IdleAnim=                                   ; Animation
IdleAnim.ConditionYellow=                   ; Animation
IdleAnim.ConditionRed=                      ; Animation
IdleAnimDamaged=                            ; Animation
IdleAnimDamaged.ConditionYellow=            ; Animation
IdleAnimDamaged.ConditionRed=               ; Animation
IdleAnim.OfflineAction=Hides                ; AttachedAnimFlag (None, Hides, Temporal, Paused or PausedTemporal)
IdleAnim.TemporalAction=Hides               ; AttachedAnimFlag (None, Hides, Temporal, Paused or PausedTemporal)
BreakAnim=                                  ; Animation
HitAnim=                                    ; Animation
BreakWeapon=                                ; WeaponType
AbsorbPercent=1.0                           ; floating point value
PassPercent=0.0                             ; floating point value
ReceivedDamage.Minimum=-2147483648          ; integer
ReceivedDamage.Maximum=2147483647           ; integer
AllowTransfer=                              ; boolean
ImmuneToBerserk=no                          ; boolean
ImmuneToCrit=no                             ; boolean

[SOMETECHNO]                                ; TechnoType
ShieldType=SOMESHIELDTYPE                   ; ShieldType; none by default

[SOMEWARHEAD]                               ; WarheadType
Shield.Penetrate=false                      ; boolean
Shield.Break=false                          ; boolean
Shield.BreakAnim=                           ; Animation
Shield.HitAnim=                             ; Animation
Shield.BreakWeapon=                         ; WeaponType
Shield.AbsorbPercent=                       ; floating point value
Shield.PassPercent=                         ; floating point value
Shield.ReceivedDamage.Minimum=              ; integer
Shield.ReceivedDamage.Maximum=              ; integer
Shield.Respawn.Duration=0                   ; integer, game frames
Shield.Respawn.Amount=0.0                   ; floating point value, percents or absolute
Shield.Respawn.Rate=-1.0                    ; floating point value, ingame minutes
Shield.Respawn.RestartTimer=false           ; boolean
Shield.SelfHealing.Duration=0               ; integer, game frames
Shield.SelfHealing.Amount=0.0               ; floating point value, percents or absolute
Shield.SelfHealing.Rate=-1.0                ; floating point value, ingame minutes
Shield.SelfHealing.RestartInCombat=         ; boolean
Shield.SelfHealing.RestartInCombatDelay=-1  ; integer, game frames
Shield.SelfHealing.RestartTimer=false       ; boolean
Shield.AffectTypes=                         ; List of ShieldType names
Shield.Penetrate.Types=                     ; List of ShieldType names
Shield.Break.Types=                         ; List of ShieldType names
Shield.Respawn.Types=                       ; List of ShieldType names
Shield.SelfHealing.Types=                   ; List of ShieldType names
Shield.AttachTypes=                         ; List of ShieldType names
Shield.RemoveTypes=                         ; List of ShieldType names
Shield.ReplaceOnly=false                    ; boolean
Shield.ReplaceNonRespawning=false           ; boolean
Shield.MinimumReplaceDelay=0                ; integer, game frames
Shield.InheritStateOnReplace=false          ; boolean
```
- Now you can have a shield for any TechnoType. It serves as a second health pool with independent `Armor` and `Strength` values.
  - Shield will not take damage if the TechnoType is under effects of `Temporal` warhead, is Iron Curtained / Force Shielded, has `Immune=true` or if it has `TypeImmune=true` and the damage source is another instance of same TechnoType belonging to same house.
  - Negative damage will recover shield, unless shield has been broken. If shield isn't full, all negative damage will be absorbed by shield.
    - Negative damage weapons will consider targets with active, but not at full health shields in need of healing / repairing unless the Warhead has `Shield.Penetrate=true`, in which case only object health is considered.
  - When a TechnoType has an unbroken shield, `[ShieldType]->Armor` will replace `[TechnoType]->Armor` for targeting and damage calculation purposes.
    - `InheritArmorFromTechno` can be set to true to override this so that `[TechnoType]->Armor` is used even if shield is active and `[ShieldType]->Armor` is ignored.
  - `InitialStrength` can be used to set a different initial strength value from maximum.
- When executing `DeploysInto` or `UndeploysInto`, if both of the TechnoTypes have shields, the transformed unit/building would keep relative shield health (in percents), same as with `Strength`. If one of the TechnoTypes doesn't have shields, it's shield's state on conversion will be preserved until converted back.
  - This also works with Ares' `Convert.*`.
- `Powered` controls whether or not the shield is active when a unit is running low on power or it is affected by EMP.
  - Attention, if TechnoType itself is not `Powered`, then the shield won't be offline when low power.
- `AbsorbOverDamage` controls whether or not the shield absorbs damage dealt beyond shield's current strength when the shield breaks.
- `SelfHealing` and `Respawn` respect the following settings: 0.0 disables the feature, 1%-100% recovers/respawns the shield strength in percentage, other number recovers/respawns the shield strength directly. Specially, `SelfHealing` with a negative number deducts the shield strength.
  - If you want shield recovers/respawns 1 HP per time, currently you need to set tag value to any number between 1 and 2, like `1.1`.
  - If `SelfHealing.RestartInCombat` is set, self-healing timer pauses and then resumes after `SelfHealing.RestartInCombatDelay` frames have passed when the shield gets damaged.
- `SelfHealing.Rate` and `Respawn.Rate` respect the following settings: 0.0 instantly recovers the shield, other values determine the frequency of shield recovers/respawns in ingame minutes.
- `IdleAnim`, if set, will be played while the shield is intact. This animation is automatically set to loop indefinitely.
  - `IdleAnim.ConditionYellow` and `IdleAnim.ConditionRed` can be used to set different animations for when shield health is at or below the percentage defined in `[AudioVisual]`->`ConditionYellow`/`ConditionRed`, respectively. If `IdleAnim.ConditionRed` is not set it falls back to `IdleAnim.ConditionYellow`, which in turn falls back to `IdleAnim`.
  - `IdleAnimDamaged`, `IdleAnimDamaged.ConditionYellow` and `IdleAnimDamaged.ConditionRed` are used in an identical manner, but only when health of the object the shield is attached to is at or below `[AudioVisual]`->`ConditionYellow`. Follows similar fallback sequence to regular `IdleAnim` variants and if none are set, falls back to the regular `IdleAnim` or variants thereof.
  - `Bouncer=true` and `IsMeteor=true` animations can exhibit irregular behaviour when used as `IdleAnim` and should be avoided.
- `IdleAnim.OfflineAction` indicates what happens to the animation when the shield is in a low power state.
- `IdleAnim.TemporalAction` indicates what happens to the animation when the shield is attacked by temporal weapons.
- `BreakAnim`, if set, will be played when the shield has been broken.
- `HitAnim`, if set, will be played when the shield is attacked, similar to `WeaponNullifyAnim` for Iron Curtain.
- `BreakWeapon`, if set, will be fired at the TechnoType once the shield breaks.
- `AbsorbPercent` controls the percentage of damage that will be absorbed by the shield. Defaults to 1.0, meaning full damage absorption.
- `PassPercent` controls the percentage of damage that will *not* be absorbed by the shield, and will be dealt to the unit directly even if the shield is active. Defaults to 0.0 - no penetration.
- `ReceivedDamage.Minimum` & `ReceivedDamage.Maximum` control the minimum and maximum amount of damage that can be dealt to shield in a single hit. This is applied after armor type and `AbsorbPercent` adjustments. If `AbsorbOverDamage=false`, the residual damage dealt to the TechnoType is still based on the original damage before the clamping to the range.
- `AllowTransfer` controls whether or not the shield can be transferred if the TechnoType changes (such as `(Un)DeploysInto` or Ares type conversion). If not set, defaults to true if shield was attached via `Shield.AttachTypes`, otherwise false.
- `ImmuneToBerserk` gives the immunity against `Psychedelic=yes` warhead. Otherwise the berserk effect penetrates shields by default. Note that this shouldn't prevent the unit from targeting at the shielded object. `Versus.shieldArmor=0%` is still required in this case.
- A TechnoType with a shield will show its shield Strength. An empty shield strength bar will be left after destroyed if it is respawnable. Several customizations are available for the shield strength pips.
  - By default, buildings use the 6th frame of `pips.shp` to display the shield strength while others use the 17th frame.
  - `Pips.Shield` can be used to specify which pip frame should be used as shield strength. If only 1 digit is set, then it will always display that frame, or if 3 digits are set, it will use those if shield's current strength is at or below `ConditionYellow` and `ConditionRed`, respectively. `Pips.Shield.Building` is used for BuildingTypes. -1 as value will use the default frame, whether it is fallback to first value or the aforementioned hardcoded defaults.
  - `Pips.Shield.Background` can be used to set the background or 'frame' for non-building pips, which defaults to `pipbrd.shp`. 4th frame is used to display an infantry's shield strength and the 3th frame for other units, or 2nd and 1st respectively if not enough frames are available.
  - `Pips.Shield.Building.Empty` can be used to set the frame of `pips.shp` displayed for empty building strength pips, defaults to 1st frame of `pips.shp`.
  - The above customizations are also available on per ShieldType basis, e.g `[ShieldType]`->`Pips` instead of `[AudioVisual]`->`Pips.Shield` and so on. ShieldType settings take precedence over the global ones, but will fall back to them if not set.
  - `BracketDelta` can be used as additional vertical offset (negative shifts it up) for shield strength bar. Much like `PixelSelectionBracketDelta`, it is not applied on buildings.
- Warheads have new options that interact with shields.
  - `Shield.Penetrate` allows the warhead ignore the shield and always deal full damage to the TechnoType itself. It also allows targeting the TechnoType as if shield doesn't exist.
  - `Shield.Break` allows the warhead to always break shields of TechnoTypes. This is done before damage is dealt.
  - `Shield.BreakAnim` will be displayed instead of ShieldType `BreakAnim` if the shield is broken by the Warhead, either through damage or `Shield.Break`.
  - `Shield.HitAnim` will be displayed instead of ShieldType `HitAnim` if set when Warhead hits the shield.
  - `Shield.BreakWeapon` will be fired instead of ShieldType `BreakWeapon` if the shield is broken by the Warhead, either through damage or `Shield.Break`.
  - `Shield.AbsorbPercent` overrides the `AbsorbPercent` value set in the ShieldType that is being damaged.
  - `Shield.PassPercent` overrides the `PassPercent` value set in the ShieldType that is being damaged.
  - `Shield.ReceivedDamage.Minimum` & `Shield.ReceivedDamage.Maximum` override the values set in in the ShieldType that is being damaged.
  - `Shield.Respawn.Rate` & `Shield.Respawn.Amount` override ShieldType `Respawn.Rate` and `Respawn.Amount` for duration of `Shield.Respawn.Duration` amount of frames. Negative rate & zero or lower amount default to ShieldType values. If `Shield.Respawn.RestartTimer` is set, currently running shield respawn timer is reset, otherwise the timer's duration is adjusted in proportion to the new `Shield.Respawn.Rate` (e.g timer will be same percentage through before and after) without restarting the timer. If the effect expires while respawn timer is running, remaining time is adjusted to proportionally match ShieldType `Respawn.Rate`. Re-applying the effect resets the duration to `Shield.Respawn.Duration`
  - `Shield.SelfHealing.Rate` & `Shield.SelfHealing.Amount` override ShieldType `SelfHealing.Rate` and `SelfHealing.Amount` for duration of `Shield.SelfHealing.Duration` amount of frames. Negative rate & zero or lower amount default to ShieldType values. If `Shield.SelfHealing.RestartTimer` is set, currently running self-healing timer is restarted, otherwise timer's duration is adjusted in proportion to the new `Shield.SelfHealing.Rate` (e.g timer will be same percentage through before and after) without restarting the timer. If the effect expires while self-healing timer is running, remaining time is adjusted to proportionally match ShieldType `SelfHealing.Rate`. Re-applying the effect resets the duration to `Shield.SelfHealing.Duration`.
    - Additionally `Shield.SelfHealing.RestartInCombat` & `Shield.SelfHealing.RestartInCombatDelay` can be used to override ShieldType settings.
  - `Shield.AffectTypes` allows listing which ShieldTypes can be affected by any of the effects listed above. If none are listed, all ShieldTypes are affected.
    -  `Shield.AffectTypes` can be overriden for specific shield interactions by using keys `Shield.Penetrate.Types`, `Shield.Break.Types`, `Shield.Respawn.Types` and `Shield.SelfHealing.Types` respectively.
  - `Shield.AttachTypes` & `Shield.RemoveTypes` allows listing ShieldTypes that are attached or removed, respectively from any targets affected by the warhead (positive `Verses` values). Normally only first listed ShieldType in `Shield.AttachTypes` is applied.
    - If `Shield.ReplaceOnly` is set, shields from `Shield.AttachTypes` are only applied to affected targets from which shields were simultaneously removed, matching the order listed in `Shield.RemoveTypes`. If `Shield.AttachTypes` contains less items than `Shield.RemoveTypes`, last item from the former is used for any remaining removed shields.
    - If `Shield.ReplaceNonRespawning` is set, shield from `Shield.AttachTypes` replaces existing shields that have been broken and cannot respawn on their own.
    - `Shield.MinimumReplaceDelay` can be used to control how long after the shield has been broken (in game frames) can it be replaced. If not enough frames have passed, it won't be replaced.
    - If `Shield.InheritStateOnReplace` is set, shields replaced via `Shield.ReplaceOnly` inherit the current strength (relative to ShieldType `Strength`) of the previous shield and whether or not the shield was currently broken. Self-healing and respawn timers are always reset.

## Animations

### Anim-to-Unit

![image](_static/images/animToUnit.gif)

- Animations can now create (or "convert" to) vehicles when they end via `CreateUnit`.
  - `CreateUnit.Owner` determines which house will own the created VehicleType. This only works as expected if the animation has owner set.
    - Vehicle [destroy animations](Fixed-or-Improved-Logics.md#destroy-animations), animations from Warhead `AnimList/SplashList` and map trigger action `41 Play Anim At` will have the owner set correctly.
    - `CreateUnit.RemapAnim`, if set to true, will cause the animation to be drawn in unit palette and remappable to owner's team color.
  - `CreateUnit.Mission` determines the initial mission of the created VehicleType.
  - `CreateUnit.Facing` determines the initial facing of created VehicleType.
    - `CreateUnit.RandomFacing`, if set to true, makes it so that a random facing is picked instead.
    - `CreateUnit.InheritFacings` and `CreateUnit.InheritTurretFacings` inherit facings for vehicle body and turret respectively from the destroyed vehicle if the animation is a vehicle destroy animation. `InheritTurretFacings` does not work with jumpjet vehicles due to technical constraints.
  - `CreateUnit.AlwaysSpawnOnGround`, if set to true, ensures the vehicle will be created on the cell at ground level even if animation is in air.
  - `CreateUnit.ConsiderPathfinding`, if set to true, will consider whether or not the cell where the animation is located is occupied by other objects or impassable to the vehicle being created and will attempt to find a nearby cell that is not. Otherwise the vehicle will be created at the animation's location despite these obstacles if possible.
  - `CreateUnit.SpawnAnim` can be used to play another animation at created unit's location after it has appeared. This animation has same owner and invoker as the parent animation.

In `artmd.ini`:
```ini
[SOMEANIM]                             ; AnimationType
CreateUnit=                            ; VehicleType
CreateUnit.Owner=Victim                ; Owner house kind, Invoker/Killer/Victim/Civilian/Special/Neutral/Random
CreateUnit.RemapAnim=false             ; boolean
CreateUnit.Mission=Guard               ; MissionType
CreateUnit.Facing=0                    ; integer, facings in range of 0-255
CreateUnit.RandomFacing=true           ; boolean
CreateUnit.InheritFacings=false        ; boolean
CreateUnit.InheritTurretFacings=false  ; boolean
CreateUnit.AlwaysSpawnOnGround=false   ; boolean
CreateUnit.ConsiderPathfinding=false   ; boolean
CreateUnit.SpawnAnim=                  ; Animation
```

```{note}
Due to technical constraints, infantry death animations including Ares' `InfDeathAnim` cannot have `CreateUnit.Owner` correctly applied to them. You can use Ares' `MakeInfantryOwner` as a workaround instead, which should function for this use-case even without `MakeInfantry` set.
```

### Attached particle system

- It is now possible to attach a particle system to an animation. Only particle systems with `BehavesLike=Smoke` are supported. This works similarly to the identically named key on `VoxelAnims`.
  - On animations with `Next`, the particle system will be deleted when the next animation starts playing and new one created in its stead if the `Next` animation defines a different particle system.

In `artmd.ini`:
```ini
[SOMEANIM]       ; AnimationType
AttachedSystem=  ; ParticleSystem
```

## Buildings

### Extended building upgrades

![image](_static/images/powersup.owner-01.png)
*Upgrading own and allied Power Plants in [CnC: Final War](https://www.moddb.com/mods/cncfinalwar)*

```{note}
Due to technical limitations, with Ares, upgrades placed through `PowersUp.Buildings` instead of `PowersUpBuilding` (note that internally `PowersUpBuilding` is set to first entry of `PowersUp.Buildings` if former is not set but latter is) **DO NOT** satisfy prerequisites. Suggested workaround is to use the upgrades to provide Superweapons that spawn in buildings via [LimboDelivery](#limbodelivery) logic to function as prerequisites, which are destroyed by another SW that becomes available if parent building is gone and so on.
```

- Building upgrades now can be placed on own buildings, on allied buildings and/or on enemy buildings. These three owners can be specified by `PowersUp.Owner`. When upgrade is placed on building, it automatically changes it's owner to match the building's owner.
- One upgrade can now be applied to multiple different types of buildings specified by `PowersUp.Buildings`.
  - Ares-introduced build limit for building upgrades works with this feature.

In `rulesmd.ini`:
```ini
[UPGRADENAME]       ; BuildingType
PowersUp.Owner=Self ; list of Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
PowersUp.Buildings= ; list of BuildingTypes
```

### Power plant enhancer

- When it exists, it can increase the power amount generated by the power plants.
  - When enchancing structures are sold or destroyed, the power amount returns to normal.

In `rulesmd.ini`:
```ini
[SOMEBUILDING]                     ; BuildingType
PowerPlantEnhancer.PowerPlants=    ; list of BuildingTypes
PowerPlantEnhancer.Amount=0        ; integer
PowerPlantEnhancer.Factor=1.0      ; floating point value
```

### Spy Effects

- Additional espionage bonuses can be toggled with `SpyEffect.Custom`.
  - `SpyEffect.VictimSuperWeapon` instantly launches a Super Weapon for the owner of the infiltrated building at building's coordinates.
  - `SpyEffect.InfiltratorSuperWeapon` behaves the same as above, with the Super Weapon's owner being the owner of the spying unit.

In `rulesmd.ini`:
```ini
[SOMEBUILDING]                     ; BuildingType
SpyEffect.Custom=false            ; boolean
SpyEffect.VictimSuperWeapon=      ; SuperWeaponType
SpyEffect.InfiltratorSuperWeapon= ; SuperWeaponType
```

## Infantry

### Customizable FLH When Infantry Is Prone Or Deployed

- Now infantry can override `PrimaryFireFLH` and `SecondaryFireFLH` if is prone (crawling) or deployed. Also works in conjunction with [burst-index specific firing offsets](#firing-offsets-for-specific-burst-shots).

In `artmd.ini`:
```ini
[SOMEINFANTRY]             ; InfantryType
PronePrimaryFireFLH=       ; integer - Forward,Lateral,Height
ProneSecondaryFireFLH=     ; integer - Forward,Lateral,Height
DeployedPrimaryFireFLH=    ; integer - Forward,Lateral,Height
DeployedSecondaryFireFLH=  ; integer - Forward,Lateral,Height
```

### Default disguise for individual InfantryTypes

- Infantry can now have its `DefaultDisguise` overridden per-type.
  - This tag's priority is higher than Ares' per-side `DefaultDisguise`.

In `rulesmd.ini`:
```ini
[SOMEINFANTRY]      ; InfantryType
DefaultDisguise=E2  ; InfantryType
```

### Random death animaton for NotHuman Infantry

- Infantry with `NotHuman=yes` can now play random death anim sequence between `Die1` to `Die5` instead of the hardcoded `Die1`.
  - Do not forget to tweak infantry anim sequences before enabling this feature, otherwise it will play invisible anim sequence.

In `rulesmd.ini`:
```ini
[SOMEINFANTRY]                    ; InfantryType
NotHuman.RandomDeathSequence=yes  ; boolean
```

### Shared Ammo

- Transports with `OpenTopped=yes` and `Ammo.Shared=yes` will transfer ammo to passengers that have `Ammo.Shared=yes`.
In addition, a transport can filter who will receive ammo if passengers have the same value in `Ammo.Shared.Group=<integer>` of the transport, ignoring other passengers with different groups values.
- Transports with `Ammo.Shared.Group=-1` will transfer ammo to any passenger with `Ammo.Shared=yes` ignoring the group.
- Transports must have ammo and should be able to reload ammo.

In `rulesmd.ini`:
```ini
[SOMETECHNO1]         ; TechnoType, transport with OpenTopped=yes
Ammo.Shared=no        ; boolean
Ammo.Shared.Group=-1  ; integer

[SOMETECHNO2]         ; TechnoType, passenger
Ammo.Shared=no        ; boolean
Ammo.Shared.Group=-1  ; integer
```


### Slaves' house decision customization when owner is killed

- You can now decide the slaves' house when the corresponding slave miner is killed using `Slaved.OwnerWhenMasterKilled`:
  - `suicide`: Kill each slave if the slave miner is killed.
  - `master`: Free the slaves but keep the house of the slave unchanged.
  - `neutral`: The slaves belong to civilian house.
  - `killer`: Free the slaves and give them to the house of the slave miner's killer. (vanilla behavior)

In `rulesmd.ini`
```ini
[SOMEINFANTRY]                       ; Slave type
Slaved=yes
Slaved.OwnerWhenMasterKilled=killer  ; enumeration (suicide | master | killer | neutral)
```

### Customizable `SlavesFreeSound`

- `SlavesFreeSound` is now dehardcoded from `AudioVisual` and can be set individually for each enslavable infantry type.

In `rulesmd.ini`

```ini
[SOMEINFANTRY]  ; Slave type
SlavesFreeSound=      ; sound entry
```

## Projectiles


### Projectile interception logic

![image](_static/images/projectile-interception-01.gif)
*Interception logic used in [Tiberium Crisis](https://www.moddb.com/mods/tiberium-crisis) mod*

- Projectiles can now be made interceptable by certain TechnoTypes by setting `Interceptable=true` on them. The TechnoType scans for interceptable projectiles within a range if it has no other target and will use one of its weapons to shoot at them. Projectiles can define `Armor` and `Strength`. Weapons that cannot target the projectile's armor type will not attempt to intercept it. On interception, if the projectile has `Armor` set, an amount equaling to the intercepting weapon's `Damage` adjusted by Warhead `Verses` and the TechnoType's firepower multipliers is deducted from the projectile's current strength. Regardless of if the current projectile strength was reduced or not, if it sits at 0 or below after interception, the projectile is detonated.
  - `Interceptor.Weapon` determines the weapon (0 = `Primary`, 1 = `Secondary`) to be used for intercepting projectiles.
    - The interceptor weapon may need `AG` and/or `AA` set to true on its projectile to be able to target projectiles depending on their elevation from ground. If you don't set those then the weapon won't be able to target low-flying or high-flying projectiles respectively.
  - `Interceptor.CanTargetHouses` controls which houses the projectiles (or rather their firers) can belong to be eligible for interception.
  - `Interceptor.GuardRange` (and `Interceptor.(Rookie|Veteran|EliteGuardRange`) is maximum range of the unit to intercept projectile. The unit weapon range will limit the unit interception range though.
  - `Interceptor.MinimumGuardRange` (and `Interceptor.(Rookie|Veteran|EliteMinimumGuardRange`) is the minimum range of the unit to intercept projectile. Any projectile under this range will not be intercepted.
  - `Interceptable.DeleteOnIntercept` determines whether or not the projectile will simply be deleted on detonation upon interception, or if it will properly detonate. Will be overridden by `Interceptor.DeleteOnIntercept` setting on the interceptor.
  - `Interceptable.WeaponOverride` can be set to a WeaponType that will be used to override characteristics such as `Damage` and `Warhead` of the current projectile for detonation after interception. Will be overridden by `Interceptor.WeaponOverride` setting on the interceptor.
    - On interceptors, `Interceptor.WeaponReplaceProjectile` can be set to true to make `Interceptor.WeaponOverride` also replace the intercepted projectile's type (including `Image` and other projectile characteristics) and `Speed` with its own. Does not replace particle systems (`AttachedSystem`, *Ares feature*).
    - On interceptors, `Interceptor.WeaponCumulativeDamage` can be set to true to make `Damage` from `Interceptor.WeaponOverride` weapon be added on the projectile's damage rather than override it.
  - `Interceptor.KeepIntact` can be set to true to allow intercepted projectiles to continue traveling as if they were not intercepted, but effects such as `Interceptor.WeaponOverride` will still be applied.

In `rulesmd.ini`:
```ini
[SOMETECHNO]                               ; TechnoType
Interceptor=false                          ; boolean
Interceptor.Weapon=0                       ; integer, weapon slot index (0 or 1)
Interceptor.CanTargetHouses=enemies        ; Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
Interceptor.GuardRange=0.0                 ; floating point value
Interceptor.VeteranGuardRange=             ; floating point value
Interceptor.EliteGuardRange=               ; floating point value
Interceptor.MinimumGuardRange=0.0          ; floating point value
Interceptor.VeteranMinimumGuardRange=      ; floating point value
Interceptor.EliteMinimumGuardRange=        ; floating point value
Interceptor.DeleteOnIntercept=false        ; boolean
Interceptor.WeaponOverride=                ; WeaponType
Interceptor.WeaponReplaceProjectile=false  ; boolean
Interceptor.WeaponCumulativeDamage=false   ; boolean
Interceptor.KeepIntact=false               ; boolean

[SOMEPROJECTILE] ; Projectile
Interceptable=false                    ; boolean
Interceptable.DeleteOnIntercept=false  ; boolean
Interceptable.WeaponOverride=          ; WeaponType
Strength=0                             ; integer
Armor=                                 ; ArmorType
```

```{note}
Currently interceptor weapons with projectiles that do not have `Inviso=true` will be unable to intercept projectiles if the firer of the interceptor weapon dies before the interceptor weapon projectile reaches its target. This may change in future.
```

### Projectile trajectories

- Projectiles can now have customizable trajectories.
  - `Trajectory` should not be combined with original game's projectile trajectory logics (`Arcing`, `ROT` or `Inviso`).
  - Initial speed of the projectile is defined by `Trajectory.Speed`, which unlike `Speed` used by `ROT` > 0 projectiles is defined on projectile not weapon.

  In `rulesmd.ini`:
```ini
[SOMEPROJECTILE]        ; Projectile
Trajectory.Speed=100.0  ; floating point value
```

#### Straight trajectory

![Straigh trajectory blasters](_static/images/straight.gif)
*Straight trajectory used to make blasters in a private mod by @brsajo#9745*

- Self-explanatory, is a straight-shot trajectory.

In `rulesmd.ini`:
```ini
[SOMEPROJECTILE]     ; Projectile
Trajectory=Straight  ; Trajectory type
```

#### Bombard trajectory

- Similar trajectory to `Straight`, but targets a coordinate above the intended target (height determined by `Trajectory.Bombard.Height`). When the projectile approaches that coordinate, it will free fall and explodes when it hits the target or ground.

In `rulesmd.ini`:
```ini
[SOMEPROJECTILE]               ; Projectile
Trajectory=Bombard             ; Trajectory type
Trajectory.Bombard.Height=0.0  ; double
```

### Shrapnel enhancement

![image](_static/images/shrapnel.gif)
*Shrapnel appearing against ground & buildings* ([Project Phantom](https://www.moddb.com/mods/project-phantom))

- Shrapnel behavior can be triggered on the ground and buildings.

In `rulesmd.ini`:
```ini
[SOMEPROJECTILE]                 ; Projectile
Shrapnel.AffectsGround=false     ; boolean
Shrapnel.AffectsBuildings=false  ; boolean
```

### Projectiles blocked by land or water

- It is now possible to make projectiles consider either land or water as obstacles that block their path by setting `SubjectToLand/Water` to true, respectively. Weapons firing such projectiles will consider targets blocked by such obstacles as out of range and will attempt to reposition themselves so they can fire without being blocked by the said obstacles before firing and if `SubjectToLand/Water.Detonate` is set to true, the projectiles will detonate if they somehow manage to collide with the said obstacles.
  - `Level=true` projectiles detonate on tiles belonging to non-water tilesets by default, but will not consider such tiles as true obstacles. This behaviour can be overridden by setting these keys.

In `rulesmd.ini`:
```ini
[SOMEPROJECTILE]              ; Projectile
SubjectToLand=                ; boolean
SubjectToLand.Detonate=true   ; boolean
SubjectToWater=               ; boolean
SubjectToWater.Detonate=true  ; boolean
```

## Super Weapons

### Convert TechnoType

- Warheads can now change TechnoTypes of affected units to other Types in the same category (infantry to infantry, vehicles to vehicles, aircraft to aircraft).
  - `ConvertN.From` (where N is 0, 1, 2...) specifies which TechnoTypes are valid for conversion. This entry can have many types listed, meanging that many types will be converted at once. When no types are included, conversion will affect all valid targets.
  - `ConvertN.To` specifies the TechnoType which is the result of conversion.
  - `ConvertN.AffectedHouses` specifies whose units can be converted.
  - `Convert.From`, `Convert.To` and `Convert.AffectedHouses` (without numbers) are a valid alternative to `Convert0.From`, `Convert0.To` and `Convert0.AffectedHouses` if only one pair is specified.
  - Conversion affects *all* existing units of set TechnoTypes, this includes units in: transports, occupied buildings, buildings with `InfantryAbsorb=yes` or `UnitAbsorb=yes`, buildings with `Bunker=yes`.

In example, this superweapon would convert all owned and friendly `SOLDIERA` and `SOLDIERB` to `NEWSOLDIER`:
```ini
[SOMESW]
Convert.From=SOLDIERA,SOLDIERB
Convert.To=NEWSOLDIER
Convert.AffectedHouses=team
```

```{warning}
This feature has the same limitations as [Ares' Type Conversion](https://ares-developers.github.io/Ares-docs/new/typeconversion.html). This feature does not support BuildingTypes.
```

```{warning}
This feature requires Ares 3.0 or higher to function! When Ares 3.0+ is not detected, not all properties of a unit may be updated.
```

In `rulesmd.ini`:
```ini
[SOMESW]                        ; SuperWeapon
ConvertN.From=                  ; list of TechnoTypes
ConvertN.To=                    ; TechnoType
ConvertN.AffectedHouses=owner   ; list of Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
; where N = 0, 1, 2, ...
; or
Convert.From=                   ; list of TechnoTypes
Convert.To=                     ; TechnoType
Convert.AffectedHouses=owner    ; list of Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
```

### LimboDelivery

- Superweapons can now deliver off-map buildings that act as if they were on the field.
  - `LimboDelivery.Types` is the list of BuildingTypes that will be created when the Superweapons fire. Superweapon `Type` and coordinates do not matter.
  - `LimboDelivery.IDs` is the list of numeric IDs that will be assigned to buildings. Necessary for LimboKill to work.

- Created buildings are not affected by any on-map threats. The only way to remove them from the game is by using a Superweapon with `LimboKill.IDs` set.
  - `LimboKill.Affected` sets which houses are affected by this feature.
  - `LimboKill.IDs` lists IDs that will be targeted. Buildings with these IDs will be removed from the game instantly.

- Delivery can be made random with these optional tags. The game will randomly choose only a single building from the list for each roll chance provided.
  - `LimboDelivery.RollChances` lists chances of each "dice roll" happening. Valid values range from 0% (never happens) to 100% (always happens). Defaults to a single sure roll.
  - `LimboDelivery.RandomWeightsN` lists the weights for each "dice roll" that increase the probability of picking a specific building. Valid values are 0 (don't pick) and above (the higher value, the bigger the likelyhood). `RandomWeights` are a valid alias for `RandomWeights0`. If a roll attempt doesn't have weights specified, the last weights will be used.

Note: This feature might not support every building flag. Flags that are confirmed to work correctly are listed below:
  - FactoryPlant
  - OrePurifier
  - SpySat
  - KeepAlive (Ares 3.0)
  - Prerequisite, PrerequisiteOverride, Prerequisite.List# (Ares 0.1), Prerequisite.Negative (Ares 0.1), GenericPrerequisites (Ares 0.1)
  - SuperWeapon, SuperWeapon2, SuperWeapons (Ares 0.9), SW.AuxBuildings (Ares 0.9), SW.NegBuildings (Ares 0.9)

Note: In order for this feature to work with AITriggerTypes conditions ("Owning house owns ???" and "Enemy house owns ???"), `LegalTarget` must be set to true.

```{warning}
Remember that Limbo Delivered buildings don't exist physically! This means they should never have enabled machanics that require interaction with the game world (i.e. factories, cloning vats, service depots, helipads). They also **should have either `KeepAlive=no` set or be killable with LimboKill** - otherwise the game might never end.
```
In `rulesmd.ini`:
```ini
[SOMESW]                        ; Superweapon
LimboDelivery.Types=            ; List of BuildingTypes
LimboDelivery.IDs=              ; List of numeric IDs. -1 cannot be used.
LimboDelivery.RollChances=      ; List of percentages.
LimboDelivery.RandomWeightsN=   ; List of integers.
LimboKill.Affected=self         ; Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
LimboKill.IDs=                  ; List of numeric IDs.
```

### Next

Superweapons can now launch other superweapons at the same target. Launched types can be additionally randomized using the same rules as with LimboDelivery (see above).
  - `SW.Next.RealLaunch` controls whether the owner who fired the initial superweapon must own all listed superweapons and sufficient funds to support `Money.Amout`. Otherwise they will be launched forcibly.
  - `SW.Next.IgnoreInhibitors` ignores `SW.Inhibitors`/`SW.AnyInhibitor` of each superweapon, otherwise only non-inhibited superweapons are launched.
  - `SW.Next.IgnoreDesignators` ignores `SW.Designators`/`SW.AnyDesignator` respectively.

In `rulesmd.ini`:
```ini
[SOMESW]                        ; Super Weapon
SW.Next=                        ; List of Superweapons
SW.Next.RealLaunch=true         ; boolean
SW.Next.IgnoreInhibitors=false  ; boolean
SW.Next.IgnoreDesignators=true  ; boolean
SW.Next.RollChances=            ; List of percentages.
SW.Next.RandomWeightsN=         ; List of integers.
```

### Warhead or Weapon detonation at target cell

- Any superweapon can now detonate a Warhead or a weapon at superweapon's target cell.
  - If both `Detonate.Warhead` and `Detonate.Weapon` are set, latter takes precedence.
  - `Detonate.Damage`, if not set, defaults to weapon `Damage` for `Detonate.Weapon` and 0 for `Detonate.Warhead`.
  - Both the weapon and Warhead behave as if fired by whatever building fired the Superweapon. This respects controls like `SW.RangeMinimum/Maximum` (similar to Ares' GenericWarhead superweapon in this regard). If firing building could not be found, the house the Superweapon belonged to is still used to deal damage and apply Phobos-introduced Warhead effects.
  - If `Detonate.AtFirer` is set to true, the weapon or Warhead is detonated at the firing building instead of the superweapon's target cell. If there is no firer, no detonation will occur.

In `rulesmd.ini`:
```ini
[SOMESW]                ; Super Weapon
Detonate.Warhead=       ; Warhead
Detonate.Weapon=        ; WeaponType
Detonate.Damage=        ; integer
Detonate.AtFirer=false  ; boolean
```

## Technos

### Automatic passenger deletion

- Transports can erase passengers over time. Passengers are deleted in order of entering the transport, from first to last.
  - `PassengerDeletion.Rate` determines the interval in game frames that it takes to erase a single passenger.
    - If `PassengerDeletion.Rate.SizeMultiply` is set to true, this time interval is multiplied by the passenger's `Size`.
  - `PassengerDeletion.UseCostAsRate`, if set to true, changes the time interval for erasing a passenger to be based on the passenger's `Cost`. This does not factor in modifiers like `FactoryPlant`.
    - `PassengerDeletion.CostMultiplier` can be used to modify the cost-based time interval.
    - `PassengerDeletion.CostRateCap` can be used to set a cap to the cost-based time interval.
  - `PassengerDeletion.AllowedHouses` determines which houses passengers can belong to be eligible for deletion.
  - `PassengerDeletion.DontScore`, if set to true, makes it so that the deleted passengers are not counted as having been killed by the transport (no experience, not recorded towards owning house's score etc).
  - If `PassengerDeletion.Soylent` is set to true, an amount of credits is refunded to the owner of the transport. The exact amount refunded is determined by the passengers `Soylent`, or if not set, its `Cost` (this is affected by modifiers such as `FactoryPlant`).
    - `PassengerDeletion.SoylentMultiplier` is a direct multiplier applied to the refunded amount of credits.
    - `PassengerDeletion.SoylentAllowedHouses` determines which houses passengers can belong to be eligible for refunding.
    - `PassengerDeletion.DisplaySoylent` can be set to true to display the amount of credits refunded on the transport. `PassengerDeletion.DisplaySoylentToHouses` determines which houses can see this and `PassengerDeletion.DisplaySoylentOffset` can be used to adjust the display offset.
  - `PassengerDeletion.ReportSound` and `PassengerDeletion.Anim` can be used to specify a sound and animation to play when a passenger is erased, respectively.

In `rulesmd.ini`:
```ini
[SOMETECHNO]                                    ; TechnoType
PassengerDeletion.Rate=0                        ; integer, game frames
PassengerDeletion.Rate.SizeMultiply=true        ; boolean
PassengerDeletion.UseCostAsRate=false           ; boolean
PassengerDeletion.CostMultiplier=1.0            ; floating point value, percents or absolute
PassengerDeletion.CostRateCap=                  ; integer, game frames
PassengerDeletion.AllowedHouses=all             ; Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
PassengerDeletion.DontScore=false               ; boolean
PassengerDeletion.Soylent=false                 ; boolean
PassengerDeletion.SoylentMultiplier=1.0         ; float, percents or absolute
PassengerDeletion.SoylentAllowedHouses=enemies  ; Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
PassengerDeletion.DisplaySoylent=false          ; boolean
PassengerDeletion.DisplaySoylentToHouses=All    ; Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
PassengerDeletion.DisplaySoylentOffset=0,0      ; X,Y, pixels relative to default
PassengerDeletion.ReportSound=                  ; Sound
PassengerDeletion.Anim=                         ; Animation
```

### Automatic passenger owner change to match transport owner

- Transports with `Passengers.SyncOwner` set to true will have the owner of their passengers changed to match the transport if transport's owner changes.
  - On `OpenTopped=true` transports this will also disable checks that prevent target acquisition by passengers when the transport is temporarily mind controlled.
  - `Passengers.SyncOwner.RevertOnExit`, if set to true (which is the default), changes the passengers' owner back to whatever it was originally when they entered the transport when they are ejected.
  - Does not work on passengers acquired through use of `Abductor=true` weapon *(Ares feature)*.

In `rulesmd.ini`:
```ini
[SOMETECHNO]                            ; TechnoType
Passengers.SyncOwner=false              ; boolean
Passengers.SyncOwner.RevertOnExit=true  ; boolean
```

### Automatically firing weapons

- You can now make TechnoType automatically fire its weapon(s) without having to scan for suitable targets by setting `AutoFire`, on either its base cell (in which case the weapon that is used for force-firing is used) or itself (in which case normal targeting and weapon selection rules and are respected) depending on if `AutoFire.TargetSelf` is set or not.

In `rulesmd.ini`:
```ini
[SOMETECHNO]               ; TechnoType
AutoFire=false             ; boolean
AutoFire.TargetSelf=false  ; boolean
```

### Customizable OpenTopped properties

- You can now override global `OpenTopped` transport properties per TechnoType.
- `OpenTopped.IgnoreRangefinding` can be used to disable `OpenTopped` transport rangefinding behaviour where smallest weapon range between transport and all passengers is used when approaching targets that are out of range and when scanning for potential targets.
- `OpenTopped.AllowFiringIfDeactivated` can be used to customize whether or not passengers can fire out when the transport is deactivated (EMP, powered unit etc).
- `OpenTopped.ShareTransportTarget` controls whether or not the current target of the transport itself is passed to the passengers as well.

```ini
[SOMETECHNO]                              ; TechnoType
OpenTopped.RangeBonus=                    ; integer, override of the global default
OpenTopped.DamageMultiplier=              ; floating point value, override of the global default
OpenTopped.WarpDistance=                  ; integer, override of the global default
OpenTopped.IgnoreRangefinding=false       ; boolean
OpenTopped.AllowFiringIfDeactivated=true  ; boolean
OpenTopped.ShareTransportTarget=true      ; boolean
```

### Disabling fallback to (Elite)Secondary weapon

- It is now possible to disable the fallback to `(Elite)Secondary` weapon from `(Elite)Primary` weapon if it cannot fire at the chosen target by setting `NoSecondaryWeaponFallback` to true (defaults to false). `NoSecondaryWeaponFallback.AllowAA` controls whether or not fallback because of projectile `AA` setting and target being in air is still allowed. This does not apply to special cases where `(Elite)Secondary` weapon is always chosen, including but not necessarily limited to the following:
  - `OpenTransportWeapon=1` on an unit firing from inside `OpenTopped=true` transport.
  - `NoAmmoWeapon=1` on an unit with  `Ammo` value higher than 0 and current ammo count lower or  equal to `NoAmmoAmount`.
  - Deployed `IsSimpleDeployer=true` units with`DeployFireWeapon=1` set or omitted.
  - `DrainWeapon=true` weapons against enemy `Drainable=yes` buildings.
  - Units with `IsLocomotor=true` set on `Warhead` of `(Elite)Primary` weapon against buildings.
  - Weapons with `ElectricAssault=true` set on `Warhead` against `Overpowerable=true` buildings belonging to owner or allies.
  - `Overpowerable=true` buildings that are currently overpowered.
  - Any system using `(Elite)WeaponX`, f.ex `Gunner=true` or `IsGattling=true` is also wholly exempt.

In `rulesmd.ini`:
```ini
[SOMETECHNO]                             ; TechnoType
NoSecondaryWeaponFallback=false          ; boolean
NoSecondaryWeaponFallback.AllowAA=false  ; boolean
```

### Disguise logic additions (disguise-based movement speed, disguise blinking visibility)

- `DisguiseBlinkingVisibility` can be used to customize which players can see disguises blinking on units. This does not affect targeting but does affect veterancy insignia visibility - blinking disguise means the original unit's insignia is visible always instead of disguise's.
- `UseDisguiseMovementSpeed`, if set, makes disguised unit adjust its movement speed to match that of the disguise, if applicable. Note that this applies even when the disguise is revealed, as long as it has not been removed.

In `rulesmd.ini`:
```ini
[General]
DisguiseBlinkingVisibility=owner  ; list of Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)

[SOMETECHNO]                      ; TechnoType
UseDisguiseMovementSpeed=false    ; boolean
```

### Firing offsets for specific Burst shots

- You can now specify separate firing offsets for each of the shots fired by weapon with `Burst` via using `(Elite)(Prone/Deployed)PrimaryFire|SecondaryFire|WeaponX|FLH.BurstN` keys, depending on which weapons your TechnoType makes use of. *N* in `BurstN` is zero-based burst shot index, and the values are parsed sequentially until no value for either regular or elite weapon is present, with elite weapon defaulting to regular weapon FLH if only it is missing. If no burst-index specific value is available, value from the base key (f.ex `PrimaryFireFLH`) is used.
- Burst-index specific firing offsets are absolute firing offsets and the lateral shifting based on burst index that occurs with the base firing offsets is not applied.

In `artmd.ini`:
```ini
[SOMETECHNO]    ; TechnoType Image
FLHKEY.BurstN=  ; integer - Forward,Lateral,Height. FLHKey refers to weapon-specific FLH key name and N is zero-based burst shot index.
```

### Forcing specific weapon against certain targets

![image](_static/images/underwater-new-attack-tag.gif)
*Naval underwater target behavior with `ForceWeapon.Naval.Decloaked` in [C&C: Reloaded](https://www.moddb.com/mods/cncreloaded)*

- Can be used to override normal weapon selection logic to force specific weapons to use against certain targets. If multiple are set and target satisfies the conditions, the first one in listed order satisfied takes effect.
  - `ForceWeapon.Naval.Decloaked` forces specified weapon to be used against uncloaked naval targets. Useful if your naval unit has one weapon only for underwater and another weapon for surface targets.
  - `ForceWeapon.Cloaked` forces specified weapon to be used against any cloaked targets.
  - `ForceWeapon.Disguised` forces specified weapon to be used against any disguised targets.

In `rulesmd.ini`:
```ini
[SOMETECHNO]                    ; TechnoType
ForceWeapon.Naval.Decloaked=-1  ; integer. 0 for primary weapon, 1 for secondary weapon, -1 to disable
ForceWeapon.Cloaked=-1          ; integer. 0 for primary weapon, 1 for secondary weapon, -1 to disable
ForceWeapon.Disguised=-1        ; integer. 0 for primary weapon, 1 for secondary weapon, -1 to disable
```

### Make units try turning to target when firing with `OmniFire=yes`
- The unit will try to turn the body to target even firing with `OmniFire=yes`
  - Recommended for jumpjets if you want it to turn to target when firing.

In `rulesmd.ini`:
```ini
[SOMEWEAPONTYPE]          ; WeaponType
OmniFire=yes
OmniFire.TurnToTarget=no  ; boolean
```

### Initial strength for TechnoTypes and cloned infantry

![image](_static/images/initialstrength.cloning-01.png)
*Initial strength for cloned infantry example in [C&C: Reloaded](https://www.moddb.com/mods/cncreloaded)*

- `InitialStrength` can be used to set how many hitpoints a TechnoType starts with.
- `InitialStrength.Cloning` can be used to specify a percentage of hitpoints (single value or a range from which a random value is picked) cloned infantry produced by `Cloning=true` building start with.

In `rulesmd.ini`:
```ini
[SOMETECHNO]              ; TechnoType
InitialStrength=          ; integer

[SOMEBUILDING]            ; BuildingType
InitialStrength.Cloning=  ; floating point value - single or comma-sep. range (percentages)
```

```{note}
Both `InitialStrength` and `InitialStrength.Cloning` never surpass the type's `Strength`, even if your values are bigger than it.
```

### Kill Object Automatically

- Objects can be destroyed automatically if *any* of these conditions is met:
  - `OnAmmoDepletion`: The object will die if the remaining ammo reaches 0.
  - `AfterDelay`: The object will die if the countdown (in frames) reaches 0.
  - `TechnosExist` / `TechnosDontExist`: The object will die if TechnoTypes exist or do not exist, respectively.
    - `Technos(Dont)Exist.Any` controls whether or not a single listed TechnoType is enough to satisfy the requirement or if all are required.
    - `Technos(Dont)Exist.AllowLimboed` controls whether or not limboed TechnoTypes (f.ex those in transports) are counted.
    - `Technos(Dont)Exist.Houses` controls which houses are checked.

- The auto-death behavior can be chosen from the following:
  - `kill`: The object will be destroyed normally.
  - `vanish`: The object will be directly removed from the game peacefully instead of actually getting killed.
  - `sell`: If the object is a **building** with buildup, it will be sold instead of destroyed.

If this option is not set, the self-destruction logic will not be enabled. `AutoDeath.VanishAnimation` can be set to animation to play at object's location if `vanish` behaviour is chosen.

```{note}
Please notice that if the object is a unit which carries passengers, they will not be released even with the `kill` option **if you are not using Ares 3.0+**.
```

This logic also supports buildings delivered by [LimboDelivery](#LimboDelivery). However in this case, all `AutoDeath.Behavior` values produce identical result where the building is simply deleted.

In `rulesmd.ini`:
```ini
[SOMETECHNO]                                   ; TechnoType
AutoDeath.Behavior=                            ; enumeration (kill | vanish | sell), default not set
AutoDeath.VanishAnimation                      ; Animation
AutoDeath.OnAmmoDepletion=no                   ; boolean
AutoDeath.AfterDelay=0                         ; positive integer
AutoDeath.TechnosDontExist=                    ; list of TechnoType names
AutoDeath.TechnosDontExist.Any=false           ; boolean
AutoDeath.TechnosDontExist.AllowLimboed=false  ; boolean
AutoDeath.TechnosDontExist.Houses=owner        ; Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
AutoDeath.TechnosExist=                        ; list of TechnoType names
AutoDeath.TechnosExist.Any=true                ; boolean
AutoDeath.TechnosExist.AllowLimboed=false       ; boolean
AutoDeath.TechnosExist.Houses=owner            ; Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
```

### Mind Control enhancement

![image](_static/images/mindcontrol-max-range-01.gif)
*Mind Control Range Limit used in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)*
![image](_static/images/mindcontrol-multiple-01.gif)
*Multiple Mind Control unit auto-releases the first victim in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)*

- Mind controllers now can have the upper limit of the control distance. Tag values greater than 0 will activate this feature.
- Mind controllers with multiple controlling slots can now release the first controlled unit when they have reached the control limit and are ordered to control a new target.
- Allows Warheads to play custom `MindControl.Anim` which defaults to `ControlledAnimationType`.

In `rulesmd.ini`:
```ini
[SOMETECHNO]                          ; TechnoType
MindControlRangeLimit=-1.0            ; floating point value
MultiMindControl.ReleaseVictim=false  ; boolean

[SOMEWARHEAD]                         ; Warhead
MindControl.Anim=                     ; Animation, defaults to ControlledAnimationType
```

### No Manual Move

- You can now specify whether a TechnoType is unable to receive move command.

```ini
[SOMETECHNO]        ; TechnoType
NoManualMove=false  ; boolean
```

### Promoted Spawns

![image](_static/images/promotedspawns-01.gif)
*Promoted Spawns in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)*

- The spawned units will promote as their owner's veterancy.

In `rulesmd.ini`:
```ini
[SOMETECHNO]                 ; TechnoType
Promote.IncludeSpawns=false  ; boolean
```

### Spawner pursuit range & spawn delay customization

![image](_static/images/spawnrange-01.gif)
*Limited pursue range for spawns in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)*

- If `Spawner.LimitRange` is set, the spawned units will abort their pursuit if the enemy is out of the range of the largest weapon `Range` of a `Spawner=true` weapon of the spawner.
  - `Spawner.ExtraLimitRange` adds extra pursuit range on top of the weapon range.
- `Spawner.DelayFrames` can be used to set the minimum number of game frames in between each spawn ejecting from the spawner. By default this is 9 frames for missiles and 20 for everything else.

In `rulesmd.ini`:
```ini
[SOMETECHNO]               ; TechnoType
Spawner.LimitRange=false   ; boolean
Spawner.ExtraLimitRange=0  ; integer, range in cells
Spawner.DelayFrames=       ; integer, game frames
```

### Weapons fired on warping in / out

- It is now possible to add weapons that are fired on a teleporting TechnoType when it warps in or out. They are at the same time as the appropriate animations (`WarpIn` / `WarpOut`) are displayed.
  - `WarpInMinRangeWeapon` is used instead of `WarpInWeapon` if the distance traveled (in leptons) was less than `ChronoRangeMinimum`. This works regardless of if `ChronoTrigger` is set or not. If `WarpInMinRangeWeapon` is not set, it defaults to `WarpInWeapon`.
  - If `WarpInWeapon.UseDistanceAsDamage` is set, `Damage` of `WarpIn(MinRange)Weapon` is overriden by the number of whole cells teleported across.

In `rulesmd.ini`:
```ini
[SOMETECHNO]                            ; TechnoType
WarpInWeapon=                           ; WeaponType
WarpInMinRangeWeapon=                   ; WeaponType
WarpInWeapon.UseDistanceAsDamage=false  ; boolean
WarpOutWeapon=                          ; WeaponType
```

### Customize EVA voice and `SellSound` when selling units

- When a building or a unit is sold, a sell sound as well as an EVA is played to the owner. These configurations have been deglobalized.

  - `EVA.Sold` is used to customize the EVA voice when selling, default to `EVA_StructureSold` for buildings and `EVA_UnitSold` for vehicles.
  - `SellSound` is used to customize the report sound when selling, default to `[AudioVisual]->SellSound`. Note that vanilla game played vehicles' SellSound globally. This has been changed in consistency with buildings' SellSound.

In `rulesmd.ini`:
```ini
[SOMETECHNO]    ; BuildingType or UnitType
EVA.Sold=       ; EVA entry
SellSound=      ; sound entry
```

### Sound entry on unit's creation

- When a unit is created, sound specified in `VoiceCreated` will be played for the unit owner.
- If `IsVoiceCreatedGlobal` is set to true, `VoiceCreated` will be played globally instead of `EVA_UnitReady`.

In `rulesmd.ini`:
```ini
[AudioVisual]
IsVoiceCreatedGlobal=false   ; boolean

[SOMETECHNO]                 ; UnitType
VoiceCreated=                ; sound entry
```

## Terrain

### Destroy animation & sound

- You can now specify a destroy animation and sound for a TerrainType that are played when it is destroyed.

In `rulesmd.ini`:
```ini
[SOMETERRAINTYPE]  ; TerrainType
DestroyAnim=       ; Animation
DestroySound=      ; Sound
```

## Warheads

```{hint}
All new warhead effects
- can be used with CellSpread and Ares' GenericWarhead superweapon where applicable.
- cannot be used with `MindControl.Permanent=yes` of Ares.
```

### Break Mind Control on impact

![image](_static/images/remove-mc.gif)
*Mind control break warhead being utilized* ([RA2: Reboot](https://www.moddb.com/mods/reboot))

- Warheads can now break mind control (doesn't apply to perma-MC-ed objects).

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]            ; Warhead
RemoveMindControl=false  ; boolean
```

### Chance-based extra damage or Warhead detonation / 'critical hits'

- Warheads can now apply additional chance-based damage or Warhead detonation ('critical hits') with the ability to customize chance, damage, affected targets, affected target HP threshold and animations of critical hit.
  - `Crit.Chance` determines chance for a critical hit to occur. By default this is checked once when the Warhead is detonated and every target that is susceptible to critical hits will be affected. If `Crit.ApplyChancePerTarget` is set, then whether or not the chance roll is successful is determined individually for each target.
  - `Crit.ExtraDamage` determines the damage dealt by the critical hit. If `Crit.Warhead` is set, the damage is used to detonate the specified Warhead on each affected target, otherwise the damage is directly dealt based on current Warhead's `Verses` settings.
  - `Crit.Affects` can be used to customize types of targets that this Warhead can deal critical hits against.
  - `Crit.AffectsHouses` can be used to customize houses that this Warhead can deal critical hits against.
  - `Crit.AffectBelowPercent` can be used to set minimum percentage of their maximum `Strength` that targets must have left to be affected by a critical hit.
  - `Crit.AnimList` can be used to set a list of animations used instead of Warhead's `AnimList` if Warhead deals a critical hit to even one target. If `Crit.AnimList.PickRandom` is set (defaults to `AnimList.PickRandom`) then the animation is chosen randomly from the list.
    - `Crit.AnimOnAffectedTargets`, if set, makes the animation(s) from `Crit.AnimList` play on each affected target *in addition* to animation from Warhead's `AnimList` playing as normal instead of replacing `AnimList` animation.
  - `Crit.SuppressWhenIntercepted`, if set, prevents critical hits from occuring at all if the warhead was detonated from a [projectile that was intercepted](#projectile-interception-logic).
  - `ImmuneToCrit` can be set on TechnoTypes and ShieldTypes to make them immune to critical hits.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]                       ; Warhead
Crit.Chance=0.0                     ; floating point value, percents or absolute (0.0-1.0)
Crit.ApplyChancePerTarget=false     ; boolean
Crit.ExtraDamage=0                  ; integer
Crit.Warhead=                       ; Warhead
Crit.Affects=all                    ; list of Affected Target Enumeration (none|land|water|empty|infantry|units|buildings|all)
Crit.AffectsHouses=all              ; list of Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
Crit.AffectBelowPercent=1.0         ; floating point value, percents or absolute (0.0-1.0)
Crit.AnimList=                      ; list of animations
Crit.AnimList.PickRandom=           ; boolean
Crit.AnimOnAffectedTargets=false    ; boolean
Crit.SuppressWhenIntercepted=false  ; boolean

[SOMETECHNO]                        ; TechnoType
ImmuneToCrit=no                     ; boolean
```

```{warning}
If you set `Crit.Warhead` to the same Warhead it is defined on, or create a chain of Warheads with it that loops back to the first one there is a possibility for the game to get stuck in a loop and freeze or crash afterwards.
```

### Convert TechnoType on impact

- Warheads can now change TechnoTypes of affected units to other Types in the same category (infantry to infantry, vehicles to vehicles, aircraft to aircraft).
  - `ConvertN.From` (where N is 0, 1, 2...) specifies which TechnoTypes are valid for conversion. This entry can have many types listed, meanging that many types will be converted at once. When no types are included, conversion will affect all valid targets.
  - `ConvertN.To` specifies the TechnoType which is the result of conversion.
  - `ConvertN.AffectedHouses` specifies whose units can be converted.
  - `Convert.From`, `Convert.To` and `Convert.AffectedHouses` (without numbers) are a valid alternative to `Convert0.From`, `Convert0.To` and `Convert0.AffectedHouses` if only one pair is specified.

In example, this warhead would convert all affected owned and friendly `SOLDIERA` and `SOLDIERB` to `NEWSOLDIER`:
```ini
[SOMEWARHEAD]
Convert.From=SOLDIERA,SOLDIERB
Convert.To=NEWSOLDIER
Convert.AffectedHouses=team
```

```{warning}
This feature has the same limitations as [Ares' Type Conversion](https://ares-developers.github.io/Ares-docs/new/typeconversion.html). This feature does not support BuildingTypes.
```

```{warning}
This feature requires Ares 3.0 or higher to function! When Ares 3.0+ is not detected, not all properties of a unit may be updated.
```

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]                   ; Warhead
ConvertN.From=                  ; list of TechnoTypes
ConvertN.To=                    ; TechnoType
ConvertN.AffectedHouses=all     ; list of Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
; where N = 0, 1, 2, ...
; or
Convert.From=                   ; list of TechnoTypes
Convert.To=                     ; TechnoType
Convert.AffectedHouses=all      ; list of Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
```

### Custom 'SplashList' on Warheads

![image](_static/images/splashlist-01.gif)
- Allows Warheads to play custom water splash animations. See vanilla's [Conventional](https://www.modenc.renegadeprojects.com/Conventional) system here. `SplashList.PickRandom` can be set to true to pick a random animation to play from the list.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]                ; Warhead
SplashList=<none>            ; list of animations
SplashList.PickRandom=false  ; boolean
```

### Detonate Warhead on all objects on map

```{warning}
While this feature can provide better performance than a large `CellSpread` value, it still has potential to slow down the game, especially if used in conjunction with things like animations, alpha lights etc. Modder discretion and use of the filter keys (`AffectTargets/Houses/Types` etc.) is advised.
```

- Setting `DetonateOnAllMapObjects` to true allows a Warhead that is detonated by a projectile (for an example, this excludes things like animation `Warhead` and Ares' GenericWarhead superweapon but includes `Crit.Warhead` and animation `Weapon`) and consequently any `Airburst/ShrapnelWeapon` that may follow to detonate on each object currently alive and existing on the map regardless of its actual target, with optional filters. Note that this is done immediately prior Warhead detonation so after `PreImpactAnim` *(Ares feature)* has been displayed.
  - `DetonateOnAllMapObjects.AffectTargets` is used to filter which types of targets (TechnoTypes) are considered valid and must be set to a valid value other than `none` for this feature to work. Only `none`, `all`, `aircraft`, `buildings`, `infantry` and `units` are valid values. This is set to `none` by default as inclusion of all object types can be performance-heavy.
  - `DetonateOnAllMapObjects.AffectHouses` is used to filter which houses targets can belong to be considered valid and must be set to a valid value other than `none` for this feature to work. Only applicable if the house that fired the projectile is known. This is set to `none` by default as inclusion of all houses can be performance-heavy.
  - `DetonateOnAllMapObjects.AffectTypes` can be used to list specific TechnoTypes to be considered as valid targets. If any valid TechnoTypes are listed, then only matching objects will be targeted. Note that `DetonateOnAllMapObjects.AffectTargets` and `DetonateOnAllMapObjects.AffectHouses` take priority over this setting.
  - `DetonateOnAllMapObjects.IgnoreTypes` can be used to list specific TechnoTypes to be never considered as valid targets.
  - `DetonateOnAllMapObjects.RequireVerses`, if set to true, only considers targets whose armor type the warhead has non-zero `Verses` value against as valid. On targets with active shields, shield's armor type is used unless the Warhead has `Shield.Penetrate=true`. This is checked after all other filters listed above.

 In `rulesmd.ini`:
```ini
[SOMEWARHEAD]                                ; Warhead
DetonateOnAllMapObjects=false                ; boolean
DetonateOnAllMapObjects.AffectTargets=none   ; list of Affected Target Enumeration (none|aircraft|buildings|infantry|units|all)
DetonateOnAllMapObjects.AffectHouses=none    ; list of Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
DetonateOnAllMapObjects.AffectTypes=         ; list of TechnoType names
DetonateOnAllMapObjects.IgnoreTypes=         ; list of TechnoType names
DetonateOnAllMapObjects.RequireVerses=false  ; boolean
```

### Generate credits on impact

![image](_static/images/hackerfinallyworks-01.gif)
*`TransactMoney` used in [Rise of the East](https://www.moddb.com/mods/riseoftheeast) mod*

- Warheads can now give credits to its owner at impact.
  - `TransactMoney.Display` can be set to display the amount of credits given or deducted. The number is displayed in green if given, red if deducted and will move upwards after appearing.
    - `TransactMoney.Display.AtFirer` if set, makes the credits display appear on firer instead of target. If set and firer is not known, it will display at target regardless.
    - `TransactMoney.Display.Houses` determines which houses can see the credits display.
    - `TransactMoney.Display.Offset` is additional pixel offset for the center of the credits display, by default (0,0) at target's/firer's center.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]                        ; Warhead
TransactMoney=0                      ; integer - credits added or subtracted
TransactMoney.Display=false          ; boolean
TransactMoney.Display.AtFirer=false  ; boolean
TransactMoney.Display.Houses=All     ; Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
TransactMoney.Display.Offset=0,0     ; X,Y, pixels relative to default
```

### Launch superweapons on impact

- Superweapons can now be launched when a warhead is detonated.
  - `LaunchSW` specifies the superweapons to launch when the warhead is detonated. If superweapon has negative `Money.Amount`, the firing house must have enough credits in order for it to be fired.
  - `LaunchSW.RealLaunch` controls whether the owner who fired the warhead must own all listed superweapons. Otherwise they will be launched out of nowhere.
  - `LaunchSW.IgnoreInhibitors` ignores `SW.Inhibitors`/`SW.AnyInhibitor` of each superweapon, otherwise only non-inhibited superweapons are launched.
  - `LaunchSW.IgnoreDesignators` ignores `SW.Designators`/`SW.AnyDesignator` respectively.
  - `LaunchSW.DisplayMoney` can be set to display the amount of credits given or deducted by the launched superweapon by `Money.Amount`. The number is displayed in green if given, red if deducted and will move upwards after appearing.
    - `LaunchSW.DisplayMoney.Houses` determines which houses can see the credits display.
    - `LaunchSW.DisplayMoney.Offset` is additional pixel offset for the center of the credits display, by default (0,0) at superweapon's target cell.

```{note}
- For animation warheads/weapons to take effect, `Damage.DealtByInvoker` must be set.

- Due to the nature of some superweapon types, not all superweapons are suitable for launch. **Please use with caution!**

- The superweapons are launched on the *cell* where the warhead is detonated, instead of being click-fired.
```

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]                     ; Warhead
LaunchSW=                         ; list of superweapons
LaunchSW.RealLaunch=true          ; boolean
LaunchSW.IgnoreInhibitors=false   ; boolean
LaunchSW.IgnoreDesignators=true   ; boolean
LaunchSW.DisplayMoney=false       ; boolean
LaunchSW.DisplayMoney.Houses=all  ; Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
LaunchSW.DisplayMoney.Offset=0,0  ; X,Y, pixels relative to default
```

### Remove disguise on impact

- Warheads can now remove disguise from disguised infantry such as spies. This will work even if the disguised was acquired by default through `PermaDisguise`.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]         ; Warhead
RemoveDisguise=false  ; boolean
```

### Reveal map for owner on impact

- Warheads can now reveal the entire map on impact.
  - Reveal only applies to the owner of the warhead.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]  ; Warhead
SpySat=false   ; boolean
```

### Shroud map for enemies on impact

- Warheads can now shroud the entire map on impact.
- Shroud only applies to enemies of the warhead owner.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]  ; Warhead
BigGap=false   ; boolean
```

### Trigger specific NotHuman infantry Death anim sequence
- Warheads are now able to trigger specific `NotHuman=yes` infantry `Death` anim sequence using the corresponding tag. It's value represents sequences from `Die1` to `Die5`.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]            ; Warhead
NotHuman.DeathSequence=  ; integer (1 to 5)
```

## Weapons

### AreaFire target customization

- You can now specify how AreaFire weapon picks its target. By default it targets the base cell the firer is currently on, but this can now be changed to fire on the firer itself or at a random cell within the radius of the weapon's `Range` by setting `AreaFire.Target` to `self` or `random` respectively.
- `AreaFire.Target=self` respects normal targeting rules (Warhead Verses etc.) against the firer itself.
- `AreaFire.Target=random` ignores cells that are ineligible or contain ineligible objects based on listed values in weapon's `CanTarget` & `CanTargetHouses`.

In `rulesmd.ini`:
```ini
[SOMEWEAPON]         ; WeaponType
AreaFire.Target=base ; AreaFire Target Enumeration (base|self|random)
```

### Burst delay customizations

- `Burst.Delays` allows specifying weapon-specific burst shot delays. Takes precedence over the old `BurstDelayX` logic available on VehicleTypes, functions with Infantry & BuildingType weapons (AircraftTypes are not supported due to their weapon firing system being completely different) and allows every shot of `Burst` to have a separate delay instead of only first four shots.
  - If no delay is defined for a shot, it falls back to last delay value defined (f.ex `Burst=3` and `Burst.Delays=10` would use 10 as delay for all shots).
  - Using `-1` as delay reverts back to old logic (`BurstDelay0-3` for VehicleTypes if available or random value between 3-5 otherwise) for that shot.
- `Burst.FireWithinSequence` is only used if the weapon is fired by InfantryTypes, and setting it to true allows infantry to fire multiple `Burst` shots within same firing sequence.
  - First shot is always fired at sequence frame determined by firing frame controls on InfantryType image (`FireUp` et al).
  - Following shots come at intervals determined by `Burst.Delays` (with minimum delay of 1 frame) or random delay between 3 to 5 frames if not defined. Note that if next shot would be fired at a frame that is beyond the firing sequence's length, burst shot count is reset and weapon starts reloading.
  - Burst shot counter is not immediately reset if firing is ceased mid-sequence after at least one shot, but the frame at which each burst shot is fired will not be influenced by this (in other words, resuming firing afterward without weapon reload taking place would restart firing sequence but no firing will take place until the frame at which next burst shot should be fired is hit).

In `rulesmd.ini`:
```ini
[SOMEWEAPON]                    ; WeaponType
Burst.Delays=-1                 ; integer - burst delays (comma-separated) for shots in order from first to last.
Burst.FireWithinSequence=false  ; boolean
```

### Extra warhead detonations

- It is now possible to have same weapon detonate multiple Warheads on impact by listing `ExtraWarheads`. The warheads are detonated at same location as the main one, after it in listed order. This only works in cases where a projectile has been fired by a weapon and still remembers it when it is detonated (due to currently existing technical limitations, this excludes `AirburstWeapon`).
  - `ExtraWarheads.DamageOverrides` can be used to override the weapon's `Damage` for the extra Warhead detonations. Value from position matching the position from `ExtraWarheads` is used if found. If not, weapon `Damage` is used.

In `rulesmd.ini`:
```ini
[SOMEWEAPON]                    ; WeaponType
ExtraWarheads=                  ; list of WarheadTypes
ExtraWarheads.DamageOverrides=  ; list of integers
```

### Feedback weapon

![image](_static/images/feedbackweapon.gif)
*`FeedbackWeapon` used to apply healing aura upon firing a weapon* ([Project Phantom](https://www.moddb.com/mods/project-phantom))

- You can now specify an auxiliary weapon to be fired on the firer itself when a weapon is fired.
  - `FireInTransport` setting of the feedback weapon is respected to determine if it can be fired when the original weapon is fired from inside `OpenTopped=true` transport. If feedback weapon is fired, it is fired on the transport. `OpenToppedDamageMultiplier` is not applied on feedback weapons.

In `rulesmd.ini`:
```ini
[SOMEWEAPON]     ; WeaponType
FeedbackWeapon=  ; WeaponType
```

### Radiation enhancements

- In addition to allowing custom radiation types, several enhancements are also available to the default radiation type defined in `[Radiation]`, such as ability to set owner & invoker or deal damage against buildings. See [Custom Radiation Types](#custom-radiation-types) for more details.

### Strafing aircraft weapon customization

![image](_static/images/strafing-01.gif)
*Strafing aircraft weapon customization in [Project Phantom](https://www.moddb.com/mods/project-phantom)*

- Some of the behavior of strafing aircraft weapons (weapon projectile has `ROT` below 2) can now be customized.
  - `Strafing.Shots` controls the number of times the weapon is fired during a single strafe run. `Ammo` is only deducted at the end of the strafe run, regardless of the number of shots fired. Valid values range from 1 to 5, any values smaller or larger are effectively treated same as either 1 or 5, respectively. Defaults to 5.
  - `Strafing.SimulateBurst` controls whether or not the shots fired during strafing simulate behavior of `Burst`, allowing for alternating firing offset. Only takes effect if weapon has `Burst` set to 1 or undefined. Defaults to false.

In `rulesmd.ini`:
```ini
[SOMEWEAPON]                  ; WeaponType
Strafing.Shots=5              ; integer
Strafing.SimulateBurst=false  ; boolean
```

### Weapon targeting filter

![image](_static/images/weaponfilter.gif)
*`Weapon target filter - different weapon used against enemies & allies as well as units & buildings* ([Project Phantom](https://www.moddb.com/mods/project-phantom))

- You can now specify which targets or houses a weapon can fire at. This also affects weapon selection, other than certain special cases where the selection is fixed.
  - Note that `CanTarget` explicitly requires either `all` or `empty` to be listed for the weapon to be able to fire at cells containing no TechnoTypes.

In `rulesmd.ini`:
```ini
[SOMEWEAPON]         ; WeaponType
CanTarget=all        ; list of Affected Target Enumeration (none|land|water|empty|infantry|units|buildings|all)
CanTargetHouses=all  ; list of Affected House Enumeration (none|owner/self|allies/ally|team|enemies/enemy|all)
```
