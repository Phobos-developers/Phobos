#include "ToggleMessageList.h"

#include <Utilities/GeneralUtils.h>
#include <Misc/MessageColumn.h>

const char* ToggleMessageListCommandClass::GetName() const
{
	return "Toggle Message Label";
}

const wchar_t* ToggleMessageListCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TOGGLE_MESSAGE", L"Toggle Message Label");
}

const wchar_t* ToggleMessageListCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ToggleMessageListCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TOGGLE_MESSAGE_DESC", L"Toggle message label in the middle of the screen.");
}

void ToggleMessageListCommandClass::Execute(WWKey eInput) const
{
	MessageColumnClass::Instance.Toggle();
}
