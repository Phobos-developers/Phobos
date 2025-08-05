#include "Body.h"

#include <OverlayTypeClass.h>

#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Utilities/EnumFunctions.h>

// Compares two weapons and returns index of which one is eligible to fire against current target (0 = first, 1 = second), or -1 if neither works.
int TechnoExt::PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback, bool allowAAFallback)
{
	auto const pWeaponStructOne = pThis->GetWeapon(weaponIndexOne);
	auto const pWeaponStructTwo = pThis->GetWeapon(weaponIndexTwo);

	if (!pWeaponStructOne && !pWeaponStructTwo)
		return -1;
	else if (!pWeaponStructTwo)
		return weaponIndexOne;
	else if (!pWeaponStructOne)
		return weaponIndexTwo;

	auto const pWeaponTwo = pWeaponStructTwo->WeaponType;
	auto const pSecondExt = WeaponTypeExt::ExtMap.Find(pWeaponTwo);

	CellClass* pTargetCell = nullptr;

	// Ignore target cell for airborne target technos.
	if (pTarget && (!pTargetTechno || !pTargetTechno->IsInAir()))
	{
		if (auto const pObject = abstract_cast<ObjectClass*, true>(pTarget))
			pTargetCell = pObject->GetCell();
		else if (auto const pCell = abstract_cast<CellClass*, true>(pTarget))
			pTargetCell = pCell;
	}

	if (!pSecondExt->SkipWeaponPicking)
	{
		if (pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pSecondExt->CanTarget, true, true))
			return weaponIndexOne;

		if (pTargetTechno)
		{
			if (!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondExt->CanTarget)
				|| !EnumFunctions::CanTargetHouse(pSecondExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner)
				|| !pSecondExt->IsHealthInThreshold(pTargetTechno)
				|| !pSecondExt->HasRequiredAttachedEffects(pTargetTechno, pThis))
			{
				return weaponIndexOne;
			}
		}
	}

	const bool secondIsAA = pTargetTechno && pTargetTechno->IsInAir() && pWeaponTwo->Projectile->AA;
	auto const pFirstExt = WeaponTypeExt::ExtMap.Find(pWeaponStructOne->WeaponType);
	const bool skipPrimaryPicking = pFirstExt->SkipWeaponPicking;
	const bool firstAllowedAE = !skipPrimaryPicking && pFirstExt->HasRequiredAttachedEffects(pTargetTechno, pThis);

	if (!allowFallback
		&& (!allowAAFallback || !secondIsAA)
		&& !TechnoExt::CanFireNoAmmoWeapon(pThis, 1)
		&& firstAllowedAE)
	{
		return weaponIndexOne;
	}

	if (!skipPrimaryPicking)
	{
		if (pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pFirstExt->CanTarget, true, true))
			return weaponIndexTwo;

		if (pTargetTechno)
		{
			if (!EnumFunctions::IsTechnoEligible(pTargetTechno, pFirstExt->CanTarget)
				|| !EnumFunctions::CanTargetHouse(pFirstExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner)
				|| !pFirstExt->IsHealthInThreshold(pTargetTechno)
				|| !firstAllowedAE)
			{
				return weaponIndexTwo;
			}
		}
	}

	// Handle special case with NavalTargeting / LandTargeting.
	if (!pTargetTechno && pTargetCell)
	{
		auto const pType = pThis->GetTechnoType();

		if (pType->NavalTargeting == NavalTargetingType::Naval_Primary
			|| pType->LandTargeting == LandTargetingType::Land_Secondary)
		{
			auto const landType = pTargetCell->LandType;

			if (landType != LandType::Water && landType != LandType::Beach)
			{
				return weaponIndexTwo;
			}
		}
	}

	return -1;
}

void TechnoExt::FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType)
{
	WeaponTypeExt::DetonateAt(pWeaponType, pThis, pThis);
}

bool TechnoExt::CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex)
{
	auto const pType = pThis->GetTechnoType();

	if (pType->Ammo > 0)
	{
		auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

		if (pThis->Ammo <= pExt->NoAmmoAmount && (pExt->NoAmmoWeapon == weaponIndex || pExt->NoAmmoWeapon == -1))
			return true;
	}

	return false;
}

