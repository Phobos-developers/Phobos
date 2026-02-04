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

#pragma region ExtendedAutoTargeting

namespace ExtendedAutoTargetingContext
{
	AbstractClass* OldTarget = nullptr;
	int OldThreat = -1;

	AbstractClass* NewTarget = nullptr;
	int NewThreat = -1;

	void Clear()
	{
		OldTarget = nullptr;
		OldThreat = -1;
		NewTarget = nullptr;
		NewThreat = -1;
	}
}

// Record old target
DEFINE_HOOK(0x6FA6C9, TechnoClass_Update_RecordOldTarget, 0x6)
{
	if (!RulesExt::Global()->ExtendedAutoTargeting)
		return 0;

	GET(TechnoClass*, pThis, ESI);

	auto pOldTarget = pThis->Target;

	if (pOldTarget && (pOldTarget->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None && pThis->IsCloseEnoughToAttack(pOldTarget) && pThis->ShouldLoseTargetNow) // OpportunityFire, in-range only
	{
		auto crd = pThis->GetCoords();
		ExtendedAutoTargetingContext::OldTarget = pOldTarget;
		ExtendedAutoTargetingContext::OldThreat = (int)pThis->ThreatCoeffients(static_cast<ObjectClass*>(pOldTarget), &crd);
		pThis->Target = 0;
	}

	return 0;
}

DEFINE_HOOK(0x4DF3A0, TechnoClass_UpdateAttackMove_RecordOldTarget1, 0x6)
{
	if (!RulesExt::Global()->ExtendedAutoTargeting)
		return 0;

	GET(FootClass*, pThis, ECX);

	auto pOldTarget = pThis->Target;

	if (pOldTarget && (pOldTarget->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None && pThis->InAuxiliarySearchRange(pOldTarget)) // AttackMove has its own range checking
	{
		auto crd = pThis->GetCoords();
		ExtendedAutoTargetingContext::OldTarget = pOldTarget;
		ExtendedAutoTargetingContext::OldThreat = (int)pThis->ThreatCoeffients(static_cast<ObjectClass*>(pOldTarget), &crd);
		pThis->Target = 0;
		pThis->HaveAttackMoveTarget = false;
	}

	return 0;
}

DEFINE_HOOK(0x4DF42A, TechnoClass_UpdateAttackMove_SkipCheck, 0x6)
{
	if (!RulesExt::Global()->ExtendedAutoTargeting)
		return 0;

	enum { SkipCheck = 0x4DF462, FuncEnd = 0x4DF4AB };

	GET(FootClass*, pThis, ESI);

	return pThis->MegaTarget ? SkipCheck : FuncEnd;
}

DEFINE_HOOK(0x4D6ED1, TechnoClass_MissionAreaGuard_RecordOldTarget, 0x6)
{
	if (!RulesExt::Global()->ExtendedAutoTargeting)
		return 0;

	GET(TechnoClass*, pThis, ESI);

	auto pOldTarget = pThis->Target;

	if (pOldTarget && (pOldTarget->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None && pThis->CanPassiveAcquireTargets() && pThis->TargetingTimerFinished() && pThis->DistanceFrom(pOldTarget) <= pThis->GetGuardRange(1)) // AreaGuard, use GetGuardRange
	{
		auto crd = pThis->ArchiveTarget->GetCoords();
		ExtendedAutoTargetingContext::OldTarget = pOldTarget;
		ExtendedAutoTargetingContext::OldThreat = (int)pThis->ThreatCoeffients(static_cast<ObjectClass*>(pOldTarget), &crd);
		pThis->Target = 0;
	}

	return 0;
}

// Use old target when targeting
DEFINE_HOOK(0x6F8E1F, TechnoClass_SelectAutoTarget_UseContext, 0x6)
{
	if (!RulesExt::Global()->ExtendedAutoTargeting)
		return 0;

	R->Stack(STACK_OFFSET(0x6C, -0x4C), ExtendedAutoTargetingContext::OldTarget);
	R->Stack(STACK_OFFSET(0x6C, -0x50), ExtendedAutoTargetingContext::OldThreat);
	return 0;
}

// Record new target
DEFINE_HOOK(0x6F936F, TechnoClass_SelectAutoTarget_RecordNew1, 0x8)
{
	if (!RulesExt::Global()->ExtendedAutoTargeting)
		return 0;

	GET(AbstractClass*, pBestTarget, EBP);
	GET(int, bestThreat, EAX);

	if (ExtendedAutoTargetingContext::OldTarget)
	{
		ExtendedAutoTargetingContext::NewTarget = pBestTarget;
		ExtendedAutoTargetingContext::NewThreat = bestThreat;
	}

	return 0;
}

DEFINE_HOOK(0x6F955B, TechnoClass_SelectAutoTarget_RecordNew2, 0x8)
{
	if (!RulesExt::Global()->ExtendedAutoTargeting)
		return 0;

	GET(AbstractClass*, pBestTarget, EDX);
	GET(int, bestThreat, EAX);

	if (ExtendedAutoTargetingContext::OldTarget)
	{
		ExtendedAutoTargetingContext::NewTarget = pBestTarget;
		ExtendedAutoTargetingContext::NewThreat = bestThreat;
	}

	return 0;
}

// Check if new target has enough threat
DEFINE_HOOK(0x709938, TechnoClass_TargetAndEstimateDamage_CheckContext, 0x6)
{
	if (!RulesExt::Global()->ExtendedAutoTargeting)
		return 0;

	GET(TechnoClass*, pThis, ESI);

	if (ExtendedAutoTargetingContext::OldTarget && ExtendedAutoTargetingContext::NewTarget && ExtendedAutoTargetingContext::OldTarget != ExtendedAutoTargetingContext::NewTarget)
	{
		auto crd = pThis->GetCoords();
		//if (ExtendedAutoTargetingContext::NewThreat < ExtendedAutoTargetingContext::OldThreat + RulesExt::Global()->ExtendedAutoTargeting_SwitchTargetThreshold)
		if (pThis->ThreatCoeffients(static_cast<ObjectClass*>(ExtendedAutoTargetingContext::NewTarget), &crd) < ExtendedAutoTargetingContext::OldThreat + RulesExt::Global()->ExtendedAutoTargeting_SwitchTargetThreshold)
			R->EAX(ExtendedAutoTargetingContext::OldTarget);
	}

	return 0;
}

// Reset context
DEFINE_HOOK_AGAIN(0x6FA6F5, ExtendedAutoTargetingContext_Clear, 0x5); // Update
DEFINE_HOOK_AGAIN(0x4DF4AB, ExtendedAutoTargetingContext_Clear, 0x5); // UpdateAttackMove
DEFINE_HOOK(0x4DF41E, ExtendedAutoTargetingContext_Clear, 0x7) // UpdateAttackMove
{
	ExtendedAutoTargetingContext::Clear();
	return 0;
}

DEFINE_HOOK(0x6FA6E6, TechnoClass_Update_SetIsOpportunityTarget, 0x6)
{
	if (!RulesExt::Global()->ExtendedAutoTargeting)
		return 0;

	GET(TechnoClass*, pThis, ESI);

	// This field indicate that the target is acquired by OpportunityFire, thus should be cleared if out of range.
	// Must set it manually because the SetTarget in TargetAndEstimateDamage will reset it.
	if (pThis->Target == ExtendedAutoTargetingContext::OldTarget)
		pThis->ShouldLoseTargetNow = true;

	return 0;
}

DEFINE_HOOK(0x4D6F0C, FootClass_MissionAreaGuard_AfterTargeting, 0x6) // AreaGuard
{
	GET(FootClass*, pThis, ESI);

	if (pThis->Target)
		pThis->ApproachTarget(0);

	ExtendedAutoTargetingContext::Clear();
	return 0;
}

// Stop command
DEFINE_HOOK(0x4C757D, EventClass_RespondToEvent_IDLE_ClearTargetingTimer, 0x6)
{
	if (!RulesExt::Global()->ExtendedAutoTargeting)
		return 0;

	GET(TechnoClass*, pThis, ESI);
	pThis->TargetingTimer.Start(0);
	return 0;
}

// Clicked mission
DEFINE_HOOK(0x4C72E8, EventClass_RespondToEvent_MegaMission_ClearTargetingTimer, 0x6)
{
	if (!RulesExt::Global()->ExtendedAutoTargeting)
		return 0;

	GET(TechnoClass*, pThis, EDI);
	pThis->TargetingTimer.Start(0);
	return 0;
}

// Target expired.
DEFINE_HOOK(0x7079D1, TechnoClass_PointerExpired_ClearTargetingTimer, 0x6)
{
	if (!RulesExt::Global()->ExtendedAutoTargeting)
		return 0;

	GET(TechnoClass*, pThis, ESI);
	pThis->TargetingTimer.Start(0);

	if (pThis->MegaMissionIsAttackMove())
		pThis->UpdateTimer.Start(0);

	return 0;
}

// ContinueMegaMission
DEFINE_HOOK(0x4DF320, FootClass_ContinueMegaMission_Start, 0x6)
{
	if (!RulesExt::Global()->ExtendedAutoTargeting)
		return 0;

	enum { RETN = 0x4DF395 };

	GET(FootClass*, pThis, ECX);

	if (!pThis->Target)
	{
		pThis->HaveAttackMoveTarget = 0;
		pThis->HaveAttackMoveTarget = pThis->TargetAndEstimateDamage(pThis->Location, ThreatType::Range);
		return pThis->HaveAttackMoveTarget ? RETN : 0;
	}

	return 0;
}

#pragma endregion
