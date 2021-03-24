#pragma once
#include <Helpers\Macro.h>
#include <ASMMacros.h>

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
namespace _djmp_ ## from { \
	ljmp_decl _pd = {0xE9, reinterpret_cast<DWORD>(to)-from-5};\
	declpatch(from, 5, &_pd);\
}};

// DEFINE_LJMP_NAKED(0x6F64A9, Demo)
// {
// 	MessageBoxA(0, "", "", 0);
// 	JMP(0x6F6AB6);
// }

#define NAKED __declspec(naked)

#define DEFINE_LJMP_NAKED(from, name) \
void name(); \
DEFINE_LJMP(from, name) \
NAKED void name()
