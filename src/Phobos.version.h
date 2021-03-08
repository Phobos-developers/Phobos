#ifndef VERSION_H
#define VERSION_H

#define wstr(x) wstr_(x)
#define wstr_(x) L ## #x
#define str(x) str_(x)
#define str_(x) #x

// Latest release build
#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_REVISION 0

// Latest devbuild
#define BUILD_NUMBER 5

// Nightly defines GIT_COMMIT and GIT_BRANCH in GH Actions

#ifdef IS_RELEASE_VER // Release build metadata
	#define SAVEGAME_ID ((VERSION_MAJOR << 24) | (VERSION_MINOR << 12) | (VERSION_REVISION))
	#define FILE_DESCRIPTION "Phobos, Ares-compatible YR engine extension"
	#define FILE_VERSION_STR str(VERSION_MAJOR) "." str(VERSION_MINOR) "." str(VERSION_REVISION)
	#define FILE_VERSION VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION
	#define PRODUCT_VERSION "Release build"
#elif defined(GIT_COMMIT) // Nightly devbuild metadata
	#define STR_GIT_COMMIT str(GIT_COMMIT)
	#define STR_GIT_BRANCH str(GIT_BRANCH)

	#define SAVEGAME_ID ((BUILD_NUMBER << 24) | (BUILD_NUMBER << 12) | (BUILD_NUMBER))
	#define FILE_DESCRIPTION "Unstable nightly devbuild of Phobos engine extension"
	#define FILE_VERSION_STR "Commit " STR_GIT_COMMIT
	#define FILE_VERSION 0
	#define PRODUCT_VERSION STR_GIT_COMMIT " @ " STR_GIT_BRANCH
#else // Regular devbuild metadata
	#define SAVEGAME_ID ((BUILD_NUMBER << 24) | (BUILD_NUMBER << 12) | (BUILD_NUMBER))
	#define FILE_DESCRIPTION "Development build of Phobos engine extension"
	#define FILE_VERSION_STR "Build #" str(BUILD_NUMBER)
	#define FILE_VERSION 0,0,0,BUILD_NUMBER
	#define PRODUCT_VERSION "Development Build #" str(BUILD_NUMBER)
#endif

#endif // VERSION_H
