#pragma once
#include <Helpers\Macro.h>
// no more than 8 characters
#define PATCH_SECTION_NAME ".patch"

struct patch_decl {
	unsigned int offset;
	unsigned int size;
	byte* pData;
};

#pragma section(PATCH_SECTION_NAME, read, write	)
namespace definePach {};

// Just an example patch that allows you to disable the _YR_CmdLineParse HOOK for Syringe

// DEFINE_PATCH( 
// /* Offset */ 0x52F639,
// /*   Data */ 0x33, 0xDB, 0x83, 0xFF, 0x01
// );

#define DEFINE_PATCH(offset, ...) \
namespace definePach { \
namespace _dp_ ## offset { \
	byte _pd[] = {__VA_ARGS__};\
	__declspec(allocate(PATCH_SECTION_NAME)) patch_decl _ph = {offset, sizeof(_pd), _pd};\
}};

