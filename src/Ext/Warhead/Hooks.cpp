#include <Helpers/Macro.h>
#include <WarheadTypeClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <MapClass.h>
#include "Body.h"

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

	if (pWHExt->BigGap) {
		ReshroudMapForOpponents(pThisHouse);
	}

	if (pWHExt->SpySat) {
		MapClass::Instance->Reveal(pThisHouse);
	}

	if (pWHExt->TransactMoney != 0) {
		pThisHouse->TransactMoney(pWHExt->TransactMoney);
	}

	return 0;
}