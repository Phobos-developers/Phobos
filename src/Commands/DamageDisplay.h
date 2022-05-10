#pragma once
#include <BuildingTypeClass.h>
#include <MessageListClass.h>

#include "Commands.h"
#include <Utilities/Debug.h>
#include <Utilities/GeneralUtils.h>

// Display damage strings command
class DamageDisplayCommandClass : public PhobosCommandClass
{
public:
	virtual const char* GetName() const override
	{
		return "Display Damage Numbers";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DISPLAY_DAMAGE", L"Display Damage Dealt");
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DISPLAY_DAMAGE_DESC", L"Display exact number of damage dealt to units & buildings on them.");
	}

	virtual void Execute(WWKey eInput) const override
	{
		if (this->CheckDebugDeactivated())
			return;

		Phobos::Debug_DisplayDamageNumbers = !Phobos::Debug_DisplayDamageNumbers;
	}
};
