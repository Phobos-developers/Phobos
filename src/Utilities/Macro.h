#pragma once
#include <Helpers/Macro.h>
#include <ASMMacros.h>

// no more than 8 characters
#define PATCH_SECTION_NAME ".patch"

#pragma section(PATCH_SECTION_NAME, read, write	)
namespace definePatch {};

// Just an example patch that allows you to disable the _YR_CmdLineParse HOOK for Syringe
// DEFINE_PATCH( 
// /* Offset */ 0x52F639,
// /*   Data */ 0x33, 0xDB, 0x83, 0xFF, 0x01
// );

// declpatch
struct patch_decl {
	unsigned int offset;
	unsigned int size;
	byte* pData;
};

#define declpatch(offset, size, patch) __declspec(allocate(PATCH_SECTION_NAME)) patch_decl _ph = {offset, size, (byte*)patch};

#define DEFINE_PATCH(offset, ...) \
namespace definePatch { \
namespace _dp_ ## offset { \
	byte _pd[] = {__VA_ARGS__};\
	declpatch(offset, sizeof(_pd), _pd);\
}};

#define DEFINE_VTABLE_PATCH(offset, to) \
namespace definePatch { \
namespace _dp_ ## offset { \
	DWORD _pd = reinterpret_cast<DWORD>(to);\
	declpatch(offset, sizeof(_pd), &_pd);\
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

#define NAKED __declspec(naked)

// LJMP
#define LJMP_LETTER  0xE9

#define DEFINE_LJMP(from, to) \
namespace definePatch { \
namespace _djmp_ ## from { \
	ljmp_decl _pd = {LJMP_LETTER, to-from-5};\
	declpatch(from, 5, &_pd);\
}};

#define DEFINE_POINTER_LJMP(from, to) \
DEFINE_LJMP(from, reinterpret_cast<DWORD>(to));

// DEFINE_LJMP_NAKED(0x6F64A9, Demo)
// {
// 	MessageBoxA(0, "", "", 0);
// 	JMP(0x6F6AB6);
// }

#define DEFINE_NAKED_LJMP(from, name) \
void name(); \
DEFINE_POINTER_LJMP(from, name) \
NAKED void name()

// CALL
#define CALL_LETTER 0xE8

#define DEFINE_CALL(from, to) \
namespace definePatch { \
namespace _djmp_ ## from { \
	ljmp_decl _pd = {CALL_LETTER, to-from-5};\
	declpatch(from, 5, &_pd);\
}};

#define DEFINE_POINTER_CALL(from, to) \
DEFINE_CALL(from, reinterpret_cast<DWORD>(to));

#define DEFINE_NAKED_CALL(from, name) \
void name(); \
DEFINE_POINTER_CALL(from, name) \
NAKED void name()
