#include "Body.h"

#include <OverlayTypeClass.h>
#include <ScenarioClass.h>
#include <TerrainClass.h>

#include <Ext/Building/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Ext/SWType/Body.h>
#include <Ext/House/Body.h>

#pragma region TechnoClass_SelectWeapon

DEFINE_HOOK(0x6F3339, TechnoClass_WhatWeaponShouldIUse_Interceptor, 0x8)
{
	enum { SkipGameCode = 0x6F3341, ReturnValue = 0x6F3406 };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));

	auto const pType = pThis->GetTechnoType();

	if (pTarget && pTarget->WhatAmI() == AbstractType::Bullet)
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (pTypeExt->InterceptorType)
		{
			R->EAX(pTypeExt->InterceptorType->Weapon);
			return ReturnValue;
		}
	}

	// Restore overridden instructions.
	R->EAX(pType);

	return SkipGameCode;
}

DEFINE_HOOK(0x6F33CD, TechnoClass_WhatWeaponShouldIUse_ForceFire, 0x6)
{
	enum { ReturnWeaponIndex = 0x6F37AF };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));

	if (const auto pCell = abstract_cast<CellClass*>(pTarget))
	{
		auto const pWeaponPrimary = pThis->GetWeapon(0)->WeaponType;
		auto const pWeaponSecondary = pThis->GetWeapon(1)->WeaponType;
		auto const pPrimaryExt = WeaponTypeExt::ExtMap.Find(pWeaponPrimary);

		if (pWeaponSecondary
			&& !pPrimaryExt->SkipWeaponPicking
			&& (!EnumFunctions::IsCellEligible(pCell, pPrimaryExt->CanTarget, true, true)
				|| !pPrimaryExt->IsHealthRatioEligible(pThis)
				|| (pPrimaryExt->AttachEffect_CheckOnFirer
					&& !pPrimaryExt->HasRequiredAttachedEffects(pThis, pThis))))
		{
			R->EAX(1);
			return ReturnWeaponIndex;
		}
		else if (pCell->OverlayTypeIndex != -1)
		{
			auto const pOverlayType = OverlayTypeClass::Array.GetItem(pCell->OverlayTypeIndex);

			if (pOverlayType->Wall && pCell->OverlayData >> 4 != pOverlayType->DamageLevels)
			{
				R->EAX(TechnoExt::GetWeaponIndexAgainstWall(pThis, pOverlayType));
				return ReturnWeaponIndex;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F3428, TechnoClass_WhatWeaponShouldIUse_ForceWeapon, 0x6)
{
	enum { UseWeaponIndex = 0x6F37AF };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	// Force weapon
	const int forceWeaponIndex = pTypeExt->SelectForceWeapon(pThis, pTarget);

	if (forceWeaponIndex >= 0)
	{
		R->EAX(forceWeaponIndex);
		return UseWeaponIndex;
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
	const bool allowFallback = !pTypeExt->NoSecondaryWeaponFallback;
	const bool allowAAFallback = allowFallback ? true : pTypeExt->NoSecondaryWeaponFallback_AllowAA;
	const int weaponIndex = TechnoExt::PickWeaponIndex(pThis, pTargetTechno, pTarget, 0, 1, allowFallback, allowAAFallback);

	if (weaponIndex != -1)
		return weaponIndex == 1 ? Secondary : Primary;

	if (!pTargetTechno)
		return Primary;

	const auto pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno);

	if (const auto pShield = pTargetExt->Shield.get())
	{
		if (pShield->IsActive())
		{
			const auto secondary = pThis->GetWeapon(1)->WeaponType;
			const bool secondaryIsAA = pTargetTechno && pTargetTechno->IsInAir() && secondary && secondary->Projectile->AA;

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

	return OriginalCheck;
}

DEFINE_HOOK(0x6F37EB, TechnoClass_WhatWeaponShouldIUse_AntiAir, 0x6)
{
	enum { Primary = 0x6F37AD, Secondary = 0x6F3807 };

	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFSET(0x18, -0x4));
	GET(WeaponTypeClass*, pSecWeapon, EAX);

	if (!pWeapon->Projectile->AA && pSecWeapon->Projectile->AA)
	{
		const auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget);

		if (pTargetTechno && pTargetTechno->IsInAir())
			return Secondary;
	}

	return Primary;
}

DEFINE_HOOK(0x6F3432, TechnoClass_WhatWeaponShouldIUse_Gattling, 0xA)
{
	enum { UseWeaponIndex = 0x6F37AF };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));

	auto const pTargetTechno = abstract_cast<TechnoClass*>(pTarget);
	const int oddWeaponIndex = 2 * pThis->CurrentGattlingStage;
	const int evenWeaponIndex = oddWeaponIndex + 1;
	int chosenWeaponIndex = oddWeaponIndex;
	const int eligibleWeaponIndex = TechnoExt::PickWeaponIndex(pThis, pTargetTechno, pTarget, oddWeaponIndex, evenWeaponIndex, true);

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
				auto const landType = pTargetTechno->GetCell()->LandType;
				const bool isOnWater = (landType == LandType::Water || landType == LandType::Beach) && !pTargetTechno->IsInAir();

				if (!pTargetTechno->OnBridge && isOnWater)
				{
					int navalTargetWeapon = pThis->SelectNavalTargeting(pTargetTechno);

					if (navalTargetWeapon == 2)
						chosenWeaponIndex = evenWeaponIndex;
				}
				else if (pTargetTechno->IsInAir())
				{
					if (!pWeaponOdd->Projectile->AA && pWeaponEven->Projectile->AA)
						chosenWeaponIndex = evenWeaponIndex;
				}
				else if (pThis->GetTechnoType()->LandTargeting == LandTargetingType::Land_Secondary)
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

#pragma endregion

#pragma region TechnoClass_GetFireError
DEFINE_HOOK(0x6FC339, TechnoClass_CanFire, 0x6)
{
	enum { CannotFire = 0x6FCB7E };

	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x20, 0x4));

	// Checking for nullptr is not required here, since the game has already executed them before calling the hook  -- Belonit
	const auto pWH = pWeapon->Warhead;
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (pWHExt)
	{
		const int nMoney = pWHExt->TransactMoney;

		if (nMoney < 0 && pThis->Owner->Available_Money() < -nMoney)
			return CannotFire;
	}

	// AAOnly doesn't need to be checked if LandTargeting=1.
	if (pThis->GetTechnoType()->LandTargeting != LandTargetingType::Land_Not_OK && pWeapon->Projectile->AA
		&& pTarget && !pTarget->IsInAir() && BulletTypeExt::ExtMap.Find(pWeapon->Projectile)->AAOnly)
	{
		return CannotFire;
	}

	const auto pTechno = abstract_cast<TechnoClass*>(pTarget);
	CellClass* pTargetCell = nullptr;

	if (pTarget)
	{
		if (const auto pObject = abstract_cast<ObjectClass*, true>(pTarget))
		{
			// Ignore target cell for technos that are in air.
			if ((pTechno && !pTechno->IsInAir()) || pObject != pTechno)
				pTargetCell = pObject->GetCell();
		}
		else if (const auto pCell = abstract_cast<CellClass*, true>(pTarget))
		{
			pTargetCell = pCell;
		}
	}

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (!pWeaponExt->SkipWeaponPicking && pTargetCell
		&& !EnumFunctions::IsCellEligible(pTargetCell, pWeaponExt->CanTarget, true, true))
	{
		return CannotFire;
	}

	if (pTechno)
	{
		if (!pWeaponExt->SkipWeaponPicking)
		{
			if (!EnumFunctions::IsTechnoEligible(pTechno, pWeaponExt->CanTarget)
				|| !EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, pTechno->Owner)
				|| !pWeaponExt->IsHealthRatioEligible(pTechno)
				|| !pWeaponExt->HasRequiredAttachedEffects(pTechno, pThis))
			{
				return CannotFire;
			}
		}

		if (pWH->Airstrike)
		{
			if (!pWHExt || !EnumFunctions::IsTechnoEligible(pTechno, pWHExt->AirstrikeTargets))
				return CannotFire;

			if (!TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType())->AllowAirstrike.Get(pTechno->AbstractFlags & AbstractFlags::Foot ? true : static_cast<BuildingClass*>(pTechno)->Type->CanC4))
				return CannotFire;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FC0C5, TechnoClass_CanFire_DisableWeapons, 0x6)
{
	enum { OutOfRange = 0x6FC0DF, Illegal = 0x6FC86A, Continue = 0x6FC0D3 };

	GET(TechnoClass*, pThis, ESI);

	if (pThis->SlaveOwner)
		return Illegal;

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->AE.DisableWeapons)
		return OutOfRange;

	return Continue;
}

