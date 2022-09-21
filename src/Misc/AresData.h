#pragma once
#include <Windows.h>

class TechnoClass;
class TechnoTypeClass;
struct AresData
{
	static HMODULE AresDllHmodule;
	static uintptr_t AresBaseAddress;
	static int AresVersionId;
	static bool CanUseAres;

	static const DWORD Ares30Hash = 0;
	static const DWORD Ares30p1Hash = 1;
	static const DWORD AresFunctionOffsets[2];
	static DWORD AresData::AresFunctionOffestsFinal[1];

	// Known Ares functions
	static void __stdcall CallHandleConvert(TechnoClass* pTechno, TechnoTypeClass* pConvertTo);

	static void Init();
	static void UnInit();
};
