#pragma once

#include <HouseClass.h>
#include <CellClass.h>

#include "Enum.h"

class EnumFunctions
{
public:
	static bool CanTargetHouse(AffectedHouse flags, HouseClass* ownerHouse, HouseClass* targetHouse);
	static bool IsCellEligible(CellClass* const pCell, AffectedTarget allowed);
	static bool IsTechnoEligible(TechnoClass* const pTechno, AffectedTarget allowed);
	static bool AreCellAndObjectsEligible(CellClass* const pCell, AffectedTarget allowed);
};