DEFINE_HOOK(0x6FC5C7, TechnoClass_CanFire_OpenTopped, 0x6)
{
	enum { Illegal = 0x6FC86A, OutOfRange = 0x6FC0DF, Continue = 0x6FC5D5 };

	// GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTransport, EAX);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType());

	if (pTransport->Deactivated && !pTypeExt->OpenTopped_AllowFiringIfDeactivated)
		return Illegal;

	if (pTransport->Transporter)
		return Illegal;

	if (pTypeExt->OpenTopped_CheckTransportDisableWeapons && TechnoExt::ExtMap.Find(pTransport)->AE.DisableWeapons)
		return OutOfRange;

	return Continue;
}

DEFINE_HOOK(0x6FC689, TechnoClass_CanFire_LandNavalTarget, 0x6)
{
	enum { DisallowFiring = 0x6FC86A };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x20, 0x4));

	const auto pType = pThis->GetTechnoType();

	if (const auto pCell = abstract_cast<CellClass*>(pTarget))
	{
		const auto landType = pCell->LandType;

		if (pType->NavalTargeting == NavalTargetingType::Naval_None
			&& (landType == LandType::Water || landType == LandType::Beach))
		{
			return DisallowFiring;
		}
	}
	else if (const auto pTerrain = abstract_cast<TerrainClass*>(pTarget))
	{
		const auto landType = pTerrain->GetCell()->LandType;

		if (pType->LandTargeting == LandTargetingType::Land_Not_OK
			&& landType != LandType::Water && landType != LandType::Beach)
		{
			return DisallowFiring;
		}
		else if (pType->NavalTargeting == NavalTargetingType::Naval_None
			&& (landType == LandType::Water || landType == LandType::Beach))
		{
			return DisallowFiring;
		}
	}

	return 0;
}

