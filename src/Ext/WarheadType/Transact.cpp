#include "Body.h"

#include <Utilities/Enum.h>
#include <Utilities/Helpers.Alex.h>

// Add or substract experience for real
int AddExpCustom(VeterancyStruct* vstruct, int targetCost, int exp)
{
	double toBeAdded = (double)exp / (targetCost * RulesClass::Instance->VeteranRatio);
	// Used in experience transfer to get the actual amount substracted
	int transffered = (int)(Math::min(vstruct->Veterancy, abs(toBeAdded))
		* (targetCost * RulesClass::Instance->VeteranRatio));
	// Don't do anything when current exp at 0
	if (exp < 0 && transffered <= 0)
	{
		vstruct->Reset();
		transffered = 0;
	}
	else
	{
		vstruct->Add(toBeAdded);
	}
	// Prevent going above elite level of 2.0
	if (vstruct->IsElite())
	{
		vstruct->SetElite();
	}

	return transffered;
}

int WarheadTypeExt::ExtData::TransactOneValue(TechnoClass* pTechno, TechnoTypeClass* pTechnoType, int transactValue, TransactValueType valueType)
{
	if (!pTechno || !pTechnoType)
		return 0;

	int transferred = 0;
	switch (valueType)
	{
		case TransactValueType::Experience:
			if (!pTechnoType->Trainable && !this->Transact_Experience_IgnoreNotTrainable)
				return 0;

			transferred = AddExpCustom(&pTechno->Veterancy,
				pTechnoType->GetActualCost(pTechno->Owner), transactValue);
			break;
		default:
			break;
	}

	return transferred;
}

int WarheadTypeExt::ExtData::TransactGetValue(TechnoClass* pTarget, TechnoClass* pOwner, int flat, double percent, bool calcFromTarget, int targetValue, int ownerValue)
{
	int percentValue = 0;
	if (!CLOSE_ENOUGH(percent, 0.0))
	{
		if (calcFromTarget && pTarget)
			percentValue = (int)(targetValue * percent);
		else if (!calcFromTarget && pOwner)
			percentValue = (int)(ownerValue * percent);
	}

	return flat + percentValue;
}

std::vector<std::vector<int>> WarheadTypeExt::ExtData::TransactGetSourceAndTarget(TechnoClass* pTarget, TechnoTypeClass* pTargetType, TechnoClass* pOwner, TechnoTypeClass* pOwnerType, int divider)
{
	std::vector<std::vector<int>> allValues;
	std::vector<int> sourceValues;
	std::vector<int> targetValues;
	std::vector<int> valueTypes;
	int targetCost = pTarget ? pTargetType->GetActualCost(pTarget->Owner) : 0;
	int ownerCost = pOwner ? pOwnerType->GetActualCost(pOwner->Owner) : 0;

	// SOURCE
	//		Experience
	int sourceExp = pOwner ? this->TransactGetValue(pTarget, pOwner,
		this->Transact_Experience_Source_Flat, this->Transact_Experience_Source_Percent,
		this->Transact_Experience_Source_RelativeToTarget, targetCost, ownerCost) : 0;
	sourceExp = Math::clamp(sourceExp, this->Transact_Experience_Source_Min, this->Transact_Experience_Source_Max);
	sourceValues.push_back(sourceExp / divider);

	allValues.push_back(sourceValues);

	// TARGET
	//		Experience
	int targetExp = pTarget ? this->TransactGetValue(pOwner, pTarget,
		this->Transact_Experience_Target_Flat, this->Transact_Experience_Target_Percent,
		this->Transact_Experience_Target_RelativeToSource,
		ownerCost, targetCost) : 0;
	targetExp = Math::clamp(targetExp, this->Transact_Experience_Target_Min, this->Transact_Experience_Target_Max);
	targetValues.push_back(targetExp / divider);

	allValues.push_back(targetValues);

	// TYPES
	//		Experience
	valueTypes.push_back((int)TransactValueType::Experience);

	allValues.push_back(valueTypes);

	return allValues;
}

