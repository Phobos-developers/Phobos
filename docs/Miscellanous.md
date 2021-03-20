# Miscellanous

This page describes every change in Phobos that wasn't categorized into a proper category yet.

## Developer tools

### Dump Object Info

![image](docs/_static/images/objectinfo-01.png)
*Object info dump from [CnC: Reloaded](https://www.moddb.com/mods/cncreloaded/)*

- There's a new hotkey to dump selected/hovered object info on press. Available only when the debug tag is set.

In `rulesmd.ini`:
```ini
[GlobalControls]
DebugKeysEnabled=yes ; boolean
```