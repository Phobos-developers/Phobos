# AI Scripting and Mapping

This page describes all AI scripting and mapping related additions and changes introduced by Phobos.

## Bugfixes and Miscellanous

- Script action `Move to cell` now obeys YR cell calculation now. Using `1000 * Y + X` as its cell value. (was `128 * Y + X` as it's a RA1 leftover)
- The game now can reads waypoints ranges in [0, 2147483647]. (was [0,701])
- Map trigger action `41 Play Animation At...` can now create 'non-inert' animations which can play sounds, deal damage and apply `TiberiumChainReaction` if a parameter is set (needs [following changes to `fadata.ini`](Whats-New.md#for-map-editor-final-alert-2).
- Map trigger action `125 Build At...` can now play buildup anim and becomes singleplayer-AI-repairable optionally (needs [following changes to `fadata.ini`](Whats-New.md#for-map-editor-final-alert-2).
- Both Global Variables (`VariableNames` in `rulesmd.ini`) and Local Variables (`VariableNames` in map) are now unlimited.
- Script action `Deploy` now has vehicles with `DeploysInto` searching for free space to deploy at if failing to do so at initial location, instead of simply getting stuck.
- Teams spawned by trigger action 7,80,107 can use IFV and opentopped logic normally. `InitialPayload` logic from Ares is not supported yet.
- If a pre-placed building has a `NaturalParticleSystem`, it used to always be created when the game starts. This has been removed.
- Superweapons used by AI for script actions `56 Chronoshift to Building`, `57 Chronoshift to a Target Type` and `10104 Chronoshift to Enemy Base` can now be explicitly set via `[General]` -> `AIChronoSphereSW` & `AIChronoWarpSW` respectively. If `AIChronoSphereSW` is set but `AIChronoWarpSW` is not, game will check former's `SW.PostDependent` for a second superweapon to use. Otherwise if not set, last superweapon listed in `[SuperWeaponTypes]` with `Type=ChronoSphere` or `Type=ChronoWarp` will be used, respectively.

## Singleplayer Mission Maps

### Base node repairing

- In singleplayer campaign missions you can now decide whether AI can repair the base nodes / buildings delivered by SW (Ares) by setting `RepairBaseNodes`.

In map file:
```ini
[Country House]
RepairBaseNodes=false,false,false  ; list of 3 booleans indicating whether AI repair basenodes in Easy / Normal / Difficult game diffculty.
```

### Default loading screen and briefing offsets

- It is now possible to set defaults for singleplayer map loading screen briefing pixel offsets and the loading screen images and palette that are used if there are no values defined for the map itself.
  - Note that despite the key name being `DefaultLS800BkgdPal`, this applies to both shapes just like the original scenario-specific `LS800BkgdPal` does.

- In `missionmd.ini`:
```ini
[Defaults]
DefaultLS640BriefLocX=0  ; integer
DefaultLS640BriefLocY=0  ; integer
DefaultLS800BriefLocX=0  ; integer
DefaultLS800BriefLocY=0  ; integer
DefaultLS640BkgdName=    ; filename - including the .shp extension.
DefaultLS800BkgdName=    ; filename - including the .shp extension.
DefaultLS800BkgdPal=     ; filename - including the .pal extension
```

### MCV redeploying

- You can now decide whether MCV can redeploy in singleplayer campaign missions by setting `MCVRedeploys`. Overrides `[MultiplayerDialogSettings]`->`MCVRedeploys` only in singleplayer campaign missions.

In map file:
```ini
[Basic]
MCVRedeploys=false  ; boolean
```

### Set par times and related string labels in missionmd.ini

- By default the singleplayer mission par times and message strings are defined in `[Ranking]` section of the map file itself. These can now also be set in the map file's section in `missionmd.ini`, taking precedence over the map file's settings but defaulting to them if not set.

In `missionmd.ini`:
```ini
[SOMEMISSION]             ; Filename of mission map
Ranking.ParTimeEasy=      ; time string (hh:mm:ss)
Ranking.ParTimeMedium=    ; time string (hh:mm:ss)
Ranking.ParTimeHard=      ; time string (hh:mm:ss)
Ranking.UnderParTitle=    ; CSF entry key
Ranking.UnderParMessage=  ; CSF entry key
Ranking.OverParTitle=     ; CSF entry key
Ranking.OverParMessage=   ; CSF entry key
```

### Show briefing dialog on startup

- You can now have the briefing dialog screen show up on singleplayer campaign mission startup by setting `ShowBriefing` to true in map file's `[Basic]` section, or in the map file's section in `missionmd.ini` (latter takes precedence over former if available). This can be disabled by user by setting `ShowBriefing` to false in `Ra2MD.ini`.
  - `BriefingTheme` (In order of precedence from highest to lowest: `missionmd.ini`, map file, side entry in `rulesmd.ini`) can be used to define a custom theme to play on this briefing screen. If not set, the loading screen theme will keep playing until the scenario starts properly.
  - String labels for the startup briefing dialog screen's resume button as well as the button's status bar text can be customized by setting `ShowBriefingResumeButtonLabel` and `ShowBriefingResumeButtonStatusLabel` respectively. They default to the same labels used by the briefing screen dialog when opened otherwise.

In `missionmd.ini`:
```ini
[SOMEMISSION]   ; Filename of mission map
ShowBriefing=   ; boolean
BriefingTheme=  ; Theme name
```

In map file:
```ini
[Basic]
ShowBriefing=false  ; boolean
BriefingTheme=      ; Theme name
```

In `rulesmd.ini`
```ini
[SOMESIDE]      ; Side
BriefingTheme=  ; Theme name
```

In `uimd.ini`
```ini
[UISettings]
ShowBriefingResumeButtonLabel=GUI:Resume                      ; CSF entry key
ShowBriefingResumeButtonStatusLabel=STT:BriefingButtonReturn  ; CSF entry key
```

In `RA2MD.ini`:
```ini
[Phobos]
ShowBriefing=true  ; boolean
```

## Script Actions

### `10000-10999` Ingame Actions

#### `10000-10049` Attack Actions

- These actions instruct the TeamType to use the TaskForce to approach and attack the target specified by the second parameter which is an index of a generic pre-defined group. Look at the tables below for the possible actions (first parameter value) and arguments (the second parameter value).
- For threat-based attack actions `TargetSpecialThreatCoefficientDefault` and `EnemyHouseThreatBonus` tags from `rulesmd.ini` are accounted.
- All aircraft that attack other air units will end the script. This behavior is intentional because without it aircraft had some bugs that weren't fixable at the time of developing the feature.
- `AITargetTypes` actions instruct the TeamType to use the TaskForce to approach and attack the target specified by the second parameter which is an index of a modder-defined group from `AITargetTypess`. Look at the tables below for the possible actions (first parameter value) and arguments (the second parameter value).

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=i,n             ; For i values check the next table
```

| *Action* | *Argument*             | *Repeats* | *Target Priority*      | *Description*                                      |
| :------: | :--------------------: | :-------: | :--------------------: | :------------------------------------------------: |
| 10000    | Target Type#           | Yes       | Closer                 |                                                    |
| 10001    | Target Type#           | No        | Closer                 | Ends when a team member kill the designated target |
| 10002    | `AITargetTypes` index# | Yes       | Closer                 |                                                    |
| 10003    | `AITargetTypes` index# | No        | Closer                 | Ends when a team member kill the designated target |
| 10004    | `AITargetTypes` index# | Yes       | Closer                 | Picks 1 random target from the list                |
| 10005    | Target Type#           | Yes       | Farther                |                                                    |
| 10006    | Target Type#           | No        | Farther                | Ends when a team member kill the designated target |
| 10007    | `AITargetTypes` index# | Yes       | Farther                |                                                    |
| 10008    | `AITargetTypes` index# | No        | Farther                | Ends when a team member kill the designated target |
| 10009    | `AITargetTypes` index# | Yes       | Farther                | Picks 1 random target from the list                |
| 10010    | Target Type#           | Yes       | Closer, higher threat  |                                                    |
| 10011    | Target Type#           | No        | Closer, higher threat  | Ends when a team member kill the designated target |
| 10012    | `AITargetTypes` index# | Yes       | Closer, higher threat  |                                                    |
| 10013    | `AITargetTypes` index# | No        | Closer, higher threat  | Ends when a team member kill the designated target |
| 10014    | Target Type#           | Yes       | Farther, higher threat |                                                    |
| 10015    | Target Type#           | No        | Farther, higher threat | Ends when a team member kill the designated target |
| 10016    | `AITargetTypes` index# | Yes       | Farther, higher threat |                                                    |
| 10017    | `AITargetTypes` index# | No        | Farther, higher threat | Ends when a team member kill the designated target |

- The following values are the *Target Type#* which can be used as second parameter of the new attack script actions:
  - 'Buildings considered as vehicles' means buildings with both `UndeploysInto` set & `Foundation=1x1` and `ConsideredVehicle` not set or buildings with `ConsideredVehicle=true`.

| *Value* | *Target Type*            | *Description*                                 |
| :-----: | :----------------------: | :-------------------------------------------: |
| 1       | Anything                 | Any enemy `VehicleTypes`, `AircraftTypes`, `InfantryTypes` and `BuildingTypes` |
| 2       | Structures               | Any enemy `BuildingTypes` that are not considered as vehicles |
| 3       | Ore Miners               | Any enemy `VehicleTypes` with `Harvester=yes` or `ResourceGatherer=yes`, `BuildingTypes` with `ResourceGatherer=yes` |
| 4       | Infantry                 | Any enemy `InfantryTypes` |
| 5       | Vehicles                 | Any enemy `VehicleTypes` or buildings considered as vehicles |
| 6       | Factories                | Any enemy `BuildingTypes` with a Factory= setting |
| 7       | Base Defenses            | Any enemy `BuildingTypes` with `IsBaseDefense=yes` |
| 8       | House Threats            | Any object that targets anything of the Team's House or any enemy that is near to the Team Leader |
| 9       | Power Plants             | Any enemy `BuildingTypes` with positive `Power=` values |
| 10      | Occupied                 | Any `BuildingTypes` with garrisoned infantry |
| 11      | Tech Buildings           | Any `BuildingTypes` with `Unsellable=yes`, `Capturable=yes`, negative `TechLevel=` values or appears in `[AI]>NeutralTechBuildings=` list |
| 12      |	Refinery                 | Any enemy `BuildingTypes` with `Refinery=yes` or `ResourceGatherer=yes`, `VehicleTypes` with `ResourceGatherer=yes` & `Harvester=no` (i.e. Slave Miner) |
| 13      | Mind Controller          | Anything `VehicleTypes`, `AircraftTypes`, `InfantryTypes` and `BuildingTypes` with `MindControl=yes` in the weapons Warheads |
| 14      | Air Units (incl. landed) | Any enemy, `AircraftTypes` and `Jumpjet=yes` `VehicleTypes` or `InfantryTypes`, including landed ones as well as any other currently airborne units |
| 15      | Naval                    | Any enemy `BuildingTypes` and `VehicleTypes` with a `Naval=yes`, any enemy `VehicleTypes`, `AircraftTypes`, `InfantryTypes` in a water cell |
| 16      | Disruptors               | Any enemy objects with positive `InhibitorRange=` values, positive `RadarJamRadius=` values, `CloakGenerator=yes` or `GapGenerator=yes` |
| 17      | Ground Vehicles          | Any enemy `VehicleTypes` without `Naval=yes`, landed `AircraftTypes` or buildings considered as vehicles |
| 18      | Economy                  | Any enemy `VehicleTypes` with `Harvester=yes` or `ResourceGatherer=yes`, `BuildingTypes` with `Refinery=yes`, `ResourceGatherer=yes` or `OrePurifier=yes` |
| 19      | Infantry Factory         | Any enemy `BuildingTypes` with `Factory=InfantryType` |
| 20      | Vehicle Factory          | Any enemy `BuildingTypes` with with `Naval=no` and `Factory=UnitType` |
| 21      | Aircraft Factory         | Any enemy `BuildingTypes` with `Factory=AircraftType` |
| 22      | Radar                    | Any enemy `BuildingTypes` with `Radar=yes` or `SpySat=yes` |
| 23      | Tech Lab                 | Any enemy `BuildingTypes` in `[AI]>BuildTech=` list |
| 24      | Naval Factory            | Any enemy `BuildingTypes` with `Naval=yes` and `Factory=UnitType` |
| 25      | Super Weapon             | Any enemy `BuildingTypes` with `SuperWeapon=`, `SuperWeapon2=` or `SuperWeapons=` |
| 26      | Construction Yard        | Any enemy `BuildingTypes` with `ConstructionYard=yes` and `Factory=BuildingType` |
| 27      | Neutrals                 | Any neutral object (Civilian) |
| 28      | Generators               | Any enemy `BuildingTypes` with `CloakGenerator=yes` or `GapGenerator=yes` |
| 29      | Radar Jammer             | Any enemy objects with positive `RadarJamRadius=` values |
| 30      | Inhibitors               | Any enemy objects with positive `InhibitorRange=` values |
| 31      | Naval Units              | Any enemy `VehicleTypes` with a `Naval=yes` or any enemy `VehicleTypes`, `AircraftTypes`, `InfantryTypes` in a water cell |
| 32      | Mobile Units             | Anything `VehicleTypes`, `AircraftTypes` and `InfantryTypes` |
| 33      | Capturable               | Any `BuildingTypes` with `Capturable=yes` or any `BuildingTypes` with `BridgeRepairHut=yes` and `Repairable=yes` |
| 34      | Area Threats             | Any enemy object that is inside of the Team Leader's Guard Area |
| 35      | Vehicle & Naval Factory  | Any enemy `BuildingTypes` with `Factory=UnitType` |
| 36      | Non-defensive Structures | Any enemy `BuildingTypes` with `IsBaseDefense=no` |

- The second parameter with a 0-based index for the `AITargetTypes` section specifies the list of possible `VehicleTypes`, `AircraftTypes`, `InfantryTypes` and `BuildingTypes` that can be evaluated.
- The *`AITargetTypes` index#* values are obtained in the new `AITargetTypes` section that must be declared in `rulesmd.ini`:

In `rulesmd.ini`:
```ini
[AITargetTypes]  ; List of TechnoType lists
0=SOMETECHNOTYPE,SOMEOTHERTECHNOTYPE,SAMPLETECHNOTYPE
1=ANOTHERTECHNOTYPE,YETANOTHERTECHNOTYPE
; ...
```

#### `10050-10099` Move Team to Techno Location actions

- These actions instructs the TeamType to use the TaskForce to approach the target specified by the second parameter. Look at the tables below for the possible actions (first parameter value).

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=i,n             ; For i values check the next table
```

| *Action* | *Argument*            | *Target Owner* | *Target Priority*      | *Description*                                |
| :------: | :-------------------: | :------------: | :--------------------: | :------------------------------------------: |
| 10050    | Target Type#          | Enemy          | Closer, higher threat  |                                              |
| 10051    | [AITargetType] index# | Enemy          | Closer, higher threat  |                                              |
| 10052    | [AITargetType] index# | Enemy          | Closer                 | Picks 1 random target from the selected list |
| 10053    | Target Type#          | Friendly       | Closer                 |                                              |
| 10054    | [AITargetType] index# | Friendly       | Closer                 |                                              |
| 10055    | [AITargetType] index# | Friendly       | Closer                 | Picks 1 random target from the selected list |
| 10056    | Target Type#          | Enemy          | Farther, higher threat |                                              |
| 10057    | [AITargetType] index# | Enemy          | Farther, higher threat |                                              |
| 10058    | [AITargetType] index# | Enemy          | Farther                | Picks 1 random target from the selected list |
| 10059    | Target Type#          | Friendly       | Farther                |                                              |
| 10060    | [AITargetType] index# | Friendly       | Farther                |                                              |
| 10061    | [AITargetType] index# | Friendly       | Farther                | Picks 1 random target from the selected list |

#### `10100-10999` General Purpose

##### `10100` Timed Area Guard

- Puts the TaskForce into Area Guard mode for the given units of time. Unlike the original timed Guard script action (`5,n`) that just stays in place doing a basic guard operation this action has a more active role attacking nearby invaders or defending units that needs protection.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=10100,n            ; integer, time in ingame seconds
```
##### `10101` Wait Until Ammo is Full

- If the TaskForce contains unit(s) that use ammo then the the script will not continue until all these units have fully refilled the ammo.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=10101,0
```
##### `10102` Regroup Temporarily Around the Team Leader

- Puts the TaskForce into Area Guard mode for the given amount of time around the Team Leader (this unit remains almost immobile until the action ends). The default radius around the leader is `[General] > CloseEnough` and the units will not leave that area.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=10102,n
```

##### `10103` Load onto Transports

- If the TaskForce contains unit(s) that can be carried by the transports of the same TaskForce then this action will make the units enter the transports. In single player missions the next action must be "Wait until fully loaded" (`43,0`) or the script will not continue.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=10103,0
```

##### `10104` Chronoshift to Enemy Base

- Chronoshifts the members of the TeamType using first available `Type=Chronosphere` superweapon to a location within `[General]` -> `AISafeDistance` (plus the additional distance defined in parameter, can be negative) cells from enemy house's base. The superweapon must be charged up to atleast `[General]` -> `AIMinorSuperReadyPercent` percentage of its recharge time to be available for use by this action.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=10104,n         ; integer, additional distance in cells
```

### `12000-12999` Suplementary/Setup Pre-actions

#### `12000` Wait if No Target Found

- When executed before a new Attack ScriptType actions like `Generic Target Type Attack` actions  and `AITargetTypes Attack` actions  the TeamType will remember that must wait 1 second if no target was selected. The second parameter is a positive value that specifies how much retries the Attack will do when no target was found before new Attack ScriptType Action is discarded & the script execution jumps to the next line. The value `0` means infinite retries.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=12000,n            ; integer n=0
```

#### `12001` Modify Target Distance

- By default `Moving Team to techno location` actions ends when the Team Leader reaches a distance declared in rulesmd.ini called `CloseEnough`. When this action is executed before the actions `Moving Team to techno location` overwrites `CloseEnough` value. This action works only the first time and `CloseEnough` will be used again the next Movement action.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=12001,n
```

#### `12002` Set Move Action End Mode

- Sets how the Movement actions ends and jumps to the next line. This action works only the first time and `CloseEnough` will be used again the next Movement action.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=12002,n
```

- The possible argument values are:

| *Argument* | *Action ends when...*                         |
| :--------: | :-------------------------------------------: |
| 0          | Team Leader reaches the minimum distance      |
| 1          | One unit reaches the minimum distance         |
| 2          | All team members reached the minimum distance |

### `14000-14999` Utility Actions

#### `14000` Team's Trigger Weight Reward

- When executed before a new Attack ScriptType actions like `Generic Target Type Attack` actions and `AITargetTypes Attack` actions the TeamType will remember that must be rewarded increasing the current weight of the AI Trigger when the TeamType Target was killed by any of the Team members. The current weight will never surprass the minimum weight and maximum weight limits of the AI Trigger. The second parameter is a positive value.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=14000,n         ; integer n=0
```

#### `14001` Increase AI Trigger Current Weight

- When executed this increases the current weight of the AI Trigger. The current weight will never surprass the minimum weight and maximum weight limits of the AI Trigger. Take note that all TeamTypes of the same AI Trigger will update the AI Trigger Current Weight sooner or later. The second parameter is a positive value. Take note that the original game only uses the first of the two Teams for calculating the AI Trigger Current weight at the end of the Trigger life, this action ignores if the Team is the first or the second of the AI Trigger and the Current weight is calculated when is executed the action.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=14001,n
```

#### `14002` Decrease AI Trigger Current Weight

- When executed this decreases the current weight of the AI Trigger. Details same as above.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=14002,n
```

#### `14003` Unregister Team Success

- Is just the opposite effect of the script action `49,0`. Like if the Team failed.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=14003,0
```

### `16000-16999` Flow Control

#### `16000` Start a Timed Jump to the Same Line

- When the timer ends the current script action ends and start again the same script action. The timer jump repeats again (infinite loop) until is stopped with action `16002` or the team is destroyed.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=16000,n           ; integer n=0, in ingame seconds
```

#### `16001` Start a Timed Jump to the Next Line

- When the timer ends the current script action ends and start the next one in the script type list.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=16001,n           ; integer n=0, in ingame seconds
```

#### `16002` Stop the Timed Jumps

- If the Timed Jumps were activated this action stop the process.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=16002,0
```

#### `16003` Randomly Skip Next Action

- When executed this action picks a random value between 1 and 100. If the value is equal or below the second parameter then the next action will be skipped. If the second parameter is 0 means that the next action will never be skipped and 100 means thay always will be skipped.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=16003,n           ; where 0 > n <= 100
```

#### `16004` Pick a Random Script

- When executed this action picks a random Script Type and replaces the current script by the new picked one. The second parameter is a 0-based index from the new section `AIScriptsList` explained below.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=16004,n
```

The second parameter is a 0-based index for the `AIScriptsList` section that specifies the list of possible `ScriptTypes` that can be evaluated. The new `AIScriptsList` section must be declared in `rulesmd.ini` for making this script work:

In `rulesmd.ini`:
```ini
[AIScriptsList]  ; List of ScriptType lists
0=SOMESCRIPTTYPE,SOMEOTHERSCRIPTTYPE,SAMPLESCRIPTTYPE
1=ANOTHERSCRIPTTYPE,YETANOTHERSCRIPTTYPE
; ...
```

### `16005` Jump Back To Previous Script

- Used in a Random Script picked by action 94. It can jump back to the previous script, and continue in the line after x=94,n.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=16005,0
```

### `18000-18999` Variable Manipulation

#### `18000-18023` Edit Variable
- Operate a variable's value
- The variable's value type is int16 instead of int32 in trigger actions for some reason, which means it ranges from -2^15 to 2^15-1.
- Any numbers exceeding this limit will lead to unexpected results!

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=i,n             ; where 18000 <= i <= 18023, n is made up of two parts, the low 16 bits is being used to store the variable index, the high 16 bits is being used for storing the param value.
```

#### `18024 - 18047` Edit Variable using Local Variable
- Operate a variable's value using a local variable's value
- Similar to 18000-18023, but the number to operate the value is being read from a local variable

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=i,n             ; where 18024 <= i <= 18047, n is made up of two parts, the low 16 bits is being used to store the variable index, the high 16 bits is being used for storing the local variable index.
```

#### `18000 - 18071` Edit Variable using Global Variable
- Operate a variable's value using a global variable's value
- Similar to 18000-18023, but the number to operate the value is being read from a global variable

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=i,n             ; where 18048 <= i <= 18071, n is made up of two parts, the low 16 bits is being used to store the variable index, the high 16 bits is being used for storing the global variable index.
```

### `19000-19999` Miscellanous/Uncategorized

This category is empty for now.

## Trigger Actions

### `500` Save Game
- Save the current game immediately (singleplayer game only).
- These vanilla CSF entries will be used: `TXT_SAVING_GAME`, `TXT_GAME_WAS_SAVED` and `TXT_ERROR_SAVING_GAME`.
- The save's description will look like `MapDescName - CSFText`.
- For example: `Allied Mission 25: Esther's Money - Money Stolen`.

In `mycampaign.map`:
```ini
[Actions]
...
ID=ActionCount,[Action1],500,4,[CSFKey],0,0,0,0,A,[ActionX]
...
```

### `501` Edit Variable
- Operate a variable's value
- The variable's value type is int32, which means it ranges from -2^31 to 2^31-1.
- Any numbers exceeding this limit will lead to unexpected results!

In `mycampaign.map`:
```ini
[Actions]
...
ID=ActionCount,[Action1],501,0,[VariableIndex],[Operation],[Number],[IsGlobalVariable],0,A,[ActionX]
...
```

| *Operation* | *Description*                                 |
| :---------: | :-------------------------------------------: |
| 0           | CurrentValue = Number                         |
| 1           | CurrentValue = CurrentValue + Number          |
| 2           | CurrentValue = CurrentValue - Number          |
| 3           | CurrentValue = CurrentValue * Number          |
| 4           | CurrentValue = CurrentValue / Number          |
| 5           | CurrentValue = CurrentValue % Number          |
| 6           | CurrentValue = CurrentValue leftshift Number  |
| 7           | CurrentValue = CurrentValue rightshift Number |
| 8           | CurrentValue = ~CurrentValue                  |
| 9           | CurrentValue = CurrentValue xor Number        |
| 10          | CurrentValue = CurrentValue or Number         |
| 11          | CurrentValue = CurrentValue and Number        |

### `502` Generate random number
- Generate a random integer ranged in [Min, Max] and store it in a given variable

In `mycampaign.map`:
```ini
[Actions]
...
ID=ActionCount,[Action1],502,0,[VariableIndex],[Min],[Max],[IsGlobalVariable],0,A,[ActionX]
...
```

### `503` Print variable value
- Print a variable value to the message list

In `mycampaign.map`:
```ini
[Actions]
...
ID=ActionCount,[Action1],503,[VariableIndex],0,[IsGlobalVariable],0,0,0,A,[ActionX]
...
```

### `504` Binary Operation
- Operate a variable's value with another variable's value
- Similar to 501, but the operation number is read from another variable

In `mycampaign.map`:
```ini
[Actions]
...
ID=ActionCount,[Action1],504,0,[VariableIndex],[Operation],[VariableForOperationIndex],[IsGlobalVariable],[IsOperationGlobalVariable],A,[ActionX]
...
```

`Operation` can be looked up at action `501`

### `505` Fire Super Weapon at specified location

- **Use with caution**
- Launch a Super Weapon from [SuperWeaponTypes] list at a specified location.
- `HouseIndex` can take various values:

| *House Index* | *Description*                             |
| :-----------: | :---------------------------------------: |
| >= 0          | The index of the current House in the map |
| 4475-4482     | Like in the index range 0-7               |
| -1            | Pick a random House that isn't Neutral    |
| -2            | Pick the first Neutral House              |
| -3            | Pick a random Human Player                |

- Coordinates X & Y can take possitive values or -1, in which case these values can take a random value from the visible map area.

In `mycampaign.map`:
```ini
[Actions]
...
ID=ActionCount,[Action1],505,0,0,[SuperWeaponTypesIndex],[HouseIndex],[CoordinateX],[CoordinateY],A,[ActionX]
...
```

### `506` Fire Super Weapon at specified Waypoint

- **Use with caution**
- Launch a Super Weapon from [SuperWeaponTypes] list at a specified waypoint.

In `mycampaign.map`:
```ini
[Actions]
...
ID=ActionCount,[Action1],506,0,0,[SuperWeaponTypesIndex],[HouseIndex],[WaypointIndex],0,A,[ActionX]
...
```

### `510` Toggle MCV redeployablility

- Force MCV's redeployablility by setting the third parameter.

In `mycampaign.map`:
```ini
[Actions]
...
ID=ActionCount,[Action1],510,0,0,[MCVRedeploy],0,0,0,A,[ActionX]
...
```

## Trigger events

### `500-511` Variable comparation
- Compares the variable's value with given number

In `mycampaign.map`:
```ini
[Events]
...
ID=EventCount,[Event1],[EVENTID],2,[VariableIndex],[Param],[EventX]
...
```

| *Event ID* | *Description*          | *Global* |
| :--------: | :--------------------: | :------: |
| 500        | CurrentValue > Number  | No       |
| 501        | CurrentValue < Number  | No       |
| 502        | CurrentValue = Number  | No       |
| 503        | CurrentValue >= Number | No       |
| 504        | CurrentValue <= Number | No       |
| 505        | CurrentValue & Number  | No       |
| 506        | CurrentValue > Number  | Yes      |
| 507        | CurrentValue < Number  | Yes      |
| 508        | CurrentValue = Number  | Yes      |
| 509        | CurrentValue >= Number | Yes      |
| 510        | CurrentValue <= Number | Yes      |
| 511        | CurrentValue & Number  | Yes      |

### `512-523` Variable comparation with local variable
- Compares the variable's value with given local variable value

In `mycampaign.map`:
```ini
[Events]
...
ID=EventCount,[Event1],[EVENTID],2,[VariableIndex],[LocalVariableIndex],[EventX]
...
```

| *Event ID* | *Description*                      | *Global* |
| :--------: | :--------------------------------: | :------: |
| 512        | CurrentValue > LocalVariableValue  | No       |
| 513        | CurrentValue < LocalVariableValue  | No       |
| 514        | CurrentValue = LocalVariableValue  | No       |
| 515        | CurrentValue >= LocalVariableValue | No       |
| 516        | CurrentValue <= LocalVariableValue | No       |
| 517        | CurrentValue & LocalVariableValue  | No       |
| 518        | CurrentValue > LocalVariableValue  | Yes      |
| 519        | CurrentValue < LocalVariableValue  | Yes      |
| 520        | CurrentValue = LocalVariableValue  | Yes      |
| 521        | CurrentValue >= LocalVariableValue | Yes      |
| 522        | CurrentValue <= LocalVariableValue | Yes      |
| 523        | CurrentValue & LocalVariableValue  | Yes      |

### `524-535` Variable comparation with global variable
- Compares the variable's value with given global variable value

In `mycampaign.map`:
```ini
[Events]
...
ID=EventCount,[Event1],[EVENTID],2,[VariableIndex],[GlobalVariableIndex],[EventX]
...
```

| *Event ID* | *Description*                       | *Global* |
| :--------: | :---------------------------------: | :------: |
| 524        | CurrentValue > GlobalVariableValue  | No       |
| 525        | CurrentValue < GlobalVariableValue  | No       |
| 526        | CurrentValue = GlobalVariableValue  | No       |
| 527        | CurrentValue >= GlobalVariableValue | No       |
| 528        | CurrentValue <= GlobalVariableValue | No       |
| 529        | CurrentValue & GlobalVariableValue  | No       |
| 530        | CurrentValue > GlobalVariableValue  | Yes      |
| 531        | CurrentValue < GlobalVariableValue  | Yes      |
| 532        | CurrentValue = GlobalVariableValue  | Yes      |
| 533        | CurrentValue >= GlobalVariableValue | Yes      |
| 534        | CurrentValue <= GlobalVariableValue | Yes      |
| 535        | CurrentValue & GlobalVariableValue  | Yes      |

### `600` The shield of the attached object is broken

In `mycampaign.map`:
```ini
[Events]
...
ID=EventCount,...,600,2,0,0,...
...
```

### `601-602` House owns/doesn't own Techno Type
- 601: Springs when specified house owns at least 1 instance of set TechnoType.
- 602: Springs when specified house doesn't own a single instance of set TechnoType.
  - Multiplayer houses (indices 4475 through 4482) are supported.

```{note}
These events, as opposed to [events 81 & 82 from Ares](https://ares-developers.github.io/Ares-docs/new/triggerevents.html#house-owns-techno-type-81-82), take house as a parameter instead of using the trigger owner.
```

In `mycampaign.map`:
```ini
[Events]
...
ID=EventCount,...,[EVENTID],2,[HouseIndex],[TechnoType],...
...
```

### `604-605` Checking if a specific Techno enters in a cell
- 604: Checks if the techno that entered in the cell has the same ID specified in the event.
- 605: Checks if the techno that entered in the cell appears in the selected list in `AITargetTypes`.
- `HouseIndex` can be customized to focus in a specified house.

In `mycampaign.map`:
```ini
[Events]
...
ID=EventCount,...,604,2,[HouseIndex],[TechnoType],...
ID=EventCount,...,605,2,[HouseIndex],[AITargetTypes index#],...
...
```

| *House Index* | *Description*                              |
| :-----------: | :----------------------------------------: |
| >= 0          | The index of the current House in the map  |
| -1            | This value is ignored (any house is valid) |
| -2            | Pick the owner of the map trigger          |
