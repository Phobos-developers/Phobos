#include "Body.h"

#include <BulletClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

#pragma region DETONATION

bool DetonationInDamageArea = true;

DEFINE_HOOK(0x46920B, BulletClass_Detonate, 0x6)
{
	GET(BulletClass* const, pBullet, ESI);

	auto const pWH = pBullet ? pBullet->WH : nullptr;

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH))
	{
		GET_BASE(const CoordStruct*, pCoords, 0x8);
		auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
		auto const pOwner = pBullet->Owner;
		auto const pHouse = pOwner ? pOwner->Owner : nullptr;
		auto const pDecidedHouse = pHouse ? pHouse : pBulletExt->FirerHouse;

		pWHExt->Detonate(pOwner, pDecidedHouse, pBulletExt, *pCoords);
	}

	DetonationInDamageArea = false;

	return 0;
}

DEFINE_HOOK(0x46A290, BulletClass_Detonate_Return, 0x5)
{
	DetonationInDamageArea = true;
	return 0;
}

DEFINE_HOOK(0x489286, MapClass_DamageArea, 0x6)
{
	if (DetonationInDamageArea)
	{
		// GET(const int, Damage, EDX);
		// GET_BASE(const bool, AffectsTiberium, 0x10);

		GET_BASE(const WarheadTypeClass*, pWH, 0x0C);

		if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH))
		{
			GET(const CoordStruct*, pCoords, ECX);
			GET_BASE(TechnoClass*, pOwner, 0x08);
			GET_BASE(HouseClass*, pHouse, 0x14);

			auto const pDecidedHouse = !pHouse && pOwner ? pOwner->Owner : pHouse;

			pWHExt->Detonate(pOwner, pDecidedHouse, nullptr, *pCoords);
		}
	}

	return 0;
}
#pragma endregion

DEFINE_HOOK(0x48A551, WarheadTypeClass_AnimList_SplashList, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	if (pWHExt && pWHExt->SplashList.size())
	{
		GET(int, nDamage, ECX);
		int idx = pWHExt->SplashList_PickRandom ?
			ScenarioClass::Instance->Random.RandomRanged(0, pWHExt->SplashList.size() - 1) :
			std::min(pWHExt->SplashList.size() * 35 - 1, (size_t)nDamage) / 35;
		R->EAX(pWHExt->SplashList[idx]);
		return 0x48A5AD;
	}

	return 0;
}

DEFINE_HOOK(0x48A5BD, SelectDamageAnimation_PickRandom, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	return pWHExt && pWHExt->AnimList_PickRandom ? 0x48A5C7 : 0;
}

DEFINE_HOOK(0x48A5B3, SelectDamageAnimation_CritAnim, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	if (pWHExt && pWHExt->HasCrit && pWHExt->Crit_AnimList.size() && !pWHExt->Crit_AnimOnAffectedTargets)
	{
		GET(int, nDamage, ECX);
		int idx = pThis->EMEffect || pWHExt->Crit_AnimList_PickRandom.Get(pWHExt->AnimList_PickRandom) ?
			ScenarioClass::Instance->Random.RandomRanged(0, pWHExt->Crit_AnimList.size() - 1) :
			std::min(pWHExt->Crit_AnimList.size() * 25 - 1, (size_t)nDamage) / 25;
		R->EAX(pWHExt->Crit_AnimList[idx]);
		return 0x48A5AD;
	}

	return 0;
}

DEFINE_HOOK(0x4896EC, Explosion_Damage_DamageSelf, 0x6)
{
	enum { SkipCheck = 0x489702 };

	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead))
	{
		if (pWHExt->AllowDamageOnSelf)
			return SkipCheck;
	}

	return 0;
}

#pragma region Fix_WW_Strength_ReceiveDamage_C4Warhead_Misuses

// Suicide=yes behavior on WeaponTypes
DEFINE_HOOK(0x6FDDCA, TechnoClass_Fire_Suicide, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);

	R->EAX(pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead,
		nullptr, true, false, nullptr));

	return 0x6FDE03;
}

// Kill the vxl unit when flipped over
DEFINE_HOOK(0x70BC6F, TechnoClass_UpdateRigidBodyKinematics_KillFlipped, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);

	R->EAX(pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead,
		pThis->DirectRockerLinkedUnit, true, false, nullptr));

	return 0x70BCA4;
}

// TODO:
// 0x4425C0, BuildingClass_ReceiveDamage_MaybeKillRadioLinks, 0x6
// 0x501477, HouseClass_IHouse_AllToHunt_KillMCInsignificant, 0xA
// 0x7187D2, TeleportLocomotionClass_7187A0_IronCurtainFuckMeUp, 0x8
// 0x718B1E

#pragma endregion Fix_WW_Strength_ReceiveDamage_C4Warhead_Misuse

DEFINE_HOOK(0x48A4F3, SelectDamageAnimation_NegativeZeroDamage, 0x6)
{
	enum { SkipGameCode = 0x48A507, NoAnim = 0x48A618 };

	GET(int, damage, ECX);
	GET(WarheadTypeClass* const, warhead, EDX);

	if (!warhead)
		return NoAnim;

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(warhead);

	if (damage == 0 && !pWHExt->AnimList_ShowOnZeroDamage)
		return NoAnim;
	else if (damage < 0)
		damage = -damage;

	R->EDI(damage);
	R->ESI(warhead);
	return SkipGameCode;
}
