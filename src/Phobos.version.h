#ifndef VERSION_H
#define VERSION_H

#define wstr(x) wstr_(x)
#define wstr_(x) L ## #x
#define str(x) str_(x)
#define str_(x) #x

#pragma region Release build version numbering

// Indicates project maturity and completeness
#define VERSION_MAJOR 0

// Indicates major changes and significant additions, like new logics
#define VERSION_MINOR 2

// Indicates minor changes, like vanilla bugfixes, unhardcodings or hacks
#define VERSION_REVISION 2

// Indicates Phobos-related bugfixes only
#define VERSION_PATCH 2

#pragma endregion

// Build number. Incremented on each released build.
#define BUILD_NUMBER 23

// Nightly defines GIT_COMMIT and GIT_BRANCH in GH Actions

#ifdef IS_RELEASE_VER // Release build metadata
	#define SAVEGAME_ID ((VERSION_MAJOR << 24) | (VERSION_MINOR << 16) | (VERSION_REVISION << 8) | VERSION_PATCH)
	#define FILE_DESCRIPTION "Phobos, Ares-compatible YR engine extension"
	#define FILE_VERSION_STR str(VERSION_MAJOR) "." str(VERSION_MINOR) "." str(VERSION_REVISION) "." str(VERSION_PATCH)
	#define FILE_VERSION VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_PATCH
	#define PRODUCT_VERSION "Release Build " FILE_VERSION_STR
#elif defined(GIT_COMMIT) // Nightly devbuild metadata
	#define STR_GIT_COMMIT str(GIT_COMMIT)
	#define STR_GIT_BRANCH str(GIT_BRANCH)

	#define SAVEGAME_ID ((BUILD_NUMBER << 24) | (BUILD_NUMBER << 12) | (BUILD_NUMBER))
	#define FILE_DESCRIPTION "Unstable nightly devbuild of Phobos engine extension"
	#define FILE_VERSION_STR "Commit " STR_GIT_COMMIT
	#define FILE_VERSION 0
	#define PRODUCT_VERSION "Nightly Build " STR_GIT_COMMIT " @ " STR_GIT_BRANCH
#else // Regular devbuild metadata
	#define SAVEGAME_ID ((BUILD_NUMBER << 24) | (BUILD_NUMBER << 12) | (BUILD_NUMBER))
	#define FILE_DESCRIPTION "Development build of Phobos engine extension"
	#define FILE_VERSION_STR "Build #" str(BUILD_NUMBER)
	#define FILE_VERSION 0,0,0,BUILD_NUMBER
	#define PRODUCT_VERSION "Development Build #" str(BUILD_NUMBER)
#endif

#endif // VERSION_H
