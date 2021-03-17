#pragma once
#include <BuildingTypeClass.h>
#include <MessageListClass.h>
#include <MapClass.h>
#include <ObjectClass.h>

#include "Commands.h"
#include <Misc/Debug.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

class NextIdleHarvesterCommandClass : public PhobosCommandClass
{
public:
	// CommandClass
	virtual const char* GetName() const override
	{
		return "Next Idle Harvester";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return L"Next Idle Harvester";
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return L"Selection";
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return L"Select the next harvester that is idle (not harvesting).";
	}

	virtual void Execute(DWORD dwUnk) const override
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

		do {
			if (auto pTechno = abstract_cast<TechnoClass*>(pNextObject)) {
				if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType())) {
					if (pTypeExt->IsCountedAsHarvester() && !TechnoExt::IsHarvesting(pTechno)) {
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
	}
};
