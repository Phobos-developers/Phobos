#include "Body.h"

#include <BulletClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>

#pragma region DETONATION

bool DetonationInDamageArea = true;

DEFINE_HOOK(46920B, BulletClass_Detonate, 6)
{
	GET(BulletClass* const, pThis, ESI);
	GET_BASE(const CoordStruct*, pCoords, 0x8);

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH)) {
		auto const pOwner = pThis->Owner ? pThis->Owner : nullptr;
		auto const pHouse = pOwner ? pOwner->Owner : nullptr;

		pWHExt->Detonate(pOwner, pHouse, pThis, *pCoords);
	}

	DetonationInDamageArea = false;

	return 0;
}

DEFINE_HOOK(489286, MapClass_DamageArea, 6)
{
	if (DetonationInDamageArea) {
		// GET(const int, Damage, EDX);
		// GET_BASE(const bool, AffectsTiberium, 0x10);

		GET(const CoordStruct*, pCoords, ECX);
		GET_BASE(TechnoClass*, pOwner, 0x08);
		GET_BASE(const WarheadTypeClass*, pWH, 0x0C);
		GET_BASE(HouseClass*, pHouse, 0x14);

		if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
			pWHExt->Detonate(pOwner, pHouse, nullptr, *pCoords);
		}
	}

	DetonationInDamageArea = true;

	return 0;
}
#pragma endregion

DEFINE_HOOK(48A512, WarheadTypeClass_SplashList, 6) 
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	if (pWHExt && pThis->Conventional && pWHExt->SplashList.size()) {
		GET(int, nDamage, ECX);
		int idx = pWHExt->SplashList_PickRandom ?
			ScenarioClass::Instance->Random.RandomRanged(0, pWHExt->SplashList.size() - 1) :
			std::min(pWHExt->SplashList.size() * 35 - 1, (size_t)nDamage) / 35;
		R->EAX<AnimTypeClass*>(pWHExt->SplashList[idx]);
		return 0x48A5AD;
	}
	return 0;
}

DEFINE_HOOK(6FC339, TechnoClass_CanFire_InsufficientFunds, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);

	// Checking for nulptr is not required here, since the game has already executed them before calling the hook
	const auto pWH = pWeapon->Warhead;
	
	if (const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
		const int nMoney = pWHExt->TransactMoney;
		if (nMoney < 0 && pThis->Owner->Available_Money() < -nMoney) {
			//VoxClass::Play("EVA_InsufficientFunds");
			return 0x6FCB7E;
		}
	}
	return 0;
}
