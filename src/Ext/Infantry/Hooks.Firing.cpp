#include <Helpers/Macro.h>
#include <InfantryClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

namespace FiringAITemp
{
	bool CanFire;
	int WeaponIndex;
	bool IsSecondary;
	WeaponTypeClass* WeaponType;
	FireError FireErrorResult;
}

DEFINE_HOOK(0x520AD2, InfantryClass_FiringAI_NoTarget, 0x7)
{
	GET(InfantryClass*, pThis, EBP);

	if (pThis->Type->IsGattling)
		pThis->GattlingRateDown(1);

	FiringAITemp::CanFire = false;
	return 0;
}

DEFINE_HOOK(0x5206D2, InfantryClass_FiringAI_SetContext, 0x6)
{
	enum { SkipGameCode = 0x5209A6 };

	GET(InfantryClass*, pThis, EBP);
	GET(int, weaponIndex, EDI);

	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

	if (!pWeapon)
	{
		if (pThis->Type->IsGattling)
			pThis->GattlingRateDown(1);

		R->AL(false);
		FiringAITemp::CanFire = false;
		return SkipGameCode;
	}

	const auto pTarget = pThis->Target;
	FiringAITemp::WeaponIndex = weaponIndex;
	FiringAITemp::IsSecondary = TechnoTypeExt::ExtMap.Find(pThis->Type)->IsSecondary(weaponIndex);
	FiringAITemp::WeaponType = pWeapon;
	FiringAITemp::FireErrorResult = pThis->GetFireError(pTarget, weaponIndex, true);
	FiringAITemp::CanFire = true;

	return 0;
}

// avoid repeatedly calling GetFireError().
DEFINE_HOOK_AGAIN(0x5209D2, InfantryClass_FiringAI_SetFireError, 0x6)
DEFINE_HOOK(0x5206E4, InfantryClass_FiringAI_SetFireError, 0x6)
{
	R->EAX(FiringAITemp::FireErrorResult);
	return R->Origin() == 0x5206E4 ? 0x5206F9 : 0x5209E4;
}

// determine if it is the second.
DEFINE_HOOK_AGAIN(0x520968, InfantryClass_UpdateFiring_IsSecondary, 0x6)
DEFINE_HOOK(0x520888, InfantryClass_UpdateFiring_IsSecondary, 0x8)
{
	GET(InfantryClass*, pThis, EBP);
	const bool isSecondary = FiringAITemp::IsSecondary;

	if (R->Origin() == 0x520888)
	{
		R->AL(pThis->Crawling);
		return isSecondary ? 0x520890 : 0x5208DC;
	}
	else if (isSecondary)
	{
		return pThis->Crawling ? 0x520970 : 0x52098A;
	}
	else
	{
		return 0x5209A0;
	}
}

DEFINE_HOOK(0x5209AF, InfantryClass_FiringAI, 0x6)
{
	enum { Continue = 0x5209CD, ReturnFromFunction = 0x520AD9 };

	GET(InfantryClass*, pThis, EBP);
	GET(int, firingFrame, EDX);

	int cumulativeDelay = 0;
	int projectedDelay = 0;
	const int weaponIndex = FiringAITemp::WeaponIndex;
	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(FiringAITemp::WeaponType);
	const bool allowBurst = pWeaponExt->Burst_FireWithinSequence;

	// Calculate cumulative burst delay as well cumulative delay after next shot (projected delay).
	if (allowBurst)
	{
		for (int i = 0; i <= pThis->CurrentBurstIndex; i++)
		{
			const int burstDelay = pWeaponExt->GetBurstDelay(i);
			int delay = (burstDelay > -1) ? burstDelay : ScenarioClass::Instance->Random.RandomRanged(3, 5);

			// Other than initial delay, treat 0 frame delays as 1 frame delay due to per-frame processing.
			if (i != 0)
				delay = Math::max(delay, 1);

			cumulativeDelay += delay;

			if (i == pThis->CurrentBurstIndex)
				projectedDelay = cumulativeDelay + delay;
		}
	}

	if (TechnoExt::HandleDelayedFireWithPauseSequence(pThis, weaponIndex, firingFrame + cumulativeDelay))
		return ReturnFromFunction;

	if (pThis->Animation.Value == firingFrame + cumulativeDelay)
	{
		if (allowBurst)
		{
			int frameCount = pThis->Type->Sequence->GetSequence(pThis->SequenceAnim).CountFrames;

			// If projected frame for firing next shot goes beyond the sequence frame count, cease firing after this shot and start rearm timer.
			if (firingFrame + projectedDelay > frameCount)
				TechnoExt::ExtMap.Find(pThis)->ForceFullRearmDelay = true;
		}

		R->EAX(weaponIndex); // Reuse the weapon index to save some time.
		return Continue;
	}

	return ReturnFromFunction;
}

DEFINE_HOOK(0x520AD9, InfantryClass_FiringAI_IsGattling, 0x5)
{
	GET(InfantryClass*, pThis, EBP);

	if (FiringAITemp::CanFire)
	{
		if (pThis->Type->IsGattling)
		{
			const FireError fireError = FiringAITemp::FireErrorResult;

			switch (fireError)
			{
			case FireError::OK:
			case FireError::REARM:
			case FireError::FACING:
			case FireError::ROTATING:
			{
				if (pThis->IsDeployed())
					pThis->GattlingRateDown(1);
				else
					pThis->GattlingRateUp(1);

				break;
			}
			default:
				pThis->GattlingRateDown(1);
				break;
			}
		}

		FiringAITemp::CanFire = false;
	}

	return 0;
}

DEFINE_HOOK(0x5209EE, InfantryClass_UpdateFiring_BurstNoDelay, 0x5)
{
	enum { SkipVanillaFire = 0x520A57 };

	GET(InfantryClass* const, pThis, EBP);
	GET(const int, wpIdx, ESI);
	GET(AbstractClass* const, pTarget, EAX);

	if (const auto pWeapon = pThis->GetWeapon(wpIdx)->WeaponType)
	{
		if (pWeapon->Burst > 1)
		{
			const auto pExt = WeaponTypeExt::ExtMap.Find(pWeapon);

			if (pExt->Burst_NoDelay && (!pExt->DelayedFire_Duration.isset() || pExt->DelayedFire_OnlyOnInitialBurst))
			{
				if (pThis->Fire(pTarget, wpIdx))
				{
					if (!pThis->CurrentBurstIndex)
						return SkipVanillaFire;

					auto rof = pThis->RearmTimer.TimeLeft;
					pThis->RearmTimer.Start(0);

					for (auto i = pThis->CurrentBurstIndex; i < pWeapon->Burst && pThis->GetFireError(pTarget, wpIdx, true) == FireError::OK && pThis->Fire(pTarget, wpIdx); ++i)
					{
						rof = pThis->RearmTimer.TimeLeft;
						pThis->RearmTimer.Start(0);
					}

					pThis->RearmTimer.Start(rof);
				}

				return SkipVanillaFire;
			}
		}
	}

	return 0;
}
