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

		int parentCountryTypeIdx = pHouse->Type->FindParentCountryIndex(); // ParentCountry can change the House in a SP map
		int houseTypeIdx = parentCountryTypeIdx >= 0 ? parentCountryTypeIdx : pHouse->Type->ArrayIndex; // Indexes in AITriggers section are 1-based
		int houseIdx = pHouse->ArrayIndex;

		int parentCountrySideTypeIdx = parentCountryTypeIdx >= 0 ? pHouse->Type->FindParentCountry()->SideIndex : pHouse->Type->SideIndex;
		int sideTypeIdx = parentCountrySideTypeIdx >= 0 ? parentCountrySideTypeIdx + 1 : pHouse->Type->SideIndex + 1; // Side indexes in AITriggers section are 1-based
		int sideIdx = pHouse->SideIndex + 1; // Side indexes in AITriggers section are 1-based -> unused variable!!


		for (int i = 0; i < AITriggerTypeClass::Array.Count; i++)
		{
			auto pTrigger = AITriggerTypeClass::Array.GetItem(i);
			if (!pTrigger || ignoreGlobalAITriggers == pTrigger->IsGlobal || !pTrigger->Team1)
				continue;

			int triggerHouse = pTrigger->HouseIndex;
			int triggerSide = pTrigger->SideIndex;

			// The trigger must be compatible with the owner
			//if ((triggerHouse == -1 || houseIdx == triggerHouse) && (triggerSide == 0 || sideIdx == triggerSide))
			if ((triggerHouse == -1 || houseTypeIdx == triggerHouse) && (triggerSide == 0 || sideTypeIdx == triggerSide))
				pHouseExt->AITriggers_ValidList.push_back(i);
		}

		Debug::Log("House %d [%s] could use %d AI triggers in this map.\n", pHouse->ArrayIndex, pHouse->Type->ID, pHouseExt->AITriggers_ValidList.size());
	}

	return 0;
}
