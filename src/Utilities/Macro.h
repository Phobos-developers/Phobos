#pragma once
#include <Helpers\Macro.h>
// no more than 8 characters
#define PATCH_SECTION_NAME ".patch"

#pragma section(PATCH_SECTION_NAME, read, write	)
namespace definePach {};

// Just an example patch that allows you to disable the _YR_CmdLineParse HOOK for Syringe

// DEFINE_PATCH( 
// /* Offset */ 0x52F639,
// /*   Data */ 0x33, 0xDB, 0x83, 0xFF, 0x01
// );

struct patch_decl {
	unsigned int offset;
	unsigned int size;
	byte* pData;
};

#define declpatch(offset, size, patch) __declspec(allocate(PATCH_SECTION_NAME)) patch_decl _ph = {offset, size, (byte*)patch};

#define DEFINE_PATCH(offset, ...) \
namespace definePach { \
namespace _dp_ ## offset { \
	byte _pd[] = {__VA_ARGS__};\
	declpatch(offset, sizeof(_pd), _pd);\
}};

#pragma pack(push, 1)
#pragma warning(push)
#pragma warning( disable : 4324)
struct ljmp_decl {
	byte command;
	DWORD to;
};
#pragma warning(pop)
#pragma pack(pop)

#define DEFINE_LJMP(from, to) \
namespace definePach { \
namespace _dp_ ## from { \
	ljmp_decl _pd = {0xE9, to-from-5};\
	declpatch(from, 5, &_pd);\
}};
