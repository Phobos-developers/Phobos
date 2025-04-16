#include "Body.h"

#include <InfantryClass.h>
#include <HouseClass.h>
#include <OverlayTypeClass.h>
#include <TerrainClass.h>

#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/EnumFunctions.h>

bool TechnoExt::IsSecondary(TechnoClass* const pThis, const int& nWeaponIndex)
{
	if (!pThis)
		return false;

	if (pThis->GetTechnoType()->IsGattling)
		return nWeaponIndex != 0 && nWeaponIndex % 2 != 0;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt && pTypeExt->MultiWeapon.Get() &&
		!pTypeExt->MultiWeapon_IsSecondary.empty())
	{
		int index = nWeaponIndex + 1;
		return pTypeExt->MultiWeapon_IsSecondary.Contains(index);
	}

	return nWeaponIndex != 0;
}

// maybe it could be used as a new SelectWeapon, but not for now.
bool TechnoExt::CheckMultiWeapon(TechnoClass* const pThis, AbstractClass* const pTarget, WeaponTypeClass* const pWeaponType)
{
	if (!pThis || !pWeaponType || pWeaponType->NeverUse ||
		!pWeaponType->Projectile || !pWeaponType->Warhead ||
		(pThis->InOpenToppedTransport && !pWeaponType->FireInTransport))
		return false;

	const auto pWH = pWeaponType->Warhead;

	if (TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pTarget))
	{
		if (pTargetTechno->Health <= 0 || !pTargetTechno->IsAlive)
			return false;

		if (const auto pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno))
		{
			const auto pShield = pTargetExt->Shield.get();

			if (pShield && pShield->IsActive() &&
				!pShield->CanBeTargeted(pWeaponType))
				return false;
		}

		if (GeneralUtils::GetWarheadVersusArmor(pWH, pTargetTechno->GetTechnoType()->Armor) == 0.0)
			return false;

		if (pTargetTechno->IsInAir())
		{
			if (!pWeaponType->Projectile->AA)
				return false;
		}
		else
		{
			const auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pWeaponType->Projectile);

			if (!pBulletTypeExt->AAOnly.Get())
				return false;
		}

		if (pWH->IsLocomotor && pTarget->WhatAmI() == AbstractType::Building)
			return false;

		if (pWH->ElectricAssault)
		{
			if (!pThis->Owner->IsAlliedWith(pTargetTechno->Owner))
				return false;

			if (pTargetTechno->WhatAmI() != AbstractType::Building)
				return false;

			const auto pBld = abstract_cast<BuildingClass*>(pTargetTechno);

			if (!pBld || !pBld->Type || !pBld->Type->Overpowerable)
				return false;
		}

		if (pWeaponType->DrainWeapon && (pTargetTechno->DrainingMe ||
			!pTargetTechno->GetTechnoType()->Drainable))
			return false;

		if (pTargetTechno->AttachedBomb)
		{
			if (pWH->IvanBomb)
				return false;

			if (pWH->BombDisarm &&
				!pThis->Owner->IsControlledByHuman() &&
				!pThis->Owner->IsAlliedWith(pTargetTechno->Owner))
				return false;
		}
		else
		{
			if (pWH->BombDisarm)
				return false;
		}

		if (pWH->Airstrike && pTargetTechno->IsInAir())
			return false;

		if (pWH->MindControl && (pTargetTechno->Owner == pThis->Owner ||
			pTargetTechno->IsMindControlled() || pTargetTechno->GetTechnoType()->ImmuneToPsionics))
			return false;

		if (pWH->Parasite && (pTargetTechno->WhatAmI() == AbstractType::Building ||
			abstract_cast<FootClass*>(pTargetTechno)->ParasiteEatingMe))
			return false;

		const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeaponType);

		if (pWeaponExt && !pWeaponExt->HasRequiredAttachedEffects(pTargetTechno, pThis))
			return false;
	}
	else
	{
		if (pWH->ElectricAssault || pWH->BombDisarm || pWH->Airstrike || pWeaponType->DrainWeapon)
			return false;

		if (pTarget->WhatAmI() == AbstractType::Terrain)
		{
			if (!pWH->Wood)
				return false;
		}
		else if (pTarget->WhatAmI() == AbstractType::Cell)
		{
			const auto pCell = abstract_cast<CellClass*>(pTarget);

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
