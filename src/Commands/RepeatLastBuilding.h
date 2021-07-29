#pragma once
#include <BuildingTypeClass.h>
#include <SidebarClass.h>
#include <NetworkEvents.h>
#include <Networking.h>
#include <HouseClass.h>
#include <FactoryClass.h>

#include "Commands.h"
#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>

class RepeatLastBuildingCommandClass : public PhobosCommandClass
{
public:

	static int LastBuildingID;

	virtual const char* GetName() const override
	{
		return "RepeatLastBuilding";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_REPEAT_LAST_BUILDING", L"Repeat Last Building");
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return StringTable::LoadString("TXT_INTERFACE");
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_REPEAT_LAST_BUILDING_DESC", L"Repeat last building you built if it is buildable.");
	}

	virtual void Execute(DWORD dwUnk) const override
	{
		if (LastBuildingID != -1)
		{
			if (auto pItem = ObjectTypeClass::GetTechnoType(AbstractType::BuildingType, LastBuildingID))
			{
				auto pPlayer = HouseClass::Player();
				if (pPlayer->CanBuild(pItem, true, true) == CanBuildResult::Buildable)
				{
					if (auto pFactory = pPlayer->GetPrimaryFactory(AbstractType::Building, false, BuildCat::DontCare))
					{
						if (pFactory->GetProgress())
							return;

						NetworkEvent Event;

						Event.FillEvent_ProduceAbandonSuspend(
							pPlayer->ArrayIndex, NetworkEvents::Produce, pItem->WhatAmI(), pItem->GetArrayIndex(), pItem->Naval
						);
						Networking::AddEvent(&Event);
						return;
					}
				}
			}
		}
	}
};
