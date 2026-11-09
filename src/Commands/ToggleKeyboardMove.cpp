#include "ToggleKeyboardMove.h"

#include <Utilities/GeneralUtils.h>

const char* ToggleKeyboardMove::GetName() const
{
	return "Toggle Keyboard Move Units";
}

const wchar_t* ToggleKeyboardMove::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TOGGLE_KEYBOARD_MOVE", L"Toggle Keyboard Move Units");
}

const wchar_t* ToggleKeyboardMove::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* ToggleKeyboardMove::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TOGGLE_KEYBOARD_MOVE_DESC", L"Toggle keyboard whether can move selected units");
}

void ToggleKeyboardMove::Execute(WWKey eInput) const
{
	Phobos::Config::ToggleKeyboardMove = !Phobos::Config::ToggleKeyboardMove;
}