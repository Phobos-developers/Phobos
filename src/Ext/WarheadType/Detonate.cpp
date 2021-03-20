#include "Body.h"

#include <InfantryClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include "../../Utilities/Helpers.Alex.h"

void applyRemoveMindControl(WarheadTypeExt::ExtData* pWHExt, HouseClass* pHouse, TechnoClass* pTechno) {
	if (auto pController = pTechno->MindControlledBy) {
		if (pWHExt->CanTargetHouse(pHouse, pTechno)) {
			pTechno->MindControlledBy->CaptureManager->FreeUnit(pTechno);
			if (!pTechno->IsHumanControlled) {
				pTechno->QueueMission(Mission::Hunt, false);
			}
		}
	}
};

void applyRemoveDisguiseToInf(WarheadTypeExt::ExtData* pWHExt, HouseClass* pHouse, TechnoClass* pTechno) {
	if (pTechno->WhatAmI() == AbstractType::Infantry) {
		auto pInf = abstract_cast<InfantryClass*>(pTechno);
		if (pInf->IsDisguised()) {
			if (pWHExt->CanTargetHouse(pHouse, pInf)) {
				pInf->ClearDisguise();
			}
		}
	}
};

void DetonateOnOneUnit(WarheadTypeExt::ExtData* pWHExt, HouseClass* pHouse, TechnoClass* pTarget) {
	if (pWHExt->RemoveDisguise) {
		applyRemoveDisguiseToInf(pWHExt, pHouse, pTarget);
	}

	if (pWHExt->RemoveMindControl) {
		applyRemoveMindControl(pWHExt, pHouse, pTarget);
	}
	
}

bool WarheadTypeExt::ExtData::CanTargetHouse(HouseClass* pHouse, TechnoClass* pTechno)
{
	if (pHouse && pTechno) {
		if (this->AffectsOwner && pTechno->Owner == pHouse) {
			return true;
		}

		bool isAllies = pHouse->IsAlliedWith(pTechno);

		if (this->OwnerObject()->AffectsAllies && isAllies) {
			return true;
		}

		if (this->AffectsEnemies && !isAllies) {
			return true;
		}

		return false;
	}
	return true;
}

void WarheadTypeExt::ExtData::Detonate(HouseClass* pHouse, BulletClass* pBullet, CoordStruct coords)
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
			DetonateOnOneUnit(this, pHouse, pTarget);
		}
	}
	else if (isCellSpreadWarhead && pBullet) {
		if (auto pTarget = abstract_cast<TechnoClass*>(pBullet->Target)){
			DetonateOnOneUnit(this, pHouse, pTarget);
		}
	}
}