WeaponTypeClass* TechnoExt::GetDeployFireWeapon(TechnoClass* pThis, int& weaponIndex)
{
	auto const pType = pThis->GetTechnoType();
	weaponIndex = pType->DeployFireWeapon;

	if (pThis->WhatAmI() == AbstractType::Unit)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		// Only apply DeployFireWeapon on vehicles if explicitly set.
		if (!pTypeExt->DeployFireWeapon.isset())
		{
			weaponIndex = 0;
			auto pCell = MapClass::Instance.GetCellAt(pThis->GetMapCoords());

			if (pThis->GetFireError(pCell, 0, true) != FireError::OK)
				weaponIndex = 1;
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

	auto pWeaponExt = WeaponTypeExt::ExtMap.TryFind(pWeapon);
	const bool aeForbidsPrimary = pWeaponExt && !pWeaponExt->SkipWeaponPicking && pWeaponExt->AttachEffect_CheckOnFirer && !pWeaponExt->HasRequiredAttachedEffects(pThis, pThis);

	if (!pWeapon || (!pWeapon->Warhead->Wall && (!pWeapon->Warhead->Wood || pWallOverlayType->Armor != Armor::Wood)) || TechnoExt::CanFireNoAmmoWeapon(pThis, 1) || aeForbidsPrimary)
	{
		int weaponIndexSec = -1;
		pWeapon = TechnoExt::GetCurrentWeapon(pThis, weaponIndexSec, true);
		pWeaponExt = WeaponTypeExt::ExtMap.TryFind(pWeapon);
		const bool aeForbidsSecondary = pWeaponExt && !pWeaponExt->SkipWeaponPicking && pWeaponExt->AttachEffect_CheckOnFirer && !pWeaponExt->HasRequiredAttachedEffects(pThis, pThis);

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
	auto const pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;
	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
	const bool hasFilters = pTypeExt->SuppressKillWeapons_Types.size() > 0;

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
			WeaponTypeExt::DetonateAt(pWHExt->KillWeapon_OnFirer, pSource, pSource);
	}
}

void TechnoExt::ApplyRevengeWeapon(TechnoClass* pThis, TechnoClass* pSource, WarheadTypeClass* pWH)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pTypeExt = pExt->TypeExtData;
	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
	auto const pThisOwner = pThis->Owner;
	auto const pSourceOwner = pSource->Owner;
	auto const& suppressType = pWHExt->SuppressRevengeWeapons_Types;
	const bool suppress = pWHExt->SuppressRevengeWeapons;
	const bool hasFilters = suppressType.size() > 0;

	if (pTypeExt->RevengeWeapon && EnumFunctions::CanTargetHouse(pTypeExt->RevengeWeapon_AffectsHouses, pThisOwner, pSourceOwner))
	{
		if (!suppress || (hasFilters && !suppressType.Contains(pTypeExt->RevengeWeapon)))
			WeaponTypeExt::DetonateAt(pTypeExt->RevengeWeapon, pSource, pThis);
	}

	for (auto const& attachEffect : pExt->AttachedEffects)
	{
		if (!attachEffect->IsActive())
			continue;

		auto const pType = attachEffect->GetType();

		if (!pType->RevengeWeapon)
			continue;

		if (suppress && (!hasFilters || suppressType.Contains(pType->RevengeWeapon)))
			continue;

		if (pType->RevengeWeapon_UseInvokerAsOwner)
		{
			auto const pInvoker = attachEffect->GetInvoker();

			if (pInvoker && EnumFunctions::CanTargetHouse(pType->RevengeWeapon_AffectsHouses, pInvoker->Owner, pSourceOwner))
				WeaponTypeExt::DetonateAt(pType->RevengeWeapon, pSource, pInvoker);
		}
		else if (EnumFunctions::CanTargetHouse(pType->RevengeWeapon_AffectsHouses, pThisOwner, pSourceOwner))
		{
			WeaponTypeExt::DetonateAt(pType->RevengeWeapon, pSource, pThis);
		}
	}
}

int TechnoExt::ExtData::ApplyForceWeaponInRange(AbstractClass* pTarget)
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

