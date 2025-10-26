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
#include "QuickSave.h"

#include <ScenarioClass.h>
#include <HouseClass.h>
#include <SessionClass.h>
#include <EventClass.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/SpawnerHelper.h>

const char* QuickSaveCommandClass::GetName() const
{
	return "Quicksave";
}

const wchar_t* QuickSaveCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE", L"Quicksave");
}

const wchar_t* QuickSaveCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* QuickSaveCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE_DESC", L"Save the current game.");
}

void QuickSaveCommandClass::Execute(WWKey eInput) const
{
	auto PrintMessage = [](const wchar_t* pMessage)
	{
		MessageListClass::Instance.PrintMessage(
			pMessage,
			RulesClass::Instance->MessageDelay,
			HouseClass::CurrentPlayer->ColorSchemeIndex,
			true
		);
	};

	if (SessionClass::IsSingleplayer())
	{
		Phobos::ScheduleGameSave(GeneralUtils::LoadStringUnlessMissing("TXT_QUICKSAVE_SUFFIX", L"Quicksaved"));
	}
	else if (SpawnerHelper::IsSaveGameEventHooked())
	{
		// Relinquish handling of the save game to spawner
		EventClass::OutList.Add(EventClass { HouseClass::CurrentPlayer->ArrayIndex, EventType::SaveGame });
	}
	else
	{
		PrintMessage(GeneralUtils::LoadStringUnlessMissing("MSG:NotAvailableInMultiplayer", L"QuickSave is not available in multiplayer"));
	}
}
