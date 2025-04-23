#include "Body.h"

#include <OverlayTypeClass.h>
#include <BulletClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Utilities/EnumFunctions.h>

// Compares two weapons and returns index of which one is eligible to fire against current target (0 = first, 1 = second), or -1 if neither works.
int TechnoExt::PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback, bool allowAAFallback)
{
	CellClass* pTargetCell = nullptr;

	// Ignore target cell for airborne target technos.
	if (!pTargetTechno || !pTargetTechno->IsInAir())
	{
		if (auto const pCell = abstract_cast<CellClass*>(pTarget))
			pTargetCell = pCell;
		else if (auto const pObject = abstract_cast<ObjectClass*>(pTarget))
			pTargetCell = pObject->GetCell();
	}

	auto const pWeaponStructOne = pThis->GetWeapon(weaponIndexOne);
	auto const pWeaponStructTwo = pThis->GetWeapon(weaponIndexTwo);

	if (!pWeaponStructOne && !pWeaponStructTwo)
		return -1;
	else if (!pWeaponStructTwo)
		return weaponIndexOne;
	else if (!pWeaponStructOne)
		return weaponIndexTwo;

	auto const pWeaponOne = pWeaponStructOne->WeaponType;
	auto const pWeaponTwo = pWeaponStructTwo->WeaponType;

	if (auto const pSecondExt = WeaponTypeExt::ExtMap.Find(pWeaponTwo))
	{
		if ((pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pSecondExt->CanTarget, true, true)) ||
			(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondExt->CanTarget) ||
				!EnumFunctions::CanTargetHouse(pSecondExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner) ||
				!pSecondExt->HasRequiredAttachedEffects(pTargetTechno, pThis))))
		{
			return weaponIndexOne;
		}
		else if (auto const pFirstExt = WeaponTypeExt::ExtMap.Find(pWeaponOne))
		{
			bool secondIsAA = pTargetTechno && pTargetTechno->IsInAir() && pWeaponTwo->Projectile->AA;
			bool firstAllowedAE = pFirstExt->HasRequiredAttachedEffects(pTargetTechno, pThis);

			if (!allowFallback && (!allowAAFallback || !secondIsAA) && !TechnoExt::CanFireNoAmmoWeapon(pThis, 1) && firstAllowedAE)
				return weaponIndexOne;

			if ((pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pFirstExt->CanTarget, true, true)) ||
				(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pFirstExt->CanTarget) ||
					!EnumFunctions::CanTargetHouse(pFirstExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner) || !firstAllowedAE)))
			{
				return weaponIndexTwo;
			}
		}
	}

	auto const pType = pThis->GetTechnoType();

	// Handle special case with NavalTargeting / LandTargeting.
	if (!pTargetTechno && (pType->NavalTargeting == NavalTargetingType::Naval_Primary ||
		pType->LandTargeting == LandTargetingType::Land_Secondary) &&
		pTargetCell->LandType != LandType::Water && pTargetCell->LandType != LandType::Beach)
	{
		return weaponIndexTwo;
	}

	return -1;
}

void TechnoExt::FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType)
{
	WeaponTypeExt::DetonateAt(pWeaponType, pThis, pThis);
}

bool TechnoExt::CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex)
{
	if (pThis->GetTechnoType()->Ammo > 0)
	{
		if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			if (pThis->Ammo <= pExt->NoAmmoAmount && (pExt->NoAmmoWeapon == weaponIndex || pExt->NoAmmoWeapon == -1))
				return true;
		}
	}

	return false;
}

WeaponTypeClass* TechnoExt::GetDeployFireWeapon(TechnoClass* pThis, int& weaponIndex)
{
	weaponIndex = pThis->GetTechnoType()->DeployFireWeapon;

	if (pThis->WhatAmI() == AbstractType::Unit)
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			// Only apply DeployFireWeapon on vehicles if explicitly set.
			if (!pTypeExt->DeployFireWeapon.isset())
			{
				weaponIndex = 0;
				auto pCell = MapClass::Instance.GetCellAt(pThis->GetMapCoords());

				if (pThis->GetFireError(pCell, 0, true) != FireError::OK)
					weaponIndex = 1;
			}
		}
	}

	if (weaponIndex < 0)
		return nullptr;

	auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

	if (!pWeapon)
	{
		weaponIndex = 0;
		pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

		if (!pWeapon)
			weaponIndex = -1;
	}

	return pWeapon;
}

WeaponTypeClass* TechnoExt::GetDeployFireWeapon(TechnoClass* pThis)
{
	int weaponIndex = 0;
	return TechnoExt::GetDeployFireWeapon(pThis, weaponIndex);
}

