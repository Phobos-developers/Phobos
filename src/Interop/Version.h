#pragma once

#include "Utilities/Macro.h"

// ============================================================================
// Interop API Semantic Versioning (independent from Phobos build number)
// See https://semver.org/ for version semantics.
// ============================================================================

#define INTEROP_API_VERSION_MAJOR 1
#define INTEROP_API_VERSION_MINOR 1
#define INTEROP_API_VERSION_PATCH 0

/// <summary>
/// Interop API version information structure.
/// Follows Semantic Versioning 2.0.0:
/// - Major: Increment for breaking changes (backward incompatible API modifications).
/// - Minor: Increment for new backward-compatible features.
/// - Patch: Increment for backward-compatible bug fixes.
/// </summary>
struct InteropAPIVersion
{
	unsigned int major;
	unsigned int minor;
	unsigned int patch;
};

/// <summary>
/// Returns the current Interop API version following Semantic Versioning.
/// Each exported API is annotated with availability range [startVersion, endVersion).
/// If endVersion == 0, the API is still active.
/// </summary>
DEFINE_EXPORT(HRESULT, GetInteropAPIVersion, InteropAPIVersion* pVersion);

// ============================================================================
// Deprecated API example (commented out for future use)
// When an API reaches end-of-life, keep its function stub but call fatal error.
// ============================================================================
/*
/// <summary>
/// DEPRECATED: Removed in Interop API v2.0.0. Use SomeNewAPI instead.
/// Calling this function will trigger a fatal error.
/// Availability: [1.0.0, 2.0.0)
/// </summary>
DEFINE_EXPORT(int, SomeOldAPI_Deprecated,
	int param1
);
*/
