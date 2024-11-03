#include "ToggleSWSidebar.h"
#include <HouseClass.h>

#include <Utilities/GeneralUtils.h>
#include <Ext/Sidebar/SWSidebar/SWSidebarClass.h>

const char* ToggleSWSidebar::GetName() const
{
	return "Toggle Super Weapon Sidebar";
}

const wchar_t* ToggleSWSidebar::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TOGGLE_SW_SIDEBAR", L"Toggle Super Weapon Sidebar");
}

const wchar_t* ToggleSWSidebar::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ToggleSWSidebar::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TOGGLE_SW_SIDEBAR_DESC", L"Toggle the Super Weapon Sidebar.");
}

void ToggleSWSidebar::Execute(WWKey eInput) const
{
	ToggleSWButtonClass::SwitchSidebar();

	if (SidebarExt::Global()->SWSidebar_Enable)
		MessageListClass::Instance->PrintMessage(GeneralUtils::LoadStringUnlessMissing("TXT_EX_SW_SIDEBAR_ENABLE", L"Super Weapon Sidebar Enabled."), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
	else
		MessageListClass::Instance->PrintMessage(GeneralUtils::LoadStringUnlessMissing("TXT_EX_SW_SIDEBAR_DISABLE", L"Super Weapon Sidebar Disabled."), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);

	if (SWSidebarClass::Global()->CurrentColumn)
		MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
}
