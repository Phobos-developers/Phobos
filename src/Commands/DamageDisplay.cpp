#include "DamageDisplay.h"

#include <MessageListClass.h>
#include <Utilities/GeneralUtils.h>

const char* DamageDisplayCommandClass::GetName() const
{
	return "Display Damage Numbers";
}

const wchar_t* DamageDisplayCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISPLAY_DAMAGE", L"Display Damage Dealt");
}

const wchar_t* DamageDisplayCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* DamageDisplayCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISPLAY_DAMAGE_DESC", L"Display exact number of damage dealt to units & buildings on them.");
}

void DamageDisplayCommandClass::Execute(WWKey eInput) const
{
	Phobos::DisplayDamageNumbers = !Phobos::DisplayDamageNumbers;
}
