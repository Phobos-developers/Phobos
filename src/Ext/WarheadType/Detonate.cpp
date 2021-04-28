#include "Body.h"

#include <InfantryClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <AnimTypeClass.h>
#include <AnimClass.h>

#include "../../Utilities/Helpers.Alex.h"
#include "../TechnoType/Body.h"

void WarheadTypeExt::ExtData::Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct coords)
{
	if (pHouse) {
		if (this->BigGap) {
			for (auto pOtherHouse : *HouseClass::Array) {
				if (pOtherHouse->ControlledByHuman() &&	  // Not AI
					!pOtherHouse->IsObserver() &&         // Not Observer
					!pOtherHouse->Defeated &&             // Not Defeated
					pOtherHouse != pHouse &&              // Not pThisHouse
					!pHouse->IsAlliedWith(pOtherHouse))   // Not Allied
				{
					pOtherHouse->ReshroudMap();
				}
			}
		}

		if (this->SpySat) {
			MapClass::Instance->Reveal(pHouse);
		}

		if (this->TransactMoney) {
			pHouse->TransactMoney(this->TransactMoney);
		}
	}

	this->RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();

	// List all Warheads here that respect CellSpread
	const bool isCellSpreadWarhead =
		this->RemoveDisguise ||
		this->RemoveMindControl ||
		this->Crit_Chance ||
		this->Experience_GivenFlat ||
		this->Experience_GivenPercent;

	const float cellSpread = this->OwnerObject()->CellSpread;
	if (cellSpread && isCellSpreadWarhead) {
		auto items = Helpers::Alex::getCellSpreadItems(coords, cellSpread, true);
		for (auto pTarget : items) {
			this->DetonateOnOneUnit(pHouse, pTarget, pOwner);
		}
	}
	else if (pBullet && isCellSpreadWarhead) {
		if (auto pTarget = abstract_cast<TechnoClass*>(pBullet->Target)) {
			this->DetonateOnOneUnit(pHouse, pTarget, pOwner);
		}
	}
}

void WarheadTypeExt::ExtData::DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner)
{
	if (!pTarget || pTarget->InLimbo || !pTarget->IsAlive || !pTarget->Health) {
		return;
	}

	if (!this->CanTargetHouse(pHouse, pTarget)) {
		return;
	}

	if (this->RemoveDisguise) {
		this->ApplyRemoveDisguiseToInf(pHouse, pTarget);
	}

	if (this->RemoveMindControl) {
		this->ApplyRemoveMindControl(pHouse, pTarget);
	}
	
	if (this->Crit_Chance) {
		this->ApplyCrit(pHouse, pTarget, pOwner);
	}

	if (this->Experience_GivenFlat || this->Experience_GivenPercent) {
		this->ApplyModifyExperience(pTarget, pOwner);
	}
}

void WarheadTypeExt::ExtData::ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (auto pController = pTarget->MindControlledBy) {
		pTarget->MindControlledBy->CaptureManager->FreeUnit(pTarget);
		if (!pTarget->IsHumanControlled) {
			pTarget->QueueMission(Mission::Hunt, false);
		}
	}
}

void WarheadTypeExt::ExtData::ApplyRemoveDisguiseToInf(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (pTarget->WhatAmI() == AbstractType::Infantry) {
		auto pInf = abstract_cast<InfantryClass*>(pTarget);
		if (pInf->IsDisguised()) {
			pInf->ClearDisguise();
		}
	}
}

void WarheadTypeExt::ExtData::ApplyCrit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner)
{
	//auto& random = ScenarioClass::Instance->Random;
	const double dice = this->RandomBuffer; //double(random.RandomRanged(1, 10)) / 10;

	if (this->Crit_Chance < dice) {
		return;
	}

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType())) {
		if (pTypeExt->ImmuneToCrit)
			return;
	}

	if (!this->IsCellEligible(pTarget->GetCell(), this->Crit_Affects)) {
		return;
	}

	if (!this->IsTechnoEligible(pTarget, this->Crit_Affects)) {
		return;
	}

	pTarget->ReceiveDamage(&this->Crit_ExtraDamage, 0, this->OwnerObject(), pOwner, false, false, pHouse);
}

// Add or substract experience for real
int AddExpCustom(VeterancyStruct* vstruct, int ownerCost, int victimCost) {
	double toBeAdded = (double)victimCost
		/ (ownerCost * RulesClass::Instance->VeteranRatio);
	// Used in experience transfer to get the actual amount carried
	int transffered = (int)(Math::min(vstruct->Veterancy, abs(toBeAdded))
		* (ownerCost * RulesClass::Instance->VeteranRatio));
	if (victimCost < 0 && transffered <= 0.0) {
		vstruct->Reset();
		transffered = 0;
	}
	else {
		vstruct->Add(toBeAdded);
	}
	return transffered;
}

// General function handling experience warhead behaviour
void WarheadTypeExt::ExtData::ApplyModifyExperience(TechnoClass* pTarget, TechnoClass* pOwner) {

	bool expTransfer = this->Experience_Transfer;
	bool expInvertDirection = this->Experience_FirerGetsExp;
	bool expPercFromFirer = this->Experience_CalculatePercentFromFirer;
	auto const pTargetTechno = (pTarget) ? pTarget->GetTechnoType() : nullptr;
	auto const pOwnerTechno = (pOwner) ? pOwner->GetTechnoType() : nullptr;

	int expGain = 0;
	// Percent experience gain
	if (this->Experience_GivenPercent) {
		// Percent from bullet source
		if (expPercFromFirer && pOwnerTechno)
			expGain = (int)((pOwnerTechno->GetActualCost(pOwner->Owner) * this->Experience_GivenPercent));
		// Percent from bullet target
		else if (!expPercFromFirer && pTargetTechno)
			expGain = (int)(pTargetTechno->GetActualCost(pTarget->Owner) * this->Experience_GivenPercent);
		else return;
	}
	// Flat gain
	else {
		expGain = this->Experience_GivenFlat;
	}

	// Exp Transfer
	if (pOwner && pTarget) {
		if (expTransfer && !expInvertDirection) {
			// Transfer from Source to Target
			int diff = AddExpCustom(&pOwner->Veterancy,
				pOwnerTechno->GetActualCost(pOwner->Owner), -expGain);
			AddExpCustom(&pTarget->Veterancy,
				pTargetTechno->GetActualCost(pTarget->Owner), diff);
		}
		else if (expTransfer && expInvertDirection) {
			// Transfer from Target to Source
			int diff = AddExpCustom(&pTarget->Veterancy,
				pTargetTechno->GetActualCost(pTarget->Owner), -expGain);
			AddExpCustom(&pOwner->Veterancy,
				pOwnerTechno->GetActualCost(pOwner->Owner), diff);
		}
	}

	if (!expTransfer && expInvertDirection && pOwner) {
		// Give Source
		AddExpCustom(&pOwner->Veterancy,
			pOwnerTechno->GetActualCost(pOwner->Owner), expGain);
	}
	else if (!expTransfer && !expInvertDirection && pTarget) {
		// Give Target
		AddExpCustom(&pTarget->Veterancy,
			pTargetTechno->GetActualCost(pTarget->Owner), expGain);
	}
}
