# Fixed / Improved Logics

This page describes all ingame logics that are fixed or improved in Phobos without adding anything significant.

## Bugfixes and miscellaneous

- Fixed the bug when deploying mindcontrolled vehicle into a building permanently transferred the control to the house which mindcontrolled it.
- Fixed the bug when units are already dead but still in map (for sinking, crashing, dying animation, etc.), they could die again.
- Fixed the bug when cloaked Desolator was unable to fire his deploy weapon.
- SHP debris shadows now respect the `Shadow` tag.
- Allowed usage of TileSet of 255 and above without making NE-SW broken bridges unrepairable.

![image](_static/images/turretoffset-01.png)  
*Side offset voxel turret in Breaking Blue project*

- `TurretOffset` tag for voxel turreted TechnoTypes now accepts FLH (forward, lateral, height) values like `TurretOffset=F,L` or `TurretOffset=F,L,H`, which means turret location can be adjusted in all three axes.
- `InfiniteMindControl` with `Damage=1` can now control more than 1 unit.
- Aircraft with `Fighter` set to false or those using strafing pattern (weapon projectile `ROT` is below 2) now take weapon's `Burst` into accord for all shots instead of just the first one.
- `EMEffect` used for random AnimList pick is now replaced by a new tag `AnimList.PickRandom` with no side effect. (EMEffect=yes on AA inviso projectile deals no damage to units in movement)
- Script action `Move to cell` now obeys YR cell calculation now. Using `1000 * Y + X` as its cell value. (was `128 * Y + X` as it's RA leftover)
- The game now can reads waypoints ranges in [0, 2147483647]. (was [0,701])
- Map trigger action `125 Build At...` can now play buildup anim optionally (needs [following changes to `fadata.ini`](Whats-New.md#for-map-editor-final-alert-2).
- Vehicles using `DeployFire` will now explicitly use weapon specified by `DeployFireWeapon` for firing the deploy weapon and respect `FireOnce` setting on weapon and any stop commands issued during firing.
- Fixed `DebrisMaximums` (spawned debris type amounts cannot go beyond specified maximums anymore). Only applied when `DebrisMaximums` values amount is more than 1 for compatibility reasons.
- Fixed building and defense tab hotkeys not enabling the placement mode after `Cannot build here.` triggered and the placement mode cancelled.
- Fixed buildings with `UndeployInto` playing `EVA_NewRallypointEstablished` on undeploying.
- Fixed buildings with `Naval=yes` ignoring `WaterBound=no` to be forced to place onto water.

![image](_static/images/remember-target-after-deploying-01.gif)  
*Nod arty keeping target on attack order in [C&C: Reloaded](https://www.moddb.com/mods/cncreloaded/)*

- Vehicle to building deployers now keep their target when deploying with `DeployToFire`.
- Fixed laser drawing code to allow for thicker lasers in house color draw mode.
- `DeathWeapon` now will properly detonate. 
  - But still some settings are ignored like `PreImpactAnim` *(Ares feature)*, this might change in future.
- Effects like lasers are no longer drawn from wrong firing offset on weapons that use Burst.
- Both Global Variables (`VariableNames` in `rulesmd.ini`) and Local Variables (`VariableNames` in map) are now unlimited.

## Technos

### Customizable Teleport/Chrono Locomotor settings per TechnoType

![image](_static/images/cust-Chrono.gif)  
*Chrono Legionnaire and Ronco (hero) from [YR:New War](https://www.moddb.com/mods/yuris-revenge-new-war)*

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

### Re-enable obsolete [JumpjetControls] 

- Re-enable obsolete [JumpjetControls], the keys in it will be as the default value of jumpjet units.
  - Moreover, added two tags for missing ones.

In `rulesmd.ini`:
```ini
[JumpjetControls]
Crash=5.0       ; float
NoWobbles=no    ; bool
```

```{note}
`CruiseHeight` is for `JumpjetHeight`, `WobblesPerSecond` is for `JumpjetWobbles`, `WobbleDeviation` is for `JumpjetDeviation`, and `Acceleration` is for `JumpjetAccel`. All other corresponding keys just simply have no Jumpjet prefix.
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

### Kill spawns on low power

- `Powered=yes` structures that spawns aircraft like Aircrafts Carriers will stop targeting the enemy if low power.
- Spawned aircrafts self-destruct if they are flying.

In `rulesmd.ini`:
```ini
[SOMESTRUCTURE]       ; BuildingType
Powered.KillSpawns=no ; boolean
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

### Toggle-able ElectricBolt visuals

- You can now specify individual ElectricBolt bolts you want to disable. Note that this is only a visual change.

In `rulesmd.ini`:
```ini
[SOMEWEAPONTYPE]       ; WeaponType
IsElectricBolt=true    ; an ElectricBolt Weapon, vanilla tag
Bolt.Disable1=false    ; boolean
Bolt.Disable2=false    ; boolean
Bolt.Disable3=false    ; boolean
```
