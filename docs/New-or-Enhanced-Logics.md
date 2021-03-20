# New / Enhanced Logics

This page describes all the engine features that are either new and introduced by Phobos or significantly extended or expanded.

## Buildings

### Extended building upgrades logic

![](https://media.moddb.com/cache/images/mods/1/35/34805/thumb_620x2000/powersup.owner.png)

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

![mcrangelimittest2](https://user-images.githubusercontent.com/17500545/107950652-b27dbc80-6f9f-11eb-8cf0-f47367130a22.gif)

- Mind controllers now can have the upper limit of the control distance. Tag values greater than 0 will activate this feature.

In rulesmd.ini:
```ini
[SOMETECHNO]               ; TechnoType
MindControlRangeLimit=-1.0 ; double
```

## Warheads

### Generate credits on impact

![image](https://media.moddb.com/cache/images/mods/1/30/29781/thumb_620x2000/hackerfinallyworks.gif)  
*`TransactMoney` used in [Rise of the East](https://www.moddb.com/mods/riseoftheeast) mod*

- Warheads can now give credits to its owner at impact.

In `rulesmd.ini`:
```ini
[SOMEWARHEAD]   ; Warhead
TransactMoney=0 ; integer - credits added or subtracted
```

### Reveal map for owner on impact

![image](https://cdn.discordapp.com/attachments/773636942775582720/803047875944775680/revealwarhead.gif)
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

![image](https://www.riseoftheeastmod.com/web/splashlistgif.gif)
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

![image](https://user-images.githubusercontent.com/29500471/110206713-57622a00-7eba-11eb-9bdc-850dd312e2cb.gif)  
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

## ScriptType Actions

### Timed Area Guard

- Puts the TaskForce into Area Guard Mode for the given units of time. Unlike the orignal timed Guard script (5,n) that just stays in place doing a basic guard operation the "Area Guard" action has a more active role attacking nearby invasors or defending units that needs protection.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=71,n            ; integer
```
### Load Onto Transports

- If the TaskForce contains unit(s) that can be carried by the transports of the same TaskForce then this action will make the units enter the transports. In Single player missions the next action must be "Wait until fully loaded" (43,0) or the script will not continue.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=72,0
```
