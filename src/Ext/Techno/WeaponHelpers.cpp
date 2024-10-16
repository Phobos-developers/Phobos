#include "Body.h"

#include <OverlayTypeClass.h>

#include <Ext/WeaponType/Body.h>
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
				auto pCell = MapClass::Instance->GetCellAt(pThis->GetMapCoords());

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

	if ((pTechnoType->TurretCount > 0 && !pTechnoType->IsGattling) || !pWallOverlayType || !pWallOverlayType->Wall)
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

		if (pWeapon && (pWeapon->Warhead->Wall || (pWeapon->Warhead->Wood && pWallOverlayType->Armor == Armor::Wood)
			&& (!TechnoTypeExt::ExtMap.Find(pTechnoType)->NoSecondaryWeaponFallback || aeForbidsPrimary)) && !aeForbidsSecondary)
		{
			return weaponIndexSec;
		}

		return weaponIndex;
	}

	return weaponIndex;
}

int TechnoExt::ApplyForceWeaponInRange(TechnoClass* pThis, std::vector<int> weaponIndices, std::vector<double> rangeOverrides, bool applyRangeModifiers, int currentDistance)
{
	int forceWeaponIndex = -1;

	for (size_t i = 0; i < weaponIndices.size(); i++)
	{
		int distance = 0;

		// Value below 0 means Range won't be overriden
		if (i < rangeOverrides.size() && rangeOverrides[i] > 0)
			distance = static_cast<int>(rangeOverrides[i] * 256.0);

		if (weaponIndices[i] >= 0)
		{
			auto const pWeapon = pThis->GetWeapon(weaponIndices[i])->WeaponType;
			distance = distance > 0 ? distance : pWeapon->Range;

			if (applyRangeModifiers)
				distance = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pThis, distance);

			if (currentDistance <= distance)
			{
				forceWeaponIndex = weaponIndices[i];
				break;
			}
		}
		else
		{
			// Apply range modifiers regardless of weapon
			if (applyRangeModifiers)
				distance = WeaponTypeExt::GetRangeWithModifiers(nullptr, pThis, distance);

			// Don't force weapon if range satisfied
			if (currentDistance <= distance)
				break;
		}
	}

	return forceWeaponIndex;
}
