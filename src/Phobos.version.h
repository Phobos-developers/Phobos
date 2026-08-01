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

#define VERSION_SHORT_STR _STR(VERSION_MAJOR) "." _STR(VERSION_MINOR)
#define VERSION_LONG_STR VERSION_SHORT_STR "." _STR(VERSION_REVISION) "." _STR(VERSION_PATCH)

// Savegame compatibility is tied to the version with the patch number left out: patch releases
// only carry Phobos bugfixes and stay compatible with each other. Nightlies and pre-releases of
// a version share the ID with the stable release they lead up to.
#define SAVEGAME_ID ((VERSION_MAJOR << 24) | (VERSION_MINOR << 16) | (VERSION_REVISION << 8))

#pragma region Build metadata

// NIGHTLY / PRERELEASE / RELEASE come from the BuildType compiler option - used by GH Actions as
// well - and none of them being set means a local build. Nightlies additionally get GIT_COMMIT
// and GIT_BRANCH. Everything below is derived from those, so that the product name, the version
// and the build type are each spelled out once and stay in sync everywhere they surface.

#define PRODUCT_NAME "Phobos"
#define PRODUCT_FILE_NAME PRODUCT_NAME ".dll"
#define PRODUCT_SUMMARY "Ares-compatible YR engine extension"

#if defined(NIGHTLY)
	#define STR_GIT_COMMIT _STR(GIT_COMMIT)
	#define STR_GIT_BRANCH _STR(GIT_BRANCH)

	#define BUILD_TYPE_NAME "nightly build"
	#define FILE_VERSION_STR VERSION_SHORT_STR "-nightly+" STR_GIT_COMMIT
	#define PRODUCT_VERSION "v" FILE_VERSION_STR " @ " STR_GIT_BRANCH
	#define FILE_VERSION VERSION_MAJOR, VERSION_MINOR, 0, 0
#elif defined(PRERELEASE)
	#define BUILD_TYPE_NAME "pre-release build"
	#define FILE_VERSION_STR VERSION_SHORT_STR "-" PRERELEASE_SUFFIX
	#define PRODUCT_VERSION "v" FILE_VERSION_STR
	#define FILE_VERSION VERSION_MAJOR, VERSION_MINOR, 0, 0
#elif defined(RELEASE)
	#define BUILD_TYPE_NAME "release build"
	#define FILE_VERSION_STR VERSION_LONG_STR
	#define PRODUCT_VERSION "v" FILE_VERSION_STR
	#define FILE_VERSION VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_PATCH
#else
	#define BUILD_TYPE_NAME "development build"
	#define FILE_VERSION_STR VERSION_LONG_STR "-dev"
	#define PRODUCT_VERSION "v" FILE_VERSION_STR
	#define FILE_VERSION VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_PATCH
#endif

#define FILE_DESCRIPTION PRODUCT_NAME ", " PRODUCT_SUMMARY " (" BUILD_TYPE_NAME ")"

#pragma endregion

#endif // VERSION_H
