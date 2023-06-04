#pragma once
#include "AresData.h"

#include <ASMMacros.h>

class TechnoClass;
class TechnoTypeClass;
class FootClass;
class HouseClass;
class BuildingTypeClass;

#define ARES_FUN(name) std::string(NAMEOF(AresFunctions::name))

class AresFunctions
{
public:
#define JMP_ARES(name) const auto x = AresData::AresFunctionOffsetsFinal[ARES_FUN(name)]; JMP(x)
#define JMP_THIS_ARES(name) const auto x = AresData::AresFunctionOffsetsFinal[ARES_FUN(name)]; JMP_THIS(x)

	// ???
	static bool ConvertTypeTo(TechnoClass* pFoot, TechnoTypeClass* pConvertTo)
	{
		JMP_ARES(ConvertTypeTo);
	}

	// TechnoExt
	static void SpawnSurvivors(FootClass* const pThis, TechnoClass* const pKiller, const bool Select, const bool IgnoreDefenses)
	{
		JMP_ARES(SpawnSurvivors);
	}

	// HouseExt
	static int HasFactory(int buffer, HouseClass* pOwner, TechnoTypeClass* pType, bool skipAircraft, bool requirePower, bool checkCanBuild, bool unknown)
	{
		JMP_ARES(HasFactory);
	}

	// TechnoTypeExt
	static bool CanBeBuiltAt(DWORD pTechnoTypeExt, BuildingTypeClass* pBuildingType)
	{
		JMP_THIS_ARES(CanBeBuiltAt);
	}

#undef JMP_ARES
#undef JMP_THIS_ARES
};
