#include <Helpers/Macro.h>
#include <WarheadTypeClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <MapClass.h>
#include "Body.h"
#include <ScenarioClass.h>

void ReshroudMapForOpponents(HouseClass* pThisHouse) {
	for (auto pOtherHouse : *HouseClass::Array) {

		if (pOtherHouse->ControlledByHuman() &&
			!pOtherHouse->IsObserver() &&
			!pOtherHouse->Defeated &&
			pOtherHouse != pThisHouse &&
			!pOtherHouse->IsAlliedWith(pThisHouse)
			)
		{
			pOtherHouse->ReshroudMap();
		}
	}
}

DEFINE_HOOK(46920B, BulletClass_Detonate, 6)
{
	GET(BulletClass * const, pThis, ESI);
	//GET_BASE(const CoordStruct * const, pCoordsDetonation, 0x8);

	auto const pWH = pThis->WH;
	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	auto const pThisHouse = pThis->Owner ? pThis->Owner->Owner : nullptr;

	if (pThisHouse) {
		if (pWHExt->BigGap) {
			ReshroudMapForOpponents(pThisHouse);
		}

		if (pWHExt->SpySat) {
			MapClass::Instance->Reveal(pThisHouse);
		}

		if (pWHExt->TransactMoney != 0) {
			pThisHouse->TransactMoney(pWHExt->TransactMoney);
		}
	}

	return 0;
}

DEFINE_HOOK(48A512, WarheadTypeClass_SplashList, 6) 
{

	GET(WarheadTypeClass* const, pThis, ESI);

	if (!pThis->Conventional) return 0;
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	if (pWHExt->SplashList.Count) {
		GET(int, Damage, ECX);
		R->EAX(pWHExt->SplashList.GetItem(
			pWHExt->SplashList_PickRandom ?
			ScenarioClass::Instance->Random.RandomRanged(0, pWHExt->SplashList.Count - 1) :
			std::min(pWHExt->SplashList.Count * 35 - 1, Damage) / 35
		));
		return 0x48A5AD;
	}

	return 0;
}