// Skips bridge-related coord checks to allow AA to target air units while both are on a bridge.
DEFINE_HOOK(0x6FCBE6, TechnoClass_CanFire_BridgeAAFix, 0x6)
{
	enum { SkipChecks = 0x6FCCBD };

	GET(TechnoClass*, pTarget, EBP);

	if (pTarget->IsInAir())
		return SkipChecks;

	return 0;
}

#pragma endregion

#pragma region TechnoClass_Fire
DEFINE_HOOK(0x6FDD7D, TechnoClass_FireAt_UpdateWeaponType, 0x5)
{
	enum { CanNotFire = 0x6FDE03 };

	GET(TechnoClass* const, pThis, ESI);

	if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
	{
		GET(WeaponTypeClass* const, pWeapon, EBX);

		if (pThis->CurrentBurstIndex && pWeapon != pExt->LastWeaponType && pExt->TypeExtData->RecountBurst.Get(RulesExt::Global()->RecountBurst))
		{
			if (pExt->LastWeaponType && pExt->LastWeaponType->Burst)
			{
				const auto ratio = static_cast<double>(pThis->CurrentBurstIndex) / pExt->LastWeaponType->Burst;
				const auto rof = static_cast<int>(ratio * pExt->LastWeaponType->ROF * pExt->AE.ROFMultiplier) - (Unsorted::CurrentFrame - pThis->LastFireBulletFrame);

				if (rof > 0)
				{
					pThis->ChargeTurretDelay = rof;
					pThis->RearmTimer.Start(rof);
					pThis->CurrentBurstIndex = 0;
					pExt->LastWeaponType = pWeapon;

					return CanNotFire;
				}
			}

			pThis->CurrentBurstIndex = 0;
		}

		pExt->LastWeaponType = pWeapon;
	}

	return 0;
}

