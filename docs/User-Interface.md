# User Interface

This page lists all user interface additions, changes, fixes that are implemented in Phobos.

## Bugfixes and miscellanous

- Enabled ability to load full-color non-paletted PCX graphics of any bitness. This applies to every single PCX file that is loaded, including the Ares-supported PCX files.
- You can specify custom `gamemd.exe` icon via `-icon` command line argument followed by absolute or relative path to an `*.ico` file (f. ex. `gamemd.exe -icon Resources/clienticon.ico`).
- Fixed `Blowfish.dll`-caused error `***FATAL*** String Manager failed to initialize properly`, which occurred if `Blowfish.dll` could not be registered in the OS, for example, it happened when the player did not have administrator rights. With Phobos, if the game did not find a registered file in the system, it will no longer try to register this file, but will load it bypassing registration.
- Fixed non-IME keyboard input to be working correctly for languages / keyboard layouts that use character ranges other than Basic Latin and Latin-1 Supplement (font support required).

```{note}
You can use the improved vanilla font which can be found on [Phobos supplementaries repo](https://github.com/Phobos-developers/PhobosSupplementaries) which has way more Unicode character coverage than the default one.
```

## Audio

- You can now specify which soundtrack themes would play on win or lose.

In `rulesmd.ini`:
```ini
[SOMESIDE]             ; Side
IngameScore.WinTheme=  ; Soundtrack theme ID
IngameScore.LoseTheme= ; Soundtrack theme ID
```

## Battle screen UI/UX

### Hide health bars

