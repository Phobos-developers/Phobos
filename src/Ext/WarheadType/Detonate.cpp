#include "Body.h"

#include <InfantryClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include "../../Utilities/Helpers.Alex.h"

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

	// List all Warheads here that respect CellSpread
	const bool isCellSpreadWarhead =
		this->RemoveDisguise ||
		this->RemoveMindControl;

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