void WarheadTypeExt::ExtData::TransactOnOneUnitCore(HouseClass* pHouse, TechnoClass* pTarget, TechnoTypeClass* pTargetType, TechnoClass* pOwner, TechnoTypeClass* pOwnerType, int divider, bool sourceOnly)
{
	// get pairs of source-target transaction values
	std::vector<std::vector<int>> allValues = this->TransactGetSourceAndTarget(pTarget, pTargetType, pOwner, pOwnerType, divider);

	for (unsigned int i = 0; i < allValues[0].size(); i++)
	{
		int sourceValue = allValues[0][i];
		int targetValue = allValues[1][i];
		auto valueType = (TransactValueType)allValues[2][i];

		// Transact (A loses B gains)
		if (!sourceOnly && sourceValue != 0 && targetValue != 0 && targetValue * sourceValue < 0)
		{
			int transactValue = abs(sourceValue) > abs(targetValue) ? abs(targetValue) : abs(sourceValue);

			if (sourceValue < 0)
			{
				transactValue = TransactOneValue(pOwner, pOwnerType, -transactValue, valueType);
				TransactOneValue(pTarget, pTargetType, transactValue, valueType);
			}
			else
			{
				transactValue = TransactOneValue(pTarget, pTargetType, -transactValue, valueType);
				TransactOneValue(pOwner, pOwnerType, transactValue, valueType);
			}

			return;
		}

		// Out-of-thin-air grants
		if (sourceValue != 0)
			TransactOneValue(pOwner, pOwnerType, sourceValue, valueType);

		if (!sourceOnly && targetValue != 0)
			TransactOneValue(pTarget, pTargetType, targetValue, valueType);
	}
}

void WarheadTypeExt::ExtData::TransactOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner, int divider)
{
	bool targetedAnything = true;

	if (!pTarget)
	{
		if (this->Transact_RequiresAnyTarget)
			return;
		targetedAnything = false;
	}

	auto const pTargetType = pTarget ? pTarget->GetTechnoType() : nullptr;
	auto const pOwnerType = pOwner ? pOwner->GetTechnoType() : nullptr;

	// Check if target is immune
	if (targetedAnything && (CLOSE_ENOUGH(GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), pTargetType->Armor), 0.0)
		|| (!pTargetType->Trainable && !this->Transact_Experience_IgnoreNotTrainable) || !this->CanTargetHouse(pHouse, pTarget)))
	{
		targetedAnything = false;
	}

	// Apply transction to valid target
	if (targetedAnything)
		TransactOnOneUnitCore(pHouse, pTarget, pTargetType, pOwner, pOwnerType, divider);

	// Apply transaction in source-only mode if we permit shooting at immune targets or no targets
	if (!targetedAnything && (!this->Transact_RequiresAnyTarget || (pTarget && !this->Transact_RequiresValidTarget)))
		TransactOnOneUnitCore(pHouse, nullptr, nullptr, pOwner, pOwnerType, 1, true);
}

void WarheadTypeExt::ExtData::TransactOnAllUnits(HouseClass* pHouse, const CoordStruct coords, const float cellSpread, TechnoClass* pOwner)
{
	auto targets = Helpers::Alex::getCellSpreadItems(coords, cellSpread, true);
	int initialNumberOfTargets = targets.size();
	int numberOfTargets = initialNumberOfTargets;
	bool targetedAnything = false;
	auto const pOwnerType = pOwner ? pOwner->GetTechnoType() : nullptr;

	// Search for not immune targets
	for (unsigned int i = 0; i < targets.size(); i++)
	{
		if (CLOSE_ENOUGH(GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), targets[i]->GetTechnoType()->Armor), 0.0)
			|| (!targets[i]->GetTechnoType()->Trainable && !this->Transact_Experience_IgnoreNotTrainable))
		{
			numberOfTargets--;
			targets[i] = nullptr;
		}
	}

	// Apply transction to valid targets
	for (auto pTarget : targets)
	{
		if (!pTarget)
			continue;
		TransactOnOneUnitCore(pHouse, pTarget, pTarget->GetTechnoType(), pOwner, pOwnerType, this->Transact_SpreadAmongTargets ? numberOfTargets : 1);
		targetedAnything = true;
	}

	// Apply transaction in source-only mode if we permit shooting at immune targets or no targets
	if (!targetedAnything && (!this->Transact_RequiresAnyTarget || (initialNumberOfTargets > 0 && !this->Transact_RequiresValidTarget)))
		TransactOnOneUnitCore(pHouse, nullptr, nullptr, pOwner, pOwnerType, 1, true);
}
