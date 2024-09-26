#include "ToggleSWSidebar.h"
#include <HouseClass.h>

#include <Utilities/GeneralUtils.h>
#include <Ext/Sidebar/Body.h>
#include <Ext/Sidebar/SWSidebar/SWSidebarClass.h>

const char* ToggleSWSidebar::GetName() const
{
	return "Toggle SuperWeapon Sidebar";
}

const wchar_t* ToggleSWSidebar::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TOGGLE_SW_SIDEBAR", L"Toggle SuperWeapon Sidebar");
}

const wchar_t* ToggleSWSidebar::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* ToggleSWSidebar::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_TOGGLE_SW_SIDEBAR_DESC", L"Toggle SuperWeapon Sidebar.");
}

void ToggleSWSidebar::Execute(WWKey eInput) const
{
	SidebarExt::Global()->ExclusiveSWSidebar_Enable = !SidebarExt::Global()->ExclusiveSWSidebar_Enable;

	if (SidebarExt::Global()->ExclusiveSWSidebar_Enable)
	{
		MessageListClass::Instance->PrintMessage(GeneralUtils::LoadStringUnlessMissing("TXT_EX_SW_BAR_VISIBLE", L"Set exclusive SW sidebar visible."), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
	}
	else
	{
		MessageListClass::Instance->PrintMessage(GeneralUtils::LoadStringUnlessMissing("TXT_EX_SW_BAR_INVISIBLE", L"Set exclusive SW sidebar invisible."), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);

		if (const auto button = SWSidebarClass::Instance.CurrentButton)
			button->OnMouseLeave();
	}
}
