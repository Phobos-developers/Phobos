#include "Version.h"
#include "Utilities/Debug.h"

DEFINE_EXPORT(HRESULT, GetInteropAPIVersion, InteropAPIVersion* pVersion)
{
	if (!pVersion)
		return E_POINTER;

	pVersion->major = INTEROP_API_VERSION_MAJOR;
	pVersion->minor = INTEROP_API_VERSION_MINOR;
	pVersion->patch = INTEROP_API_VERSION_PATCH;

	return S_OK;
}

// ============================================================================
// Deprecated API implementation example (commented out for future use)
// ============================================================================
/*
DEFINE_EXPORT(int, SomeOldAPI_Deprecated, int param1)
{
	// Trigger fatal error with explanation.
	Debug::Fatal("SomeOldAPI_Deprecated has been removed in Interop API v2.0.0 (was available in v1.0.0-v1.x.x). "
		"Please update your code to use the replacement API.");
	return 0;  // unreachable
}
*/
