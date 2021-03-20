# User Interface

This page lists all user interface additions, changes, fixes that are implemented in Phobos.

## Audio

- You can now specify which soundtrack themes would play on win or lose.

In `rulesmd.ini`:
```ini
[SOMESIDE]             ; Side
IngameScore.WinTheme=  ; soundtrack theme ID
IngameScore.LoseTheme= ; soundtrack theme ID
```

## Bugfixes and miscellanous

- Enabled ability to load full-color non-paletted PCX graphics of any bitness. This applies to every single PCX file that is loaded, including the Ares-supported PCX files.
- You can specify custom `gamemd.exe` icon via `-icon` command line argument followed by absolute or relative path to an *.ico file (f. ex. `gamemd.exe -icon Resources/clienticon.ico`).

## Battle screen UI/UX

### Low priority for box selection

![smartvesters](https://user-images.githubusercontent.com/17500545/107152249-8e065c80-696f-11eb-994d-93cb37a8d125.gif)  
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

![](https://cdn.discordapp.com/attachments/773636942775582720/800924566808428564/healthbar.hide.png)  
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

- It's now possible to switch hardcoded sidebar button coords to use GDI sidebar coords.

In `rulesmd.ini`:
```ini
[SOMESIDE]            ; Side
Sidebar.GDIPositions= ; boolean
                      ; default values are:
                      ; yes for the first side
                      ; no for others
```

## Tooltips

![image](docs/_static/images/tooltips-01.png)
*Extended tooltips used in [CnC: Final War](https://www.moddb.com/mods/cncfinalwar)*

- Sidebar tooltips can now display extended information about the TechnoType/SWType when hovered over it's cameo. In addition the low character limit is lifted when the feature is enabled via the corresponding tag, allowing for 1024 character long tooltips.
- TechnoType's tooltip would display it's name, cost, power and description (when applicable).
- SWType's tooltip would display it's name, cost,  and recharge time (when applicable).
- Extended tooltips don't use `TXT_MONEY_FORMAT_1` and `TXT_MONEY_FORMAT_2`. Instead you can specify cost, power and time labels (displayed before correspoding values) with the corresponding tags. Characters `$ U+0024`, `⚡ U+26A1` and `⌚ U+231A` are used by default.
- Fixed a bug when switching build queue tabs via QWER didn't make tooltips disappear as they should, resulting in stuck tooltips.
- The tooltips can now go over the sidebar bounds to accomodate for longer contents. You can control maximum text width with a new tag (paddings are excluded from the number you specify).

:::{note}
If you use the vanilla font in your mod, you can use ![the improved font](docs/_static/files/ImprovedFont-v3.zip) (v3 and higher) which among everything already includes the mentioned icons. Otherwise you'd need to draw them yourself using [WWFontEditor](http://nyerguds.arsaneus-design.com/project_stuff/2016/WWFontEditor/release/?C=M;O=D), for example.
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
