#pragma once

#include <TechnoClass.h>
#include <HouseClass.h>
#include <AbstractClass.h>
// Interop exports for AttachEffect operations.
// C# P/Invoke examples (simplified):
// ```csharp
// [DllImport("Phobos.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "AE_Attach")]
// public static extern int AE_Attach(IntPtr pTarget, IntPtr pInvokerHouse, IntPtr pInvoker, IntPtr pSource, IntPtr effectTypeNames, int typeCount, int durationOverride, int delay, int initialDelay, int recreationDelay);
//
// [DllImport("Phobos.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "AE_Detach")]
// public static extern int AE_Detach(IntPtr pTarget, IntPtr effectTypeNames, int typeCount, out int pRemovedCount);
// ```

#include "Utilities/Macro.h"

/// <summary>
/// Attaches AttachEffect instances to a target unit.
/// On success, pAttachedCount receives the number of effects attached.
/// </summary>
DEFINE_EXPORT(HRESULT, AE_Attach,
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
);

/// <summary>
/// Removes AttachEffect instances matching given types from a unit.
/// On success, pRemovedCount receives the number of effects removed.
/// </summary>
DEFINE_EXPORT(HRESULT, AE_Detach,
	TechnoClass* pTarget,
	const char** effectTypeNames,
	int typeCount,
	int* pRemovedCount
);

/// <summary>
/// Removes AttachEffect instances matching given groups from a unit.
/// On success, pRemovedCount receives the number of effects removed.
/// </summary>
DEFINE_EXPORT(HRESULT, AE_DetachByGroups,
	TechnoClass* pTarget,
	const char** groupNames,
	int groupCount,
	int* pRemovedCount
);

/// <summary>
/// Transfers AttachEffect instances from one unit to another.
/// </summary>
DEFINE_EXPORT(HRESULT, AE_TransferEffects,
	TechnoClass* pSource,
	TechnoClass* pTarget
);