WeaponTypeClass* TechnoExt::GetCurrentWeapon(TechnoClass* pThis, int& weaponIndex, bool getSecondary)
{
	if (!pThis)
		return nullptr;

	auto const pType = pThis->GetTechnoType();
	weaponIndex = getSecondary ? 1 : 0;

	if (pType->TurretCount > 0 && !pType->IsGattling)
	{
		if (getSecondary)
		{
			weaponIndex = -1;
			return nullptr;
		}

		weaponIndex = pThis->CurrentWeaponNumber != 0xFFFFFFFF ? pThis->CurrentWeaponNumber : 0;
	}
	else if (pType->IsGattling)
	{
		weaponIndex = pThis->CurrentGattlingStage * 2 + weaponIndex;
	}

	return pThis->GetWeapon(weaponIndex)->WeaponType;
}

WeaponTypeClass* TechnoExt::GetCurrentWeapon(TechnoClass* pThis, bool getSecondary)
{
	int weaponIndex = 0;
	return TechnoExt::GetCurrentWeapon(pThis, weaponIndex, getSecondary);
}

// Gets weapon index for a weapon to use against wall overlay.
int TechnoExt::GetWeaponIndexAgainstWall(TechnoClass* pThis, OverlayTypeClass* pWallOverlayType)
{
	auto const pTechnoType = pThis->GetTechnoType();
	int weaponIndex = -1;
	auto pWeapon = TechnoExt::GetCurrentWeapon(pThis, weaponIndex);

	if ((pTechnoType->TurretCount > 0 && !pTechnoType->IsGattling) || !pWallOverlayType || !pWallOverlayType->Wall || !RulesExt::Global()->AllowWeaponSelectAgainstWalls)
		return weaponIndex;
	else if (weaponIndex == -1)
		return 0;

	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	bool aeForbidsPrimary = pWeaponExt && pWeaponExt->AttachEffect_CheckOnFirer && !pWeaponExt->HasRequiredAttachedEffects(pThis, pThis);

	if (!pWeapon || (!pWeapon->Warhead->Wall && (!pWeapon->Warhead->Wood || pWallOverlayType->Armor != Armor::Wood)) || TechnoExt::CanFireNoAmmoWeapon(pThis, 1) || aeForbidsPrimary)
	{
		int weaponIndexSec = -1;
		pWeapon = TechnoExt::GetCurrentWeapon(pThis, weaponIndexSec, true);
		pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
		bool aeForbidsSecondary = pWeaponExt && pWeaponExt->AttachEffect_CheckOnFirer && !pWeaponExt->HasRequiredAttachedEffects(pThis, pThis);

		if (pWeapon && (pWeapon->Warhead->Wall || (pWeapon->Warhead->Wood && pWallOverlayType->Armor == Armor::Wood))
			&& (!TechnoTypeExt::ExtMap.Find(pTechnoType)->NoSecondaryWeaponFallback || aeForbidsPrimary) && !aeForbidsSecondary)
		{
			return weaponIndexSec;
		}

		return weaponIndex;
	}

	return weaponIndex;
}