bool TechnoExt::MultiWeaponCanFire(TechnoClass* const pThis, AbstractClass* const pTarget, WeaponTypeClass* const pWeaponType)
{
	if (!pWeaponType || pWeaponType->NeverUse
		|| (pThis->InOpenToppedTransport && !pWeaponType->FireInTransport))
	{
		return false;
	}

	const auto rtti = pTarget->WhatAmI();
	const bool isBuilding = rtti == AbstractType::Building;
	const auto pWH = pWeaponType->Warhead;
	const auto pBulletType = pWeaponType->Projectile;

	const auto pTechno = abstract_cast<TechnoClass*, true>(pTarget);
	const bool isInAir = pTechno ? pTechno->IsInAir() : false;

	const auto pOwner = pThis->Owner;
	const auto pTechnoOwner = pTechno ? pTechno->Owner : nullptr;
	const bool isAllies = pTechnoOwner ? pOwner->IsAlliedWith(pTechnoOwner) : false;

	if (isInAir)
	{
		if (!pBulletType->AA)
			return false;
	}
	else
	{
		if (BulletTypeExt::ExtMap.Find(pBulletType)->AAOnly.Get())
		{
			return false;
		}
		else if (pWH->ElectricAssault)
		{
			if (!isBuilding || !isAllies
				|| !static_cast<BuildingClass*>(pTarget)->Type->Overpowerable)
			{
				return false;
			}
		}
		else if (pWH->IsLocomotor)
		{
			if (isBuilding)
				return false;
		}
	}

	CellClass* pTargetCell = nullptr;

	// Ignore target cell for airborne target technos.
	if (!pTechno || !isInAir)
	{
		if (auto const pObject = abstract_cast<ObjectClass*, true>(pTarget))
			pTargetCell = pObject->GetCell();
		else if (auto const pCell = abstract_cast<CellClass*, true>(pTarget))
			pTargetCell = pCell;
	}

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeaponType);

	if (!pWeaponExt->SkipWeaponPicking)
	{
		if (pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pWeaponExt->CanTarget, true, true))
			return false;

		if (pTechno)
		{
			if (!EnumFunctions::IsTechnoEligible(pTechno, pWeaponExt->CanTarget)
				|| !EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pOwner, pTechnoOwner)
				|| !pWeaponExt->IsHealthInThreshold(pTechno)
				|| !pWeaponExt->HasRequiredAttachedEffects(pTechno, pThis))
			{
				return false;
			}
		}
	}

	if (pTechno)
	{
		if (pTechno->AttachedBomb ? pWH->IvanBomb : pWH->BombDisarm)
			return false;

		if (!pWH->Temporal && pTechno->BeingWarpedOut)
			return false;

		if (pWH->Parasite
			&& (isBuilding || static_cast<FootClass*>(pTechno)->ParasiteEatingMe))
		{
			return false;
		}

		const auto pTechnoType = pTechno->GetTechnoType();

		if (pWH->MindControl
			&& (pTechnoType->ImmuneToPsionics || pTechno->IsMindControlled() || pOwner == pTechnoOwner))
		{
			return false;
		}

		if (pWeaponType->DrainWeapon
			&& (!pTechnoType->Drainable || pTechno->DrainingMe || isAllies))
		{
			return false;
		}

		if (pWH->Airstrike)
		{
			if (!EnumFunctions::IsTechnoEligible(pTechno, WarheadTypeExt::ExtMap.Find(pWH)->AirstrikeTargets))
				return false;

			const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

			if (pTechno->AbstractFlags & AbstractFlags::Foot)
			{
				if (!pTechnoTypeExt->AllowAirstrike.Get(true))
					return false;
			}
			else if (pTechnoTypeExt->AllowAirstrike.Get(static_cast<BuildingTypeClass*>(pTechnoType)->CanC4)
				&& (!pTechnoType->ResourceDestination || !pTechnoType->ResourceGatherer))
			{
				return false;
			}
		}

		if (GeneralUtils::GetWarheadVersusArmor(pWH, pTechno, pTechnoType) == 0.0)
			return false;
	}
	else if (rtti == AbstractType::Cell)
	{
		if (pTargetCell->OverlayTypeIndex >= 0)
		{
			const auto pOverlayType = OverlayTypeClass::Array.GetItem(pTargetCell->OverlayTypeIndex);

			if (pOverlayType->Wall && !pWH->Wall && (!pWH->Wood || pOverlayType->Armor != Armor::Wood))
				return false;
		}
	}
	else if (rtti == AbstractType::Terrain)
	{
		if (!pWH->Wood)
			return false;
	}

	return true;
}
