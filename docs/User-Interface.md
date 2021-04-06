# User Interface

This page lists all user interface additions, changes, fixes that are implemented in Phobos.

## Bugfixes and miscellanous

- Enabled ability to load full-color non-paletted PCX graphics of any bitness. This applies to every single PCX file that is loaded, including the Ares-supported PCX files.
- You can specify custom `gamemd.exe` icon via `-icon` command line argument followed by absolute or relative path to an `*.ico` file (f. ex. `gamemd.exe -icon Resources/clienticon.ico`).
- Fixed `Blowfish.dll`-caused error `***FATAL*** String Manager failed to initilaize properly`, which occurred if `Blowfish.dll` could not be registered in the OS, for example, it happened when the player did not have administrator rights. With Phobos, if the game did not find a registered file in the system, it will no longer try to register this file, but will load it bypassing registration. 

## Audio

- You can now specify which soundtrack themes would play on win or lose.

In `rulesmd.ini`:
```ini
[SOMESIDE]             ; Side
IngameScore.WinTheme=  ; soundtrack theme ID
IngameScore.LoseTheme= ; soundtrack theme ID
```

## Hotkey Commands

### `[ ]` Next Idle Harvester

- Selects and centers the camera on the next TechnoType that is counted via the [harvester counter](#harvester-counter) and is currently idle.
- If need localization, just add `TXT_NEXT_IDLE_HARVESTER` and `TXT_NEXT_IDLE_HARVESTER_DESC` into your `.csf` file.

### `[ ]` Dump Object Info

- Writes currently hovered or last selected object info in log and shows a message. See [this](Miscellanous.md#dump-object-info) for details.
- If need localization, just add `TXT_DUMP_OBJECT_INFO` and `TXT_DUMP_OBJECT_INFO_DESC` into your `.csf` file.

## Battle screen UI/UX

### Low priority for box selection

![smartvesters](_static/images/lowpriority-01.gif)  
*Harvesters not selected together with battle units in [Rise of the East](https://www.moddb.com/mods/riseoftheeast) mod*

- You can now set lower priority for an ingame object (currently has effect on units mostly), which means it will be excluded from box selection if there's at least one normal priority unit in the box. Otherwise it would be selected as normal. Works with box+type selecting (type select hotkey + drag) and regular box selecting. Box shift-selection adds low-priority units to the group if there are no normal priority units among the appended ones.

In `rulesmd.ini`:
```ini
[SOMETECHNO]            ; TechnoType
LowSelectionPriority=no ; boolean
```

- This behavior is designed to be toggleable by users. For now you can only do that externally via client or manually.

In `RA2MD.ini`:
```ini
[Phobos]
PrioritySelectionFiltering=yes ; bool
```

### Hide health bars

![image](_static/images/healthbar.hide-01.png)  
*Health bars hidden in [CnC: Final War](https://www.moddb.com/mods/cncfinalwar)*

- Health bar display can now be turned off as needed, hiding both the health bar box and health pips.

In `rulesmd.ini`:
```ini
[SOMENAME]         ; TechnoType
HealthBar.Hide=no  ; boolean
```

## Loading screen

- PCX files can now be used as loadscreen images.
  - You can specify custom loadscreen with Ares tag `File.LoadScreen`.
- The loadscreen size can now be different from the default `800x600` one; if the image is bigger than the screen it's centered and cropped.
  - This feature works in conjunction with CnCNet5 spawner DLL which resizes loadscreen window to actual monitor size and places the image in center. If there's no CnCNet5 spawner loaded, the window resolution will be always `800x600`.
- You can now disable hardcoded black dots that YR engine shows over empty spawn locations, which allows to use prettier and more correctly placed markers that are produced by Map Renderer instead.

In `uimd.ini`:
```ini
[LoadingScreen]
DisableEmptySpawnPositions=no ; boolean
```

## Sidebar / Battle UI

### Specify Sidebar style

- It's now possible to switch hardcoded sidebar button coords to use GDI sidebar coords.

In `rulesmd.ini`:
```ini
[SOMESIDE]            ; Side
Sidebar.GDIPositions= ; boolean
                      ; default values are:
                      ; yes for the first side
                      ; no for others
```

### Harvester counter

![image](_static/images/harvestercounter-01.gif)  
*Harvester Counter in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)*

- An additional counter for your active/total harvesters can be added near the credits indicator.
- You can specify which TechnoType shoule be counted as a Harvester. If not set, the techno with `Harvester=yes` or `Enslaves=SOMESLAVE` will be counted.
- The counter is displayed with the format of `Label(Active Harvesters)/(Total Harvesters)`. The label is `⛏ U+26CF` by default.
- You can adjust counter position by `Sidebar.HarvesterCounter.Offset`, negative means left/up, positive means right/down.
- By setting `HarvesterCounter.ConditionYellow` and `HarvesterCounter.ConditionRed`, the game will warn player by changing the color of counter whenever the active percentage of harvesters less than or equals to them, like HP changing with `ConditionYellow` and `ConditionRed`.

In `uimd.ini`:
```ini
[Sidebar]
HarvesterCounter.Show=no                 ; boolean
HarvesterCounter.Label=<none>            ; CSF entry key
HarvesterCounter.ConditionYellow=99%     ; double, percentage
HarvesterCounter.ConditionRed=50%        ; double, percentage
```

In `rulesmd.ini`:
```ini
[SOMETECHNO]        ; TechnoType
Harvester.Counted=  ; boolean
                    ; if set yes to a BuildingType like Oil Derricks
                    ; when producing cash, it will be counted as active

[SOMESIDE]                                     ; Side
Sidebar.HarvesterCounter.Offset=0,0            ; X,Y, pixels relative to default
Sidebar.HarvesterCounter.ColorYellow=255,255,0 ; R,G,B
Sidebar.HarvesterCounter.ColorRed=255,0,0      ; R,G,B
```

:::{note}
If you use the vanilla font in your mod, you can use {download}`the improved font <_static/files/ImprovedFont-v4.zip>` (v4 and higher) which among everything already includes the mentioned icons. Otherwise you'd need to draw them yourself using [WWFontEditor](http://nyerguds.arsaneus-design.com/project_stuff/2016/WWFontEditor/release/?C=M;O=D), for example.
:::

## Tooltips

![image](_static/images/tooltips-01.png)  
*Extended tooltips used in [CnC: Final War](https://www.moddb.com/mods/cncfinalwar)*

- Sidebar tooltips can now display extended information about the TechnoType/SWType when hovered over it's cameo. In addition the low character limit is lifted when the feature is enabled via the corresponding tag, allowing for 1024 character long tooltips.
- TechnoType's tooltip would display it's name, cost, power and description (when applicable).
- SWType's tooltip would display it's name, cost,  and recharge time (when applicable).
- Extended tooltips don't use `TXT_MONEY_FORMAT_1` and `TXT_MONEY_FORMAT_2`. Instead you can specify cost, power and time labels (displayed before correspoding values) with the corresponding tags. Characters `$ U+0024`, `⚡ U+26A1` and `⌚ U+231A` are used by default.
- Fixed a bug when switching build queue tabs via QWER didn't make tooltips disappear as they should, resulting in stuck tooltips.
- The tooltips can now go over the sidebar bounds to accomodate for longer contents. You can control maximum text width with a new tag (paddings are excluded from the number you specify).

:::{note}
Same as with harvester counter, you can download {download}`the improved font <_static/files/ImprovedFont-v4.zip>` (v3 and higher) or draw your own icons.
:::

In `uimd.ini`:
```ini
[ToolTips]
ExtendedToolTips=no ; boolean
CostLabel=<none>    ; CSF entry key
PowerLabel=<none>   ; CSF entry key
TimeLabel=<none>    ; CSF entry key
MaxWidth=0          ; integer, pixels
```
In `rulesmd.ini`:
```ini
[SOMENAME]           ; TechnoType or SWType
UIDescription=<none> ; CSF entry key
```

- The descriptions are designed to be toggleable by users. For now you can only do that externally via client or manually.

In `RA2MD.ini`:
```ini
[Phobos]
ToolTipDescriptions=yes ; bool
```
