#include "Body.h"

#include <AITriggerTypeClass.h>
#include <Ext/House/Body.h>

DEFINE_HOOK(0x687C9B, ReadScenarioINI_AITeamSelector_PreloadValidTriggers, 0x7)
{
	bool ignoreGlobalAITriggers = ScenarioClass::Instance->IgnoreGlobalAITriggers;

	// For each house save a list with only AI Triggers that can be used
	for (HouseClass* pHouse : HouseClass::Array)
	{
		auto pHouseExt = HouseExt::ExtMap.Find(pHouse);
		pHouseExt->AITriggers_ValidList.clear();
		int houseIdx = pHouse->ArrayIndex;
		int sideIdx = pHouse->SideIndex + 1;

		for (int i = 0; i < AITriggerTypeClass::Array.Count; i++)
		{
			auto pTrigger = AITriggerTypeClass::Array.GetItem(i);
			if (!pTrigger || ignoreGlobalAITriggers == pTrigger->IsGlobal || !pTrigger->Team1)
				continue;

			int triggerHouse = pTrigger->HouseIndex;
			int triggerSide = pTrigger->SideIndex;

			// The trigger must be compatible with the owner
			if ((triggerHouse == -1 || houseIdx == triggerHouse) && (triggerSide == 0 || sideIdx == triggerSide))
				pHouseExt->AITriggers_ValidList.push_back(i);
		}

		Debug::Log("House %d [%s] could use %d AI triggers in this map.\n", pHouse->ArrayIndex, pHouse->Type->ID, pHouseExt->AITriggers_ValidList.size());
	}

	return 0;
}
