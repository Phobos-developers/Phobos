#include "Body.h"
#include <TechnoClass.h>
#include <Ext/Scenario/Body.h>

DEFINE_HOOK(0x535005, ScenarioClass_LoadSide_SetEVAIndex, 0x6)
{
	if (ScenarioExt::Global()->EVAIndex != -2)
	{
		VoxClass::EVAIndex = ScenarioExt::Global()->EVAIndex;
	}
	// I don't know why, but if you don't do it this way, it might not work.
	else if (SessionClass::Instance.IsCampaign())
	{
		if (const auto pHouse = HouseClass::CurrentPlayer)
		{
			const int EVAIndex = HouseTypeExt::Fetch(pHouse->Type)->EVATag;

			if (EVAIndex != -2)
				VoxClass::EVAIndex = EVAIndex;
		}
	}

	return 0;
}

DEFINE_HOOK(0x68AD0C, ScenarioClass_ReadMap_SetEVAIndex, 0x7)
{
	if (const auto pHouse = HouseClass::CurrentPlayer)
	{
		const int EVAIndex = HouseTypeExt::Fetch(pHouse->Type)->EVATag;

		if (EVAIndex != -2)
			VoxClass::EVAIndex = EVAIndex;
	}

	return 0;
}

DEFINE_HOOK(0x707DCF, TechnoClass_GetCrew_NationalOverride, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	if (!pThis)
		return 0;

	HouseClass* pHouse = pThis->Owner;

	if (!pHouse)
		return 0;

	auto const pHouseTypeExt = HouseTypeExt::Fetch(pHouse->Type);

	if (pHouseTypeExt->Crew.isset())
		R->EAX(pHouseTypeExt->Crew.Get());

	return 0;
}

DEFINE_HOOK(0x442D1B, BuildingClass_Init_CountryBuildingVeteran, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	const auto pOwner = pThis->Owner;
	if (!pOwner)
		return 0;

	const auto pType = pThis->Type;
	if (!pType)
		return 0;

	const auto pCountryExt = HouseTypeExt::Fetch(pOwner->Type);

	const bool isDefense = pType->BuildCat == BuildCat::Combat;
	const auto& pVeteranList = isDefense
		? pCountryExt->VeteranDefenses
		: pCountryExt->VeteranBuildings;

	if (pVeteranList.Contains(pType))
		pThis->Veterancy.SetVeteran();

	return 0;
}
