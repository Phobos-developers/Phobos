#pragma once

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
			GeneralUtils::ProduceBuilding(HouseClass::Player, LastBuildingID);
		}
	}
};
