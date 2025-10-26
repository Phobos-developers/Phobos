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
