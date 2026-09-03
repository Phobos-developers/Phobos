---
name: 生成INI标签 Generate INI tag
description: "A \"tag\" is an INI key that generally corresponds to a C++ property. Defining a tag requires modifications in four places within the target `ExtData` class: **declaration**, **initialization**, **serialization**, and **INI loading**. If the user does not provide all information explicitly, the AI should infer the missing pieces, present them to the user for confirmation, and only proceed after approval."
---

#### 1. Determine the Class

Game entities have **instance classes** (e.g., `BuildingClass`) and **static type classes** (e.g., `BuildingTypeClass`). Tags are almost always defined in the static type class's `ExtData` — when the user says "add a tag for buildings", they mean `BuildingTypeExt::ExtData`. Same pattern applies:

| User says | Corresponding ExtData |
|-----------|----------------------|
| Building / BuildingType | `BuildingTypeExt::ExtData` |
| Vehicle / Unit / UnitType | `UnitTypeExt::ExtData` (or `TechnoTypeExt::ExtData`) |
| Infantry / InfantryType | `InfantryTypeClass` → `TechnoTypeExt::ExtData` |
| Aircraft / AircraftType | `AircraftTypeClass` → `TechnoTypeExt::ExtData` |
| Warhead | `WarheadTypeExt::ExtData` |
| Weapon | `WeaponTypeExt::ExtData` |
| Bullet / Projectile | `BulletTypeExt::ExtData` |
| SuperWeapon / SW | `SWTypeExt::ExtData` |
| Animation / Anim | `AnimTypeExt::ExtData` |
| Particle | `ParticleTypeExt::ExtData` |
| Terrain / TerrainType | `TerrainTypeExt::ExtData` |
| Overlay / OverlayType | `OverlayTypeExt::ExtData` |
| Global / Rules | `RulesExt::ExtData` |

Note: `TechnoTypeExt::ExtData` is the **common base class** extension for `BuildingType`, `UnitType`, `InfantryType`, and `AircraftType`. If a tag applies to all Techno types, place it in `TechnoTypeExt`; if it applies only to a specific subtype, place it in the corresponding subclass Ext (e.g., buildings only → `BuildingTypeExt`).

#### 2. Determine the Section

- **Non-global tags**: Read from `pSection` (the INI section matching the instance ID/name) inside `LoadFromINIFile`. Determine whether to use `pSection` or `pArtSection`:
  - `pSection` = `pThis->ID` — reads from the identically-named section in `rulesmd.ini`. The vast majority of tags go here.
  - `pArtSection` = `pThis->ImageFile` — reads from the identically-named section in `artmd.ini`, using `INI_Art` / `exArtINI`. Generally used for visual, graphical, or animation-related tags. Check whether the class's `LoadFromINIFile` defines a `pArtSection` variable and `exArtINI` to see if this is supported.
- **Global tags**: For `RulesExt::ExtData`, read from fixed global sections such as `GameStrings::General` (`[General]`), `GameStrings::CombatDamage` (`[CombatDamage]`), `GameStrings::Radiation` (`[Radiation]`), `GameStrings::AudioVisual` (`[AudioVisual]`), `GameStrings::AI` (`[AI]`), etc. Infer the appropriate section based on the key name's semantics.

#### 3. Determine the Key Name

The INI key name specified by the user. **Critical conversion rule: dots `.` in the key name become underscores `_` in the C++ property name.**

For example: `"Factory.IsSuper"` → C++ property name `Factory_IsSuper`

#### 4. Determine the Type

Infer the C++ type based on semantics. Common types and their wrappers:

| Semantics | C++ Type | Wrapper |
|-----------|---------|---------|
| Boolean on/off switch | `bool` | `Valueable<bool>` |
| Integer (count, frames, int percentage) | `int` | `Valueable<int>` |
| Floating point (ratio, multiplier, speed) | `double` | `Valueable<double>` |
| In-game distance (Lepton) | `Leptons` | `Valueable<Leptons>` |
| Coordinate | `CoordStruct` / `Point2D` | `Valueable<CoordStruct>` |
| Color | `ColorStruct` | `Valueable<ColorStruct>` |
| Nullable (omitting means use default logic) | any | `Nullable<T>` |
| List (comma-separated values) | `std::vector<T>` | `ValueableVector<T>` |
| Pointer to another game type | `WeaponTypeClass*` etc. | `Valueable<WeaponTypeClass*>` |
| Index lookup (sounds etc.) | see `ValueableIdx` | `ValueableIdx<VocClass>` |
| Enum | game enum | `Valueable<AffectedHouse>` etc. |

**Inference rules:**
- Key name contains `Is`/`Can`/`Allow`/`Use`/`Has`/`Enable`/`Disable` → likely `Valueable<bool>`
- Key name contains `Amount`/`Count`/`Max`/`Min`/`Delay`/`Rate`/`Frame` (integer) → likely `Valueable<int>`
- Key name contains `Factor`/`Mult`/`Percent`/`Speed`/`Ratio`/`Chance` → likely `Valueable<double>`
- Key name contains `Type` (singular pointer) → `Valueable<SomeTypeClass*>`
- Key name contains `Types` (plural) → `ValueableVector<SomeTypeClass*>`
- When unsure about optionality, default to `Valueable<T>` (required); use `Nullable<T>` if omitting is semantically valid.

#### 5. Implementation Steps

Modify the `Body.h` and `Body.cpp` of the target ExtData. Follow these four steps:

**A. Declare the property (Body.h → ExtData public section)**

Follow existing declaration style, e.g.:
```cpp
Valueable<bool> Factory_IsSuper;
```

**B. Initialize the property (Body.h → ExtData constructor initializer list)**

Add a default value in the constructor, matching the existing format (comma-first, aligned):
```cpp
, Factory_IsSuper { false }
```

**C. Register in serialization (Body.cpp → Serialize function)**

Insert into the `Stm` chain in `Serialize(T& Stm)`, ordered alphabetically or logically:
```cpp
.Process(this->Factory_IsSuper)
```

**D. INI loading (Body.cpp → LoadFromINIFile function)**

Based on the section determined in step 2, choose `exINI` or `exArtINI`, and `pSection` or `pArtSection`:
```cpp
// From rulesmd.ini (most common)
this->Factory_IsSuper.Read(exINI, pSection, "Factory.IsSuper");

// From artmd.ini (visual/graphical)
this->Factory_IsSuper.Read(exArtINI, pArtSection, "Factory.IsSuper");

// Global tag example (RulesExt)
this->Factory_IsSuper.Read(exINI, GameStrings::General, "Factory.IsSuper");
```

If the property is a pointer to another game type (e.g., `Valueable<WeaponTypeClass*>`), use `.Read<true>` to enable auto-creation:
```cpp
this->SomeWeapon.Read<true>(exINI, pSection, "SomeWeapon");
```

#### 6. Full Example

User says: "add a tag called `Factory.IsSuper` for buildings"

Inference process:
- Class: `BuildingTypeExt::ExtData` ("buildings" → building type static data)
- Section: `pSection` (not visual-related)
- Key: `Factory.IsSuper` → property name `Factory_IsSuper`
- Type: `Valueable<bool>` (contains `Is`, boolean semantics)
- Default: `false` (conservative default)

After confirmation, modify:
- [Body.h](file:///f:/RA2%20Engine%20Extension/MJobos/src/Ext/BuildingType/Body.h): declaration + initialization
- [Body.cpp](file:///f:/RA2%20Engine%20Extension/MJobos/src/Ext/BuildingType/Body.cpp): Serialize + LoadFromINIFile