![image](_static/images/healthbar.hide-01.png)
*Health bars hidden in [CnC: Final War](https://www.moddb.com/mods/cncfinalwar)*

- Health bar display can now be turned off as needed, hiding both the health bar box and health pips.

In `rulesmd.ini`:
```ini
[SOMENAME]            ; TechnoType
HealthBar.Hide=false  ; boolean
```

### Low priority for box selection

![smartvesters](_static/images/lowpriority-01.gif)
*Harvesters not selected together with battle units in [Rise of the East](https://www.moddb.com/mods/riseoftheeast) mod*

- You can now set lower priority for an ingame object (currently has effect on units mostly), which means it will be excluded from box selection if there's at least one normal priority unit in the box. Otherwise it would be selected as normal. Works with box+type selecting (type select hotkey + drag) and regular box selecting. Box shift-selection adds low-priority units to the group if there are no normal priority units among the appended ones.

In `rulesmd.ini`:
```ini
[SOMETECHNO]                ; TechnoType
LowSelectionPriority=false  ; boolean
```

- This behavior is designed to be toggleable by users. For now you can only do that externally via client or manually.

In `RA2MD.ini`:
```ini
[Phobos]
PrioritySelectionFiltering=true  ; boolean
```

### Placement preview

![placepreview](_static/images/placepreview.png)
*Building placement preview using 50% translucency in [Rise of the East](https://www.moddb.com/mods/riseoftheeast)*

- Building previews can now be enabled when placing a building for construction. This can be enabled on a global basis with `BuildingPlacementPreview.DefaultTranslucentLevel` and then further customized for each building with `PlacementPreview.TranslucentLevel`.
- The building placement grid *(place.shp)* translucency setting can be adjusted via `BuildingPlacementGrid.TranslucentLevel`.
- If using the building's appropriate `Buildup` is not desired, customizations allow for you to choose the exact SHP and frame you'd prefer to show as preview instead through `PlacementPreview.Shape` and `PlacementPreview.ShapeFrame`
- `PlacementPreview.ShapeFrame=` tag defaults to building's artmd.ini `Buildup` entry's last non-shadow frame. If there is no 'Buildup' specified it will instead attempt to default to the building's normal first frame (animation frames and bibs are not included in this preview).

In `rulesmd.ini`:
```ini
[AUDIOVISUAL]
BuildingPlacementGrid.TranslucentLevel=0            ; integer, 0=0% 1=25% 2=50% 3=75%
BuildingPlacementPreview.DefaultTranslucentLevel=3  ; integer, 0=0% 1=25% 2=50% 3=75%

[BUILDINGTYPE]
PlacementPreview.Show=                              ; boolean, defaults to [Phobos]->ShowBuildingPlacementPreview
PlacementPreview.Shape=                             ; filename - including the .shp extension. If not set uses building's artmd.ini Buildup SHP (based on Building's Image)
PlacementPreview.ShapeFrame=                        ; integer, zero-based frame index used for displaying the preview
PlacementPreview.Offset=0,-15,1                     ; integer, expressed in X,Y,Z used to alter position preview
PlacementPreview.Remap=true                         ; boolean, does this preview use player remap colors
PlacementPreview.Palette=                           ; filename - including the .pal extension. This option is not used if PlacementPreview.Remap is set to true
PlacementPreview.TranslucentLevel=                  ; integer, defaults to [AudioVisual]->BuildingPlacementPreview.DefaultTranslucentLevel
```

- This behavior is designed to be toggleable by users. For now you can only do that externally via client or manually.

In `ra2md.ini`:
```ini
[Phobos]
ShowBuildingPlacementPreview=false  ; boolean
```

## Hotkey Commands

### `[ ]` Dump Object Info

- Writes currently hovered or last selected object info in log and shows a message. See [this](Miscellanous.md#dump-object-info) for details.
- If need localization, just add `TXT_DUMP_OBJECT_INFO` and `TXT_DUMP_OBJECT_INFO_DESC` into your `.csf` file.

### `[ ]` Next Idle Harvester

- Selects and centers the camera on the next TechnoType that is counted via the [harvester counter](#harvester-counter) and is currently idle.
- If need localization, just add `TXT_NEXT_IDLE_HARVESTER` and `TXT_NEXT_IDLE_HARVESTER_DESC` into your `.csf` file.

### `[ ]` Quicksave

- Save the current singleplayer game.
- If need localization, just add `TXT_QUICKSAVE`, `TXT_QUICKSAVE_DESC`, `TXT_QUICKSAVE_SUFFIX` and `MSG:NotAvailableInMultiplayer` into your `.csf` file.
    - These vanilla CSF entries will be used: `TXT_SAVING_GAME`, `TXT_GAME_WAS_SAVED` and `TXT_ERROR_SAVING_GAME`.
    - The save should be looks like `Allied Mission 25: Esther's Money - QuickSaved`


## Loading screen

- PCX files can now be used as loadscreen images.
  - You can specify custom loadscreen with Ares tag `File.LoadScreen`.
  - Campaign loading screen (`missionmd.ini->[LS800BkgdName]`) can also use PCX image.
- The loadscreen size can now be different from the default `800x600` one; if the image is bigger than the screen it's centered and cropped.
  - This feature works in conjunction with CnCNet5 spawner DLL which resizes loadscreen window to actual monitor size and places the image in center. If there's no CnCNet5 spawner loaded, the window resolution will be always `800x600`.
  - Same applies to campaign loading screen (`missionmd.ini->[LS800BkgdName]`).
- You can now disable hardcoded black dots that YR engine shows over empty spawn locations, which allows to use prettier and more correctly placed markers that are produced by Map Renderer instead.

In `uimd.ini`:
```ini
[LoadingScreen]
DisableEmptySpawnPositions=false  ; boolean
```

## Sidebar / Battle UI

### Cameo Sorting

- You can now specify Cameo Priority for any TechnoType/SuperWeaponType. Vanilla sorting rules are [here](https://modenc.renegadeprojects.com/Cameo_Sorting).
  - The Cameo Priority is checked just before evevything vanilla. Greater `CameoPriority` wins.

In `rulesmd.ini`:
```ini
[SOMENAME]             ; TechnoType / SuperWeaponType
CameoPriority=0        ; integer
```

### Custom Missing Cameo (`XXICON.SHP`)

- You can now specify any SHP/PCX file as XXICON.SHP for missing cameo.

In `rulesmd.ini`:
```ini
[AudioVisual]
MissingCameo=XXICON.SHP  ; filename - including the .shp/.pcx extension
```

### Harvester counter

![image](_static/images/harvestercounter-01.gif)
*Harvester Counter in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)*

- An additional counter for your active/total harvesters can be added near the credits indicator.
- You can specify which TechnoType should be counted as a Harvester with `Harvester.Counted`. If not set, the techno with `Harvester=yes` or `Enslaves=SOMESLAVE` will be counted.
  - Can be set to true on buildings with `ProduceCashAmount` to count them as active 'harvesters' while generating credits.
- The counter is displayed with the format of `Label(Active Harvesters)/(Total Harvesters)`. The label is `⛏ U+26CF` by default.
- You can adjust counter position by `Sidebar.HarvesterCounter.Offset`, negative means left/up, positive means right/down.
- By setting `HarvesterCounter.ConditionYellow` and `HarvesterCounter.ConditionRed`, the game will warn player by changing the color of counter whenever the active percentage of harvesters less than or equals to them, like HP changing with `ConditionYellow` and `ConditionRed`.

In `uimd.ini`:
```ini
[Sidebar]
HarvesterCounter.Show=false           ; boolean
HarvesterCounter.Label=<none>         ; CSF entry key
HarvesterCounter.ConditionYellow=99%  ; floating point value, percents
HarvesterCounter.ConditionRed=50%     ; floating point value, percents
```

In `rulesmd.ini`:
```ini
[SOMETECHNO]                                    ; TechnoType
Harvester.Counted=                              ; boolean

[SOMESIDE]                                      ; Side
Sidebar.HarvesterCounter.Offset=0,0             ; X,Y, pixels relative to default
Sidebar.HarvesterCounter.ColorYellow=255,255,0  ; integer - R,G,B
Sidebar.HarvesterCounter.ColorRed=255,0,0       ; integer - R,G,B
```

```{note}
If you use the vanilla font in your mod, you can use the improved font (v4 and higher; can be found on [Phobos supplementaries repo](https://github.com/Phobos-developers/PhobosSupplementaries)) which among everything already includes the mentioned icons. Otherwise you'd need to draw them yourself using [WWFontEditor](http://nyerguds.arsaneus-design.com/project_stuff/2016/WWFontEditor/release/?C=M;O=D), for example.
```

### Power delta counter

![image](_static/images/powerdelta-01.gif)
*Power delta Counter in [Assault Amerika](https://www.moddb.com/mods/assault-amerika)*

- An additional counter for your power delta (surplus) can be added near the credits indicator.
- The counter is displayed with the format of `Label(sign)(Power Delta)`. The label is PowerLabel used in tooltips (by default `⚡ U+26A1`).
- You can adjust counter position by `Sidebar.PowerDelta.Offset`, negative means left/up, positive means right/down.
- You can adjust counter text alignment by `Sidebar.PowerDelta.Align`, acceptable values are left, right, center/centre.
- By setting `PowerDelta.ConditionYellow` and `PowerDelta.ConditionRed`, the game will warn player by changing the color of counter whenever the percentage of used power exceeds the value (i.e. when drain to output ratio is above 100%, the counter will turn red).
- The exception for this rule is when both power output and drain are 0 - in this case the counter will default to yellow.

In `uimd.ini`:
```ini
[Sidebar]
PowerDelta.Show=false           ; boolean
PowerDelta.ConditionYellow=75%  ; floating point value, percents
PowerDelta.ConditionRed=100%    ; floating point value, percents
```

In `rulesmd.ini`:
```ini
[SOMESIDE]                                ; Side
Sidebar.PowerDelta.Offset=0,0             ; X,Y, pixels relative to default
Sidebar.PowerDelta.ColorGreen=0,255,0     ; integer - R,G,B
Sidebar.PowerDelta.ColorYellow=255,255,0  ; integer - R,G,B
Sidebar.PowerDelta.ColorRed=255,0,0       ; integer - R,G,B
Sidebar.PowerDelta.Align=left             ; Alignment enumeration - left|center|centre|right
```

```{note}
If you use the vanilla font in your mod, you can use the improved font (v4 and higher; can be found on [Phobos supplementaries repo](https://github.com/Phobos-developers/PhobosSupplementaries)) which among everything already includes the mentioned icons. Otherwise you'd need to draw them yourself using [WWFontEditor](http://nyerguds.arsaneus-design.com/project_stuff/2016/WWFontEditor/release/?C=M;O=D), for example.
```

### Producing Progress

![image](_static/images/producing-progress-01.gif)
*Producing Progress bars in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)*

- You can now know your factories' status via sidebar!
- You need to draw your own assets (`tab0xpp.shp`, x is replaced by 0-3) and put them into `sidec0x.mix`.

In `uimd.ini`:
```ini
[Sidebar]
ProducingProgress.Show=false  ; boolean
```

In `rulesmd.ini`:
```ini
[SOMESIDE]                            ; Side
Sidebar.ProducingProgress.Offset=0,0  ; X,Y, pixels relative to default
```

### Specify Sidebar style

- It's now possible to switch hardcoded sidebar button coords to use GDI sidebar coords by setting `Sidebar.GDIPosition`. Defaults to true for first side, false for all others.

In `rulesmd.ini`:
```ini
[SOMESIDE]             ; Side
Sidebar.GDIPositions=  ; boolean
```

## Tooltips

![image](_static/images/tooltips-01.png)
*Extended tooltips used in [CnC: Final War](https://www.moddb.com/mods/cncfinalwar)*

- Sidebar tooltips can now display extended information about the TechnoType/SWType when hovered over it's cameo. In addition the low character limit is lifted when the feature is enabled via the corresponding tag, allowing for 1024 character long tooltips.
- TechnoType's tooltip would display it's name, cost, power, build time and description (when applicable).
- SWType's tooltip would display it's name, cost,  and recharge time (when applicable).
- Extended tooltips don't use `TXT_MONEY_FORMAT_1` and `TXT_MONEY_FORMAT_2`. Instead you can specify cost, power and time labels (displayed before correspoding values) with the corresponding tags. Characters `$ U+0024`, `⚡ U+26A1` and `⌚ U+231A` are used by default.
- Fixed a bug when switching build queue tabs via QWER didn't make tooltips disappear as they should, resulting in stuck tooltips.
- The tooltips can now go over the sidebar bounds to accommodate for longer contents. You can control maximum text width with a new tag (paddings are excluded from the number you specify).

```{note}
Same as with harvester counter, you can download the improved font (v4 and higher; can be found on [Phobos supplementaries repo](https://github.com/Phobos-developers/PhobosSupplementaries)) or draw your own icons.
```

In `uimd.ini`:
```ini
[ToolTips]
ExtendedToolTips=false  ; boolean
CostLabel=<none>        ; CSF entry key
PowerLabel=<none>       ; CSF entry key
TimeLabel=<none>        ; CSF entry key
MaxWidth=0              ; integer, pixels
```
In `rulesmd.ini`:
```ini
[SOMENAME]            ; TechnoType or SWType
UIDescription=<none>  ; CSF entry key
```

- The descriptions are designed to be toggleable by users. For now you can only do that externally via client or manually.

In `RA2MD.ini`:
```ini
[Phobos]
ToolTipDescriptions=true  ; boolean
```
