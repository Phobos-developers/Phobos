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
	static bool CheckVeterancy(TechnoClass* const pTechno, VeterancyType allowed);
	static bool CheckHPPercentage(TechnoClass* const pTechno, HPPercentageType allowed);
	static bool HouseParameterCompare(HouseClass* pHouse, HouseParameterType hpt, ComparatorType ct, int value);
	static bool ParameterCompare(int param, ComparatorType ct, int value);
};
