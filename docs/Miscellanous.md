# Miscellanous

This page describes every change in Phobos that wasn't categorized into a proper category yet.

## Developer tools

### Dump Object Info

![](https://cdn.discordapp.com/attachments/773636942775582720/818783465423634453/unknown.png)  
*Object info dump from [CnC: Reloaded](https://www.moddb.com/mods/cncreloaded/)*

- There's a new hotkey to dump selected/hovered object info on press. Available only when the debug tag is set.

In `rulesmd.ini`:
```ini
[GlobalControls]
DebugKeysEnabled=yes ; boolean
```