#include "Body.h"

#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

// Compares two weapons and returns index of which one is eligible to fire against current target (0 = first, 1 = second), or -1 if neither works.
int TechnoExt::PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback, bool allowAAFallback)
{
	CellClass* pTargetCell = nullptr;

	// Ignore target cell for airborne target technos.
	if (!pTargetTechno || !pTargetTechno->IsInAir())
	{
		if (const auto pCell = abstract_cast<CellClass*>(pTarget))
			pTargetCell = pCell;
		else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
			pTargetCell = pObject->GetCell();
	}

	const auto pWeaponStructOne = pThis->GetWeapon(weaponIndexOne);
	const auto pWeaponStructTwo = pThis->GetWeapon(weaponIndexTwo);

	if (!pWeaponStructOne && !pWeaponStructTwo)
		return -1;
	else if (!pWeaponStructTwo)
		return weaponIndexOne;
	else if (!pWeaponStructOne)
		return weaponIndexTwo;

	const auto pWeaponOne = pWeaponStructOne->WeaponType;
	const auto pWeaponTwo = pWeaponStructTwo->WeaponType;

	if (const auto pSecondExt = WeaponTypeExt::ExtMap.Find(pWeaponTwo))
	{
		if ((pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pSecondExt->CanTarget, true)) ||
			(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondExt->CanTarget) ||
				!EnumFunctions::CanTargetHouse(pSecondExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
		{
			return weaponIndexOne;
		}
		else if (const auto pFirstExt = WeaponTypeExt::ExtMap.Find(pWeaponOne))
		{
			bool secondaryIsAA = pTargetTechno && pTargetTechno->IsInAir() && pWeaponTwo->Projectile->AA;

			if (!allowFallback && (!allowAAFallback || !secondaryIsAA) && !TechnoExt::CanFireNoAmmoWeapon(pThis, 1))
				return weaponIndexOne;

			if ((pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, pFirstExt->CanTarget, true)) ||
				(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pFirstExt->CanTarget) ||
					!EnumFunctions::CanTargetHouse(pFirstExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
			{
				return weaponIndexTwo;
			}
		}
	}

	const auto pType = pThis->GetTechnoType();

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
		if (const auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			if (pThis->Ammo <= pExt->NoAmmoAmount && (pExt->NoAmmoWeapon = weaponIndex || pExt->NoAmmoWeapon == -1))
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
