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
#pragma once

#include <HouseClass.h>
#include <CellClass.h>

#include "Enum.h"

class EnumFunctions
{
public:
	static bool CanTargetHouse(AffectedHouse flags, HouseClass* ownerHouse, HouseClass* targetHouse);
	static bool IsCellEligible(CellClass* const pCell, AffectedTarget allowed, bool explicitEmptyCells = false, bool considerBridgesLand = false);
	static bool IsTechnoEligible(TechnoClass* const pTechno, AffectedTarget allowed, bool considerAircraftSeparately = false);
	static bool AreCellAndObjectsEligible(CellClass* const pCell, AffectedTarget allowed, AffectedHouse allowedHouses, HouseClass* owner, bool explicitEmptyCells = false, bool considerAircraftSeparately = false, bool allowBridges = false);
};
