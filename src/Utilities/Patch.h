#pragma once
#include <windows.h>

// no more than 8 characters
#define PATCH_SECTION_NAME ".patch"
#pragma section(PATCH_SECTION_NAME, read)

#pragma pack(push, 1)
#pragma warning(push)
#pragma warning( disable : 4324)
struct __declspec(novtable)
Patch
{
	unsigned int offset;
	unsigned int size;
	byte* pData;

	static void ApplyStatic();
	void Apply();
};
#pragma warning(pop)
#pragma pack(pop)
