#include "Body.h"

#include <Utilities/Helpers.Alex.h>

// Add or substract experience for real
int AddExpCustom(VeterancyStruct* vstruct, int targetCost, int exp) {
	double toBeAdded = (double)exp / (targetCost * RulesClass::Instance->VeteranRatio);
	// Used in experience transfer to get the actual amount substracted
	int transffered = (int)(Math::min(vstruct->Veterancy, abs(toBeAdded))
		* (targetCost * RulesClass::Instance->VeteranRatio));
	// Don't do anything when current exp at 0
	if (exp < 0 && transffered <= 0) {
		vstruct->Reset();
		transffered = 0;
	}
	else {
		vstruct->Add(toBeAdded);
	}
	return transffered;
}

void WarheadTypeExt::ExtData::TransactOnOneUnit(TechnoClass* pTarget, TechnoClass* pOwner)
{
	// Experience transaction
	if (this->Transact_Source_Experience_Flat ||
		this->Transact_Source_Experience_Percent ||
		this->Transact_Target_Experience_Flat ||
		this->Transact_Target_Experience_Percent)
		TransactExperience(pTarget, pOwner);

	// Future transactions here
}

void WarheadTypeExt::ExtData::TransactOnAllUnits(HouseClass* pHouse, const CoordStruct coords, const float cellSpread, TechnoClass* pOwner)
{
	// Experience transaction
	if (this->Transact_Source_Experience_Flat ||
		this->Transact_Source_Experience_Percent ||
		this->Transact_Target_Experience_Flat ||
		this->Transact_Target_Experience_Percent)
	{
		for (auto pTarget : Helpers::Alex::getCellSpreadItems(coords, cellSpread, true))
		{
			TransactExperience(pTarget, pOwner);
		}
	}

	// Future transactions here
}

void WarheadTypeExt::ExtData::TransactExperience(TechnoClass* pTarget, TechnoClass* pOwner)
{
	auto const pTargetTechno = (pTarget) ? pTarget->GetTechnoType() : nullptr;
	auto const pOwnerTechno = (pOwner) ? pOwner->GetTechnoType() : nullptr;
	int sourceExp = 0, targetExp = 0;

	// Flat from Source
	if (this->Transact_Source_Experience_Flat != 0 && pOwnerTechno)
		sourceExp = this->Transact_Source_Experience_Flat;

	// Percent from Source
	if (!CLOSE_ENOUGH(this->Transact_Source_Experience_Percent, 0.0) && pOwnerTechno)
	{
		int sourceExpFromPercent;
		if (this->Transact_Source_Experience_Percent_CalcFromTarget && pTargetTechno)
			sourceExpFromPercent = (int)((pTargetTechno->GetActualCost(pTarget->Owner) * this->Transact_Source_Experience_Percent));
		else
			sourceExpFromPercent = (int)((pOwnerTechno->GetActualCost(pOwner->Owner) * this->Transact_Source_Experience_Percent));

		sourceExp = abs(sourceExpFromPercent) > abs(sourceExp) ? sourceExpFromPercent : sourceExp;
	}

	// Flat from Target
	if (this->Transact_Target_Experience_Flat != 0 && pTargetTechno)
		targetExp = this->Transact_Target_Experience_Flat;

	// Percent from Target
	if (!CLOSE_ENOUGH(this->Transact_Target_Experience_Percent, 0.0) && pTargetTechno)
	{
		int targetExpFromPercent;
		if (this->Transact_Target_Experience_Percent_CalcFromSource && pOwnerTechno)
			targetExpFromPercent = (int)((pOwnerTechno->GetActualCost(pOwner->Owner) * this->Transact_Target_Experience_Percent));
		else
			targetExpFromPercent = (int)((pTargetTechno->GetActualCost(pTarget->Owner) * this->Transact_Target_Experience_Percent));

		targetExp = abs(targetExpFromPercent) > abs(targetExp) ? targetExpFromPercent : targetExp;
	}

	// Transact (A loses B gains)
	if (sourceExp != 0 && targetExp != 0 && targetExp * sourceExp < 0)
	{
		int transExp = abs(sourceExp) > abs(targetExp) ? abs(targetExp) : abs(sourceExp);

		if (sourceExp < 0)
		{
			transExp = AddExpCustom(&pOwner->Veterancy,
				pOwnerTechno->GetActualCost(pOwner->Owner), -transExp);
			AddExpCustom(&pTarget->Veterancy,
				pTargetTechno->GetActualCost(pTarget->Owner), transExp);
		}
		else
		{
			transExp = AddExpCustom(&pTarget->Veterancy,
				pTargetTechno->GetActualCost(pTarget->Owner), -transExp);
			AddExpCustom(&pOwner->Veterancy,
				pOwnerTechno->GetActualCost(pOwner->Owner), transExp);
		}

		return;
	}
	// Out-of-thin-air grants
	if (sourceExp != 0)
	{
		AddExpCustom(&pOwner->Veterancy,
			pOwnerTechno->GetActualCost(pOwner->Owner), sourceExp);
	}
	if (targetExp != 0)
	{
		AddExpCustom(&pTarget->Veterancy,
			pTargetTechno->GetActualCost(pTarget->Owner), targetExp);
	}
}
