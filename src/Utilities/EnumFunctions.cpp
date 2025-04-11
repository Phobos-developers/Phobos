#include "EnumFunctions.h"

bool EnumFunctions::CanTargetHouse(AffectedHouse flags, HouseClass* ownerHouse, HouseClass* targetHouse)
{
    if (flags == AffectedHouse::All)
        return true;

    AffectedHouse relation = AffectedHouse::None;

    if (ownerHouse == targetHouse)
        relation |= AffectedHouse::Owner;
    else if (ownerHouse->IsAlliedWith(targetHouse))
        relation |= AffectedHouse::Allies;
    else
        relation |= AffectedHouse::Enemies;

    return (flags & relation) != AffectedHouse::None;
}

bool EnumFunctions::IsCellEligible(CellClass* const pCell, AffectedTarget allowed, bool explicitEmptyCells, bool considerBridgesLand)
{
	if (allowed == AffectedTarget::All)
		return true;

	if (explicitEmptyCells)
	{
		auto pTechno = pCell->GetContent() ? abstract_cast<TechnoClass*>(pCell->GetContent()) : nullptr;

		if (!pTechno && !(allowed & AffectedTarget::NoContent))
			return false;
	}

	AffectedTarget cellType = AffectedTarget::None;

    if (pCell->LandType == LandType::Water)
    {
        if (considerBridgesLand && pCell->ContainsBridge())
            cellType |= AffectedTarget::Land;
        else
            cellType |= AffectedTarget::Water;
    }
    else
    {
        cellType |= AffectedTarget::Land;
    }

    return (allowed & cellType) != AffectedTarget::None;
}

bool EnumFunctions::IsTechnoEligible(TechnoClass* const pTechno, AffectedTarget allowed, bool considerAircraftSeparately)
{
	if (allowed == AffectedTarget::All)
        return true;

    if (!pTechno)
        return (allowed & AffectedTarget::NoContent) != AffectedTarget::None;

    AffectedTarget technoType = AffectedTarget::None;

    switch (pTechno->WhatAmI())
    {
        case AbstractType::Infantry:
            technoType |= AffectedTarget::Infantry;
            break;
        case AbstractType::Unit:
            technoType |= AffectedTarget::Unit;
            break;
        case AbstractType::Aircraft:
            if (!considerAircraftSeparately)
                technoType |= AffectedTarget::Unit;
            else
                technoType |= AffectedTarget::Aircraft;
            break;
        case AbstractType::Building:
            if (pTechno->IsStrange())
                technoType |= AffectedTarget::Unit;
            else
                technoType |= AffectedTarget::Building;
            break;
        default:
            return false;
    }

    return (allowed & technoType) != AffectedTarget::None;
}

bool EnumFunctions::AreCellAndObjectsEligible(CellClass* const pCell, AffectedTarget allowed, AffectedHouse allowedHouses, HouseClass* owner, bool explicitEmptyCells, bool considerAircraftSeparately, bool allowBridges)
{
	if (!pCell)
		return false;

	if (!EnumFunctions::IsCellEligible(pCell, allowed, explicitEmptyCells, allowBridges))
        return false;

	auto object = pCell->FirstObject;

	while (object)
    {
        if (auto pTechno = abstract_cast<TechnoClass*>(object))
        {
            if (owner && !EnumFunctions::CanTargetHouse(allowedHouses, owner, pTechno->Owner))
                return false;

            if (!EnumFunctions::IsTechnoEligible(pTechno, allowed, considerAircraftSeparately))
                return false;
        }

        object = object->NextObject;
    }

	return true;
}
