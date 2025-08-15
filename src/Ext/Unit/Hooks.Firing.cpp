#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

DEFINE_JUMP(LJMP, 0x741406, 0x741427)

DEFINE_HOOK(0x736F61, UnitClass_UpdateFiring_FireUp, 0x6)
{
	enum { SkipFiring = 0x737063 };

	GET(UnitClass*, pThis, ESI);
	GET(int, weaponIndex, EDI);

	const auto pType = pThis->Type;

	if (pType->Turret || pType->Voxel)
		return 0;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	// SHP vehicles have no secondary action frames, so it does not need SecondaryFire.
	const auto pTypeExt = pExt->TypeExtData;
	const int fireUp = pTypeExt->FireUp;
	CDTimerClass& timer = pExt->FiringAnimationTimer;

	if (fireUp >= 0 && !pType->OpportunityFire && pThis->Locomotor->Is_Really_Moving_Now())
	{
		if (timer.InProgress())
			timer.Stop();

		return SkipFiring;
	}

	const int frames = pType->FiringFrames;

	if (!timer.InProgress() && frames >= 1)
	{
		pThis->CurrentFiringFrame = 2 * frames - 1;
		timer.Start(pThis->CurrentFiringFrame);
	}

	if (fireUp >= 0 && frames >= 1)
	{
		int cumulativeDelay = 0;
		int projectedDelay = 0;
		auto const pWeaponExt = WeaponTypeExt::ExtMap.TryFind(pThis->GetWeapon(weaponIndex)->WeaponType);
		const bool allowBurst = pWeaponExt && pWeaponExt->Burst_FireWithinSequence;
		const int currentBurstIndex = pThis->CurrentBurstIndex;
		auto& random = ScenarioClass::Instance->Random;

		// Calculate cumulative burst delay as well cumulative delay after next shot (projected delay).
		if (allowBurst)
		{
			for (int i = 0; i <= currentBurstIndex; i++)
			{
				const int burstDelay = pWeaponExt->GetBurstDelay(i);
				int delay = (burstDelay > -1) ? burstDelay : random.RandomRanged(3, 5);

				// Other than initial delay, treat 0 frame delays as 1 frame delay due to per-frame processing.
				if (i != 0)
					delay = Math::max(delay, 1);

				cumulativeDelay += delay;

				if (i == currentBurstIndex)
					projectedDelay = cumulativeDelay + delay;
			}
		}

		if (TechnoExt::HandleDelayedFireWithPauseSequence(pThis, weaponIndex, fireUp + cumulativeDelay))
			return SkipFiring;

		int frame = (timer.TimeLeft - timer.GetTimeLeft());

		if (frame % 2 != 0)
			return SkipFiring;

		if (frame / 2 != fireUp + cumulativeDelay)
		{
			return SkipFiring;
		}
		else if (allowBurst)
		{
			// If projected frame for firing next shot goes beyond the sequence frame count, cease firing after this shot and start rearm timer.
			if (fireUp + projectedDelay > frames)
				pExt->ForceFullRearmDelay = true;
		}
	}

	return 0;
}

DEFINE_HOOK(0x736F67, UnitClass_UpdateFiring_BurstNoDelay, 0x6)
{
	enum { SkipVanillaFire = 0x737063 };

	GET(UnitClass* const, pThis, ESI);
	GET(const int, wpIdx, EDI);
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
