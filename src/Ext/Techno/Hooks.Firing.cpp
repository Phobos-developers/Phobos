#include "Body.h"

#include <ScenarioClass.h>
#include <TerrainClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

// Weapon Selection

DEFINE_HOOK(0x6F3339, TechnoClass_WhatWeaponShouldIUse_Interceptor, 0x8)
{
	enum { SkipGameCode = 0x6F3341, ReturnValue = 0x6F3406 };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));

	if (pThis && pTarget && pTarget->WhatAmI() == AbstractType::Bullet)
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pTypeExt->InterceptorType)
		{
			R->EAX(pTypeExt->InterceptorType->Weapon);
			return ReturnValue;
		}
	}

	// Restore overridden instructions.
	R->EAX(pThis->GetTechnoType());

	return SkipGameCode;
}

DEFINE_HOOK(0x6F33CD, TechnoClass_WhatWeaponShouldIUse_ForceFire, 0x6)
{
	enum { Secondary = 0x6F3745, UseWeaponIndex = 0x6F37AF };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));

	if (const auto pCell = abstract_cast<CellClass*>(pTarget))
	{
		if (const auto pPrimaryExt = WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(0)->WeaponType))
		{
			if (pThis->GetWeapon(1)->WeaponType && !EnumFunctions::IsCellEligible(pCell, pPrimaryExt->CanTarget, true))
				return Secondary;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F3428, TechnoClass_WhatWeaponShouldIUse_ForceWeapon, 0x6)
{
	enum { UseWeaponIndex = 0x6F37AF };

	GET(TechnoClass*, pThis, ECX);

	if (pThis && pThis->Target)
	{
		auto const pTarget = abstract_cast<TechnoClass*>(pThis->Target);

		if (!pTarget)
			return 0;

		int forceWeaponIndex = -1;
		auto const pTargetType = pTarget->GetTechnoType();
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pTypeExt->ForceWeapon_Naval_Decloaked >= 0 &&
			pTargetType->Cloakable && pTargetType->Naval &&
			pTarget->CloakState == CloakState::Uncloaked)
		{
			forceWeaponIndex = pTypeExt->ForceWeapon_Naval_Decloaked;
		}
		else if (pTypeExt->ForceWeapon_Cloaked >= 0 &&
			pTarget->CloakState == CloakState::Cloaked)
		{
			forceWeaponIndex = pTypeExt->ForceWeapon_Cloaked;
		}
		else if (pTypeExt->ForceWeapon_Disguised >= 0 &&
			pTarget->IsDisguised())
		{
			forceWeaponIndex = pTypeExt->ForceWeapon_Disguised;
		}

		if (forceWeaponIndex >= 0)
		{
			R->EAX(forceWeaponIndex);
			return UseWeaponIndex;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F36DB, TechnoClass_WhatWeaponShouldIUse, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));

	enum { Primary = 0x6F37AD, Secondary = 0x6F3745, OriginalCheck = 0x6F36E3 };

	const auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	bool allowFallback = !pTypeExt->NoSecondaryWeaponFallback;
	bool allowAAFallback = allowFallback ? true : pTypeExt->NoSecondaryWeaponFallback_AllowAA;
	int weaponIndex = TechnoExt::PickWeaponIndex(pThis, pTargetTechno, pTarget, 0, 1, allowFallback, allowAAFallback);

	if (weaponIndex != -1)
		return weaponIndex == 1 ? Secondary : Primary;

	if (!pTargetTechno)
		return Primary;

	if (const auto pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno))
	{
		if (const auto pShield = pTargetExt->Shield.get())
		{
			if (pShield->IsActive())
			{
				auto const secondary = pThis->GetWeapon(1)->WeaponType;
				bool secondaryIsAA = pTargetTechno && pTargetTechno->IsInAir() && secondary && secondary->Projectile->AA;

				if (secondary && (allowFallback || (allowAAFallback && secondaryIsAA) || TechnoExt::CanFireNoAmmoWeapon(pThis, 1)))
				{
					if (!pShield->CanBeTargeted(pThis->GetWeapon(0)->WeaponType))
						return Secondary;
				}
				else
				{
					return Primary;
				}
			}
		}
	}

	return OriginalCheck;
}

