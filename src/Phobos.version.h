#ifndef VERSION_H
#define VERSION_H

#define _WSTR(x) _WSTR_(x)
#define _WSTR_(x) L ## #x
#define _STR(x) _STR_(x)
#define _STR_(x) #x

#pragma region Release build version numbering

// Indicates project maturity and completeness
#define VERSION_MAJOR 0

// Indicates major changes and significant additions, like new logics
#define VERSION_MINOR 3

// Indicates minor changes, like vanilla bugfixes, unhardcodings or hacks
#define VERSION_REVISION 0

// Indicates Phobos-related bugfixes only
#define VERSION_PATCH 1

#pragma endregion

// Pre-release build number. Incremented on each pre-release build.
#define PRERELEASE_NUM 1

// Nightly defines GIT_COMMIT and GIT_BRANCH in GH Actions
// NIGHTLY / PRERELEASE / RELEASE come from compiler option BuildType - used by GH Actions as well

#if defined(NIGHTLY)
	#define STR_GIT_COMMIT _STR(GIT_COMMIT)
	#define STR_GIT_BRANCH _STR(GIT_BRANCH)
	#define SAVEGAME_ID ((VERSION_MAJOR << 24) | (VERSION_MINOR << 16) | (VERSION_REVISION << 8) | VERSION_PATCH)
	#define FILE_DESCRIPTION "Unstable nightly build of Phobos engine extension"
	#define FILE_VERSION_STR "Commit " STR_GIT_COMMIT
	#define FILE_VERSION 0
	#define PRODUCT_VERSION "Nightly Build " STR_GIT_COMMIT " @ " STR_GIT_BRANCH
#elif defined(PRERELEASE)
	#define SAVEGAME_ID ((VERSION_MAJOR << 24) | (VERSION_MINOR << 16) | (PRERELEASE_NUM << 8))
	#define FILE_DESCRIPTION "Pre-release build of Phobos, Ares-compatible YR engine extension"
	#define FILE_VERSION_STR _STR(VERSION_MAJOR) "." _STR(VERSION_MINOR) "-beta" _STR(PRERELEASE_NUM)
	#define FILE_VERSION VERSION_MAJOR, VERSION_MINOR, 0, 0
	#define PRODUCT_VERSION "v." _STR(VERSION_MAJOR) "." _STR(VERSION_MINOR) "-beta" _STR(PRERELEASE_NUM)
#else
	#define SAVEGAME_ID ((VERSION_MAJOR << 24) | (VERSION_MINOR << 16) | (VERSION_REVISION << 8) | VERSION_PATCH)
	#define FILE_DESCRIPTION "Phobos, Ares-compatible YR engine extension"
	#define FILE_VERSION_STR _STR(VERSION_MAJOR) "." _STR(VERSION_MINOR) "." _STR(VERSION_REVISION) "." _STR(VERSION_PATCH)
	#define FILE_VERSION VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_PATCH
	#define PRODUCT_VERSION "v." FILE_VERSION_STR
#endif

#endif // VERSION_H
