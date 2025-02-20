#include "Body.h"

#include <BulletClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

#pragma region Detonation

DEFINE_HOOK(0x46920B, BulletClass_Detonate, 0x6)
{
	GET(BulletClass* const, pBullet, ESI);
	GET_BASE(const CoordStruct*, pCoords, 0x8);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pBullet->WH);
	auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
	auto const pOwner = pBullet->Owner;
	auto const pHouse = pOwner ? pOwner->Owner : nullptr;
	auto const pDecidedHouse = pHouse ? pHouse : pBulletExt->FirerHouse;
	pWHExt->Detonate(pOwner, pDecidedHouse, pBulletExt, *pCoords);
	pWHExt->InDamageArea = false;

	return 0;
}

DEFINE_HOOK(0x489286, MapClass_DamageArea, 0x6)
{
	GET_BASE(const WarheadTypeClass*, pWH, 0x0C);
	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (pWHExt->InDamageArea)
	{
		GET(const CoordStruct*, pCoords, ECX);
		GET_BASE(TechnoClass*, pOwner, 0x08);
		GET_BASE(HouseClass*, pHouse, 0x14);

		auto const pDecidedHouse = !pHouse && pOwner ? pOwner->Owner : pHouse;
		pWHExt->Detonate(pOwner, pDecidedHouse, nullptr, *pCoords);
	}

	return 0;
}
#pragma endregion

DEFINE_HOOK(0x48A551, WarheadTypeClass_AnimList_SplashList, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	GET(int, nDamage, EDI);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis);
	auto const animTypes = pWHExt->SplashList.GetElements(RulesClass::Instance->SplashList);
	pWHExt->Splashed = true;

	int idx = pWHExt->SplashList_PickRandom ?
		ScenarioClass::Instance->Random.RandomRanged(0, animTypes.size() - 1) :
		std::min(animTypes.size() * 35 - 1, (size_t)nDamage) / 35;

	R->EAX(animTypes.size() > 0 ? animTypes[idx] : nullptr);
	return 0x48A5AD;
}

DEFINE_HOOK(0x48A5BD, SelectDamageAnimation_PickRandom, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	return pWHExt->AnimList_PickRandom ? 0x48A5C7 : 0;
}

DEFINE_HOOK(0x48A5B3, SelectDamageAnimation_CritAnim, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	GET(int, nDamage, EDI);

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	if (pWHExt->Crit_Active && pWHExt->Crit_AnimList.size() && !pWHExt->Crit_AnimOnAffectedTargets)
	{
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

DEFINE_HOOK(0x44224F, BuildingClass_ReceiveDamage_DamageSelf, 0x5)
{
	enum { SkipCheck = 0x442268 };

	REF_STACK(args_ReceiveDamage const, receiveDamageArgs, STACK_OFFSET(0x9C, 0x4));

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(receiveDamageArgs.WH))
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

DEFINE_HOOK(0x489B49, MapClass_DamageArea_Rocker, 0xA)
{
	GET_BASE(WarheadTypeClass*, pWH, 0xC);
	GET_STACK(int, damage, STACK_OFFSET(0xE0, -0xBC));

	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
	double rocker = pWHExt->Rocker_AmplitudeOverride.Get(damage);
	rocker *= 0.01 * pWHExt->Rocker_AmplitudeMultiplier;

	_asm fld rocker

	return 0x489B53;
}

#pragma region Nonprovocative

// Do not retaliate against being hit by these Warheads.
DEFINE_HOOK(0x708B0B, TechnoClass_AllowedToRetaliate_Nonprovocative, 0x5)
{
	enum { SkipEvents = 0x708B17 };

	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0x18, 0x8));

	auto const pTypeExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	return pTypeExt->Nonprovocative ? SkipEvents : 0;
}

// Do not spring 'attacked' trigger events by these Warheads.
DEFINE_HOOK(0x5F57CF, ObjectClass_ReceiveDamage_Nonprovocative, 0x6)
{
	enum { SkipEvents = 0x5F580C };

	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0x24, 0xC));

	auto const pTypeExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	return pTypeExt->Nonprovocative ? SkipEvents : 0;
}

// Do not consider ToProtect technos hit by weapon as having been attacked e.g provoking response from AI.
DEFINE_HOOK(0x7027E6, TechnoClass_ReceiveDamage_Nonprovocative, 0x8)
{
	enum { SkipGameCode = 0x7027EE };

	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pSource, EAX);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0xC4, 0xC));

	auto const pTypeExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	if (!pTypeExt->Nonprovocative)
	{
		pThis->BaseIsAttacked(pSource);

		return SkipGameCode;
	}

	return SkipGameCode;
}

// Do not consider Whiner=true team members hit by weapon as having been attacked e.g provoking response from AI.
DEFINE_HOOK(0x4D7493, FootClass_ReceiveDamage_Nonprovocative, 0x5)
{
	enum { SkipChecks = 0x4D74CD, SkipEvents = 0x4D74A3 };

	GET(TechnoClass*, pSource, EBX);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0x1C, 0xC));

	if (!pSource)
		return SkipChecks;

	auto const pTypeExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	return pTypeExt->Nonprovocative ? SkipEvents : 0;
}

// Suppress harvester under attack notification - Also covered by Ares' Malicious.
DEFINE_HOOK(0x7384BD, UnitClass_ReceiveDamage_Nonprovocative, 0x6)
{
	enum { SkipEvents = 0x738535 };

	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0x44, 0xC));

	auto const pTypeExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	return pTypeExt->Nonprovocative ? SkipEvents : 0;
}

// Do not consider buildings hit by weapon as having been attacked e.g provoking response from AI.
DEFINE_HOOK(0x442290, BuildingClass_ReceiveDamage_Nonprovocative1, 0x6)
{
	enum { SkipEvents = 0x4422C1 };

	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0x9C, 0xC));

	auto const pTypeExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	return pTypeExt->Nonprovocative ? SkipEvents : 0;
}

// Suppress all events and alerts that come from attacking a building, unlike Ares' Malicious this includes all EVA notifications AND events
DEFINE_HOOK(0x442956, BuildingClass_ReceiveDamage_Nonprovocative2, 0x6)
{
	enum { SkipEvents = 0x442980 };

	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0x9C, 0xC));

	auto const pTypeExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	return pTypeExt->Nonprovocative ? SkipEvents : 0;
}

#pragma endregion

DEFINE_HOOK(0x4D73DE, FootClass_ReceiveDamage_RemoveParasite, 0x5)
{
	enum { Continue = 0x4D73E3, Skip = 0x4D7413 };

	GET(WarheadTypeClass*, pWarhead, EBP);
	GET(int*, damage, EDI);

	auto const pTypeExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	if (!pTypeExt->RemoveParasite.Get(*damage < 0))
		return Skip;

	return Continue;
}
