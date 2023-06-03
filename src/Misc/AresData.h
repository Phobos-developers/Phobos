#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <Windows.h>
#include <ASMMacros.h>

class TechnoClass;
class TechnoTypeClass;
class FootClass;
class HouseClass;
class BuildingTypeClass;

class AresData
{
public:
	enum class Version
	{
		Unknown = -1,
		Ares30 = 0,
		Ares30p,
	};

private:
	// timestamp bytes for each version
	static const std::unordered_map<DWORD, Version> AresTimestampBytes;
	// storage for absolute addresses of functions (module base + offset)
	static std::unordered_map<std::string, DWORD> AresFunctionOffsetsFinal;

	static void GetGameModulesBaseAddresses();

public:
	static HMODULE AresDllHmodule;
	static uintptr_t AresBaseAddress;
	static uintptr_t PhobosBaseAddress;

	// offsets of function addresses for each version
	static const std::unordered_map<std::string, std::vector<DWORD>> AresFunctionOffsets;
	// numeric id of currently used version, zero-indexed, -1 is unknown or missing
	static Version AresVersion;
	// is Ares detected and version known?
	static bool CanUseAres;

	static void Init();

#define JMP_ARES(name) const auto x = AresFunctionOffsetsFinal[name]; JMP(x)
#define JMP_THIS_ARES(name) const auto x = AresFunctionOffsetsFinal[name]; JMP_THIS(x)

	// here be known Ares functions
	static bool ConvertTypeTo(TechnoClass* pFoot, TechnoTypeClass* pConvertTo)
	{
		JMP_ARES("ConvertTypeTo");
	}

	static void SpawnSurvivors(FootClass* const pThis, TechnoClass* const pKiller, const bool Select, const bool IgnoreDefenses)
	{
		JMP_ARES("SpawnSurvivors");
	}

	static int HasFactory(int buffer, HouseClass* pOwner, TechnoTypeClass* pType, bool skipAircraft, bool requirePower, bool checkCanBuild, bool unknown)
	{
		JMP_ARES("HasFactory");
	}

	static bool CanBeBuiltAt(DWORD pTechnoTypeExt, BuildingTypeClass* pBuildingType)
	{
		JMP_THIS_ARES("CanBeBuiltAt");
	}

#undef JMP_ARES
#undef JMP_THIS_ARES
};
