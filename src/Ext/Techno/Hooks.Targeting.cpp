#include "Body.h"

DEFINE_HOOK(0x6FA697, TechnoClass_Update_DontScanIfUnarmed, 0x6)
{
	enum { SkipTargeting = 0x6FA6F5, DoTargeting = 0 };

	GET(TechnoClass*, pThis, ESI);

	if (pThis->IsArmed())
		return DoTargeting;
	else
		return SkipTargeting;
}

DEFINE_HOOK(0x709866, TechnoClass_TargetAndEstimateDamage_ScanDelayGuardArea, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto const pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;
	auto const pOwner = pThis->Owner;
	int delay = 1;

	if (pOwner->IsHumanPlayer || pOwner->IsControlledByHuman())
	{
		delay = pTypeExt->PlayerGuardAreaTargetingDelay.Get(RulesExt::Global()->
			PlayerGuardAreaTargetingDelay.Get(RulesClass::Instance->GuardAreaTargetingDelay));
	}
	else
	{
		delay = pTypeExt->AIGuardAreaTargetingDelay.Get(RulesExt::Global()->
			AIGuardAreaTargetingDelay.Get(RulesClass::Instance->GuardAreaTargetingDelay));
	}

	R->ECX(delay);
	return 0;
}

DEFINE_HOOK(0x70989C, TechnoClass_TargetAndEstimateDamage_ScanDelayNormal, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto const pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;
	auto const pOwner = pThis->Owner;
	int delay = ScenarioClass::Instance->Random.RandomRanged(0, 2);

	if (pOwner->IsHumanPlayer || pOwner->IsControlledByHuman())
	{
		delay += pTypeExt->PlayerNormalTargetingDelay.Get(RulesExt::Global()->
			PlayerNormalTargetingDelay.Get(RulesClass::Instance->NormalTargetingDelay));
	}
	else
	{
		delay += pTypeExt->AINormalTargetingDelay.Get(RulesExt::Global()->
			AINormalTargetingDelay.Get(RulesClass::Instance->NormalTargetingDelay));
	}

	R->ECX(delay);

	return 0;
}

DEFINE_HOOK(0x6FA67D, TechnoClass_Update_DistributeTargetingFrame, 0xA)
{
	enum { Targeting = 0x6FA687, SkipTargeting = 0x6FA6F5 };
	GET(TechnoClass* const, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pTypeExt = pExt->TypeExtData;
	const bool allowed = !pThis->Owner->IsControlledByHuman() ? true : !RulesExt::Global()->DistributeTargetingFrame_AIOnly;

	if (allowed && pTypeExt->DistributeTargetingFrame.Get(RulesExt::Global()->DistributeTargetingFrame))
	{
		if (Unsorted::CurrentFrame % 16 != pExt->MyTargetingFrame)
			return SkipTargeting;
	}

	R->EAX(pThis->MegaMissionIsAttackMove());
	return Targeting;
}
