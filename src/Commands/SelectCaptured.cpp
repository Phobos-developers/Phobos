#include "SelectCaptured.h"

#include <BuildingTypeClass.h>
#include <MessageListClass.h>
#include <MapClass.h>
#include <ObjectClass.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

const char* SelectCapturedCommandClass::GetName() const
{
	return "Select Captured Units";
}

const wchar_t* SelectCapturedCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SELECT_CAPTURED", L"Select Captured Units");
}

const wchar_t* SelectCapturedCommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* SelectCapturedCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SELECT_CAPTURED_DESC", L"Select the captured units in the current screen.");
}

void SelectCapturedCommandClass::Execute(WWKey eInput) const
{
	// Debug::Log("[Phobos] Dummy command runs.\n");
	// MessageListClass::Instance->PrintMessage(L"[Phobos] Dummy command rums");

	MapClass::Instance->SetTogglePowerMode(0);
	MapClass::Instance->SetWaypointMode(0, false);
	MapClass::Instance->SetRepairMode(0);
	MapClass::Instance->SetSellMode(0);

	auto pObjectToSelect = MapClass::Instance->NextObject(
		ObjectClass::CurrentObjects->Count ? ObjectClass::CurrentObjects->GetItem(0) : nullptr);

	bool idleHarvestersPresent = false;
	auto pNextObject = pObjectToSelect;

	do
	{
		if (auto pTechno = abstract_cast<TechnoClass*>(pNextObject))
		{
			if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType()))
			{
				if (pTypeExt->Harvester_Counted.Get() && !TechnoExt::IsHarvesting(pTechno))
				{
					pObjectToSelect = pNextObject;
					idleHarvestersPresent = true;
					break;
				}
			}
		}

		pNextObject = MapClass::Instance->NextObject(pNextObject);
	}
	while (pNextObject != pObjectToSelect);

	if (idleHarvestersPresent && pObjectToSelect)
	{
		MapClass::UnselectAll();
		pObjectToSelect->Select();
		MapClass::Instance->CenterMap();
		MapClass::Instance->MarkNeedsRedraw(1);
	}
	else
	{
		MessageListClass::Instance->PrintMessage(StringTable::LoadString("MSG:NothingSelected"), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
	}
}
