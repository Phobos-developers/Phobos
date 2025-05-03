#include "Body.h"

#include <InfantryClass.h>
#include <HouseClass.h>
#include <OverlayTypeClass.h>
#include <TerrainClass.h>
#include <SpawnManagerClass.h>
#include <SlaveManagerClass.h>
#include <AirstrikeClass.h>
#include <Kamikaze.h>

#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/EnumFunctions.h>

bool TechnoExt::CheckMultiWeapon(TechnoClass* const pThis, AbstractClass* const pTarget, WeaponTypeClass* const pWeaponType)
{
	if (!pThis || !pTarget)
		return false;

	if (!pWeaponType || pWeaponType->NeverUse)
		return false;

	if (pThis->InOpenToppedTransport && !pWeaponType->FireInTransport)
		return false;

	const auto pBulletType = pWeaponType->Projectile;
	const auto pWhatAmI = pTarget->WhatAmI();
	const bool isBuilding = pWhatAmI != AbstractType::Building;
	const auto pWH = pWeaponType->Warhead;
	const auto pOwner = pThis->Owner;

	if (pTarget->IsInAir())
	{
		if (!pBulletType->AA)
			return false;

		if (pWH->Airstrike)
			return false;
	}
	else
	{
		const auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBulletType);

		if (pBulletTypeExt->AAOnly.Get())
		{
			return false;
		}
		else if(pWH->ElectricAssault)
		{
			if (isBuilding)
				return false;

			const auto pBuilding = static_cast<BuildingClass*>(pTarget);

			if (!pOwner->IsAlliedWith(pBuilding->Owner) ||
				!pBuilding->Type || !pBuilding->Type->Overpowerable)
				return false;
		}
		else if (pWH->IsLocomotor)
		{
			if (isBuilding)
				return false;
		}
	}

	if (pTarget->AbstractFlags & AbstractFlags::Techno)
	{
		TechnoClass* pTechno = static_cast<TechnoClass*>(pTarget);

		if (pTechno->Health <= 0 || !pTechno->IsAlive)
			return false;

		if (pTechno->AttachedBomb)
		{
			if (pWH->IvanBomb)
				return false;
		}
		else if (pWH->BombDisarm)
		{
			return false;
		}

		const auto pTechnoType = pTechno->GetTechnoType();

		if (pWH->MindControl && (pTechnoType->ImmuneToPsionics ||
			pTechno->IsMindControlled() || pOwner == pTechno->Owner))
			return false;

		if (pWH->Parasite && (isBuilding ||
			static_cast<FootClass*>(pTechno)->ParasiteEatingMe))
			return false;

		if (!pWH->Temporal && pTechno->BeingWarpedOut)
			return false;

		if (pWeaponType->DrainWeapon && (!pTechnoType->Drainable ||
			(pTechno->DrainingMe || pOwner->IsAlliedWith(pTechno->Owner))))
			return false;

		if (const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeaponType))
		{
			if (!pWeaponExt->HasRequiredAttachedEffects(pTechno, pThis))
				return false;
		}

		const auto pTargetExt = TechnoExt::ExtMap.Find(pTechno);
		const auto pShield = pTargetExt->Shield.get();

		if (pShield && pShield->IsActive() &&
			!pShield->CanBeTargeted(pWeaponType))
		{
			return false;
		}
		else if (GeneralUtils::GetWarheadVersusArmor(pWH, pTechno->GetTechnoType()->Armor) == 0.0)
		{
			return false;
		}
	}
	else
	{
		if (pWhatAmI == AbstractType::Terrain)
		{
			if (!pWH->Wood)
				return false;
		}
		else if (pWhatAmI == AbstractType::Cell)
		{
			const auto pCell = static_cast<CellClass*>(pTarget);

			if (pCell && pCell->OverlayTypeIndex >= 0)
			{
				auto overlayType = OverlayTypeClass::Array.GetItem(pCell->OverlayTypeIndex);

				if (overlayType->Wall && !pWH->Wall)
					return false;
			}
		}
	}

	return true;
}
