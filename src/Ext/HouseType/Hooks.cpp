#include "Body.h"

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

DEFINE_HOOK_AGAIN(0x70B1F2, TechnoClass_RevealHouses, 0x6)	// TechnoClass::vt_entry_48C
DEFINE_HOOK_AGAIN(0x70B15A, TechnoClass_RevealHouses, 0x6)	// TechnoClass::UpdateSight
DEFINE_HOOK(0x70AF22, TechnoClass_RevealHouses, 0x6)		// TechnoClass::See
{
	const DWORD address = R->Origin();
	auto const pTechno = address == 0x70B1F2 ? R->ECX<TechnoClass*>() : R->ESI<TechnoClass*>();

	auto const pPlayer = HouseClass::CurrentPlayer;
	auto const pHouse = pTechno->Owner;
	const bool canShow = pPlayer ? EnumFunctions::CanTargetHouse(HouseTypeExt::ExtMap.Find(pHouse->Type)->RevealHouses, pHouse, pPlayer) : false;

	switch (address)
	{
	case 0x70B1F2:
		R->ESI(canShow ? pPlayer : nullptr);
		break;
	case 0x70B15A:
		R->EDX(canShow ? pPlayer : nullptr);
		return 0x70B160;
	default:
		R->EDX(canShow ? pPlayer : nullptr);
		return 0x70AF28;
	}

	return 0;
}