DEFINE_HOOK(0x6FDDC0, TechnoClass_FireAt_DiscardAEOnFire, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->AE.HasOnFireDiscardables)
	{
		for (const auto& attachEffect : pExt->AttachedEffects)
		{
			if ((attachEffect->GetType()->DiscardOn & DiscardCondition::Firing) != DiscardCondition::None)
				attachEffect->ShouldBeDiscarded = true;
		}
	}

	return 0;
}

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
		const auto canTarget = pExt->CanTarget;
		const auto canTargetHouses = pExt->CanTargetHouses;
		const bool skipWeaponPicking = pExt->SkipWeaponPicking;
		const bool onBridge = pThis->OnBridge;
		const int level = pThis->GetCell()->Level;

		if (pExt->AreaFire_Target == AreaFireTarget::Random)
		{
			const auto range = WeaponTypeExt::GetRangeWithModifiers(pWeaponType, pThis) / static_cast<double>(Unsorted::LeptonsPerCell);
			const auto pOwner = pThis->Owner;
			const auto mapCoords = pCell->MapCoords;
			std::vector<CellStruct> adjacentCells = GeneralUtils::AdjacentCellsInRange(static_cast<size_t>(range + 0.99));
			const size_t size = adjacentCells.size();

			for (unsigned int i = 0; i < size; i++)
			{
				int rand = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);
				unsigned int cellIndex = (i + rand) % size;
				CellStruct tgtPos = mapCoords + adjacentCells[cellIndex];
				CellClass* tgtCell = MapClass::Instance.TryGetCellAt(tgtPos);
				bool allowBridges = tgtCell && tgtCell->ContainsBridge() && (onBridge || tgtCell->Level + CellClass::BridgeLevels == level);

				if (skipWeaponPicking || EnumFunctions::AreCellAndObjectsEligible(tgtCell, canTarget, canTargetHouses, pOwner, true, false, allowBridges))
				{
					R->EAX(tgtCell);
					return 0;
				}
			}

			return DoNotFire;
		}
		else if (pExt->AreaFire_Target == AreaFireTarget::Self)
		{
			if (!skipWeaponPicking && !EnumFunctions::AreCellAndObjectsEligible(pThis->GetCell(), canTarget, canTargetHouses, nullptr, false, false, pThis->OnBridge))
				return DoNotFire;

			R->EAX(pThis);
			return SkipSetTarget;
		}

		bool allowBridges = pCell->ContainsBridge() && (onBridge || pCell->Level + CellClass::BridgeLevels == level);

		if (!skipWeaponPicking && !EnumFunctions::AreCellAndObjectsEligible(pCell, canTarget, canTargetHouses, nullptr, false, false, allowBridges))
			return DoNotFire;
	}

	return 0;
}

DEFINE_HOOK(0x6FF43F, TechnoClass_FireAt_FeedbackWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	if (auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		if (auto const pWeaponFeedback = pWeaponExt->FeedbackWeapon)
		{
			if (pThis->InOpenToppedTransport && !pWeaponFeedback->FireInTransport)
				return 0;

			WeaponTypeExt::DetonateAt(pWeaponFeedback, pThis, pThis);
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FF905, TechnoClass_FireAt_FireOnce, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	if (auto const pInf = abstract_cast<InfantryClass*>(pThis))
	{
		if (!WeaponTypeExt::ExtMap.Find(pWeapon)->FireOnce_ResetSequence)
			TechnoExt::ExtMap.Find(pInf)->SkipTargetChangeResetSequence = true;
	}

	return 0;
}

DEFINE_HOOK(0x6FF660, TechnoClass_FireAt_Interceptor, 0x6)
{
	GET(TechnoClass* const, pSource, ESI);
	GET_BASE(AbstractClass* const, pTarget, 0x8);
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

	if (pWeapon->IsLaser)
	{
		if (auto const pExt = BuildingExt::ExtMap.Find(abstract_cast<BuildingClass*, true>(pThis)))
		{
			if (!pExt->CurrentLaserWeaponIndex.has_value())
				pExt->CurrentLaserWeaponIndex = weaponIndex;
			else
				pExt->CurrentLaserWeaponIndex.reset();
		}
	}

	return 0;
}

static inline void SetChargeTurretDelay(TechnoClass* pThis, int rearmDelay, WeaponTypeClass* pWeapon)
{
	pThis->ChargeTurretDelay = rearmDelay;
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWeaponExt->ChargeTurret_Delays.size() > 0)
	{
		const size_t burstIndex = pWeapon->Burst > 1 ? pThis->CurrentBurstIndex - 1 : 0;
		const size_t index = burstIndex < pWeaponExt->ChargeTurret_Delays.size() ? burstIndex : pWeaponExt->ChargeTurret_Delays.size() - 1;
		const int delay = pWeaponExt->ChargeTurret_Delays[index];

		if (delay <= 0)
			return;

		pThis->ChargeTurretDelay = delay;
		TechnoExt::ExtMap.Find(pThis)->ChargeTurretTimer.Start(delay);
	}
}

DEFINE_HOOK(0x6FE4A4, TechnoClass_FireAt_ChargeTurret1, 0x6)
{
	enum { SkipGameCode = 0x6FE4AA };

	GET(TechnoClass*, pThis, ESI);
	GET(int, rearmDelay, EAX);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFSET(0xB0, -0x70));

	SetChargeTurretDelay(pThis, rearmDelay, pWeapon);

	return SkipGameCode;
}

