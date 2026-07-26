# Interoperability

This page documents the exported interfaces in [Interop](https://github.com/Phobos-developers/Phobos/tree/develop/src/Interop).

## API Convention

All exported API functions return `HRESULT` to indicate success or failure following COM convention:

| HRESULT        | Meaning                                                                      |
|----------------|------------------------------------------------------------------------------|
| `S_OK`         | Operation completed successfully.                                            |
| `S_FALSE`      | Operation completed but had nothing to do (e.g., no matching effects found). |
| `E_POINTER`    | A required pointer parameter was null.                                       |
| `E_INVALIDARG` | One or more arguments are invalid.                                           |
| `E_UNEXPECTED` | An unexpected internal error occurred (e.g., extension data not found).      |
| `E_FAIL`       | The operation failed.                                                        |

Functions that produce output data take an additional output pointer parameter that receives the result. Use `SUCCEEDED(hr)` / `FAILED(hr)` to check the return value.

## Antares

[Antares](https://github.com/Phobos-developers/Antares) is an open-source
reimplementation of Ares. Phobos supports it alongside Ares, but reaches it a
different way.

Everything Phobos uses from Ares is either an address relative to `Ares.dll`'s base
or a field at a known offset inside an Ares extension. Neither survives a different
compile, so none of it can be pointed at Antares. Antares instead exports
`GetAntaresAPI`, which hands back a versioned table of function pointers plus
accessors that return extension data as real objects.

### Detection

Antares is recognised by that export, not by filename or PE timestamp. This matters:
Antares used to inherit Ares' `OriginalFilename`, so a name-based check found it,
failed the timestamp match, and disabled integration in a way that also broke
superweapon chaining.

`AresHelper` reports it as `Version::Antares` and sets `CanUseAntares`. Note the
three flags are not interchangeable:

| Flag | Meaning |
|------|---------|
| `CanUseAres` | Genuine Ares, at a build whose addresses we know. Gates everything that reads an Ares RVA, patches Ares' code, or reads its extension data by offset. **False for Antares.** |
| `CanUseAntares` | Antares detected and its table obtained. |
| `CanUseExtension` | Either of the above. Gates anything that goes through `AresFunctions`. |

### Nothing Ares-specific runs

`Apply_Ares3_0p1_Patches` and the RVA resolution in `AresAddressInit.cpp` are only
reachable for genuine Ares. Those patches must never be applied to Antares -- they
would land on unrelated code.

The Ares extension layouts live in one place, `AresLayout` in `AresAddressInit.cpp`,
behind the same accessors Antares' table fills. Call sites go through
`AresFunctions::GetDriverKilled` and friends and do not know which is behind them.

### Feature handover

Rather than being patched, Antares can be asked to stand a subsystem down at runtime
so Phobos owns it instead. Phobos currently takes over `EBolt` and `AlphaImage`.

### Known gaps

Some Ares-only paths have no Antares equivalent yet, and degrade rather than fail:

- Voxel turrets past index 18 fall back to the vanilla limit, since reading them
  needs the Ares `TechnoTypeExt` layout.
- Warhead `Verses` lookup, the spy-effect income counter, `WeaponIndex_Warp`, and the
  `InitialPayload` team fix stay Ares-only for the same reason. Antares fixes the
  payload bug in its own source, so the last one is not needed there.
- The patch-driven takeovers -- EBolt reimplementation, permanent mind control
  wrappers, paradrop wrappers, `KillDriver`, `getCellSpreadItems`, `RemoveCameo`,
  promotion animations and the laser weapon pick -- are Ares-only. Where Antares
  needs to yield instead, that is what feature handover is for.

## API Version Tracking

### Semantic Versioning Rules

- **Major (X)**: Increment for breaking changes (backward incompatible API modifications).\
  Example: Deleting an API, changing function parameters, modifying data structures that break existing code.\
  When bumped, reset Minor and Patch to 0 (e.g., 1.2.3 → 2.0.0).

- **Minor (Y)**: Increment for backward-compatible new features.\
  Example: Adding a new API function, new parameters that don't break existing code.\
  When bumped, reset Patch to 0 (e.g., 1.2.3 → 1.3.0).

- **Patch (Z)**: Increment for backward-compatible miscs (no API changes).\
  Example: Fixing crashes, correcting calculations, optimizing internals without changing interface (e.g., 1.2.3 → 1.2.4).

### GetInteropAPIVersion

```cpp
HRESULT GetInteropAPIVersion(InteropAPIVersion* pVersion)
```

Returns the current Interop API version via `pVersion` output parameter.

- Parameters:
  - `pVersion`: Receives the version structure `{ major, minor, patch }`.
- Returns `S_OK` on success, `E_POINTER` if `pVersion` is null.

**Example (C# P/Invoke):**
```csharp
[StructLayout(LayoutKind.Sequential)]
public struct InteropAPIVersion
{
    public uint major;
    public uint minor;
    public uint patch;
}

[DllImport("Phobos.dll", CallingConvention = CallingConvention.StdCall)]
public static extern int GetInteropAPIVersion(out InteropAPIVersion pVersion);

InteropAPIVersion version;
int hr = GetInteropAPIVersion(out version);
if (hr >= 0 && version.major >= 1)
{
    // Safe to use features from v1.0.0 onwards
}
```

### Deprecated API Handling

When an API is deprecated, its function stub is retained but with a fatal error handler. The calling application will receive a descriptive error message and must stop execution. This ensures:
1. Broken links are immediately detected at runtime (not a silent crash).
2. Clear messaging guides developers to the replacement API and version range.
3. Migration timeline is documented in the error message.

**Example** (not currently in use):

```cpp
// DEPRECATED: Removed in Interop API v2.0.0. Use NewAPI instead.
// Availability: [1.0.0, 2.0.0)
// Calling this will trigger a fatal error with the message:
// "SomeOldAPI_Deprecated has been removed in Interop API v2.0.0 (was available in v1.0.0-v1.x.x). Please use NewAPI."
```

## Available APIs

| Module       | API                                        | Availability   | Status |
|--------------|--------------------------------------------|----------------|--------|
| AttachEffect | AE_Attach                                  | `[1.0.0, ∞)`  | Active |
| AttachEffect | AE_Detach                                  | `[1.0.0, ∞)`  | Active |
| AttachEffect | AE_DetachByGroups                          | `[1.0.0, ∞)`  | Active |
| AttachEffect | AE_TransferEffects                         | `[1.0.0, ∞)`  | Active |
| BulletExt    | Bullet_SetFirerOwner                       | `[1.0.0, ∞)`  | Active |
| EventExt     | EventExt_AddEvent                          | `[1.0.0, ∞)`  | Active |
| TechnoExt    | ConvertToType_Phobos                       | `[1.0.0, ∞)`  | Active |
| TechnoExt    | RegisterCalculateExtraThreatCallback       | `[1.0.0, ∞)`  | Active |
| ScenarioExt  | Variables_GetLocal_Phobos                  | `[1.1.0, ∞)`  | Active |
| ScenarioExt  | Variables_SetLocal_Phobos                  | `[1.1.0, ∞)`  | Active |
| ScenarioExt  | Variables_GetGlobal_Phobos                 | `[1.1.0, ∞)`  | Active |
| ScenarioExt  | Variables_SetGlobal_Phobos                 | `[1.1.0, ∞)`  | Active |

### AttachEffect

#### AE_Attach

**Availability:** `[1.0.0, ∞)`

```cpp
HRESULT AE_Attach(
		TechnoClass* pTarget,
		HouseClass* pInvokerHouse,
		TechnoClass* pInvoker,
		AbstractClass* pSource,
		const char** effectTypeNames,
		int typeCount,
		int durationOverride,
		int delay,
		int initialDelay,
		int recreationDelay,
		int* pAttachedCount
)
```

Attaches one or more AttachEffect types to the target.

- Parameters:
  - pTarget: Target unit to receive effects.
  - pInvokerHouse: Invoker house context.
  - pInvoker: Invoker techno context.
  - pSource: Optional source object context.
  - effectTypeNames: Array of AttachEffect type names.
  - typeCount: Number of entries in effectTypeNames.
  - durationOverride: If non-zero, duration override is applied.
  - delay: If >= 0, delay override is applied.
  - initialDelay: If >= 0, initial delay override is applied.
  - recreationDelay: If >= -1, recreation delay override is applied.
  - pAttachedCount: Receives the number of effects attached.
- Returns `S_OK` on success, `S_FALSE` if no valid effect type names were found.
- Fails with `E_POINTER` when: pTarget, effectTypeNames, or pAttachedCount is null.
- Fails with `E_INVALIDARG` when: typeCount <= 0.

#### AE_Detach

**Availability:** `[1.0.0, ∞)`

```cpp
HRESULT AE_Detach(
		TechnoClass* pTarget,
		const char** effectTypeNames,
		int typeCount,
		int* pRemovedCount
)
```

Detaches effects by explicit effect type names.

- Parameters:
  - pTarget: Target unit to remove effects from.
  - effectTypeNames: Array of AttachEffect type names to remove.
  - typeCount: Number of entries in effectTypeNames.
  - pRemovedCount: Receives the number of effects removed.
- Returns `S_OK` on success, `S_FALSE` if no matching effects were found.
- Fails with `E_POINTER` when: pTarget, effectTypeNames, or pRemovedCount is null.
- Fails with `E_INVALIDARG` when: typeCount <= 0.

#### AE_DetachByGroups

**Availability:** `[1.0.0, ∞)`

```cpp
HRESULT AE_DetachByGroups(
		TechnoClass* pTarget,
		const char** groupNames,
		int groupCount,
		int* pRemovedCount
)
```

Detaches effects by AttachEffect group name.

- Parameters:
  - pTarget: Target unit to remove effects from.
  - groupNames: Array of group names.
  - groupCount: Number of entries in groupNames.
  - pRemovedCount: Receives the number of effects removed.
- Returns `S_OK` on success, `S_FALSE` if no matching groups were found.
- Fails with `E_POINTER` when: pTarget, groupNames, or pRemovedCount is null.
- Fails with `E_INVALIDARG` when: groupCount <= 0.

#### AE_TransferEffects

**Availability:** `[1.0.0, ∞)`

```cpp
HRESULT AE_TransferEffects(
		TechnoClass* pSource,
		TechnoClass* pTarget
)
```

Transfers all attached effects from source to target.

- Parameters:
  - pSource: Source unit.
  - pTarget: Target unit.
- Returns `S_OK` on success.
- Fails with `E_POINTER` when: pSource or pTarget is null.

## Vanilla class extension

### TechnoExt

#### ConvertToType_Phobos

**Availability:** `[1.0.0, ∞)`

```cpp
HRESULT ConvertToType_Phobos(FootClass* pThis, TechnoTypeClass* toType)
```

Converts a FootClass instance to another TechnoType.

- Parameters:
  - pThis: Unit to convert.
  - toType: Destination TechnoType.
- Returns `S_OK` if conversion succeeds.
- Returns `E_INVALIDARG` if types are incompatible.
- Notes:
  - This API forwards directly to TechnoExt::ConvertToType.

#### RegisterCalculateExtraThreatCallback

**Availability:** `[1.0.0, ∞)`

```cpp
typedef double (*CalculateExtraThreatCallback)(TechnoClass* pThis, ObjectClass* pTarget, double originalThreat);

HRESULT RegisterCalculateExtraThreatCallback(CalculateExtraThreatCallback callback)
```

Registers a callback function to calculate extra threat for a unit.

- Parameters:
  - callback: Callback function pointer that returns the calculated threat modifier. Signature: `double callback(TechnoClass* pThis, ObjectClass* pTarget, double originalThreat)`.
- Returns `S_OK` on success.
- Fails with `E_POINTER` when: callback is null.

- Behavior:
  - If callback is non-null, it is added to the internal callback list.
  - When threat calculations occur, all registered callbacks are invoked to compute additional threat contributions.
  - Multiple callbacks can be registered and are executed in registration order.

- Notes:
  - The callback invocation method is `totalThreat = cb(pThis, pTarget, totalThreat)`.

### BulletExt

#### Bullet_SetFirerOwner

**Availability:** `[1.0.0, ∞)`

```cpp
HRESULT Bullet_SetFirerOwner(BulletClass* pBullet, HouseClass* pHouse)
```

Updates the recorded firer house for a bullet extension.

- Parameters:
  - pBullet: Bullet instance.
  - pHouse: New firer house (can be null if caller intentionally clears ownership).
- Returns `S_OK` if the bullet extension is found and updated.
- Fails with `E_POINTER` when: pBullet is null.
- Fails with `E_UNEXPECTED` when: no BulletExt entry exists for pBullet.

### ScenarioExt

#### Variables_GetLocal_Phobos

**Availability:** `[1.1.0, ∞)`

```cpp
HRESULT Variables_GetLocal_Phobos(int index, int* pValue)
```

Gets the value of a local variable by index. Does not create the variable if missing — pValue is set to 0 when the index is not found.

- Parameters:
  - index: Variable index.
  - pValue: Receives the variable value.
- Returns `S_OK` if the variable exists, `S_FALSE` if it does not.
- Fails with `E_POINTER` when: pValue is null.
- Fails with `E_FAIL` when: ScenarioExt is not initialized.

#### Variables_SetLocal_Phobos

**Availability:** `[1.1.0, ∞)`

```cpp
HRESULT Variables_SetLocal_Phobos(int index, int value)
```

Sets the value of a local variable by index. Creates the variable if the index does not exist.

- Parameters:
  - index: Variable index.
  - value: New value for the variable.
- Returns `S_OK` on success.
- Fails with `E_FAIL` when: ScenarioExt is not initialized.

#### Variables_GetGlobal_Phobos

**Availability:** `[1.1.0, ∞)`

```cpp
HRESULT Variables_GetGlobal_Phobos(int index, int* pValue)
```

Gets the value of a global variable by index. Does not create the variable if missing — pValue is set to 0 when the index is not found.

- Parameters:
  - index: Variable index.
  - pValue: Receives the variable value.
- Returns `S_OK` if the variable exists, `S_FALSE` if it does not.
- Fails with `E_POINTER` when: pValue is null.
- Fails with `E_FAIL` when: ScenarioExt is not initialized.

#### Variables_SetGlobal_Phobos

**Availability:** `[1.1.0, ∞)`

```cpp
HRESULT Variables_SetGlobal_Phobos(int index, int value)
```

Sets the value of a global variable by index. Creates the variable if the index does not exist.

- Parameters:
  - index: Variable index.
  - value: New value for the variable.
- Returns `S_OK` on success.
- Fails with `E_FAIL` when: ScenarioExt is not initialized.

### EventExt

#### EventExt_AddEvent

**Availability:** `[1.0.0, ∞)`

```cpp
HRESULT EventExt_AddEvent(EventExt* pEventExt)
```

Invokes AddEvent on an EventExt object.

- Parameters:
  - pEventExt: Event extension instance.
- Returns `S_OK` if AddEvent succeeds, `S_FALSE` if AddEvent returns false.
- Fails with `E_POINTER` when: pEventExt is null.
