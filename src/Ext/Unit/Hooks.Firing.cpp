#include <Helpers/Macro.h>
#include <Utilities/Macro.h>

#include <UnitClass.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

DEFINE_JUMP(LJMP, 0x741406, 0x741427)

DEFINE_HOOK(0x736F61, UnitClass_UpdateFiring_FireUp, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(int, nWeaponIndex, EDI);
	enum { SkipFiring = 0x736F73 };

	const auto pType = pThis->Type;

	if (pType->Turret || pType->Voxel)
		return 0;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	// SHP vehicles have no secondary action frames, so it does not need SecondaryFire.
	const auto pTypeExt = pExt->TypeExtData;
	int fireUp = pTypeExt->FireUp.Get();
	CDTimerClass& Timer = pExt->FiringAnimationTimer;

	if (fireUp >= 0 && !pType->OpportunityFire &&
		pThis->Locomotor->Is_Really_Moving_Now())
	{
		if (Timer.InProgress())
			Timer.Stop();

		return SkipFiring;
	}

	int frames = pType->FiringFrames;
	if (!Timer.InProgress() && frames >= 1)
	{
		pThis->CurrentFiringFrame = 2 * frames - 1;
		Timer.Start(pThis->CurrentFiringFrame);
	}

	if (fireUp >= 0 && frames >= 1)
	{
		int cumulativeDelay = 0;
		int projectedDelay = 0;
		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(nWeaponIndex)->WeaponType);
		bool allowBurst = pWeaponExt && pWeaponExt->Burst_FireWithinSequence.Get();

		// Calculate cumulative burst delay as well cumulative delay after next shot (projected delay).
		if (allowBurst)
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

		int frame = (Timer.TimeLeft - Timer.GetTimeLeft());
		if (frame % 2 != 0)
			return SkipFiring;

		int value = frame / 2;
		if (value != fireUp + cumulativeDelay)
		{
			return SkipFiring;
		}
		else if (allowBurst)
		{
			// If projected frame for firing next shot goes beyond the sequence frame count, cease firing after this shot and start rearm timer.
			if (fireUp + projectedDelay > frames)
			{
				pExt->ForceFullRearmDelay = true;
			}
		}
	}

	return 0;
}
