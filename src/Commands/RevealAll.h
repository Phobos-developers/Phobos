#pragma once
#include "Commands.h"

#include <Utilities/GeneralUtils.h>

#include <HouseClass.h>
#include <RadarClass.h>

class RevealAllCommandClass : public PhobosCommandClass
{
public:

	virtual const char* GetName() const override
	{
		return "Reveal All";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_REVEAL_ALL", L"Reveal All");
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_REVEAL_ALL_DESC", L"Reveal the whole map.");
	}

	virtual void Execute(WWKey eInput) const override
	{
		if (this->CheckDebugDeactivated())
			return;

		if (!HouseClass::CurrentPlayer->Visionary)
		{
			HouseClass::CurrentPlayer->Visionary = 1;
			MapClass::Instance->CellIteratorReset();
			for (auto i = MapClass::Instance->CellIteratorNext(); i; i = MapClass::Instance->CellIteratorNext())
				RadarClass::Instance->MapCell(&i->MapCoords, HouseClass::CurrentPlayer);

			GScreenClass::Instance->MarkNeedsRedraw(1);
		}
	}
};