DEFINE_HOOK(0x6FF29E, TechnoClass_FireAt_ChargeTurret2, 0x6)
{
	enum { SkipGameCode = 0x6FF2A4 };

	GET(TechnoClass*, pThis, ESI);
	GET(int, rearmDelay, EAX);
	GET(WeaponTypeClass*, pWeapon, EBX);

	SetChargeTurretDelay(pThis, rearmDelay, pWeapon);

	return SkipGameCode;
}

#pragma endregion

#pragma region TechnoClass_GetFLH
// Feature: Allow Units using AlternateFLHs - by Trsdy
// I don't want to rewrite something new, so I use the Infantry one directly
// afaik it has no check for infantry-specific stuff here so far
// and neither Ares nor Phobos has touched it, even that crawling flh one was in TechnoClass
DEFINE_JUMP(VTABLE, 0x7F5D20, 0x523250);// Redirect UnitClass::GetFLH to InfantryClass::GetFLH (used to be TechnoClass::GetFLH)

// 4.4.2025 - Starkku: Consolidated all the FLH hooks into single one & using TechnoExt::GetFLHAbsoluteCoord() to get the actual coordinate.
DEFINE_HOOK(0x6F3AEB, TechnoClass_GetFLH, 0x6)
{
	enum { SkipGameCode = 0x6F3D50 };

	GET(TechnoClass*, pThis, EBX);
	GET(TechnoTypeClass*, pType, EAX);
	GET(int, weaponIndex, ESI);
	GET_STACK(CoordStruct*, pCoords, STACK_OFFSET(0xD8, 0x4));

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	bool allowOnTurret = true;
	bool useBurstMirroring = true;
	CoordStruct flh = CoordStruct::Empty;

	if (weaponIndex >= 0)
	{
		bool found = false;
		flh = TechnoExt::GetBurstFLH(pThis, weaponIndex, found);

		if (!found)
		{
			if (auto const pInf = abstract_cast<InfantryClass*>(pThis))
				flh = TechnoExt::GetSimpleFLH(pInf, weaponIndex, found);

			if (!found)
				flh = pThis->GetWeapon(weaponIndex)->FLH;
		}
		else
		{
			useBurstMirroring = false;
		}
	}
	else
	{
		int index = -weaponIndex - 1;
		useBurstMirroring = false;

		if (index < static_cast<int>(pTypeExt->AlternateFLHs.size()))
			flh = pTypeExt->AlternateFLHs[index];

		if (!pTypeExt->AlternateFLH_OnTurret)
			allowOnTurret = false;
	}

	if (useBurstMirroring && pThis->CurrentBurstIndex % 2 != 0)
		flh.Y = -flh.Y;

	*pCoords = TechnoExt::GetFLHAbsoluteCoords(pThis, flh, allowOnTurret);
	R->EAX(pCoords);

	return SkipGameCode;
}

#pragma endregion

