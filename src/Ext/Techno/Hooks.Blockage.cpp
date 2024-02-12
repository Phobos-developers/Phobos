#include "Body.h"

#include <OverlayTypeClass.h>

#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>

/* Hooks & helper functions concerning pathfinding blockages e.g enemy units on occupying cells on the way. */

class PathfindingBlockageHelper
{
public:

	static bool CanTargetObject(TechnoClass* pThis, ObjectClass* pTarget)
	{
		int primaryWeaponIndex = pThis->GetTechnoType()->TurretCount > 0 ? pThis->CurrentWeaponNumber : 0;
		auto const pWeaponPrimary = pThis->GetWeapon(primaryWeaponIndex)->WeaponType;

		if (!pWeaponPrimary || !PathfindingBlockageHelper::CanDealDamageToObject(pWeaponPrimary, pTarget) || pThis->GetFireError(pTarget, primaryWeaponIndex, true) == FireError::ILLEGAL)
		{
			auto pWeaponSecondary = pThis->GetWeapon(1)->WeaponType;

			if (pWeaponSecondary && PathfindingBlockageHelper::CanDealDamageToObject(pWeaponSecondary, pTarget) && pThis->GetFireError(pTarget, 1, true) != FireError::ILLEGAL)
				return true;

			return false;
		}

		return true;
	}

	static bool CanTargetOverlay(TechnoClass* pThis, OverlayTypeClass* pOverlayType)
	{
		if (pThis->GetTechnoType()->LandTargeting == LandTargetingType::Land_Not_OK)
			return false;

		int primaryWeaponIndex = pThis->GetTechnoType()->TurretCount > 0 ? pThis->CurrentWeaponNumber : 0;
		auto pWeaponPrimary = pThis->GetWeapon(primaryWeaponIndex)->WeaponType;

		if (!pWeaponPrimary || ((pWeaponPrimary->Damage < 1 || pWeaponPrimary->NeverUse)
			&& !WeaponTypeExt::ExtMap.Find(pWeaponPrimary)->BlockageTargetingBypassDamageOverride.Get(false))
			|| (!pWeaponPrimary->Warhead->Wall && (!pWeaponPrimary->Warhead->Wood || pOverlayType->Armor != Armor::Wood))
			|| BulletTypeExt::ExtMap.Find(pWeaponPrimary->Projectile)->AAOnly)
		{
			auto pWeaponSecondary = pThis->GetWeapon(1)->WeaponType;

			if (pOverlayType->Wall && pWeaponSecondary && ((pWeaponSecondary->Damage > 0 && !pWeaponPrimary->NeverUse)
				|| WeaponTypeExt::ExtMap.Find(pWeaponSecondary)->BlockageTargetingBypassDamageOverride.Get(false))
				&& (pWeaponSecondary->Warhead->Wall || (pWeaponSecondary->Warhead->Wood && pOverlayType->Armor == Armor::Wood))
				&& !BulletTypeExt::ExtMap.Find(pWeaponSecondary->Projectile)->AAOnly)
			{
				return true;
			}

			return false;
		}

		return true;
	}

	static bool CanDealDamageToObject(WeaponTypeClass* pThis, ObjectClass* pTarget)
	{
		if (!pThis || !pTarget)
			return false;

		auto const pExt = WeaponTypeExt::ExtMap.Find(pThis);

		if (pExt->BlockageTargetingBypassDamageOverride.isset())
			return pExt->BlockageTargetingBypassDamageOverride.Get();

		int damage = pThis->Damage;
		auto pWarhead = pThis->Warhead;

		if (pThis->NeverUse)
			return false;

		if (damage < 1 && pThis->AmbientDamage > 0 && !pExt->AmbientDamage_IgnoreTarget)
		{
			damage = pThis->AmbientDamage;
			pWarhead = pExt->AmbientDamage_Warhead.Get(pWarhead);
		}

		double multiplier = GeneralUtils::GetWarheadVersusArmor(pWarhead, pTarget->GetType()->Armor);

		if (damage * multiplier > 0)
			return true;

		return false;
	}
};

// Hooks

DEFINE_HOOK(0x51C1F1, InfantryClass_CanEnterCell_BlockageOverlay, 0x5)
{
	enum { IsBlockage = 0x51C7D0, Continue = 0x51C20D };

	GET(InfantryClass*, pThis, EBP);
	GET(OverlayTypeClass*, pOverlayType, ESI);

	if (Phobos::Config::UseImprovedPathfindingBlockageHandling)
	{
		if (!PathfindingBlockageHelper::CanTargetOverlay(pThis, pOverlayType))
			return IsBlockage;
	}
	else
	{
		auto const pWeapon = pThis->GetWeapon(0)->WeaponType;

		if (!pWeapon || !pWeapon->Warhead->Wall)
			return IsBlockage;
	}

	return Continue;
}

