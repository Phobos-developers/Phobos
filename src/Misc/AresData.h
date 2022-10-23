#pragma once
#include <Windows.h>

class TechnoClass;
class TechnoTypeClass;
class FootClass;

struct AresData
{
	enum FunctionIndices
	{
		ConvertTypeToID,
	};

	enum Version
	{
		Unknown = -1,
		Ares30 = 0,
		Ares30p,
	};

	static HMODULE AresDllHmodule;
	static uintptr_t AresBaseAddress;

	// number of Ares functions we use
	static constexpr int AresFunctionCount = 1;
	// number of Ares versions we support
	static constexpr int AresVersionCount = 2;

	// timestamp bytes for each version
	static constexpr DWORD AresTimestampBytes[AresData::AresVersionCount] =
	{
		0x5fc37ef6,	// 3.0
		0x61daa114, // 3.0p
	};

	// offsets of function addresses for each version
	static constexpr DWORD AresFunctionOffsets[AresData::AresVersionCount * AresData::AresFunctionCount] =
	{
		0x043650, 0x044130,	// ConvertTypeTo
	};

	// storage for absolute addresses of functions (module base + offset)
	static DWORD AresFunctionOffsetsFinal[AresData::AresFunctionCount];
	// numeric id of currently used version, zero-indexed, -1 is unknown or missing
	static int AresVersionId;
	// is Ares detected and version known?
	static bool CanUseAres;

	// Here be known Ares functions
	static bool __stdcall ConvertTypeTo(TechnoClass* pFoot, TechnoTypeClass* pConvertTo);

	static void Init();
	static void UnInit();
};
