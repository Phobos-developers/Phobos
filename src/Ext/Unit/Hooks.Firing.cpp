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

	if (fireUp >= 0 && !pType->OpportunityFire && pThis->Locomotor->Is_Really_Moving_Now())
	{
		if (pThis->CurrentFiringFrame != -1)
			pThis->CurrentFiringFrame = -1;

		return SkipFiring;
	}

	const int firingFrames = pType->FiringFrames;
	const int frames = 2 * firingFrames - 1;

	if (frames >= 0 && pThis->CurrentFiringFrame == -1)
		pThis->CurrentFiringFrame = frames;

	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (fireUp >= 0)
	{
		int cumulativeDelay = 0;
		int projectedDelay = 0;
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

		const int frame = (frames - pThis->CurrentFiringFrame) / 2;
		const int firingFrame = fireUp + cumulativeDelay;

		if (TechnoExt::HandleDelayedFireWithPauseSequence(pThis, pWeapon, weaponIndex, frame, firingFrame))
			return SkipFiring;

		if (frame != firingFrame)
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
	else if (TechnoExt::HandleDelayedFireWithPauseSequence(pThis, pWeapon, weaponIndex, 0, -1))
	{
		return SkipFiring;
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
