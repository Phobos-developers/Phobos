#include "Body.h"

#include <AITriggerTypeClass.h>
#include <Ext/House/Body.h>

DEFINE_HOOK(0x687C9B, ReadScenarioINI_AITeamSelector_PreloadValidTriggers, 0x7)
{
	if (!RulesExt::Global()->NewTeamsSelector)
		return 0;

	bool ignoreGlobalAITriggers = ScenarioClass::Instance->IgnoreGlobalAITriggers;

	struct HousePreloadInfo
	{
		HouseExt::ExtData* pExt;
		int HouseTypeIdx;
		int SideTypeIdx;
		AIDifficulty Difficulty;
	};

	std::vector<HousePreloadInfo> houses;
	houses.reserve(HouseClass::Array.Count);

	// Pre-cache house metadata once
	for (HouseClass* pHouse : HouseClass::Array)
	{
		auto pHouseExt = HouseExt::ExtMap.Find(pHouse);
		if (!pHouseExt)
			continue;

		pHouseExt->AITriggers_ValidList.clear();

		int parentCountryTypeIdx = pHouse->Type->FindParentCountryIndex();
		int houseTypeIdx = parentCountryTypeIdx >= 0 ? parentCountryTypeIdx : pHouse->Type->ArrayIndex;

		int parentCountrySideTypeIdx = parentCountryTypeIdx >= 0 ? pHouse->Type->FindParentCountry()->SideIndex : pHouse->Type->SideIndex;
		int sideTypeIdx = parentCountrySideTypeIdx >= 0 ? parentCountrySideTypeIdx + 1 : pHouse->Type->SideIndex + 1;

		houses.push_back({ pHouseExt, houseTypeIdx, sideTypeIdx, pHouse->AIDifficulty });
	}

	// Single pass over all triggers
	for (int i = 0; i < AITriggerTypeClass::Array.Count; i++)
	{
		auto pTrigger = AITriggerTypeClass::Array.GetItem(i);

		if (!pTrigger || (ignoreGlobalAITriggers && pTrigger->IsGlobal && !pTrigger->IsEnabled) || !pTrigger->Team1)
			continue;

		int triggerHouse = pTrigger->HouseIndex;
		int triggerSide = pTrigger->SideIndex;

		for (const auto& house : houses)
		{
			// Check difficulty compatibility
			if (((int)house.Difficulty == 0 && !pTrigger->Enabled_Hard)
				|| ((int)house.Difficulty == 1 && !pTrigger->Enabled_Normal)
				|| ((int)house.Difficulty == 2 && !pTrigger->Enabled_Easy))
			{
				continue;
			}

			// Check house and side compatibility
			if ((triggerHouse == -1 || house.HouseTypeIdx == triggerHouse) && (triggerSide == 0 || house.SideTypeIdx == triggerSide))
			{
				house.pExt->AITriggers_ValidList.push_back(i);
			}
		}
	}

	for (HouseClass* pHouse : HouseClass::Array)
	{
		auto pHouseExt = HouseExt::ExtMap.Find(pHouse);
		if (pHouseExt)
		{
			Debug::Log("AITeamsSelector - The house %d [%s](%s) should be able to use %d AI triggers in this map.\n",
				pHouse->ArrayIndex, pHouse->Type->ID, pHouse->PlainName, pHouseExt->AITriggers_ValidList.size());
		}
	}

	return 0;
}
