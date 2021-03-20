#include "Body.h"

#include <BulletClass.h>
#include <ScenarioClass.h>

#pragma region DETONATION

bool DetonationInDamageArea = true;

DEFINE_HOOK(46920B, BulletClass_Detonate, 6)
{
	GET(BulletClass * const, pThis, ESI);
	GET_BASE(const CoordStruct * const, pCoords, 0x8);
	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto const pHouse = pThis->Owner ? pThis->Owner->Owner : nullptr;

	pWHExt->Detonate(pHouse, pThis, *pCoords);
	DetonationInDamageArea = false;

	return 0;
}

DEFINE_HOOK(489286, MapClass_DamageArea, 6)
{
	if (DetonationInDamageArea){
		// GET(const int, Damage, EDX);
		// GET_BASE(const TechnoClass*, SourceObject, 0x08);
		// GET_BASE(const bool, AffectsTiberium, 0x10);

		GET(const CoordStruct*, pCoords, ECX);
		GET_BASE(HouseClass*, pHouse, 0x14);
		GET_BASE(const WarheadTypeClass*, pWH, 0x0C);
		auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

		pWHExt->Detonate(pHouse, nullptr, *pCoords);
	}
	DetonationInDamageArea = true;
	return 0;
}
#pragma endregion

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
