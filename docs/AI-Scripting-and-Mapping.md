# AI Scripting and Mapping

This page describes all AI scripting and mapping related additions and changes introduced by Phobos.

## Bugfixes and miscellanous

- Script action `Move to cell` now obeys YR cell calculation now. Using `1000 * Y + X` as its cell value. (was `128 * Y + X` as it's a RA1 leftover)
- The game now can reads waypoints ranges in [0, 2147483647]. (was [0,701])
- Map trigger action `125 Build At...` can now play buildup anim optionally (needs [following changes to `fadata.ini`](Whats-New.md#for-map-editor-final-alert-2).
- Both Global Variables (`VariableNames` in `rulesmd.ini`) and Local Variables (`VariableNames` in map) are now unlimited.
- Script action `Deploy` now has vehicles with `DeploysInto` searching for free space to deploy at if failing to do so at initial location, instead of simply getting stuck.

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

### `74-81` Generic Target Type Attack Action

- These Actions instruct the TeamType to use the TaskForce to approach and attack the target specified by the second parameter which is an index of a generic pre-defined group. Look at the tables below for the possible Actions (first parameter value) and Arguments (the second parameter value).
  - For threat-based attack actions `TargetSpecialThreatCoefficientDefault` and `EnemyHouseThreatBonus` tags from `rulesmd.ini` are accounted.
  - All Aircrafts that attack other air units will end the script. This behavior is intentional because without it aircrafts had some bugs that weren't fixable at the time of developing the feature.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=i,n             ; where 74 <= i <= 81
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

- The following values are the *Target Type#* which can be used as second parameter of the new attack script actions:

| *Value* | *Target Type*     | *Description*                                 |
| :-----: | :---------------: | :-------------------------------------------: |
| 1         | Anything          | Any enemy `VehicleTypes`, `AircraftTypes`, `InfantryTypes` and `BuildingTypes` |
| 2         | Structures        | Any enemy `BuildingTypes` without `Artillary=yes`, `TickTank=yes`, `ICBMLauncher=yes` or `SensorArray=yes` |
| 3         | Ore Miners        | Any enemy `VehicleTypes` with `Harvester=yes` or `ResourceGatherer=yes`, `BuildingTypes` with `ResourceGatherer=yes` |
| 4         | Infantry          | Any enemy `InfantryTypes` |
| 5         | Vehicles          | Any enemy `VehicleTypes`, `AircraftTypes`, `BuildingTypes` with `Artillary=yes`, `TickTank=yes`, `ICBMLauncher=yes` & `SensorArray=yes` |
| 6         | Factories         | Any enemy `BuildingTypes` with a Factory= setting |
| 7         | Base Defenses     | Any enemy `BuildingTypes` with `IsBaseDefense=yes` |
| 8         | House Threats     | Any object that targets anything of the Team's House or any enemy that is near to the Team Leader |
| 9         | Power Plants      | Any enemy `BuildingTypes` with positive `Power=` values |
| 10        | Occupied          | Any `BuildingTypes` with garrisoned infantry |
| 11        | Tech Buildings    | Any `BuildingTypes` with `Unsellable=yes`, `Capturable=yes`, negative `TechLevel=` values or appears in `[AI]>NeutralTechBuildings=` list |
| 12        |	Refinery        | Any enemy `BuildingTypes` with `Refinery=yes` or `ResourceGatherer=yes`, `VehicleTypes` with `ResourceGatherer=yes` & `Harvester=no` (i.e. Slave Miner) |
| 13        | Mind Controller   | Anything `VehicleTypes`, `AircraftTypes`, `InfantryTypes` and `BuildingTypes` with `MindControl=yes` in the weapons Warheads |
| 14        | Air Units         | Any enemy `AircraftTypes`, flying `VehicleTypes` or `InfantryTypes` |
| 15        | Naval             | Any enemy `BuildingTypes` and `VehicleTypes` with a `Naval=yes`, any enemy `VehicleTypes`, `AircraftTypes`, `InfantryTypes` in a water cell |
| 16        | Disruptors        | Any enemy objects with positive `InhibitorRange=` values, positive `RadarJamRadius=` values, `CloakGenerator=yes` or `GapGenerator=yes` |
| 17        | Ground Vehicles   | Any enemy `VehicleTypes` without `Naval=yes`, landed `AircraftTypes`, Deployed vehicles into `BuildingTypes` |
| 18        | Economy           | Any enemy `VehicleTypes` with `Harvester=yes` or `ResourceGatherer=yes`, `BuildingTypes` with `Refinery=yes`, `ResourceGatherer=yes` or `OrePurifier=yes` |
| 19        | Infantry Factory  | Any enemy `BuildingTypes` with `Factory=InfantryType` |
| 20        | Vehicle Factory   | Any enemy `BuildingTypes` with with `Naval=no` and `Factory=UnitType` |
| 21        | Aircraft Factory  | Any enemy `BuildingTypes` with `Factory=AircraftType` |
| 22        | Radar             | Any enemy `BuildingTypes` with `Radar=yes` or `SpySat=yes` |
| 23        | Tech Lab          | Any enemy `BuildingTypes` in `[AI]>BuildTech=` list |
| 24        | Naval Factory     | Any enemy `BuildingTypes` with `Naval=yes` and `Factory=UnitType` |
| 25        | Super Weapon      | Any enemy `BuildingTypes` with `SuperWeapon=`, `SuperWeapon2=` or `SuperWeapons=` |
| 26        | Construction Yard | Any enemy `BuildingTypes` with `ConstructionYard=yes` and `Factory=BuildingType` |
| 27        | Neutrals          | Any neutral object (Civilian) |
| 28        | Generators        | Any enemy `BuildingTypes` with `CloakGenerator=yes` or `GapGenerator=yes` |
| 29        | Radar Jammer      | Any enemy objects with positive `RadarJamRadius=` values |
| 30        | Inhibitors        | Any enemy objects with positive `InhibitorRange=` values |
| 31        | Naval Units       | Any enemy `VehicleTypes` with a `Naval=yes` or any enemy `VehicleTypes`, `AircraftTypes`, `InfantryTypes` in a water cell |
| 32        | Mobile Units      | Anything `VehicleTypes`, `AircraftTypes` and `InfantryTypes` |
| 33        | Capturable        | Any `BuildingTypes` with `Capturable=yes` or any `BuildingTypes` with `BridgeRepairHut=yes` and `Repairable=yes` |
| 34        | Area Threats      | Any enemy object that is inside of the Team Leader's Guard Area |
| 35        | Vehicle & Naval Factory | Any enemy `BuildingTypes` with `Factory=UnitType` |
| 36        | Non-defensive Structures | Any enemy `BuildingTypes` with `IsBaseDefense=no` |

### `82` Decrease AI Trigger Current Weight

- When executed this decreases the current Weight of the AI Trigger.The current Weight will never surprass the Minimum Weight and Maximum Weight limits of the AI Trigger. Take note that all TeamTypes of the same AI Trigger will update sooner or later the AI Trigger Current Weight. The second parameter is a positive value. Take note that the original game only uses the first of the two Teams for calculating the AI Trigger Current Weight at the end of the Trigger life, this action ignores if the Team is the first or the second of the AI Trigger and the Current Weight is calculated when is executed the action.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=82,n
```

### `83` Increase AI Trigger Current Weight

- When executed this increases the current Weight of the AI Trigger.The current Weight will never surprass the Minimum Weight and Maximum Weight limits of the AI Trigger. Take note that all TeamTypes of the same AI Trigger will update sooner or later the AI Trigger Current Weight. The second parameter is a positive value. Take note that the original game only uses the first of the two Teams for calculating the AI Trigger Current Weight at the end of the Trigger life, this action ignores if the Team is the first or the second of the AI Trigger and the Current Weight is calculated when is executed the action.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=83,n
```

### `84-91`, `104-105` `AITargetTypes` Attack Action

- These Actions instruct the TeamType to use the TaskForce to approach and attack the target specified by the second parameter which is an index of a modder-defined group from `AITargetTypess`. Look at the tables below for the possible Actions (first parameter value) and Arguments (the second parameter value).
  - For threat-based attack actions `TargetSpecialThreatCoefficientDefault` and `EnemyHouseThreatBonus` tags from `rulesmd.ini` are accounted.
  - All Aircrafts that attack other air units will end the script. This behavior is intentional because without it aircrafts had some bugs that weren't fixable at the time of developing the feature.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=i,n             ; where 84 <= i <= 91 or 104 <= i <= 105
```

| *Action* | *Argument*   | *Repeats* | *Target Priority* | *Description*                                 |
| :------: | :----------: | :-------: | :---------------: | :-------------------------------------------: |
84         | `AITargetTypes` index# | Yes | Closer, higher threat |  |
85         | `AITargetTypes` index# | Yes | Farther, higher threat |  |
86         | `AITargetTypes` index# | Yes | Closer |  |
87         | `AITargetTypes` index# | Yes | Farther |  |
88         | `AITargetTypes` index# | No | Closer, higher threat | Ends when a team member kill the designated target |
89         | `AITargetTypes` index# | No | Farther, higher threat | Ends when a team member kill the designated target |
90         | `AITargetTypes` index# | No | Closer | Ends when a team member kill the designated target |
91         | `AITargetTypes` index# | No | Farther | Ends when a team member kill the designated target |
104        | `AITargetTypes` index# | Yes | Closer | Picks 1 random target from the list |
105        | `AITargetTypes` index# | Yes | Farther | Picks 1 random target from the list |

- The second parameter with a 0-based index for the `AITargetTypes` section specifies the list of possible `VehicleTypes`, `AircraftTypes`, `InfantryTypes` and `BuildingTypes` that can be evaluated. The new `AITargetTypes` section must be declared in `rulesmd.ini` for making this script work:

In `rulesmd.ini`:
```ini
[AITargetTypes]  ; List of TechnoType lists
0=SOMETECHNOTYPE,SOMEOTHERTECHNOTYPE,SAMPLETECHNOTYPE
1=ANOTHERTECHNOTYPE,YETANOTHERTECHNOTYPE
; ...
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

- When executed this action picks a random Script Type and replaces the current script by the new picked. The second parameter is a 0-based index from the new section `AIScriptsList` explained below.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=94,n
```

The second parameter is a 0-based index for the `AIScriptsList` section that specifies the list of possible `ScriptTypes` that can be evaluated. The new `AIScriptsList` section must be declared in `rulesmd.ini` for making this script work:

In `rulesmd.ini`:
```ini
[AIScriptsList]  ; List of ScriptType lists
0=SOMESCRIPTTYPE,SOMEOTHERSCRIPTTYPE,SAMPLESCRIPTTYPE
1=ANOTHERSCRIPTTYPE,YETANOTHERSCRIPTTYPE
; ...
```

### `95-98`, `106-109` Moving Team to techno location

- These Actions instructs the TeamType to use the TaskForce to approach the target specified by the second parameter. Look at the tables below for the possible Actions (first parameter value).

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=i,n             ; where 95 <= i <= 98 or 106 <= i <= 109
```

| *Action* | *Argument*    | Target Owner | *Target Priority* | *Description*                                 |
| :------: | :-----------: | :----------: | :---------------: | :-------------------------------------------: |
95         | Target Type# | Enemy | Closer, higher threat |  |
96         | Target Type# | Enemy | Farther, higher threat |  |
97         | Target Type# | Friendly | Closer |  |
98         | Target Type# | Friendly | Farther |  |
99         | [AITargetType] index# | Enemy | Closer, higher threat |  |
100        | [AITargetType] index# | Enemy | Farther, higher threat |  |
101        | [AITargetType] index# | Friendly | Closer |  |
102        | [AITargetType] index# | Friendly | Farther |  |
106        | [AITargetType] index# | Enemy | Closer | Picks 1 random target from the selected list |
107        | [AITargetType] index# | Enemy | Farther | Picks 1 random target from the selected list |
108        | [AITargetType] index# | Friendly | Closer | Picks 1 random target from the selected list |
109        | [AITargetType] index# | Friendly | Farther | Picks 1 random target from the selected list |

### `103` Modify Target Distance

- By default Movement actions `95-102` & `106-109` ends when the Team Leader reaches a distance declared in rulesmd.ini called CloseEnough. When this action is  executed before the Movement actions `95-102` overwrites CloseEnough value. This action works only the first time and CloseEnough will be used again the next Movement action.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=103,n
```

### `110` Set Move Action End Mode

- Sets how the Movement actions ends and jumps to the next line. This action works only the first time and CloseEnough will be used again the next Movement action.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=110,n
```

- The possible argument values are:

| *Argument* | *Action ends when...*                       |
| :------: | :-------------------------------------------: |
0         | Team Leader reaches the minimum distance |
1         | One unit reaches the minimum distance |
2         | All team members reached the minimum distance |

### `111` Un-register Team success

- Is just the opposite effect of the script action `49,0`. Like if the Team failed.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=111,0
```

### `112` Regroup temporarily around the Team Leader

- Puts the TaskForce into Area Guard Mode for the given amount of time around the Team Leader (this unit remains almost immobile until the action ends). The default radius around the Leader is `[General] > CloseEnough` and the units will not leave that area.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=112,n
```

### `113` Randomly Skip Next Action

- When executed this action picks a random value between 1 and 100. If the value is equal or below the second parameter then the next action will be skipped. If the second parameter is 0 means that the next action will never be skipped and 100 means thay always will be skipped.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=113,n           ; where 0 > n <= 100
```

### `124` Stop the Timed Jumps

- If the Timed Jumps were activated this action stop the process.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=124,0
```

### `125` Start a Timed Jump to the next line

- When the timer ends the current script action ends and start the next one in the script type list.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=125,n           ; integer n=0, in frames
```

### `126` Start a Timed Jump to the same line

- When the timer ends the current script action ends and start again the same script action. The timer jump repeats again (infinite loop) until is stopped with action 124 or the team is destroyed.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=126,n           ; integer n=0, in frames
```

### `135` Unload from Transports

- A new unload action similar to x=8,n, but can avoid treating `VehicleTypes` with `OpenTopped=yes` and `Gunner=yes` as transports. If `TransportsReturnOnUnload` is set to `yes` in this TeamType, and transports are lost, they will return after unloading.

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=135,n
```

- The possible argument values are:

| *Argument* | *Description*                       |
| :------: | :-------------------------------------------: |
0         | Keep transports and units. |
1         | Keep transports and lose units. |
2         | Lose transports and keep units. |
3         | Lose transports and units. |
4         | Only `VehicleTypes` with largest `SizeLimit` will be regarded as transports. Keep transports and units. |
5         | Only `VehicleTypes` with largest `SizeLimit` will be regarded as transports. Keep transports and lose units. |
6         | Only `VehicleTypes` with largest `SizeLimit` will be regarded as transports. Lose transports and keep units. |
7         | Only `VehicleTypes` with largest `SizeLimit` will be regarded as transports. Lose transports and units. |


### `500 - 523` Edit Variable
- Operate a variable's value
    - The variable's value type is int16 instead of int32 in trigger actions for some reason, which means it ranges from -2^15 to 2^15-1.
        - Any numbers exceeding this limit will lead to unexpected results!

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=i,n             ; where 500 <= i <= 523, n is made up of two parts, the low 16 bits is being used to store the variable index, the high 16 bits is being used for storing the param value.
```

### `524 - 547` Edit Variable by Local Variable
- Operate a variable's value by a local variable's value
    - Similar to 500-523, but the number to operate the value is being read from a local variable

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=i,n             ; where 524 <= i <= 547, n is made up of two parts, the low 16 bits is being used to store the variable index, the high 16 bits is being used for storing the local variable index.
```

### `548 - 571` Edit Variable by Global Variable
- Operate a variable's value by a global variable's value
    - Similar to 500-523, but the number to operate the value is being read from a global variable

In `aimd.ini`:
```ini
[SOMESCRIPTTYPE]  ; ScriptType
x=i,n             ; where 548 <= i <= 571, n is made up of two parts, the low 16 bits is being used to store the variable index, the high 16 bits is being used for storing the global variable index.
```

## Trigger actions

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

| *Operation*  | *Description*                                 |
| :------: | :-------------------------------------------: |
0         | CurrentValue = Number |
1         | CurrentValue = CurrentValue + Number |
2         | CurrentValue = CurrentValue - Number |
3         | CurrentValue = CurrentValue * Number |
4         | CurrentValue = CurrentValue / Number |
5         | CurrentValue = CurrentValue % Number |
6         | CurrentValue = CurrentValue leftshift Number |
7         | CurrentValue = CurrentValue rightshift Number |
8         | CurrentValue = ~CurrentValue |
9         | CurrentValue = CurrentValue xor Number |
10         | CurrentValue = CurrentValue or Number |
11         | CurrentValue = CurrentValue and Number |

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

- Launch a Super Weapon from [SuperWeaponTypes] list at a specified location.
- `HouseIndex` can take various values:

| *House Index* | *Description*                                 |
| :-------: | :-------------------------------------------: |
| >= 0      | The index of the current House in the map |
| 4475-4482 | Like in the index range 0-7 |
| -1        | Pick a random House that isn't Neutral |
| -2        | Pick the first Neutral House |
| -3        | Pick a random Human Player |

- Coordinates X & Y can take possitive values or -1, in which case these values can take a random value from the visible map area.

In `mycampaign.map`:
```ini
[Actions]
...
ID=ActionCount,[Action1],505,0,0,[SuperWeaponTypesIndex],[HouseIndex],[CoordinateX],[CoordinateY],A,[ActionX]
...
```

### `506` Fire Super Weapon at specified Waypoint

- Launch a Super Weapon from [SuperWeaponTypes] list at a specified waypoint.

In `mycampaign.map`:
```ini
[Actions]
...
ID=ActionCount,[Action1],506,0,0,[SuperWeaponTypesIndex],[HouseIndex],[WaypointIndex],0,A,[ActionX]
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

| *Event ID*  | *Description*                                 | *Global* |
| :------: | :-------------------------------------------: | :-------: |
500         | CurrentValue > Number | No |
501         | CurrentValue < Number | No |
502         | CurrentValue = Number | No |
503         | CurrentValue >= Number | No |
504         | CurrentValue <= Number | No |
505         | CurrentValue & Number | No |
506         | CurrentValue > Number | Yes |
507         | CurrentValue < Number | Yes |
508         | CurrentValue = Number | Yes |
509         | CurrentValue >= Number | Yes |
510         | CurrentValue <= Number | Yes |
511         | CurrentValue & Number | Yes |

### `512-523` Variable comparation with local variable
- Compares the variable's value with given local variable value

In `mycampaign.map`:
```ini
[Events]
...
ID=EventCount,[Event1],[EVENTID],2,[VariableIndex],[LocalVariableIndex],[EventX]
...
```

| *Event ID*  | *Description*                                 | *Global* |
| :------: | :-------------------------------------------: | :-------: |
512         | CurrentValue > LocalVariableValue | No |
513         | CurrentValue < LocalVariableValue | No |
514         | CurrentValue = LocalVariableValue | No |
515         | CurrentValue >= LocalVariableValue | No |
516         | CurrentValue <= LocalVariableValue | No |
517         | CurrentValue & LocalVariableValue | No |
518         | CurrentValue > LocalVariableValue | Yes |
519         | CurrentValue < LocalVariableValue | Yes |
520         | CurrentValue = LocalVariableValue | Yes |
521         | CurrentValue >= LocalVariableValue | Yes |
522         | CurrentValue <= LocalVariableValue | Yes |
523         | CurrentValue & LocalVariableValue | Yes |

### `524-535` Variable comparation with global variable
- Compares the variable's value with given global variable value

In `mycampaign.map`:
```ini
[Events]
...
ID=EventCount,[Event1],[EVENTID],2,[VariableIndex],[GlobalVariableIndex],[EventX]
...
```

| *Event ID*  | *Description*                                 | *Global* |
| :------: | :-------------------------------------------: | :-------: |
524         | CurrentValue > GlobalVariableValue | No |
525         | CurrentValue < GlobalVariableValue | No |
526         | CurrentValue = GlobalVariableValue | No |
527         | CurrentValue >= GlobalVariableValue | No |
528         | CurrentValue <= GlobalVariableValue | No |
529         | CurrentValue & GlobalVariableValue | No |
530         | CurrentValue > GlobalVariableValue | Yes |
531         | CurrentValue < GlobalVariableValue | Yes |
532         | CurrentValue = GlobalVariableValue | Yes |
533         | CurrentValue >= GlobalVariableValue | Yes |
534         | CurrentValue <= GlobalVariableValue | Yes |
535         | CurrentValue & GlobalVariableValue | Yes |
