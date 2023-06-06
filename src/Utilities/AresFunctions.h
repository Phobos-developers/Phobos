#pragma once
#include "AresHelper.h"

#include <nameof/nameof.h>
#include <ASMMacros.h>
#include <functional>

class TechnoClass;
class TechnoTypeClass;
class FootClass;
class HouseClass;
class BuildingTypeClass;

class AresTechnoTypeExt
{
public:
	__declspec(noinline) bool __thiscall CanBeBuiltAt(BuildingTypeClass* pBuildingType) { return 0; }
};


class AresFunctions
{
public:
#define CALL_ARES(name, ...) std::invoke(reinterpret_cast<decltype(name)*>(AresHelper::AresFunctionOffsetsFinal[ARES_FUN(name)]), __VA_ARGS__)
#define THIS_CALL_ARES(name, ...) std::invoke(*reinterpret_cast<decltype(name)*>(AresHelper::AresFunctionOffsetsFinal[ARES_FUN_M(name)]), __VA_ARGS__)

	// ???
	static bool ConvertTypeTo(TechnoClass* pFoot, TechnoTypeClass* pConvertTo)
	{
		return CALL_ARES(ConvertTypeTo, pFoot, pConvertTo);
	}

	// TechnoExt
	static void SpawnSurvivors(FootClass* const pThis, TechnoClass* const pKiller, const bool Select, const bool IgnoreDefenses)
	{
		CALL_ARES(SpawnSurvivors, pThis, pKiller, Select, IgnoreDefenses);
	}

	// HouseExt
	static int HasFactory(int buffer, HouseClass* pOwner, TechnoTypeClass* pType, bool skipAircraft, bool requirePower, bool checkCanBuild, bool unknown)
	{
		return CALL_ARES(HasFactory, buffer, pOwner, pType, skipAircraft, requirePower, checkCanBuild, unknown);
	}

	// TechnoTypeExt
	static bool CanBeBuiltAt(AresTechnoTypeExt* pExt, BuildingTypeClass* pBuildingType)
	{
		return THIS_CALL_ARES(&AresTechnoTypeExt::CanBeBuiltAt, pExt, pBuildingType);
	}

#undef CALL_ARES
#undef THIS_CALL_ARES
};
