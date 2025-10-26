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
#include "EnumFunctions.h"

bool EnumFunctions::CanTargetHouse(AffectedHouse flags, HouseClass* ownerHouse, HouseClass* targetHouse)
{
	if (flags == AffectedHouse::All)
		return true;
	if (ownerHouse == targetHouse)
		return (flags & AffectedHouse::Owner) != AffectedHouse::None;
	if (ownerHouse->IsAlliedWith(targetHouse))
		return (flags & AffectedHouse::Allies) != AffectedHouse::None;
	return (flags & AffectedHouse::Enemies) != AffectedHouse::None;
}

bool EnumFunctions::IsCellEligible(CellClass* const pCell, AffectedTarget allowed, bool explicitEmptyCells, bool considerBridgesLand)
{
	if (allowed == AffectedTarget::All)
		return true;

	if (explicitEmptyCells)
	{
		const auto pTechno = abstract_cast<TechnoClass*>(pCell->GetContent());

		if (!pTechno && !(allowed & AffectedTarget::NoContent))
			return false;
	}

	if (allowed & AffectedTarget::AllCells)
	{
		if (pCell->LandType == LandType::Water && (!considerBridgesLand || !pCell->ContainsBridge())) // check whether it supports water
			return (allowed & AffectedTarget::Water) != AffectedTarget::None;
		else                                    // check whether it supports non-water
			return (allowed & AffectedTarget::Land) != AffectedTarget::None;
	}

	return allowed != AffectedTarget::None;
}

bool EnumFunctions::IsTechnoEligible(TechnoClass* const pTechno, AffectedTarget allowed, bool considerAircraftSeparately)
{
	if (allowed == AffectedTarget::All)
		return true;

	if (allowed & AffectedTarget::AllContents)
	{
		if (pTechno)
		{
			switch (pTechno->WhatAmI())
			{
			case AbstractType::Infantry:
				return (allowed & AffectedTarget::Infantry) != AffectedTarget::None;
			case AbstractType::Unit:
				return (allowed & AffectedTarget::Unit) != AffectedTarget::None;
			case AbstractType::Aircraft:
				if (!considerAircraftSeparately)
					return (allowed & AffectedTarget::Unit) != AffectedTarget::None;
				else
					return (allowed & AffectedTarget::Aircraft) != AffectedTarget::None;
			case AbstractType::Building:
				if (pTechno->IsStrange())
					return (allowed & AffectedTarget::Unit) != AffectedTarget::None;
				else
					return (allowed & AffectedTarget::Building) != AffectedTarget::None;
			}
		}
		else
		{
			// is the target cell allowed to be empty?
			return (allowed & AffectedTarget::NoContent) != AffectedTarget::None;
		}
	}

	return allowed != AffectedTarget::None;
}

bool EnumFunctions::AreCellAndObjectsEligible(CellClass* const pCell, AffectedTarget allowed, AffectedHouse allowedHouses, HouseClass* owner, bool explicitEmptyCells, bool considerAircraftSeparately, bool allowBridges)
{
	if (!pCell)
		return false;

	if (!EnumFunctions::IsCellEligible(pCell, allowed, explicitEmptyCells, allowBridges))
		return false;

	for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
	{
		if (const auto pTechno = abstract_cast<TechnoClass*, true>(pObject))
		{
			if (owner && !EnumFunctions::CanTargetHouse(allowedHouses, owner, pTechno->Owner))
				return false;

			if (!EnumFunctions::IsTechnoEligible(pTechno, allowed, considerAircraftSeparately))
				return false;
		}
	}

	return true;
}
