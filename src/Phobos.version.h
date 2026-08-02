#ifndef VERSION_H
#define VERSION_H

#define _STR(x) _STR_(x)
#define _STR_(x) #x

#pragma region Version numbering

// Indicates project maturity and completeness
#define VERSION_MAJOR 0

// Indicates major changes and significant additions, like new logics
#define VERSION_MINOR 5

// Indicates minor changes, like vanilla bugfixes, unhardcodings or hacks
#define VERSION_REVISION 0

// Indicates Phobos-related bugfixes only
#define VERSION_PATCH 0

// Identifier of the pre-release being prepared, spelled out in full rather than as a number,
// so that it can be anything semantic versioning allows - "alpha1", "beta2", "rc3".
// Only ever used by PRERELEASE builds; bump it on each pre-release.
#define PRERELEASE_SUFFIX "beta1"

#pragma endregion

#define VERSION_LONG_STR _STR(VERSION_MAJOR) "." _STR(VERSION_MINOR) "." _STR(VERSION_REVISION) "." _STR(VERSION_PATCH)

// Savegame compatibility is tied to the version with the patch number left out: patch releases
// only carry Phobos bugfixes and stay compatible with each other. Nightlies and pre-releases of
// a version share the ID with the stable release they lead up to.
#define SAVEGAME_ID ((VERSION_MAJOR << 24) | (VERSION_MINOR << 16) | (VERSION_REVISION << 8))

#pragma region Build metadata

// NIGHTLY / PRERELEASE / RELEASE come from the BuildType compiler option - used by GH Actions as
// well - and none of them being set means a local build. GIT_COMMIT / GIT_BRANCH / GIT_DIRTY are
// defined by Phobos.props whenever Git info is available (derived from the repository at build
// time; the branch is the full ref, e.g. refs/heads/develop). Pre-releases and releases embed
// that info as auxiliary metadata (crash reports, logs, file resources) while keeping a clean
// version string; nightly and local builds carry the Git info in the version itself and differ
// only in the build type name. Everything below is derived from those inputs so that the product
// name, the version and the build type are each spelled out once and stay in sync everywhere they
// surface.

#define PRODUCT_NAME "Phobos"
#define PRODUCT_FILE_NAME PRODUCT_NAME ".dll"
#define PRODUCT_SUMMARY "Ares-compatible YR engine extension"

// The "v" prefix is a human-facing convention for version numbers and is only ever applied in
// front of a numeric version - never in front of a branch or commit. It is applied exactly once
// here in PRODUCT_VERSION and no other macro in this header prepends it.
#define VERSION_PREFIX "v"

#ifdef GIT_COMMIT
	#define STR_GIT_COMMIT _STR(GIT_COMMIT)
#endif
#ifdef GIT_BRANCH
	#define STR_GIT_BRANCH _STR(GIT_BRANCH)
#endif
#ifdef GIT_DIRTY
	#define STR_GIT_DIRTY "-dirty"
#else
	#define STR_GIT_DIRTY ""
#endif

// The numeric file version is the full four-part version for every build type.
#define FILE_VERSION VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_PATCH

#if defined(PRERELEASE)
	#define BUILD_TYPE_NAME "pre-release build"
	#define FILE_VERSION_STR VERSION_LONG_STR "-" PRERELEASE_SUFFIX
	#define PRODUCT_VERSION VERSION_PREFIX FILE_VERSION_STR
#elif defined(RELEASE)
	#define BUILD_TYPE_NAME "release build"
	#define FILE_VERSION_STR VERSION_LONG_STR
	#define PRODUCT_VERSION VERSION_PREFIX FILE_VERSION_STR
#else
	// Nightly (CI) and local builds are one unified development build differing only in the
	// build type name; the version string carries the Git commit and branch when available.
	#ifdef NIGHTLY
		#define BUILD_TYPE_NAME "nightly build"
	#else
		#define BUILD_TYPE_NAME "development build"
	#endif
	#ifdef GIT_COMMIT
		#define FILE_VERSION_STR VERSION_LONG_STR "+" STR_GIT_COMMIT STR_GIT_DIRTY
	#else
		#define FILE_VERSION_STR VERSION_LONG_STR
	#endif
	#ifdef GIT_BRANCH
		#define PRODUCT_VERSION VERSION_PREFIX FILE_VERSION_STR " @ " STR_GIT_BRANCH
	#else
		#define PRODUCT_VERSION VERSION_PREFIX FILE_VERSION_STR
	#endif
#endif

#define FILE_DESCRIPTION PRODUCT_NAME ", " PRODUCT_SUMMARY " (" BUILD_TYPE_NAME ")"

#pragma endregion

#endif // VERSION_H
