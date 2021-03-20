#include "Body.h"

#include <InfantryClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include "../../Utilities/Helpers.Alex.h"

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

	std::vector<TechnoClass*> cellSpreadItems;
	const float cellSpread = this->OwnerObject()->CellSpread;
	if (cellSpread) {

		// List all Warheads here that respect CellSpread
		const bool isCellSpreadWarhead =
			this->RemoveDisguise ||
			this->RemoveMindControl;

		if (isCellSpreadWarhead) {
			cellSpreadItems = Helpers::Alex::getCellSpreadItems(coords, cellSpread, true);
		}
	}

	if (this->RemoveDisguise) {
		auto applyRemoveDisguiseToInf = [this, &pHouse](AbstractClass* pTechno) {
			if (pTechno->WhatAmI() == AbstractType::Infantry) {
				auto pInf = abstract_cast<InfantryClass*>(pTechno);
				if (pInf->IsDisguised()) {
					if (this->CanTargetHouse(pHouse, pInf)) {
						pInf->ClearDisguise();
					}
				}
			}
		};

		if (cellSpread) {
			for (auto member : cellSpreadItems) {
				applyRemoveDisguiseToInf(member);
			}
		}
		else if (pBullet) {
			applyRemoveDisguiseToInf(pBullet->Target);
		}
	}

	if (this->RemoveMindControl) {
		auto applyRemoveMindControl = [this, &pHouse](TechnoClass* pTechno) {
			if (auto pController = pTechno->MindControlledBy) {
				if (this->CanTargetHouse(pHouse, pTechno)) {
					pTechno->MindControlledBy->CaptureManager->FreeUnit(pTechno);
					if (!pTechno->IsHumanControlled) {
						pTechno->QueueMission(Mission::Hunt, false);
					}
				}
			}
		};

		if (cellSpread) {
			for (auto member : cellSpreadItems) {
				applyRemoveMindControl(member);
			}
		}
		else if (pBullet) {
			if (auto pTarget = abstract_cast<TechnoClass*>(pBullet->Target)) {
				applyRemoveMindControl(pTarget);
			}
		}
	}
}
