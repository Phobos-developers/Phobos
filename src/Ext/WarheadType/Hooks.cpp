#include "Body.h"

#include <BulletClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

#pragma region DETONATION

namespace Detonation
{
	bool InDamageArea = true;
}

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

	Detonation::InDamageArea = false;

	return 0;
}

DEFINE_HOOK(0x46A290, BulletClass_Detonate_Return, 0x5)
{
	Detonation::InDamageArea = true;
	return 0;
}

DEFINE_HOOK(0x489286, MapClass_DamageArea, 0x6)
{
	if (Detonation::InDamageArea)
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

// Cylinder CellSpread
DEFINE_HOOK(0x489430, MapClass_DamageArea_Cylinder_1, 0x7)
{
	//GET(int, nDetoCrdZ, EDX);
	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET_STACK(int, nVictimCrdZ, STACK_OFFSET(0xE0, -0x5C));

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (pWHExt && pWHExt->CellSpread_Cylinder)
	{
		R->EDX(nVictimCrdZ);
	}

	return 0;
}

DEFINE_HOOK(0x4894C1, MapClass_DamageArea_Cylinder_2, 0x5)
{
	//GET(int, nDetoCrdZ, EDX);
	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, ESI);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (pWHExt && pWHExt->CellSpread_Cylinder)
	{
		R->EDX(nVictimCrdZ);
	}

	return 0;
}

DEFINE_HOOK(0x48979C, MapClass_DamageArea_Cylinder_3, 0x8)
{
	//GET(int, nDetoCrdZ, ECX);
	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, EDX);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (pWHExt && pWHExt->CellSpread_Cylinder)
	{
		R->ECX(nVictimCrdZ);
	}

	return 0;
}

DEFINE_HOOK(0x4897C3, MapClass_DamageArea_Cylinder_4, 0x5)
{
	//GET(int, nDetoCrdZ, ECX);
	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, EDX);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (pWHExt && pWHExt->CellSpread_Cylinder)
	{
		R->ECX(nVictimCrdZ);
	}

	return 0;
}

DEFINE_HOOK(0x48985A, MapClass_DamageArea_Cylinder_5, 0x5)
{
	//GET(int, nDetoCrdZ, ECX);
	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, EDX);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (pWHExt && pWHExt->CellSpread_Cylinder)
	{
		R->ECX(nVictimCrdZ);
	}

	return 0;
}

DEFINE_HOOK(0x4898BF, MapClass_DamageArea_Cylinder_6, 0x5)
{
	//GET(int, nDetoCrdZ, EDX);
	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET(int, nVictimCrdZ, ECX);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (pWHExt && pWHExt->CellSpread_Cylinder)
	{
		R->EDX(nVictimCrdZ);
	}

	return 0;
}

// AffectsInAir and AffectsOnFloor
DEFINE_HOOK(0x489416, MapClass_DamageArea_CheckHeight_1, 0x6)
{
	enum { SkipThisObject = 0x489547 };

	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET(ObjectClass*, pObject, EBX);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (!pWHExt || !pObject ||
		((pWHExt->AffectsInAir && pObject->IsInAir()) ||
			(pWHExt->AffectsOnFloor && !pObject->IsInAir())))
	{
		return 0;
	}

	return SkipThisObject;
}

DEFINE_HOOK(0x489710, MapClass_DamageArea_CheckHeight_2, 0x7)
{
	enum { SkipThisObject = 0x4899B3 };

	GET_BASE(WarheadTypeClass* const, pWH, 0x0C);
	GET(ObjectClass*, pObject, ESI);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (!pWHExt || !pObject ||
		((pWHExt->AffectsInAir && pObject->IsInAir()) ||
			(pWHExt->AffectsOnFloor && !pObject->IsInAir())))
	{
		return 0;
	}

	return SkipThisObject;
}

#pragma endregion

DEFINE_HOOK(0x48A551, WarheadTypeClass_AnimList_SplashList, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	GET(int, nDamage, EDI);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);
	pWHExt->Splashed = true;
	auto animTypes = pWHExt->SplashList.GetElements(RulesClass::Instance->SplashList);

	int idx = pWHExt->SplashList_PickRandom ?
		ScenarioClass::Instance->Random.RandomRanged(0, animTypes.size() - 1) :
		std::min(animTypes.size() * 35 - 1, (size_t)nDamage) / 35;

	R->EAX(animTypes.size() > 0 ? animTypes[idx] : nullptr);
	return 0x48A5AD;
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

	if (pWHExt && pWHExt->Crit_Active && pWHExt->Crit_AnimList.size() && !pWHExt->Crit_AnimOnAffectedTargets)
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

	pWHExt->Splashed = false;

	if (damage == 0 && !pWHExt->CreateAnimsOnZeroDamage)
		return NoAnim;
	else if (damage < 0)
		damage = -damage;

	R->EDI(damage);
	R->ESI(warhead);
	return SkipGameCode;
}

#pragma region NegativeDamageModifiers

namespace NegativeDamageTemp
{
	bool ApplyNegativeDamageModifiers = false;
}

DEFINE_HOOK(0x4891AF, GetTotalDamage_NegativeDamageModifiers1, 0x6)
{
	enum { ApplyModifiers = 0x4891C6 };

	GET(WarheadTypeClass* const, pWarhead, EDI);
	GET(int, damage, ESI);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	if (damage < 0 && pWHExt->ApplyModifiersOnNegativeDamage)
	{
		NegativeDamageTemp::ApplyNegativeDamageModifiers = true;
		return ApplyModifiers;
	}

	return 0;
}

DEFINE_HOOK(0x48922D, GetTotalDamage_NegativeDamageModifiers2, 0x5)
{
	enum { SkipGameCode = 0x489235 };

	GET(int, damage, ESI);

	if (NegativeDamageTemp::ApplyNegativeDamageModifiers)
	{
		NegativeDamageTemp::ApplyNegativeDamageModifiers = false;
		R->ECX(damage);

	}
	else
	{
		R->ECX(damage < 0 ? 0 : damage);
	}


	return SkipGameCode;
}

#pragma endregion


DEFINE_HOOK(0x701A54, TechnoClass_ReceiveDamage_PenetratesIronCurtain, 0x6)
{
	enum { AllowDamage = 0x701AAD };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0xC4, 0xC));

	if (WarheadTypeExt::ExtMap.Find(pWarhead)->CanAffectInvulnerable(pThis))
		return AllowDamage;

	return 0;
}

DEFINE_HOOK(0x489968, Explosion_Damage_PenetratesIronCurtain, 0x5)
{
	enum { BypassInvulnerability = 0x48996D };

	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);

	if (WarheadTypeExt::ExtMap.Find(pWarhead)->PenetratesIronCurtain)
		return BypassInvulnerability;

	return 0;
}
