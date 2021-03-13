#include <Helpers/Macro.h>
#include <WarheadTypeClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <MapClass.h>
#include "Body.h"
#include <ScenarioClass.h>
#include <InfantryClass.h>
#include <TechnoClass.h>

#include "../../Utilities/Helpers.Alex.h"

/*
DEFINE_HOOK(46920B, BulletClass_Detonate, 6)
{
	GET(BulletClass * const, pThis, ESI);
	GET_BASE(const CoordStruct * const, pCoordsDetonation, 0x8);

	auto const pWH = pThis->WH;
	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	auto const pThisHouse = pThis->Owner ? pThis->Owner->Owner : nullptr;

	return 0;
}
*/

DEFINE_HOOK(489286, MapClass_DamageArea, 6)
{
	GET(const CoordStruct*, pCoordsDetonation, ECX);
	// GET(const int, Damage, EDX);

	// GET_BASE(const TechnoClass*, SourceObject, 0x08);
	GET_BASE(const WarheadTypeClass*, pWH, 0x0C);
	// GET_BASE(const bool, AffectsTiberium, 0x10);
	GET_BASE(HouseClass*, pThisHouse, 0x14);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (pThisHouse) {
		if (pWHExt->BigGap) {
			for (auto pOtherHouse : *HouseClass::Array) {
				if (pOtherHouse->ControlledByHuman() &&	    // Not AI
					!pOtherHouse->IsObserver() &&           // Not Observer
					!pOtherHouse->Defeated &&               // Not Defeated
					pOtherHouse != pThisHouse &&            // Not pThisHouse
					!pThisHouse->IsAlliedWith(pOtherHouse)) // Not Allied
				{
					pOtherHouse->ReshroudMap();
				}
			}
		}

		if (pWHExt->SpySat) {
			MapClass::Instance->Reveal(pThisHouse);
		}

		if (pWHExt->TransactMoney != 0) {
			pThisHouse->TransactMoney(pWHExt->TransactMoney);
		}

		if (pWHExt->RemoveDisguise) {
			auto applyRemoveDisguiseToInf = [&pWHExt, &pThisHouse](AbstractClass* pTechno)
			{
				if (pTechno->WhatAmI() == AbstractType::Infantry)
				{
					auto pInf = abstract_cast<InfantryClass*>(pTechno);
					if (pInf->IsDisguised())
					{
						bool bIsAlliedWith = pThisHouse->IsAlliedWith(pInf);
						if (pWHExt->RemoveDisguise_AffectAllies || (!pWHExt->RemoveDisguise_AffectAllies && !bIsAlliedWith))
							pInf->ClearDisguise();
					}
				}
			};
			
			auto coords = *pCoordsDetonation;
			auto CellSpread = pWH->CellSpread;
			if (pWHExt->RemoveDisguise_ApplyCellSpread && CellSpread) {
				const auto items = Helpers::Alex::getCellSpreadItems(coords, CellSpread, true);
				for (auto member : items)
					applyRemoveDisguiseToInf(member);
			}
		}

		if (pWHExt->RemoveMindControl) {
			auto applyRemoveMindControl = [&pWHExt, &pThisHouse](TechnoClass* pTechno)
			{
				if (auto pController = pTechno->MindControlledBy)
				{
					bool bIsAlliedWith = pThisHouse->IsAlliedWith(pTechno);
					if (pWHExt->RemoveMindControl_AffectAllies || (!pWHExt->RemoveMindControl_AffectAllies && !bIsAlliedWith))
					{
						pTechno->MindControlledBy->CaptureManager->FreeUnit(pTechno);
						if (!pTechno->IsHumanControlled)
							pTechno->QueueMission(Mission::Hunt, false);
					}
				}
			};

			auto coords = *pCoordsDetonation;
			auto CellSpread = pWH->CellSpread;
			if (pWHExt->RemoveDisguise_ApplyCellSpread && CellSpread) {
				const auto items = Helpers::Alex::getCellSpreadItems(coords, CellSpread, true);
				for (auto member : items)
					applyRemoveMindControl(member);
			}
		}
	}

	return 0;
}

DEFINE_HOOK(48A512, WarheadTypeClass_SplashList, 6) 
{
	GET(WarheadTypeClass* const, pThis, ESI);
	if (!pThis->Conventional) return 0;
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	if (pWHExt->SplashList.size()) {
		GET(int, nDamage, ECX);
		int idx = pWHExt->SplashList_PickRandom ?
			ScenarioClass::Instance->Random.RandomRanged(0, pWHExt->SplashList.size() - 1) :
			std::min(pWHExt->SplashList.size() * 35 - 1, (size_t)nDamage) / 35;
		R->EAX<AnimTypeClass*>(pWHExt->SplashList[idx]);
		return 0x48A5AD;
	}
	return 0;
}