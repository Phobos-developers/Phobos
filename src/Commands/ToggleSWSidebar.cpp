#include "ToggleSWSidebar.h"

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

	if (!SidebarExt::Global()->ExclusiveSWSidebar_Enable)
	{
		if (const auto button = SWSidebarClass::Instance.CurrentButton)
			button->OnMouseLeave();
	}
}
