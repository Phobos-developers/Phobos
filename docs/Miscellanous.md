# Miscellanous

This page describes every change in Phobos that wasn't categorized into a proper category yet.

## Developer tools

### Dump Object Info

![image](_static/images/objectinfo-01.png)
*Object info dump from [CnC: Reloaded](https://www.moddb.com/mods/cncreloaded/)*

- There's a new hotkey to dump selected/hovered object info on press. Available only if `DebugKeysEnabled` under `[GlobalControls]` is set to true in `rulesmd.ini`.

### Display Damage Numbers

- There's a new hotkey to show exact numbers of damage dealt on units & buildings. The numbers are shown in red (blue against shields) for damage, and for healing damage in green (cyan against shields). They are shown on the affected units and will move upwards after appearing. Available only if `DebugKeysEnabled` under `[GlobalControls]` is set to true in `rulesmd.ini`.

### Frame Step In

- There's a new hotkey to execute the game frame by frame for development usage.
	- You can switch to frame by frame mode and then use frame step in command to forward 1, 5, 10, 15, 30 or 60 frames by one hit.

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

## Game Speed

### Single player game speed

- It is now possible to change the default (GS4/Fast/30FPS) campaign game speed with `CampaignDefaultGameSpeed`.
- It is now possible to change the *values* of single player game speed, by inputing a pair of values. This feature must be enabled with `CustomGS=true`. **Only values between 10 and 60 FPS can be consistently achieved.**
  - Custom game speed is achieved by periodically manipulating the delay between game frames, thus increasing or decreasing FPS.
  - `CustomGSN.ChangeInterval` describes the frame interval between applying the effect. A value of 2 means "every other frame", 3 means "every 3 frames" etc. Increase of speedup/slowdown is approximately logarithmic.
  - `CustomGSN.ChangeDelay` sets the delay (game speed number) to use every `CustomGSN.ChangeInterval` frames.
  - `CustomGSN.DefaultDelay` sets the delay (game speed number) to use on other frames.
  - Using game speed 6 (Fastest) in either `CustomGSN.ChangeDelay` or `CustomGSN.DefaultDelay` allows to set FPS above 60. **However, the resulting FPS may vary on different machines.**

```{note}
Currently there is no way to set desired FPS directly. Use the generator below to get required values. The generator supports values from 10 to 60.
```

In `rulesmd.ini`:
```ini
[General]
CustomGS=false              ; boolean
CustomGSN.ChangeInterval=-1 ; integer >= 1
CustomGSN.ChangeDelay=N     ; integer between 0 and 6
CustomGSN.DefaultDelay=N    ; integer between 0 and 6
; where N = 0, 1, 2, 3, 4, 5, 6
```

In `ra2md.ini`:
```ini
[Phobos]
CampaignDefaultGameSpeed=4  ; integer
```

<details>
<summary>Click to show the generator</summary>
<input id="customGameSpeedIn" type=number placeholder="Enter desired FPS" oninput="onInput()">
<p>Results (remember to replace N with your game speed number!):</p>
<div id="codeBlockHere1"></div>
</details>
<script>
makeINICodeBlock(document.getElementById("codeBlockHere1"), "customGameSpeedOut", 400);
let fpsArray = [];
for (let d = 0; d <= 5; d++) {
	for (let c = 0; c <= 5; c++) {
		for (let i = 1; i <= 40; i++) {
			fpsArray.push(Math.round(formula(c, d, i)));
		}
	}
}
function formula(c, d, i) {
	return (60/(6-c)+60/(6-d)*((i-1)/(6-c)))/(1+(i-1)/(6-c));
}
function onInput() {
	let fps = document.getElementById("customGameSpeedIn");
	let out = document.getElementById("customGameSpeedOut");
	out.textContent = ''; // remove all children
	out.appendChild(document.createElement("span"));
	let j = 0;
	let foundAny = false;
	while (true) {
		j = fpsArray.indexOf(parseInt(fps.value), j);
		if (j == -1) {
			break;
		}
		d = Math.floor(j / 240);
		c = Math.floor(j % 240 / 40);
		i = j % 40 + 1;
		j += 1;
		let content = [];
		if (foundAny) {
			content.push({key: null, value: null, comment: "// Or"});
		}
		content.push({key: "CustomGSN.DefaultDelay", value: d, comment: null});
		content.push({key: "CustomGSN.ChangeDelay", value: c, comment: null});
		content.push({key: "CustomGSN.ChangeInterval", value: i, comment: null});
		content.forEach(line => addINILine(out, line));
		foundAny = true;
	}
	if (!foundAny) {
		addINILine(out, {key: null, value: null, comment: "// Sorry, couldn't find anything!"});
	}
}
</script>

## INI

### Section inheritance
- You can now make sections (children) inherit entries from other sections (parents) with `$Inherits`.
  - Every parent entry will be inherited verbatim, without checking if it is valid.
  - When there are multiple parents, they will be inherited in the order they were given.
  - In the child entry, entries before `$Inherits` can be overwritten if they are defined inside parents. **It is advised to always keep `$Inherits` as the first entry in a section.**

```{warning}
Currently, this feature only works in files included by [Ares #include feature](https://ares-developers.github.io/Ares-docs/new/misc/include.html).
```

In any file:
```ini
[PARENT1SECTION]

[PARENT2SECTION]

[CHILDSECTION]
$Inherits=PARENT1SECTION,PARENT2SECTION...  ; section names
```
