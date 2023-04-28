#pragma once
#include <Windows.h>

class TechnoClass;
class TechnoTypeClass;
class FootClass;

class AresData
{
private:
	enum FunctionIndices
	{
		ConvertTypeToID = 0,
		SpawnSurvivorsID = 1,
	};

	enum Version
	{
		Unknown = -1,
		Ares30 = 0,
		Ares30p,
	};

	static void GetGameModulesBaseAddresses();

public:
	static HMODULE AresDllHmodule;
	static uintptr_t AresBaseAddress;
	static uintptr_t PhobosBaseAddress;

	// number of Ares functions we use
	static constexpr int AresFunctionCount = 2;
	// number of Ares versions we support
	static constexpr int AresVersionCount = 2;

	// timestamp bytes for each version
	static constexpr DWORD AresTimestampBytes[AresData::AresVersionCount] =
	{
		0x5fc37ef6,	// 3.0
		0x61daa114, // 3.0p
	};

	// offsets of function addresses for each version
	static constexpr DWORD AresFunctionOffsets[AresData::AresFunctionCount][AresData::AresVersionCount] =
	{
		// ConvertTypeTo
		{
			0x043650,
			0x044130,
		},
		// TechnoExt::SpawnSurvivors
		{
			0x0464C0,
			0x047030,
		},
	};

	// storage for absolute addresses of functions (module base + offset)
	static DWORD AresFunctionOffsetsFinal[AresData::AresFunctionCount];
	// numeric id of currently used version, zero-indexed, -1 is unknown or missing
	static int AresVersionId;
	// is Ares detected and version known?
	static bool CanUseAres;

	static void Init();
	static void UnInit();

	// here be known Ares functions
	static bool ConvertTypeTo(TechnoClass* pFoot, TechnoTypeClass* pConvertTo);
	static void SpawnSurvivors(FootClass* const pThis, TechnoClass* const pKiller, const bool Select, const bool IgnoreDefenses);
};
