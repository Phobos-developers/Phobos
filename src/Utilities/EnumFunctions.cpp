#include "EnumFunctions.h"
#include "Debug.h"

bool EnumFunctions::CanTargetHouse(AffectedHouse flags, HouseClass* ownerHouse, HouseClass* targetHouse)
{
	if (ownerHouse == targetHouse)
		return (flags & AffectedHouse::Owner) != AffectedHouse::None;
	if (ownerHouse->IsAlliedWith(targetHouse))
		return (flags & AffectedHouse::Allies) != AffectedHouse::None;
	return (flags & AffectedHouse::Enemies) != AffectedHouse::None;
}

bool EnumFunctions::IsCellEligible(CellClass* const pCell, AffectedTarget allowed, bool explicitEmptyCells, bool considerBridgesLand)
{
	if (explicitEmptyCells)
	{
		auto pTechno = pCell->GetContent() ? abstract_cast<TechnoClass*>(pCell->GetContent()) : nullptr;

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

	auto object = pCell->FirstObject;
	bool eligible = EnumFunctions::IsCellEligible(pCell, allowed, explicitEmptyCells, allowBridges);

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

			eligible = EnumFunctions::IsTechnoEligible(pTechno, allowed, considerAircraftSeparately);
		}

		object = object->NextObject;
	}

	return eligible;
}

bool EnumFunctions::CheckVeterancy(TechnoClass* const pTechno, VeterancyType allowed)
{
	VeterancyType veterancy;
	if (pTechno->Veterancy.IsRookie())
		veterancy = VeterancyType::Rookie;
	else if (pTechno->Veterancy.IsVeteran())
		veterancy = VeterancyType::Veteran;
	else if (pTechno->Veterancy.IsElite())
		veterancy = VeterancyType::Elite;
	else
		veterancy = VeterancyType::None;
	return (veterancy & allowed) != VeterancyType::None;
}

bool EnumFunctions::CheckHPPercentage(TechnoClass* const pTechno, HPPercentageType allowed)
{
	HPPercentageType hpPercentageType;
	if (pTechno->Health >= pTechno->GetTechnoType()->Strength)
		hpPercentageType = HPPercentageType::Full;
	else if (pTechno->IsGreenHP())
		hpPercentageType = HPPercentageType::GreenNotFull;
	else if (pTechno->IsYellowHP())
		hpPercentageType = HPPercentageType::Yellow;
	else if (pTechno->IsRedHP())
		hpPercentageType = HPPercentageType::Red;
	else
		hpPercentageType = HPPercentageType::None;
	return (hpPercentageType & allowed) != HPPercentageType::None;
}

bool EnumFunctions::HouseParameterCompare(HouseClass* pHouse, HouseParameterType hpt, ComparatorType ct, int value)
{
	int param = -1;
	switch (hpt)
	{
	case HouseParameterType::Credits:
		param = pHouse->Available_Money();
		break;
	case HouseParameterType::PowerSurplus:
		param = pHouse->PowerSurplus;
		break;
	case HouseParameterType::PowerOutput:
		param = pHouse->PowerOutput;
		break;
	case HouseParameterType::PowerDrain:
		param = pHouse->PowerDrain;
		break;
	default:
		return false;
	}

	switch (ct)
	{
	case ComparatorType::Equal:
		return param == value;
	case ComparatorType::Greater:
		return param > value;
	case ComparatorType::GreaterEqual:
		return param >= value;
	case ComparatorType::Lower:
		return param < value;
	case ComparatorType::LowerEqual:
		return param <= value;
	case ComparatorType::NotEqual:
		return param != value;
	}

	return false;
}