DEFINE_HOOK(0x6F37EB, TechnoClass_WhatWeaponShouldIUse_AntiAir, 0x6)
{
	enum { Primary = 0x6F37AD, Secondary = 0x6F3807 };

	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFSET(0x18, -0x4));
	GET(WeaponTypeClass*, pSecWeapon, EAX);

	const auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget);

	if (!pWeapon->Projectile->AA && pSecWeapon->Projectile->AA && pTargetTechno && pTargetTechno->IsInAir())
		return Secondary;

	return Primary;
}

DEFINE_HOOK(0x6F3432, TechnoClass_WhatWeaponShouldIUse_Gattling, 0xA)
{
	enum { UseWeaponIndex = 0x6F37AF };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));

	auto const pTargetTechno = abstract_cast<TechnoClass*>(pTarget);
	int oddWeaponIndex = 2 * pThis->CurrentGattlingStage;
	int evenWeaponIndex = oddWeaponIndex + 1;
	int chosenWeaponIndex = oddWeaponIndex;
	int eligibleWeaponIndex = TechnoExt::PickWeaponIndex(pThis, pTargetTechno, pTarget, oddWeaponIndex, evenWeaponIndex, true);

	if (eligibleWeaponIndex != -1)
	{
		chosenWeaponIndex = eligibleWeaponIndex;
	}
	else if (pTargetTechno)
	{
		auto const pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno);
		auto const pWeaponOdd = pThis->GetWeapon(oddWeaponIndex)->WeaponType;
		auto const pWeaponEven = pThis->GetWeapon(evenWeaponIndex)->WeaponType;
		bool skipRemainingChecks = false;

		if (const auto pShield = pTargetExt->Shield.get())
		{
			if (pShield->IsActive() && !pShield->CanBeTargeted(pWeaponOdd))
			{
				chosenWeaponIndex = evenWeaponIndex;
				skipRemainingChecks = true;
			}
		}

		if (!skipRemainingChecks)
		{
			if (GeneralUtils::GetWarheadVersusArmor(pWeaponOdd->Warhead, pTargetTechno->GetTechnoType()->Armor) == 0.0)
			{
				chosenWeaponIndex = evenWeaponIndex;
			}
			else
			{
				auto const pCell = pTargetTechno->GetCell();
				bool isOnWater = (pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach) && !pTargetTechno->IsInAir();

				if (!pTargetTechno->OnBridge && isOnWater)
				{
					int navalTargetWeapon = pThis->SelectNavalTargeting(pTargetTechno);

					if (navalTargetWeapon == 2)
						chosenWeaponIndex = evenWeaponIndex;
				}
				else if ((pTargetTechno->IsInAir() && !pWeaponOdd->Projectile->AA && pWeaponEven->Projectile->AA) ||
					!pTargetTechno->IsInAir() && pThis->GetTechnoType()->LandTargeting == LandTargetingType::Land_Secondary)
				{
					chosenWeaponIndex = evenWeaponIndex;
				}
			}
		}
	}

	R->EAX(chosenWeaponIndex);
	return UseWeaponIndex;
}

DEFINE_HOOK(0x5218F3, InfantryClass_WhatWeaponShouldIUse_DeployFireWeapon, 0x6)
{
	GET(TechnoTypeClass*, pType, ECX);

	if (pType->DeployFireWeapon == -1)
		return 0x52194E;

	return 0;
}

// Pre-Firing Checks