DEFINE_HOOK(0x51C540, InfantryClass_CanEnterCell_BlockageGate, 0x9)
{
	enum { IsBlockage = 0x51C7D0, Continue = 0x51C549, SkipToNext = 0x51C70F };

	GET(InfantryClass*, pThis, EBP);
	GET(ObjectClass*, pOccupier, ESI);
	GET(Move, currentMoveType, EBX);

	if (Phobos::Config::UseImprovedPathfindingBlockageHandling)
	{
		if (!PathfindingBlockageHelper::CanTargetObject(pThis, pOccupier))
			return IsBlockage;
	}
	else if (!pThis->IsArmed())
	{
		return IsBlockage;
	}

	if (currentMoveType < Move::Destroyable)
		return Continue;

	return SkipToNext;
}

DEFINE_HOOK(0x51C5C8, InfantryClass_CanEnterCell_BlockageGeneral1, 0x6)
{
	enum { IsBlockage = 0x51C7D0, Continue = 0x51C5E0 };

	GET(InfantryClass*, pThis, EBP);
	GET(ObjectClass*, pOccupier, ESI);

	bool condition = false;

	if (Phobos::Config::UseImprovedPathfindingBlockageHandling)
		condition = !PathfindingBlockageHelper::CanTargetObject(pThis, pOccupier);
	else
		condition = pThis->CombatDamage(-1) <= 0;

	if (pOccupier->WhatAmI() != AbstractType::Terrain && condition)
		return IsBlockage;

	return Continue;
}

DEFINE_HOOK(0x51C84A, InfantryClass_CanEnterCell_BlockageGeneral2, 0x7)
{
	enum { IsBlockage = 0x51C7D0, Continue = 0x51C853, Skip = 0x51C864 };

	GET(InfantryClass*, pThis, EBP);
	GET(ObjectClass*, pOccupier, ESI);
	GET(Move, currentMoveType, EBX);

	if (Phobos::Config::UseImprovedPathfindingBlockageHandling)
	{
		if (!PathfindingBlockageHelper::CanTargetObject(pThis, pOccupier))
			return IsBlockage;
	}
	else if (pThis->CombatDamage(-1) <= 0)
	{
		return IsBlockage;
	}

	if (currentMoveType < Move::Destroyable)
		return Continue;

	return Skip;
}

DEFINE_HOOK(0x73F495, UnitClass_CanEnterCell_BlockageOverlay, 0x6)
{
	enum { IsBlockage = 0x73FCD0, Continue = 0x73F4D8 };

	GET(UnitClass*, pThis, EBX);
	GET(OverlayTypeClass*, pOverlayType, ESI);

	if (Phobos::Config::UseImprovedPathfindingBlockageHandling)
	{
		if (!PathfindingBlockageHelper::CanTargetOverlay(pThis, pOverlayType))
			return IsBlockage;
	}
	else
	{
		auto const pWarhead = pThis->GetWeapon(0)->WeaponType->Warhead;

		if (pWarhead->Wall && (!pWarhead->Wood || pOverlayType->Armor != Armor::Wood))
			return IsBlockage;
	}

	return Continue;
}

DEFINE_HOOK(0x73F734, UnitClass_CanEnterCell_BlockageGate, 0x9)
{
	enum { IsBlockage = 0x73FCD0, Continue = 0x73F73D, SkipToNext = 0x73FA87 };

	GET(UnitClass*, pThis, EBX);
	GET(ObjectClass*, pOccupier, ESI);
	GET(Move, currentMoveType, EBP);

	if (Phobos::Config::UseImprovedPathfindingBlockageHandling && !PathfindingBlockageHelper::CanTargetObject(pThis, pOccupier))
		return IsBlockage;

	if (currentMoveType < Move::Destroyable)
		return Continue;

	return SkipToNext;
}

DEFINE_HOOK(0x73FB71, UnitClass_CanEnterCell_BlockageGeneral1, 0x6)
{
	enum { IsBlockage = 0x73FCD0, Continue = 0x73FB96 };

	GET(UnitClass*, pThis, EBX);
	GET(ObjectClass*, pOccupier, ESI);

	if (Phobos::Config::UseImprovedPathfindingBlockageHandling)
	{
		if (!pThis->Type->IsTrain && !PathfindingBlockageHelper::CanTargetObject(pThis, pOccupier))
			return IsBlockage;
	}
	else if (!pThis->Type->IsTrain && !pThis->GetWeapon(0)->WeaponType)
	{
		return IsBlockage;
	}

	return Continue;
}

DEFINE_HOOK(0x73FCA1, UnitClass_CanEnterCell_BlockageGeneral2, 0x6)
{
	enum { IsBlockage = 0x73FCD0, Continue = 0x73FCE2 };

	GET(UnitClass*, pThis, EBX);
	GET(ObjectClass*, pOccupier, ESI);

	bool condition = false;

	if (Phobos::Config::UseImprovedPathfindingBlockageHandling)
		condition = !PathfindingBlockageHelper::CanTargetObject(pThis, pOccupier);
	else
		condition = !pThis->GetWeapon(0)->WeaponType || !pThis->GetWeapon(0)->WeaponType->Projectile->AG;

	if (condition)
		return IsBlockage;

	return Continue;
}
