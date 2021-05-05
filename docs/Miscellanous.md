# Miscellanous

This page describes every change in Phobos that wasn't categorized into a proper category yet.

## Developer tools

### Dump Object Info

![image](_static/images/objectinfo-01.png)  
*Object info dump from [CnC: Reloaded](https://www.moddb.com/mods/cncreloaded/)*

- There's a new hotkey to dump selected/hovered object info on press. Available only when the debug tag is set.

In `rulesmd.ini`:
```ini
[GlobalControls]
DebugKeysEnabled=yes ; boolean
```

### Semantic locomotor aliases

- It's now possible to write locomotor aliases instead of their CLSIDs in the `Locomotor` tag value. Use the table below to find the needed alias for a locomotor.

| *Alias*| *CLSID*                                  |
| -----: | :--------------------------------------: |
Drive    | `{4A582741-9839-11d1-B709-00A024DDAFD1}` |
Jumpjet  | `{92612C46-F71F-11d1-AC9F-006008055BB5}` |
Hover    | `{4A582742-9839-11d1-B709-00A024DDAFD1}` |
Rocket   | `{B7B49766-E576-11d3-9BD9-00104B972FE8}` |
Tunnel   | `{4A582743-9839-11d1-B709-00A024DDAFD1}` |
Walk     | `{4A582744-9839-11d1-B709-00A024DDAFD1}` |
DropPod  | `{4A582745-9839-11d1-B709-00A024DDAFD1}` |
Fly      | `{4A582746-9839-11d1-B709-00A024DDAFD1}` |
Teleport | `{4A582747-9839-11d1-B709-00A024DDAFD1}` |
Mech     | `{55D141B8-DB94-11d1-AC98-006008055BB5}` |
Ship     | `{2BEA74E1-7CCA-11d3-BE14-00104B62A16C}` |