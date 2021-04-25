# Fixed / Improved Logics

This page describes all ingame logics that are fixed or improved in Phobos without adding anything significant.

## Bugfixes and miscellanous

- Fixed the bug when deploying mindcontrolled vehicle into a building permanently trasferred the control to the house which mindcontrolled it.
- Fixed the bug when units are already dead but still in map (for sinking, crashing, dying animation, etc.), they could die again.
- Fixed the bug when cloaked Desolator was unable to fire his deploy weapon.
- SHP debris shadows now respect the `Shadow` tag.
- Allowed usage of TileSet of 255 and above without making NE-SW broken bridges unrepairable.
- `TurretOffset` tag for voxel turreted technos now accepts FLH (forward, lateral, height) values like `TurretOffset=F,L` or `TurretOffset=F,L,H`, which means turret location can be adjusted in all three axes.
- `InfiniteMindControl` with `Damage=1` can now control more than 1 unit.
- Aircraft with `Fighter` set to false or those using strafing pattern (weapon projectile `ROT` is below 2) now take weapon's `Burst` into accord for all shots instead of just the first one.

![image](_static/images/remember-target-after-deploying-01.gif)  
*Nod arty keeping target on attack order in [C&C: Reloaded](https://www.moddb.com/mods/cncreloaded/)*

- Vehicle to building deployers now keep their target when deploying with `DeployToFire`.

## Vehicles

### Customizable disk laser radius

![image](_static/images/disklaser-radius-values-01.gif)  
- You can now set disk laser animation radius using a new tag.

In `rulesmd.ini`:
```ini
[SOMEWEAPON]          ; WeaponType
DiskLaser.Radius=38.2 ; floating point value
                      ; 38.2 is roughly the default saucer disk radius
```

### Kill spawns on low power

- `Powered=yes` structures that spawns aircraft like Aircrafts Carriers will stop targeting the enemy if low power.
- Spawned aircrafts self-destruct if they are flying.

In `rulesmd.ini`:
```ini
[SOMESTRUCTURE]       ; BuildingType
Powered.KillSpawns=no ; boolean
```

## Terrain

### Customizable ore spawners

- You can now specify which type of tiberium certain TerrainType would generate.
- It's also now possible to specify a range value for an ore generation area different compared to standard 3x3 rectangle. Ore will be uniformly distributed across all affected cells in a spread range.
- You can specify which tiberium growth stage will be spawned and how much cells will be filled with ore per tiberium generation animation. Corresponding tags accept either a single integer value or two comma-separated values to allow randomized growth stages from the range (inclusive).

In `rulesmd.ini`:
```ini
[SOMETERRAINTYPE]             ; TerrainType
SpawnsTiberium.Type=0         ; tiberium type index
SpawnsTiberium.Range=1        ; integer, radius in cells
SpawnsTiberium.GrowthStage=3  ; single int / comma-sep. range
SpawnsTiberium.CellsPerAnim=1 ; single int / comma-sep. range
```

## TechnoType

### Customizable Teleport/Chrono Locomotor settings per TechnoType 

![image](_static/images/cust-Chrono.gif)  
*Chrono Legionere and Ronco (hero) from [YR:New War] (https://www.moddb.com/mods/yuris-revenge-new-war)*

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

## Weapons

### Togglable ElectricBolt visuals

- You can now specify individual ElectricBolt bolts you want to disable. Note that this is only a visual change.

In `rulesmd.ini`:
```ini
[SOMEWEAPONTYPE]       ; WeaponType
IsElectricBolt=true    ; an ElectricBolt Weapon, vanilla tag
Bolt.Disable1=false    ; boolean
Bolt.Disable2=false    ; boolean
Bolt.Disable3=false    ; boolean
```