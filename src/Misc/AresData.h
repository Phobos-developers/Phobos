#pragma once
#include <Windows.h>

class TechnoClass;
class TechnoTypeClass;
class FootClass;
class HouseClass;
class BuildingTypeClass;

class AresData
{
private:
	enum FunctionIndices
	{
		ConvertTypeToID = 0,
		SpawnSurvivorsID = 1,
		HasFactoryID = 2,
		CanBeBuiltAtID = 3,
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
	static constexpr int AresFunctionCount = 4;
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
		// HouseExt::HasFactory
		{
			0x0217C0,
			0x0217C0,
		},
		// TechnoTypeExt::CanBeBuiltAt
		{
			0x03E3B0,
			0x03E3B0,
		},
	};

	// storage for absolute addresses of functions (module base + offset)
	static DWORD AresFunctionOffsetsFinal[AresData::AresFunctionCount];
	// numeric id of currently used version, zero-indexed, -1 is unknown or missing
	static int AresVersionId;
	// is Ares detected and version known?
	static bool CanUseAres;

	static void Init();

	// here be known Ares functions
	static bool ConvertTypeTo(TechnoClass* pFoot, TechnoTypeClass* pConvertTo);
	static void SpawnSurvivors(FootClass* const pThis, TechnoClass* const pKiller, const bool Select, const bool IgnoreDefenses);
	static int HasFactory(int buffer, HouseClass* pOwner, TechnoTypeClass* pType, bool skipAircraft, bool requirePower, bool checkCanBuild, bool unknown);
	static bool CanBeBuiltAt(DWORD pTechnoTypeExt, BuildingTypeClass* pBuildingType);
};
