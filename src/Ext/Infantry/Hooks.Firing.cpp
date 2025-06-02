#include <Helpers/Macro.h>
#include <InfantryClass.h>
#include <ScenarioClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

namespace FiringAITemp
{
	bool canFire;
	int weaponIndex;
	bool isSecondary;
	WeaponTypeClass* WeaponType;
	FireError fireError;
}

DEFINE_HOOK(0x520AD2, InfantryClass_FiringAI_NoTarget, 0x7)
{
	GET(InfantryClass*, pThis, EBP);

	if (pThis->Type->IsGattling)
	{
		pThis->GattlingRateDown(1);
	}

	FiringAITemp::canFire = false;
	return 0;
}

DEFINE_HOOK(0x5206D2, InfantryClass_FiringAI_SetContext, 0x6)
{
	GET(InfantryClass*, pThis, EBP);
	GET(int, WeaponIndex, EDI);
	enum { SkipGameCode = 0x5209A6 };

	auto const pWeapon = pThis->GetWeapon(WeaponIndex)->WeaponType;

	if (!pWeapon)
	{
		R->AL(false);
		return SkipGameCode;
	}

	const auto pTarget = pThis->Target;
	FiringAITemp::weaponIndex = WeaponIndex;
	FiringAITemp::isSecondary = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->IsSecondary(WeaponIndex);
	FiringAITemp::WeaponType = pWeapon;
	FiringAITemp::fireError = pThis->GetFireError(pTarget, WeaponIndex, true);
	FiringAITemp::canFire = true;

	return 0;
}

// avoid repeatedly calling GetFireError().
DEFINE_HOOK_AGAIN(0x5209D2, InfantryClass_FiringAI_SetFireError, 0x6)
DEFINE_HOOK(0x5206E4, InfantryClass_FiringAI_SetFireError, 0x6)
{
	R->EAX(FiringAITemp::fireError);
	return R->Origin() == 0x5206E4 ? 0x5206F9 : 0x5209E4;
}

// Do you think the infantry's way of determining that weapons are secondary is stupid?
DEFINE_HOOK(0x520968, InfantryClass_UpdateFiring_IsSecondary, 0x6)
{
	enum { Secondary = 0x52096C, SkipGameCode = 0x5209A0 };

	return FiringAITemp::isSecondary ? Secondary : SkipGameCode;
}

// I think it's kind of stupid.
DEFINE_HOOK(0x520888, InfantryClass_UpdateFiring_IsSecondary2, 0x8)
{
	GET(InfantryClass*, pThis, EBP);
	enum { Primary = 0x5208D6, Secondary = 0x520890 };

	R->AL(pThis->Crawling);
	return FiringAITemp::isSecondary ? Secondary : Primary;
}

DEFINE_HOOK(0x5209AF, InfantryClass_FiringAI_BurstDelays, 0x6)
{
	enum { Continue = 0x5209CD, ReturnFromFunction = 0x520AD9 };

	GET(InfantryClass*, pThis, EBP);
	GET(int, firingFrame, EDX);

	int cumulativeDelay = 0;
	int projectedDelay = 0;
	int weaponIndex = FiringAITemp::weaponIndex;
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(FiringAITemp::WeaponType);

	// Calculate cumulative burst delay as well cumulative delay after next shot (projected delay).
	if (pWeaponExt->Burst_FireWithinSequence.Get())
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

	if (pThis->Animation.Value == firingFrame + cumulativeDelay)
	{
		if (pWeaponExt->Burst_FireWithinSequence.Get())
		{
			int frameCount = pThis->Type->Sequence->GetSequence(pThis->SequenceAnim).CountFrames;

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

DEFINE_HOOK(0x520AD9, InfantryClass_FiringAI_IsGattling, 0x5)
{
	GET(InfantryClass*, pThis, EBP);

	if (FiringAITemp::canFire)
	{
		if (pThis->Type->IsGattling)
		{
			FireError fireError = FiringAITemp::fireError;

			switch (fireError)
			{
			case FireError::OK:
			case FireError::REARM:
			case FireError::FACING:
			case FireError::ROTATING:
				pThis->GattlingRateUp(1);
				break;
			default:
				pThis->GattlingRateDown(1);
				break;
			}
		}

		FiringAITemp::canFire = false;
	}

	return 0;
}
