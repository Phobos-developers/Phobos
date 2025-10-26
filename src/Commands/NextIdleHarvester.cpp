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


#include "NextIdleHarvester.h"

#include <BuildingTypeClass.h>
#include <MessageListClass.h>
#include <MapClass.h>
#include <ObjectClass.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

const char* NextIdleHarvesterCommandClass::GetName() const
{
	return "Next Idle Harvester";
}

const wchar_t* NextIdleHarvesterCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_NEXT_IDLE_HARVESTER", L"Next Idle Harvester");
}

const wchar_t* NextIdleHarvesterCommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* NextIdleHarvesterCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_NEXT_IDLE_HARVESTER_DESC", L"Select the next harvester that is idle (not harvesting).");
}

void NextIdleHarvesterCommandClass::Execute(WWKey eInput) const
{
	// Debug::Log("[Phobos] Dummy command runs.\n");
	// MessageListClass::Instance.PrintMessage(L"[Phobos] Dummy command rums");

	MapClass::Instance.SetTogglePowerMode(0);
	MapClass::Instance.SetWaypointMode(0, false);
	MapClass::Instance.SetRepairMode(0);
	MapClass::Instance.SetSellMode(0);

	auto pObjectToSelect = MapClass::Instance.NextObject(
		ObjectClass::CurrentObjects.Count ? ObjectClass::CurrentObjects.GetItem(0) : nullptr);

	bool idleHarvestersPresent = false;
	auto pNextObject = pObjectToSelect;

	do
	{
		if (auto const pTechno = abstract_cast<TechnoClass*>(pNextObject))
		{
			auto const pTypeExt = TechnoExt::ExtMap.Find(pTechno)->TypeExtData;

			if (pTypeExt->Harvester_Counted && !TechnoExt::IsHarvesting(pTechno))
			{
				pObjectToSelect = pNextObject;
				idleHarvestersPresent = true;
				break;
			}
		}

		pNextObject = MapClass::Instance.NextObject(pNextObject);
	}
	while (pNextObject != pObjectToSelect);

	if (idleHarvestersPresent && pObjectToSelect)
	{
		MapClass::UnselectAll();
		pObjectToSelect->Select();
		MapClass::Instance.CenterMap();
		MapClass::Instance.MarkNeedsRedraw(1);
	}
	else
	{
		MessageListClass::Instance.PrintMessage(StringTable::LoadString("MSG:NothingSelected"), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
	}
}
