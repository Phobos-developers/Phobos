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
			const int EVAIndex = HouseTypeExt::ExtMap.Find(pHouse->Type)->EVATag;

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
		const int EVAIndex = HouseTypeExt::ExtMap.Find(pHouse->Type)->EVATag;

		if (EVAIndex != -2)
			VoxClass::EVAIndex = EVAIndex;
	}

	return 0;
}
