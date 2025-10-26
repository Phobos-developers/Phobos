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
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x56BD8B, MapClass_PlaceRandomCrate_Sampling, 0x5)
{
	enum { SpawnCrate = 0x56BE7B, SkipSpawn = 0x56BE91 };

	const int XP = 2 * MapClass::Instance.VisibleRect.X - MapClass::Instance.MapRect.Width
		+ ScenarioClass::Instance->Random.RandomRanged(0, 2 * MapClass::Instance.VisibleRect.Width);

	const int YP = 2 * MapClass::Instance.VisibleRect.Y + MapClass::Instance.MapRect.Width
		+ ScenarioClass::Instance->Random.RandomRanged(0, 2 * MapClass::Instance.VisibleRect.Height + 2);

	CellStruct candidate { (short)((XP + YP) / 2),(short)((YP - XP) / 2) };
	const auto pCell = MapClass::Instance.TryGetCellAt(candidate);

	if (!pCell)
		return SkipSpawn;

	if (!MapClass::Instance.IsWithinUsableArea(pCell, true))
		return SkipSpawn;

	const bool isWater = pCell->LandType == LandType::Water;

	if (isWater && RulesExt::Global()->CrateOnlyOnLand.Get())
		return SkipSpawn;

	REF_STACK(CellStruct, cell, STACK_OFFSET(0x28, -0x18));

	cell = MapClass::Instance.NearByLocation(pCell->MapCoords,
		isWater ? SpeedType::Float : SpeedType::Track,
		-1, MovementZone::Normal, false, 1, 1, false, false, false, true, CellStruct::Empty, false, false);

	if (cell == CellStruct::Empty)
		return SkipSpawn;

	R->EAX(&cell);

	return SpawnCrate;
}

// Change RulesClass->FreeMCV default from 0 to 1.
DEFINE_PATCH(0x6656B3, 0x89, 0x4E);

DEFINE_HOOK(0x481BB8, CellClass_GoodieCheck_FreeMCV, 0x6)
{
	enum { SkipForcedMCV = 0x481C03, EnableForcedMCV = 0x481BF6 };

	GET(HouseClass*, pHouse, EDI);
	GET_STACK(UnitTypeClass*, pBaseUnit, STACK_OFFSET(0x188, -0x138));

	if (RulesClass::Instance->FreeMCV
		&& pHouse->Available_Money() > RulesExt::Global()->FreeMCV_CreditsThreshold
		&& SessionClass::Instance.Config.Bases
		&& !pHouse->OwnedBuildings
		&& !pHouse->CountOwnedNow(pBaseUnit))
	{
		return EnableForcedMCV;
	}

	return SkipForcedMCV;
}

DEFINE_HOOK(0x481C27, CellClass_GoodieCheck_UnitCrateVehicleCap, 0x5)
{
	enum { Capped = 0x481C44, NotCapped = 0x481C4A };

	GET(HouseClass*, pHouse, EDX);

	if (RulesExt::Global()->UnitCrateVehicleCap < 0 || pHouse->OwnedUnits <= RulesExt::Global()->UnitCrateVehicleCap)
		return NotCapped;

	return Capped;
}

DEFINE_HOOK(0x4821BD, CellClass_GoodieCheck_CrateGoodie, 0x6)
{
	enum { SkipGameCode = 0x4821C3 };

	GET(UnitTypeClass*, pUnitType, EDI);

	bool crateGoodie = pUnitType->CrateGoodie;

	if (crateGoodie)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pUnitType);

		if (pTypeExt->CrateGoodie_RerollChance > 0.0)
			crateGoodie = pTypeExt->CrateGoodie_RerollChance < ScenarioClass::Instance->Random.RandomDouble();
	}

	R->CL(crateGoodie);

	return SkipGameCode;
}
