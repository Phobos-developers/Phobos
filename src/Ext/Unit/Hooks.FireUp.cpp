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
	enum { SkipGameCode = 0x736F73 };

	const auto pType = pThis->Type;

	if (pType->Turret || pType->Voxel)
		return 0;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto pTypeExt = pExt->TypeExtData;

	// SHP vehicles have no secondary action frames, so it does not need SecondaryFire.
	int fireUp = pTypeExt->FireUp.Get();
	CDTimerClass& pTimer = pExt->FiringAnimationTimer;

	if (fireUp > 0 && !pType->OpportunityFire &&
		pThis->Locomotor->Is_Really_Moving_Now())
	{
		if (pTimer.InProgress())
			pTimer.Stop();

		return SkipGameCode;
	}

	int frames = pType->FiringFrames;

	if (!pTimer.InProgress())
	{
		if (frames >= 0)
		{
			pThis->unknown_int_6C0 = 2 * frames - 1;
			pTimer.Start(frames);
		}
	}

	int cumulativeDelay = 0;
	int projectedDelay = 0;
	auto const pWeapon = pThis->GetWeapon(nWeaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	// Calculate cumulative burst delay as well cumulative delay after next shot (projected delay).
	if (pWeaponExt && pWeaponExt->Burst_FireWithinSequence)
	{
		for (int i = 0; i <= pThis->CurrentBurstIndex; i++)
		{
			int burstDelay = pWeaponExt->GetBurstDelay(i);
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

	if (fireUp > 0 && frames >= 0)
	{
		int value = Unsorted::CurrentFrame - pTimer.StartTime;

		if (value != fireUp)
		{
			return SkipGameCode;
		}
		else if (pWeaponExt && pWeaponExt->Burst_FireWithinSequence)
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
