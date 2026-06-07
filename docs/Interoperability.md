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
