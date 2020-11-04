#ifndef VERSION_H
#define VERSION_H

// define this to switch to release version
//#define IS_RELEASE_VER

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_REVISION 0

#define wstr(x) wstr_(x)
#define wstr_(x) L ## #x
#define str(x) str_(x)
#define str_(x) #x

// Alternative version display name for release versions
#ifdef IS_RELEASE_VER
	#define File_Description "Ares-compatible YR engine extension"
	#define VERSION_STR str(VERSION_MAJOR) "." str(VERSION_MINOR) "." str(VERSION_REVISION)
	#define VERSION_WSTR wstr(VERSION_MAJOR) L"." wstr(VERSION_MINOR) L"." wstr(VERSION_REVISION)
	#define FILE_VERSION VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION
	
    #define PRODUCT_MAJOR VERSION_MAJOR
    #define PRODUCT_MINOR VERSION_MINOR
    #define PRODUCT_REVISION VERSION_REVISION
    #define PRODUCT_STR VERSION_STR
    #define DISPLAY_STR PRODUCT_STR
#else
	#define File_Description "Unstable build"
	#define FILE_VERSION 0,0,VERSION_MAJOR
	#define VERSION_MINOR 0
	#define VERSION_REVISION 0
	#define VERSION_STR str(VERSION_MAJOR)
	#define VERSION_WSTR wstr(VERSION_MAJOR)

    #define PRODUCT_MAJOR VERSION_MAJOR
    #define PRODUCT_MINOR 0
    #define PRODUCT_REVISION 0
    #define PRODUCT_STR VERSION_STR
    #define DISPLAY_STR VERSION_STR
#endif // IS_RELEASE_VER


#endif
