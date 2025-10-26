// SPDX-License-Identifier: GPL-3.0-or-later
// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

	if (SWSidebarClass::Instance.CurrentColumn)
		MouseClass::Instance.UpdateCursor(MouseCursorType::Default, false);
}
