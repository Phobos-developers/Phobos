#pragma once

#include "Commands.h"
#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>

class RepeatLastCombatCommandClass : public PhobosCommandClass
{
public:

	static int LastBuildingID;

	virtual const char* GetName() const override
	{
		return "RepeatLastCombat";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_REPEAT_LAST_COMBAT", L"Repeat Last Combat");
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return StringTable::LoadString("TXT_INTERFACE");
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_REPEAT_LAST_COMBAT_DESC", L"Repeat last combat you built if it is buildable.");
	}

	virtual void Execute(DWORD dwUnk) const override
	{
		if (LastBuildingID != -1)
		{
			GeneralUtils::ProduceBuilding(HouseClass::Player, LastBuildingID);
		}
	}
};
