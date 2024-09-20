#pragma once

#include "Commands.h"

#include <Utilities/GeneralUtils.h>
#include <Misc/TacticalButtons.h>

// Super Weapon Sidebar Keyboard shortcut command class
constexpr const char* ShortcutNames[11] = { "SW Sidebar Display",
	"SW Sidebar Shortcuts Num 01", "SW Sidebar Shortcuts Num 02", "SW Sidebar Shortcuts Num 03",
	"SW Sidebar Shortcuts Num 04", "SW Sidebar Shortcuts Num 05", "SW Sidebar Shortcuts Num 06",
	"SW Sidebar Shortcuts Num 07", "SW Sidebar Shortcuts Num 08", "SW Sidebar Shortcuts Num 09",
	"SW Sidebar Shortcuts Num 10" };
constexpr const char* ShortcutUINamesTXT[11] = { "TXT_EX_SW_SWITCH",
	"TXT_EX_SW_BUTTON_01", "TXT_EX_SW_BUTTON_02", "TXT_EX_SW_BUTTON_03", "TXT_EX_SW_BUTTON_04", "TXT_EX_SW_BUTTON_05",
	"TXT_EX_SW_BUTTON_06", "TXT_EX_SW_BUTTON_07", "TXT_EX_SW_BUTTON_08", "TXT_EX_SW_BUTTON_09", "TXT_EX_SW_BUTTON_10" };
constexpr const wchar_t* ShortcutUINames[11] = { L"SW sidebar display",
	L"Quick Select SW 01", L"Quick Select SW 02", L"Quick Select SW 03", L"Quick Select SW 04", L"Quick Select SW 05",
	L"Quick Select SW 06", L"Quick Select SW 07", L"Quick Select SW 08", L"Quick Select SW 09", L"Quick Select SW 10" };
constexpr const char* ShortcutUIDescriptionsTXT[11] = { "TXT_EX_SW_SWITCH_DESC",
	"TXT_EX_SW_BUTTON_01_DESC", "TXT_EX_SW_BUTTON_02_DESC", "TXT_EX_SW_BUTTON_03_DESC",
	"TXT_EX_SW_BUTTON_04_DESC", "TXT_EX_SW_BUTTON_05_DESC", "TXT_EX_SW_BUTTON_06_DESC",
	"TXT_EX_SW_BUTTON_07_DESC", "TXT_EX_SW_BUTTON_08_DESC", "TXT_EX_SW_BUTTON_09_DESC",
	"TXT_EX_SW_BUTTON_10_DESC" };
constexpr const wchar_t* ShortcutUIDescriptions[11] = {
	L"Switch between visible/invisible modes for exclusive SW sidebar",
	L"Select No.01 SW in left sidebar", L"Select No.02 SW in left sidebar", L"Select No.03 SW in left sidebar",
	L"Select No.04 SW in left sidebar", L"Select No.05 SW in left sidebar", L"Select No.06 SW in left sidebar",
	L"Select No.07 SW in left sidebar", L"Select No.08 SW in left sidebar", L"Select No.09 SW in left sidebar",
	L"Select No.10 SW in left sidebar"};

template<size_t KeyIndex>
class SWShortcutsCommandClass : public CommandClass
{
public:
	// CommandClass
	virtual const char* GetName() const override;
	virtual const wchar_t* GetUIName() const override;
	virtual const wchar_t* GetUICategory() const override;
	virtual const wchar_t* GetUIDescription() const override;
	virtual void Execute(WWKey eInput) const override;
};

template<size_t KeyIndex>
inline const char* SWShortcutsCommandClass<KeyIndex>::GetName() const
{
	return ShortcutNames[KeyIndex];
}

template<size_t KeyIndex>
inline const wchar_t* SWShortcutsCommandClass<KeyIndex>::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing(ShortcutUINamesTXT[KeyIndex], ShortcutUINames[KeyIndex]);
}

template<size_t KeyIndex>
inline const wchar_t* SWShortcutsCommandClass<KeyIndex>::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

template<size_t KeyIndex>
inline const wchar_t* SWShortcutsCommandClass<KeyIndex>::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing(ShortcutUIDescriptionsTXT[KeyIndex], ShortcutUIDescriptions[KeyIndex]);
}

template<size_t KeyIndex>
inline void SWShortcutsCommandClass<KeyIndex>::Execute(WWKey eInput) const
{
	if (KeyIndex > 0)
	{
		TacticalButtonsClass::Instance.KeyboardCall = true;
		TacticalButtonsClass::Instance.SWSidebarTrigger(KeyIndex);
		return;
	}

	TacticalButtonsClass::Instance.SWSidebarSwitch();
}