// Basically a hack to make game and Ares pick laser properties from non-Primary weapons.
DEFINE_HOOK(0x70E1A0, TechnoClass_GetTurretWeapon_LaserWeapon, 0x5)
{
	enum { ReturnResult = 0x70E1C8 };

	GET(TechnoClass* const, pThis, ECX);

	if (auto const pBuilding = abstract_cast<BuildingClass*>(pThis))
	{
		auto const pExt = BuildingExt::ExtMap.Find(pBuilding);

		if (pExt->CurrentLaserWeaponIndex.has_value())
		{
			auto weaponStruct = pThis->GetWeapon(*pExt->CurrentLaserWeaponIndex);
			R->EAX(weaponStruct);
			return ReturnResult;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FCFE0, TechnoClass_RearmDelay_CanCloakDuringRearm, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);

	TechnoExt::ExtMap.Find(pThis)->CanCloakDuringRearm = !pWeapon->DecloakToFire;

	return 0;
}

DEFINE_HOOK(0x6FD0B5, TechnoClass_RearmDelay_ROF, 0x6)
{
	enum { SkipGameCode = 0x6FD0BB };

	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);

	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const range = pWeaponExt->ROF_RandomDelay.Get(RulesExt::Global()->ROF_RandomDelay);
	const double rof = pWeapon->ROF * pExt->AE.ROFMultiplier;
	pExt->LastRearmWasFullDelay = true;

	R->EAX(GeneralUtils::GetRangedRandomOrSingleValue(range));
	__asm { fld rof };

	return SkipGameCode;
}