void TechnoExt::ApplyKillWeapon(TechnoClass* pThis, TechnoClass* pSource, WarheadTypeClass* pWH)
{
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
	bool hasFilters = pTypeExt->SuppressKillWeapons_Types.size() > 0;

	// KillWeapon can be triggered without the source
	if (pWHExt->KillWeapon && (!pSource || (EnumFunctions::CanTargetHouse(pWHExt->KillWeapon_AffectsHouses, pSource->Owner, pThis->Owner)
		&& EnumFunctions::IsTechnoEligible(pThis, pWHExt->KillWeapon_Affects))))
	{
		if (!pTypeExt->SuppressKillWeapons || (hasFilters && !pTypeExt->SuppressKillWeapons_Types.Contains(pWHExt->KillWeapon)))
			WeaponTypeExt::DetonateAt(pWHExt->KillWeapon, pThis, pSource);
	}

	// KillWeapon.OnFirer must have a source
	if (pWHExt->KillWeapon_OnFirer && pSource && EnumFunctions::CanTargetHouse(pWHExt->KillWeapon_OnFirer_AffectsHouses, pSource->Owner, pThis->Owner)
		&& EnumFunctions::IsTechnoEligible(pThis, pWHExt->KillWeapon_OnFirer_Affects))
	{
		if (!pTypeExt->SuppressKillWeapons || (hasFilters && !pTypeExt->SuppressKillWeapons_Types.Contains(pWHExt->KillWeapon_OnFirer)))
		{
			if (pWHExt->KillWeapon_OnFirer_RealLaunch)
			{
				auto const pWeapon = pWHExt->KillWeapon_OnFirer;

				if (BulletClass* pBullet = pWeapon->Projectile->CreateBullet(pSource, pSource,
					pWeapon->Damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
				{
					pBullet->WeaponType = pWeapon;
					pBullet->MoveTo(pSource->Location, BulletVelocity::Empty);
					BulletExt::ExtMap.Find(pBullet)->FirerHouse = pSource->Owner;
				}
			}
			else
			{
				WeaponTypeExt::DetonateAt(pWHExt->KillWeapon_OnFirer, pSource, pSource);
			}
		}
	}
}

void TechnoExt::ApplyRevengeWeapon(TechnoClass* pThis, TechnoClass* pSource, WarheadTypeClass* pWH)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pTypeExt = pExt->TypeExtData;
	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
	bool hasFilters = pWHExt->SuppressRevengeWeapons_Types.size() > 0;

	if (pTypeExt->RevengeWeapon && EnumFunctions::CanTargetHouse(pTypeExt->RevengeWeapon_AffectsHouses, pThis->Owner, pSource->Owner))
	{
		if (!pWHExt->SuppressRevengeWeapons || (hasFilters && !pWHExt->SuppressRevengeWeapons_Types.Contains(pTypeExt->RevengeWeapon)))
		{
			if (pTypeExt->RevengeWeapon_RealLaunch)
			{
				auto const pWeapon = pTypeExt->RevengeWeapon;

				if (BulletClass* pBullet = pWeapon->Projectile->CreateBullet(pSource, pThis,
					pWeapon->Damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
				{
					pBullet->WeaponType = pWeapon;
					pBullet->MoveTo(pSource->Location, BulletVelocity::Empty);
					BulletExt::ExtMap.Find(pBullet)->FirerHouse = pThis->Owner;
				}
			}
			else
			{
				WeaponTypeExt::DetonateAt(pTypeExt->RevengeWeapon, pSource, pThis);
			}
		}
	}

	for (auto& attachEffect : pExt->AttachedEffects)
	{
		if (!attachEffect->IsActive())
			continue;

		auto const pType = attachEffect->GetType();

		if (!pType->RevengeWeapon)
			continue;

		if (pWHExt->SuppressRevengeWeapons && (!hasFilters || pWHExt->SuppressRevengeWeapons_Types.Contains(pType->RevengeWeapon)))
			continue;

		if (EnumFunctions::CanTargetHouse(pType->RevengeWeapon_AffectsHouses, pThis->Owner, pSource->Owner))
		{
			if (pType->RevengeWeapon_RealLaunch)
			{
				auto const pWeapon = pType->RevengeWeapon;

				if (BulletClass* pBullet = pWeapon->Projectile->CreateBullet(pSource, pThis,
					pWeapon->Damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright))
				{
					pBullet->WeaponType = pWeapon;
					pBullet->MoveTo(pSource->Location, BulletVelocity::Empty);
					BulletExt::ExtMap.Find(pBullet)->FirerHouse = pThis->Owner;
				}
			}
			else
			{
				WeaponTypeExt::DetonateAt(pType->RevengeWeapon, pSource, pThis);
			}
		}
	}
}

int TechnoExt::ExtData::ApplyForceWeaponInRange(TechnoClass* pTarget)
{
	int forceWeaponIndex = -1;
	auto const pThis = this->OwnerObject();
	auto const pTypeExt = this->TypeExtData;

	const bool useAASetting = !pTypeExt->ForceAAWeapon_InRange.empty() && pTarget->IsInAir();
	auto const& weaponIndices = useAASetting ? pTypeExt->ForceAAWeapon_InRange : pTypeExt->ForceWeapon_InRange;
	auto const& rangeOverrides = useAASetting ? pTypeExt->ForceAAWeapon_InRange_Overrides : pTypeExt->ForceWeapon_InRange_Overrides;
	const bool applyRangeModifiers = useAASetting ? pTypeExt->ForceAAWeapon_InRange_ApplyRangeModifiers : pTypeExt->ForceWeapon_InRange_ApplyRangeModifiers;

	const int defaultWeaponIndex = pThis->SelectWeapon(pTarget);
	const int currentDistance = pThis->DistanceFrom(pTarget);
	auto const pDefaultWeapon = pThis->GetWeapon(defaultWeaponIndex)->WeaponType;

	for (size_t i = 0; i < weaponIndices.size(); i++)
	{
		int range = 0;

		// Value below 0 means Range won't be overriden
		if (i < rangeOverrides.size() && rangeOverrides[i] > 0)
			range = static_cast<int>(rangeOverrides[i] * Unsorted::LeptonsPerCell);

		if (weaponIndices[i] >= 0)
		{
			if (range > 0 || applyRangeModifiers)
			{
				auto const pWeapon = weaponIndices[i] == defaultWeaponIndex ? pDefaultWeapon : pThis->GetWeapon(weaponIndices[i])->WeaponType;
				range = range > 0 ? range : pWeapon->Range;

				if (applyRangeModifiers)
					range = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pThis, range);
			}

			if (currentDistance <= range)
			{
				forceWeaponIndex = weaponIndices[i];
				break;
			}
		}
		else
		{
			if (range > 0 || applyRangeModifiers)
			{
				range = range > 0 ? range : pDefaultWeapon->Range;

				if (applyRangeModifiers)
					range = WeaponTypeExt::GetRangeWithModifiers(pDefaultWeapon, pThis, range);
			}

			// Don't force weapon if range satisfied
			if (currentDistance <= range)
				break;
		}
	}

	return forceWeaponIndex;
}
