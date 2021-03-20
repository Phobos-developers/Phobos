# New / Enhanced Logics

This page describes all the engine features that are either new and introduced by Phobos or significantly extended or expanded.

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

## TechnoTypes

### Mind control maximum range

![image](_static/images/mindcontrol-max-range-01.gif)  
- Mind controllers now can have the upper limit of the control distance. Tag values greater than 0 will activate this feature.

In rulesmd.ini:
```ini
[SOMETECHNO]               ; TechnoType
MindControlRangeLimit=-1.0 ; double
```

## Warheads

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
- Set Attributes to determine whether this logic affects allies or apply cellspread.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]                      ; Warhead
RemoveDisguise=no                  ; boolean
RemoveDisguise.AffectAllies=no     ; boolean
RemoveDisguise.ApplyCellSpread=no  ; boolean
```

### Break Mind Control on impact

- Warheads can now break mind control (doesn't apply to perma-MC-ed objects).
- Set Attributes to determine whether this logic affects allies or apply cellspread.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]                        ; Warhead
RemoveMindControl=no                 ; boolean
RemoveMindControl.AffectAllies=no    ; boolean
RemoveMindControl.ApplyCellSpread=no ; boolean
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

### Kill spawned aircrafts on Low Power

- `Powered=yes` structures that spawns like Aircrafts Carriers will stop targeting the enemy if low power.
- Spawned aircrafts self-destructs if they are flying.

In `rulesmd.ini`:
```ini
[SOMESTRUCTURE]       ; BuildingType
Powered.KillSpawns=no ; boolean
```

### Spawns Limit Range

- The spawned units will abort the infinite pursuit if the enemy is out of range.
`Spawner.ExtraLimitRange` adds extra pursuit range to the spawned units.

In `rulesmd.ini`:
```ini
[SOMETECHNO]              ; TechnoType
Spawner.LimitRange=no     ; boolean
Spawner.ExtraLimitRange=0 ; integer
```

## Projectiles

### Projectile Interception Logic

![image](_static/images/projectile-interception-01.gif)  
*Interception logic used in [Tiberium Crisis](https://www.moddb.com/mods/tiberium-crisis) mod*

- Projectiles can now be made targetable by certain TechnoTypes. Interceptor TechnoType's projectile must be `Inviso=yes` in order for it to work and the projectile must be used in a primary Weapon. 

In `rulesmd.ini`:
```ini
[SOMETECHNO]                    ; TechnoType
Interceptor=no                  ; boolean
Interceptor.GuardRange=0.0      ; double
Interceptor.EliteGuardRange=0.0 ; double

[SOMEPROJECTILE] ; Projectile
Interceptable=no ; boolean
```