DEFINE_HOOK(0x6FD054, TechnoClass_RearmDelay_ForceFullDelay, 0x6)
{
	enum { ApplyFullRearmDelay = 0x6FD09E };

	GET(TechnoClass*, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->LastRearmWasFullDelay = false;

	if (pExt->ForceFullRearmDelay)
		return ApplyFullRearmDelay;

	return 0;
}

// Issue #271: Separate burst delay for weapon type
// Author: Starkku
DEFINE_HOOK(0x6FD05E, TechnoClass_RearmDelay_BurstDelays, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET(int, idxCurrentBurst, ECX);

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	const int burstDelay = pWeaponExt->GetBurstDelay(pThis->CurrentBurstIndex);

	if (burstDelay >= 0)
	{
		R->EAX(burstDelay);
		return 0x6FD099;
	}

	// Restore overridden instructions
	return idxCurrentBurst <= 0 || idxCurrentBurst > 4 ? 0x6FD084 : 0x6FD067;
}

// Update ammo rounds
DEFINE_HOOK(0x6FB086, TechnoClass_Reload_ReloadAmount, 0x8)
{
	GET(TechnoClass* const, pThis, ECX);

	TechnoExt::UpdateSharedAmmo(pThis);

	return 0;
}

namespace FiringAITemp
{
	int weaponIndex;
}

DEFINE_HOOK(0x5206D2, InfantryClass_FiringAI_SetContext, 0x6)
{
	GET(int, weaponIndex, EDI);

	FiringAITemp::weaponIndex = weaponIndex;

	return 0;
}

DEFINE_HOOK(0x5209AF, InfantryClass_FiringAI_BurstDelays, 0x6)
{
	enum { Continue = 0x5209CD, ReturnFromFunction = 0x520AD9 };

	GET(InfantryClass*, pThis, EBP);
	GET(int, firingFrame, EDX);

	int cumulativeDelay = 0;
	int projectedDelay = 0;
	const int weaponIndex = FiringAITemp::weaponIndex;
	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	// Calculate cumulative burst delay as well cumulative delay after next shot (projected delay).
	if (pWeaponExt && pWeaponExt->Burst_FireWithinSequence)
	{
		for (int i = 0; i <= pThis->CurrentBurstIndex; i++)
		{
			const int burstDelay = pWeaponExt->GetBurstDelay(i);
			int delay = 0;

			if (burstDelay > -1)
				delay = burstDelay;
			else
				delay = ScenarioClass::Instance->Random.RandomRanged(3, 5);

			// Other than initial delay, treat 0 frame delays as 1 frame delay due to per-frame processing.
			if (i != 0)
				delay = Math::max(delay, 1);

			cumulativeDelay += delay;

			if (i == pThis->CurrentBurstIndex)
				projectedDelay = cumulativeDelay + delay;
		}
	}

	if (pThis->Animation.Value == firingFrame + cumulativeDelay)
	{
		if (pWeaponExt && pWeaponExt->Burst_FireWithinSequence)
		{
			const int frameCount = pThis->Type->Sequence->GetSequence(pThis->SequenceAnim).CountFrames;

			// If projected frame for firing next shot goes beyond the sequence frame count, cease firing after this shot and start rearm timer.
			if (firingFrame + projectedDelay > frameCount)
			{
				auto const pExt = TechnoExt::ExtMap.Find(pThis);
				pExt->ForceFullRearmDelay = true;
			}
		}

		R->EAX(weaponIndex); // Reuse the weapon index to save some time.
		return Continue;
	}

	return ReturnFromFunction;
}

// Author: Otamaa
DEFINE_HOOK(0x5223B3, InfantryClass_Approach_Target_DeployFireWeapon, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	R->EDI(pThis->Type->DeployFireWeapon == -1 ? pThis->SelectWeapon(pThis->Target) : pThis->Type->DeployFireWeapon);
	return 0x5223B9;
}

DEFINE_HOOK(0x44CD18, BuildingClass_MissionMissile_EMPulseCannon_InaccurateRadius, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	HouseClass* pHouse = pThis->Owner;
	const auto pCell = MapClass::Instance.TryGetCellAt(pHouse->EMPTarget);

	if (!pThis->Type->EMPulseCannon || !pCell)
		return 0;

	// Obtain the weapon used by the EMP weapon
	int weaponIndex = 0;
	const auto pExt = BuildingExt::ExtMap.Find(pThis);

	if (pExt->EMPulseSW)
	{
		const auto pSWExt = SWTypeExt::ExtMap.Find(pExt->EMPulseSW->Type);

		if (pSWExt->EMPulse_WeaponIndex >= 0)
		{
			weaponIndex = pSWExt->EMPulse_WeaponIndex;
		}
		else
		{
			AbstractClass* pTarget = pCell;

			if (const auto pObject = pCell->GetContent())
				pTarget = pObject;

			weaponIndex = pThis->SelectWeapon(pTarget);
		}
	}

	const auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

	// Innacurate random strike Area calculation
	int radius = BulletTypeExt::ExtMap.Find(pWeapon->Projectile)->EMPulseCannon_InaccurateRadius;
	radius = radius < 0 ? 0 : radius;

	if (radius > 0)
	{
		if (pExt->RandomEMPTarget == CellStruct::Empty)
		{
			// Calculate a new valid random target coordinate
			do
			{
				pExt->RandomEMPTarget.X = (short)ScenarioClass::Instance->Random.RandomRanged(pHouse->EMPTarget.X - radius, pHouse->EMPTarget.X + radius);
				pExt->RandomEMPTarget.Y = (short)ScenarioClass::Instance->Random.RandomRanged(pHouse->EMPTarget.Y - radius, pHouse->EMPTarget.Y + radius);
			}
			while (!MapClass::Instance.IsWithinUsableArea(pExt->RandomEMPTarget, false));
		}

		pHouse->EMPTarget = pExt->RandomEMPTarget; // Value overwrited every frame
	}

	if (pThis->MissionStatus != 3)
		return 0;

	pExt->RandomEMPTarget = CellStruct::Empty;

	// Restart the super weapon firing process if there is enough ammo set for the current weapon
	if (pThis->Type->Ammo > 0 && pThis->Ammo > 0)
	{
		int ammo = WeaponTypeExt::ExtMap.Find(pWeapon)->Ammo.Get(1);
		pThis->Ammo -= ammo;
		pThis->Ammo = pThis->Ammo < 0 ? 0 : pThis->Ammo;

		if (pThis->Ammo >= ammo)
			pThis->MissionStatus = 0;

		if (!pThis->ReloadTimer.InProgress())
			pThis->ReloadTimer.Start(pThis->Type->Reload);

		if (pThis->Ammo == 0 && pThis->Type->EmptyReload >= 0 && pThis->ReloadTimer.GetTimeLeft() > pThis->Type->EmptyReload)
			pThis->ReloadTimer.Start(pThis->Type->EmptyReload);
	}

	return 0;
}
