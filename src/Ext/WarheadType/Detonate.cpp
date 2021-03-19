#include "Body.h"

#include <InfantryClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include "../../Utilities/Helpers.Alex.h"

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

		if (this->RemoveDisguise) {
			auto applyRemoveDisguiseToInf = [this, &pHouse](AbstractClass* pTechno)
			{
				if (pTechno->WhatAmI() == AbstractType::Infantry)
				{
					auto pInf = abstract_cast<InfantryClass*>(pTechno);
					if (pInf->IsDisguised())
					{
						bool bIsAlliedWith = pHouse->IsAlliedWith(pInf);
						if (this->RemoveDisguise_AffectAllies || (!this->RemoveDisguise_AffectAllies && !bIsAlliedWith))
							pInf->ClearDisguise();
					}
				}
			};

			auto cellSpread = this->OwnerObject()->CellSpread;
			if (this->RemoveDisguise_ApplyCellSpread && cellSpread) {
				const auto items = Helpers::Alex::getCellSpreadItems(coords, cellSpread, true);
				for (auto member : items)
					applyRemoveDisguiseToInf(member);
			}
			else if (pBullet){
				applyRemoveDisguiseToInf(pBullet->Target);
			}
		}

		if (this->RemoveMindControl) {
			auto applyRemoveMindControl = [this, &pHouse](TechnoClass* pTechno)
			{
				if (auto pController = pTechno->MindControlledBy)
				{
					bool bIsAlliedWith = pHouse->IsAlliedWith(pTechno);
					if (this->RemoveMindControl_AffectAllies || (!this->RemoveMindControl_AffectAllies && !bIsAlliedWith))
					{
						pTechno->MindControlledBy->CaptureManager->FreeUnit(pTechno);
						if (!pTechno->IsHumanControlled)
							pTechno->QueueMission(Mission::Hunt, false);
					}
				}
			};

			auto cellSpread = this->OwnerObject()->CellSpread;
			if (this->RemoveDisguise_ApplyCellSpread && cellSpread) {
				const auto items = Helpers::Alex::getCellSpreadItems(coords, cellSpread, true);
				for (auto member : items)
					applyRemoveMindControl(member);
			}
			else if (pBullet) {
				if (auto pTarget = abstract_cast<TechnoClass*>(pBullet->Target)) {
					applyRemoveMindControl(pTarget);
				}
			}
		}
	}
}
