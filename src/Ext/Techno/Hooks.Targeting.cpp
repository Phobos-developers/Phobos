#include "Body.h"

#include <Ext/WeaponType/Body.h>

DEFINE_HOOK(0x6FA697, TechnoClass_Update_DontScanIfUnarmed, 0x6)
{
	enum { SkipTargeting = 0x6FA6F5 };
	GET(TechnoClass* const, pThis, ESI);
	return pThis->IsArmed() ? 0 : SkipTargeting;
}

DEFINE_HOOK(0x70982C, TechnoClass_TargetAndEstimateDamage_TargetingDelay, 0x8)
{
	enum { SkipGameCode = 0x70989C };

	GET(TechnoClass* const, pThis, ESI);
	GET(const int, frame, EAX);

	pThis->unknown_4FC = frame;
	int delay = ScenarioClass::Instance->Random.RandomRanged(0, 2);
	const auto pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;

	if (pThis->MegaMissionIsAttackMove())
	{
		delay += pThis->Owner->IsControlledByHuman()
			? pTypeExt->PlayerAttackMoveTargetingDelay.Get(RulesExt::Global()->PlayerAttackMoveTargetingDelay.Get(RulesClass::Instance->NormalTargetingDelay))
			: pTypeExt->AIAttackMoveTargetingDelay.Get(RulesExt::Global()->AIAttackMoveTargetingDelay.Get(RulesClass::Instance->NormalTargetingDelay));
	}
	else if (pThis->CurrentMission == Mission::Area_Guard)
	{
		delay += pThis->Owner->IsControlledByHuman()
			? pTypeExt->PlayerGuardAreaTargetingDelay.Get(RulesExt::Global()->PlayerGuardAreaTargetingDelay.Get(RulesClass::Instance->GuardAreaTargetingDelay))
			: pTypeExt->AIGuardAreaTargetingDelay.Get(RulesExt::Global()->AIGuardAreaTargetingDelay.Get(RulesClass::Instance->GuardAreaTargetingDelay));
	}
	else
	{
		delay += pThis->Owner->IsControlledByHuman()
			? pTypeExt->PlayerNormalTargetingDelay.Get(RulesExt::Global()->PlayerNormalTargetingDelay.Get(RulesClass::Instance->NormalTargetingDelay))
			: pTypeExt->AINormalTargetingDelay.Get(RulesExt::Global()->AINormalTargetingDelay.Get(RulesClass::Instance->NormalTargetingDelay));
	}

	R->ECX(delay);
	R->EDX(frame);

	return SkipGameCode;
}

DEFINE_HOOK(0x6F7CE2, TechnoClass_CanAutoTargetObject_IronCurtain, 0x6)
{
	enum { ReturnFalse = 0x6F894F };

	GET(TechnoClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);

	if (pThis->Owner->IsControlledByHuman() && pTarget->IsIronCurtained())
	{
		GET(WeaponTypeClass*, pWeapon, EBP);

		if (pWeapon)
		{
			const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

			if (pWeaponExt->AutoTarget_IronCurtained.isset())
				return pWeaponExt->AutoTarget_IronCurtained.Get() ? 0 : ReturnFalse;
		}

		return RulesExt::Global()->AutoTarget_IronCurtained ? 0 : ReturnFalse;
	}

	return 0;
}

// WW adds an optimization that: If the techno get a target in 1/4 or 1/2 of their targeting range, then it will not checking other targets.
DEFINE_HOOK(0x6F9AF4, TechnoClass_SelectAutoTarget_DisableStupid, 0x6)
{
	return RulesExt::Global()->DisableOveroptimizationInTargeting ? 0x6F9B1B : 0;
}
