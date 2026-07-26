#pragma once


// Interop exports for external engine extensions.
// C# P/Invoke example:
// ```csharp
// [DllImport("Phobos.dll", CallingConvention = CallingConvention.StdCall, EntryPoint = "ConvertToType_Phobos")]
// [return: MarshalAs(UnmanagedType.I1)]
// public static extern bool ConvertToType_Phobos(IntPtr pThis, IntPtr toType);
// ```
// Use `IntPtr` or `unsafe` pointers in managed code. The function below exposes
// the same signature as used internally (FootClass*, TechnoTypeClass*).

#include "Utilities/Macro.h"
#include <Utilities/TemplateDef.h>

#include <vector>

#include <Ext/Techno/Body.h>

DEFINE_CALLBACK(double, CalculateExtraThreatCallback, TechnoClass* pThis, ObjectClass* pTarget, double originalThreat);
DEFINE_CALLBACK(double, CalculateSightCallback, TechnoClass* pThis, double originalSight);

class TechnoExtInterop
{
public:
	static std::vector<CalculateExtraThreatCallback> CalculateExtraThreatCallbacks;
	static std::vector<CalculateSightCallback> CalculateSightCallbacks;
};

/// <summary>
/// Converts a unit to a different type.
/// </summary>
/// <param name="pThis">Pointer to the FootClass instance to convert</param>
/// <param name="toType">Pointer to the target TechnoTypeClass</param>
/// <returns>S_OK if conversion was successful, E_INVALIDARG if types are incompatible, E_FAIL if conversion failed</returns>
DEFINE_EXPORT(HRESULT, ConvertToType_Phobos, FootClass* pThis, TechnoTypeClass* toType);

DEFINE_EXPORT(HRESULT, RegisterCalculateExtraThreatCallback, CalculateExtraThreatCallback callback);

DEFINE_EXPORT(HRESULT, RegisterCalculateSightCallback, CalculateSightCallback callback);
