# Fixed / Improved Logics

This page describes all ingame logics that are fixed or improved in Phobos without adding anything significant.

## Bugfixes and miscellanous

- Fixed the bug when deploying mindcontrolled vehicle into a building permanently trasferred the control to the house which mindcontrolled it.
  - Currently doesn't work with superweapons attached to the deployed building or similiar logics.
- SHP debris shadows now respect the `Shadow` tag.
- Allowed usage of TileSet of 255 and above without making NE-SW broken bridges unrepairable.
- `TurretOffset` tag for voxel turreted technos now accepts FLH (forward, lateral, height) values like `TurretOffset=F,L` or `TurretOffset=F,L,H`, which means turret location can be adjusted in all three axes.

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

### Remember target when deployed

![image](_static/images/remember-target-after-deploying-01.gif)  
*Vehicle keeping target after deployed to building in [C&C: Reloaded](https://www.moddb.com/mods/cncreloaded/)*

- Vehicle to building deployers can now keep their target when deploying.

In `rulesmd.ini`:
```ini
[SOMEVEHICLE]              ; TechnoType
Deployed.RememberTarget=no ; boolean
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

### Customizable spawned ore types

- Allows to specify which type of tiberium this TerrainType would generate.

In `rulesmd.ini`:
```ini
[SOMETERRAINTYPE]      ; TerrainType
SpawnsTiberium.Type=0  ; Tiberium type index
```