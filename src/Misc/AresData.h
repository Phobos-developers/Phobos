#pragma once
#include <Windows.h>

class TechnoClass;
class TechnoTypeClass;
struct AresData
{
	static HMODULE AresDllHmodule;
	static uintptr_t AresBaseAddress;

	// timestamp bytes of Ares 3.0
	static const DWORD Ares30IdBytes = 0x5fc37ef6;
	// timestamp bytes of Ares 3.0p1
	static const DWORD Ares30p1IdBytes = 0x61daa114;
	// number of Ares functions we use
	static const int AresFunctionNumber = 1;
	// number of Ares versions we support
	static const int AresVersionsNumber = 2;
	// offsets of function addresses for each version
	static const DWORD AresFunctionOffsets[AresData::AresVersionsNumber * AresData::AresFunctionNumber];
	// storage for absolute addresses of functions (module base + offset)
	static DWORD AresData::AresFunctionOffestsFinal[AresData::AresFunctionNumber];

	// Numeric id of currently used version, zero-indexed, -1 is unknown or missing
	static int AresVersionId;
	static bool CanUseAres;

	// Here be known Ares functions
	static void __stdcall CallHandleConvert(TechnoClass* pTechno, TechnoTypeClass* pConvertTo);

	static void Init();
	static void UnInit();
};
