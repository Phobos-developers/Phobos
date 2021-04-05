# Fixed / Improved Logics

This page describes all ingame logics that are fixed or improved in Phobos without adding anything significant.

## Bugfixes and miscellanous

- Fixed the bug when deploying mindcontrolled vehicle into a building permanently trasferred the control to the house which mindcontrolled it.
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
- You can specify which tiberium growth stage will be spawned. Corresponding tag accepts either a single integer value or two comma-separated values to allow randomized growth stages from the range (inclusive).

In `rulesmd.ini`:
```ini
[SOMETERRAINTYPE]             ; TerrainType
SpawnsTiberium.Type=0         ; tiberium type index
SpawnsTiberium.Range=1        ; integer, radius in cells
SpawnsTiberium.GrowthStage=3  ; single int / comma-sep. range
```

## Techno

### Customizable Teleport/Chrono Locomotor properties per attached Techno 

-Now fully customizeable per techno type direcly 
-Unfilled value will still default to Rules
-This properties able to used by Techno that have Teleport Locomotor/Chrono attached

In `rulesmd.ini`:
```ini
[SOMETECHNO]			; TechnoType 
Locomotor={4A582747-9839-11d1-B709-00A024DDAFD1} ;required 
WarpOut					;Anim (played when Techno Warping out)			
WarpIn					;Anim (played when techno warping in)
WarpAway				;Anim (played when unit dies)
ChronoTrigger			;bool
ChronoDistanceFactor	;int
ChronoMinimumDelay		;int
ChronoRangeMinimum		;int
ChronoDelay				;int
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