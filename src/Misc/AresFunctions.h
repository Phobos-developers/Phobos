#pragma once
#include "AresData.h"

#include <ASMMacros.h>

namespace AresFunctions
{
#define JMP_ARES(name) const auto x = AresData::AresFunctionOffsetsFinal[name]; JMP(x)
#define JMP_THIS_ARES(name) const auto x = AresData::AresFunctionOffsetsFinal[name]; JMP_THIS(x)

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
}