DEFINE_HOOK(0x6FC339, TechnoClass_CanFire, 0x6)
{
	enum { CannotFire = 0x6FCB7E };

	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x20, 0x4));
	// Checking for nullptr is not required here, since the game has already executed them before calling the hook  -- Belonit
	const auto pWH = pWeapon->Warhead;

	if (const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH))
	{
		const int nMoney = pWHExt->TransactMoney;
		if (nMoney < 0 && pThis->Owner->Available_Money() < -nMoney)
			return CannotFire;
	}
	if (const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		const auto pTechno = abstract_cast<TechnoClass*>(pTarget);

		CellClass* pTargetCell = nullptr;

		if (pTarget)
		{
			if (const auto pCell = abstract_cast<CellClass*>(pTarget))
			{
				pTargetCell = pCell;
			}
			else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
			{
				// Ignore target cell for technos that are in air.
				if ((pTechno && !pTechno->IsInAir()) || pObject != pTechno)
					pTargetCell = pObject->GetCell();
			}
		}

		if (pTargetCell)
		{
			if (!EnumFunctions::IsCellEligible(pTargetCell, pWeaponExt->CanTarget, true))
				return CannotFire;
		}

		if (pTechno)
		{
			if (!EnumFunctions::IsTechnoEligible(pTechno, pWeaponExt->CanTarget) ||
				!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, pTechno->Owner))
			{
				return CannotFire;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FC587, TechnoClass_CanFire_OpenTopped, 0x6)
{
	enum { DisallowFiring = 0x6FC86A };

	GET(TechnoClass*, pThis, ESI);

	if (auto const pTransport = pThis->Transporter)
	{
		if (auto pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			if (pTransport->Deactivated && !pExt->OpenTopped_AllowFiringIfDeactivated)
				return DisallowFiring;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FC689, TechnoClass_CanFire_LandNavalTarget, 0x6)
{
	enum { DisallowFiring = 0x6FC86A };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x20, 0x4));

	const auto pType = pThis->GetTechnoType();
	auto pCell = abstract_cast<CellClass*>(pTarget);

	if (pCell)
	{
		if (pType->NavalTargeting == NavalTargetingType::Naval_None &&
			(pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach))
		{
			return DisallowFiring;
		}
	}
	else if (const auto pTerrain = abstract_cast<TerrainClass*>(pTarget))
	{
		pCell = pTerrain->GetCell();

		if (pType->LandTargeting == LandTargetingType::Land_Not_OK &&
			pCell->LandType != LandType::Water && pCell->LandType != LandType::Beach)
		{
			return DisallowFiring;
		}
		else if (pType->NavalTargeting == NavalTargetingType::Naval_None &&
			(pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach))
		{
			return DisallowFiring;
		}
	}

	return 0;
}

// Weapon Firing

DEFINE_HOOK(0x6FE43B, TechnoClass_FireAt_OpenToppedDmgMult, 0x8)
{
	enum { ApplyDamageMult = 0x6FE45A, ContinueCheck = 0x6FE460 };

	GET(TechnoClass* const, pThis, ESI);

	//replacing whole check due to `fild`
	if (pThis->InOpenToppedTransport)
	{
		GET_STACK(int, nDamage, STACK_OFFSET(0xB0, -0x84));
		float nDamageMult = static_cast<float>(RulesClass::Instance->OpenToppedDamageMultiplier);

		if (auto pTransport = pThis->Transporter)
		{
			if (auto pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
			{
				//it is float isnt it YRPP ? , check tomson26 YR-IDB !
				nDamageMult = pExt->OpenTopped_DamageMultiplier.Get(nDamageMult);
			}
		}

		R->EAX(Game::F2I(nDamage * nDamageMult));
		return ApplyDamageMult;
	}

	return ContinueCheck;
}

DEFINE_HOOK(0x6FE19A, TechnoClass_FireAt_AreaFire, 0x6)
{
	enum { DoNotFire = 0x6FE4E7, SkipSetTarget = 0x6FE1D5 };

	GET(TechnoClass* const, pThis, ESI);
	GET(CellClass* const, pCell, EAX);
	GET_STACK(WeaponTypeClass*, pWeaponType, STACK_OFFSET(0xB0, -0x70));

	if (auto pExt = WeaponTypeExt::ExtMap.Find(pWeaponType))
	{
		if (pExt->AreaFire_Target == AreaFireTarget::Random)
		{
			auto const range = pWeaponType->Range / static_cast<double>(Unsorted::LeptonsPerCell);

			std::vector<CellStruct> adjacentCells = GeneralUtils::AdjacentCellsInRange(static_cast<size_t>(range + 0.99));
			size_t size = adjacentCells.size();

			for (unsigned int i = 0; i < size; i++)
			{
				int rand = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);
				unsigned int cellIndex = (i + rand) % size;
				CellStruct tgtPos = pCell->MapCoords + adjacentCells[cellIndex];
				CellClass* tgtCell = MapClass::Instance->GetCellAt(tgtPos);

				if (EnumFunctions::AreCellAndObjectsEligible(tgtCell, pExt->CanTarget, pExt->CanTargetHouses, pThis->Owner, true))
				{
					R->EAX(tgtCell);
					return 0;
				}
			}

			return DoNotFire;
		}
		else if (pExt->AreaFire_Target == AreaFireTarget::Self)
		{
			if (!EnumFunctions::AreCellAndObjectsEligible(pThis->GetCell(), pExt->CanTarget, pExt->CanTargetHouses, nullptr, false))
				return DoNotFire;

			R->EAX(pThis);
			return SkipSetTarget;
		}

		if (!EnumFunctions::AreCellAndObjectsEligible(pCell, pExt->CanTarget, pExt->CanTargetHouses, nullptr, false))
			return DoNotFire;
	}

	return 0;
}

DEFINE_HOOK(0x6FF43F, TechnoClass_FireAt_FeedbackWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	if (auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		if (pWeaponExt->FeedbackWeapon.isset())
		{
			auto fbWeapon = pWeaponExt->FeedbackWeapon.Get();

			if (pThis->InOpenToppedTransport && !fbWeapon->FireInTransport)
				return 0;

			WeaponTypeExt::DetonateAt(fbWeapon, pThis, pThis);
		}
	}

	return 0;
}


DEFINE_HOOK(0x6FF660, TechnoClass_FireAt_Interceptor, 0x6)
{
	GET(TechnoClass* const, pSource, ESI);
	GET_BASE(AbstractClass* const, pTarget, 0x8);
	GET(WeaponTypeClass* const, pWeaponType, EBX);
	GET_STACK(BulletClass* const, pBullet, STACK_OFFSET(0xB0, -0x74));

	auto const pSourceTypeExt = TechnoTypeExt::ExtMap.Find(pSource->GetTechnoType());

	if (pSourceTypeExt->InterceptorType)
	{
		if (auto const pTargetObject = specific_cast<BulletClass* const>(pTarget))
		{
			if (auto const pBulletExt = BulletExt::ExtMap.Find(pBullet))
			{
				pBulletExt->IsInterceptor = true;
				pBulletExt->InterceptedStatus = InterceptedStatus::Targeted;
			}

			// If using Inviso projectile, can intercept bullets right after firing.
			if (pTargetObject->IsAlive && pWeaponType->Projectile->Inviso)
			{
				if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWeaponType->Warhead))
					pWHExt->InterceptBullets(pSource, pWeaponType, pTargetObject->Location);
			}
		}
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6FF660, TechnoClass_FireAt_ToggleLaserWeaponIndex, 0x6)
DEFINE_HOOK(0x6FF4CC, TechnoClass_FireAt_ToggleLaserWeaponIndex, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);
	GET_BASE(int, weaponIndex, 0xC);

	if (pThis->WhatAmI() == AbstractType::Building && pWeapon->IsLaser)
	{
		if (auto const pExt = TechnoExt::ExtMap.Find(pThis))
		{
			if (pExt->CurrentLaserWeaponIndex.empty())
				pExt->CurrentLaserWeaponIndex = weaponIndex;
			else
				pExt->CurrentLaserWeaponIndex.clear();
		}
	}

	return 0;
}

// Feature: Allow Units using AlternateFLHs - by Trsdy
// I don't want to rewrite something new, so I use the Infantry one directly
// afaik it has no check for infantry-specific stuff here so far
// and neither Ares nor Phobos has touched it, even that crawling flh one was in TechnoClass
DEFINE_JUMP(VTABLE, 0x7F5D20, 0x523250);// Redirect UnitClass::GetFLH to InfantryClass::GetFLH (used to be TechnoClass::GetFLH)
