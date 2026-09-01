#include "EnumFunctions.h"

#include <Utilities/GeneralUtils.h>

bool EnumFunctions::CanTargetHouse(AffectedHouse flags, HouseClass* ownerHouse, HouseClass* targetHouse)
{
	if (flags == AffectedHouse::All)
		return true;

	// Match civilian/special/neutral by house identity instead of alliance
	// status, as their ally/enemy relation to other houses varies by spawner.
	if ((flags & AffectedHouse::Civilian) != AffectedHouse::None && targetHouse == HouseClass::FindCivilianSide())
		return true;
	if ((flags & AffectedHouse::Special) != AffectedHouse::None && targetHouse == HouseClass::FindSpecial())
		return true;
	if ((flags & AffectedHouse::Neutral) != AffectedHouse::None && targetHouse == HouseClass::FindNeutral())
		return true;

	if (ownerHouse == targetHouse)
		return (flags & AffectedHouse::Owner) != AffectedHouse::None;
	if (ownerHouse->IsAlliedWith(targetHouse))
		return (flags & AffectedHouse::Allies) != AffectedHouse::None;
	if ((flags & AffectedHouse::Enemies) != AffectedHouse::None)
		return true;
	return false;
}

bool EnumFunctions::CanTargetVeterancy(AffectedVeterancy flags, TechnoClass* pTechno)
{
	if (flags == AffectedVeterancy::All)
		return true;

	switch (pTechno->Veterancy.GetRemainingLevel())
	{
	case Rank::Elite:
		return (flags & AffectedVeterancy::Elite) != AffectedVeterancy::None;
	case Rank::Veteran:
		return (flags & AffectedVeterancy::Veteran) != AffectedVeterancy::None;
	default:
		return (flags & AffectedVeterancy::Rookie) != AffectedVeterancy::None;
	}
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

bool EnumFunctions::CalcValueWithStackingMode(int& oldValue, int newValue, StackingMode stackingMode)
{
	bool valueChanged = true;
	int oldValueTemp = oldValue;

	switch (stackingMode)
	{
	case StackingMode::Override:
		oldValue = newValue;
		break;
	case StackingMode::SetIfZero:
		if (oldValue == 0)
			oldValue = newValue;
		else
			valueChanged = false;
		break;
	case StackingMode::Min:
		oldValue = Math::min(oldValue, newValue);
		break;
	case StackingMode::Max:
		oldValue = Math::max(oldValue, newValue);
		break;
	case StackingMode::Add:
		oldValue += newValue;
		break;
	case StackingMode::Subtract:
		oldValue -= newValue;
		break;
	case StackingMode::Multiply:
		oldValue = GeneralUtils::SafeMultiply(oldValue, newValue);
		break;
	case StackingMode::Divide:
		if (newValue != 0)
			oldValue /= newValue;
		else
			valueChanged = false;
		break;
	default:
		valueChanged = false;
		break;
	}

	return valueChanged && oldValueTemp != oldValue;
}
