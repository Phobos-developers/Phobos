#include "EnumFunctions.h"

bool EnumFunctions::CanTargetHouse(AffectedHouse flags, HouseClass* ownerHouse, HouseClass* targetHouse)
{
	return (flags & AffectedHouse::Owner) && ownerHouse == targetHouse ||
		(flags & AffectedHouse::Allies) && ownerHouse != targetHouse && ownerHouse->IsAlliedWith(targetHouse) ||
		(flags & AffectedHouse::Enemies) && ownerHouse != targetHouse && !ownerHouse->IsAlliedWith(targetHouse);
}

bool EnumFunctions::IsCellEligible(CellClass* const pCell, AffectedTarget allowed, bool explicitEmptyCells)
{
	if (explicitEmptyCells)
	{
		auto pTechno = pCell->FirstObject ? abstract_cast<TechnoClass*>(pCell->FirstObject) : nullptr;

		if (!pTechno && !(allowed & AffectedTarget::NoContent))
			return false;
	}

	if (allowed & AffectedTarget::AllCells)
	{
		if (pCell->LandType == LandType::Water) // check whether it supports water
			return (allowed & AffectedTarget::Water) != AffectedTarget::None;
		else                                    // check whether it supports non-water
			return (allowed & AffectedTarget::Land) != AffectedTarget::None;
	}

	return allowed != AffectedTarget::None ? true : false;
}

bool EnumFunctions::IsTechnoEligible(TechnoClass* const pTechno, AffectedTarget allowed)
{
	if (allowed & AffectedTarget::AllContents)
	{
		if (pTechno)
		{
			switch (pTechno->WhatAmI())
			{
			case AbstractType::Infantry:
				return (allowed & AffectedTarget::Infantry) != AffectedTarget::None;
			case AbstractType::Unit:
			case AbstractType::Aircraft:
				return (allowed & AffectedTarget::Unit) != AffectedTarget::None;
			case AbstractType::Building:
				return (allowed & AffectedTarget::Building) != AffectedTarget::None;
			}
		}
		else
		{
			// is the target cell allowed to be empty?
			return (allowed & AffectedTarget::NoContent) != AffectedTarget::None;
		}
	}

	return allowed != AffectedTarget::None ? true : false;
}

bool EnumFunctions::AreCellAndObjectsEligible(CellClass* const pCell, AffectedTarget allowed, AffectedHouse allowedHouses, HouseClass* owner, bool explicitEmptyCells)
{
	if (!pCell)
		return false;

	auto object = pCell->FirstObject;
	bool eligible = EnumFunctions::IsCellEligible(pCell, allowed, explicitEmptyCells);

	while (true)
	{
		if (!object || !eligible)
			break;

		if (auto pTechno = abstract_cast<TechnoClass*>(object))
		{
			if (owner)
			{
				eligible = EnumFunctions::CanTargetHouse(allowedHouses, owner, pTechno->Owner);

				if (!eligible)
					break;
			}

			eligible = EnumFunctions::IsTechnoEligible(pTechno, allowed);
		}

		object = object->NextObject;
	}

	return eligible;
}