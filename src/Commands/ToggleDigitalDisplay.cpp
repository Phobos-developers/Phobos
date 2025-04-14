#include "ToggleDigitalDisplay.h"

#include <MessageListClass.h>
#include <Utilities/GeneralUtils.h>

const char* ToggleDigitalDisplayCommandClass::GetName() const
{
	return "Toggle Digital Display";
}

const wchar_t* ToggleDigitalDisplayCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DIGITAL_DISPLAY", L"Toggle Digital Display");
}

const wchar_t* ToggleDigitalDisplayCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ToggleDigitalDisplayCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DIGITAL_DISPLAY_DESC", L"Show/hide digital display of unit data.");
}

void ToggleDigitalDisplayCommandClass::Execute(WWKey eInput) const
{
	Phobos::Config::DigitalDisplay_Enable = !Phobos::Config::DigitalDisplay_Enable;
}
