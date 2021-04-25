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
		this->GattlingStage > 0 ||
		this->GattlingRateUp != 0 ||
		this->ReloadAmmo != 0;

	const float cellSpread = this->OwnerObject()->CellSpread;
	if (cellSpread && isCellSpreadWarhead) {
		auto items = Helpers::Alex::getCellSpreadItems(coords, cellSpread, true);
		for (auto pTarget : items) {
			this->DetonateOnOneUnit(pHouse, pTarget);
		}
	}
	else if (pBullet && isCellSpreadWarhead) {
		if (auto pTarget = abstract_cast<TechnoClass*>(pBullet->Target)) {
			this->DetonateOnOneUnit(pHouse, pTarget);
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

	if (this->GattlingStage > 0) {
		this->ApplyGattlingStage(pTarget, this->GattlingStage);
	}
	
	if (this->GattlingRateUp != 0) {
		this->ApplyGattlingRateUp(pTarget, this->GattlingRateUp);
	}

	if (this->ReloadAmmo != 0)
	{
		this->ApplyReloadAmmo(pTarget, this->ReloadAmmo);
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

void WarheadTypeExt::ExtData::ApplyGattlingStage(TechnoClass* pTarget, int Stage)
{
	auto pData = pTarget->GetTechnoType();
	if (pData->IsGattling)
	{
		// if exceeds, pick the largest stage
		if (Stage > pData->WeaponStages)
		{
			Stage = pData->WeaponStages;
		}

		pTarget->CurrentGattlingStage = Stage - 1;
		if (Stage == 1) {
			pTarget->GattlingValue = 0;
			pTarget->unknown_bool_4B8 = false;
		}
		else {
			pTarget->GattlingValue = pTarget->Veterancy.IsElite() ? pData->EliteStage[Stage - 2] : pData->WeaponStage[Stage - 2];
			pTarget->unknown_bool_4B8 = true;
		}
	}
}

void WarheadTypeExt::ExtData::ApplyGattlingRateUp(TechnoClass* pTarget, int RateUp)
{
	auto pData = pTarget->GetTechnoType();
	if (pData->IsGattling) {
		auto curValue = pTarget->GattlingValue + RateUp;
		auto maxValue = pTarget->Veterancy.IsElite() ? pData->EliteStage[pData->WeaponStages - 1] : pData->WeaponStage[pData->WeaponStages - 1];
		
		//set current weapon stage manually
		if (curValue <= 0) {
			pTarget->GattlingValue = 0;
			pTarget->CurrentGattlingStage = 0;
			pTarget->unknown_bool_4B8 = false;
		}
		else if (curValue >= maxValue) {
			pTarget->GattlingValue = maxValue;
			pTarget->CurrentGattlingStage = pData->WeaponStages - 1;
			pTarget->unknown_bool_4B8 = true;
		}
		else {
			pTarget->GattlingValue = curValue;
			pTarget->unknown_bool_4B8 = true;
			for (int i = 0; i < pData->WeaponStages; i++) {
				if (pTarget->Veterancy.IsElite() && curValue < pData->EliteStage[i]) {
					pTarget->CurrentGattlingStage = i;
					break;
				}
				else if (curValue < pData->WeaponStage[i]) {
					pTarget->CurrentGattlingStage = i;
					break;
				}
			}
		}
	}
}

void WarheadTypeExt::ExtData::ApplyReloadAmmo(TechnoClass* pTarget, int ReloadAmount)
{
	auto pData = pTarget->GetTechnoType();
	if (pData->Ammo > 0)
	{
		auto const ammo = pTarget->Ammo + ReloadAmount;
		pTarget->Ammo = Math::clamp(ammo, 0, pData->Ammo);
	}
}
