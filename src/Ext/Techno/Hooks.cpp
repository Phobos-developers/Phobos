#include <InfantryClass.h>
#include <ScenarioClass.h>

#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>

DEFINE_HOOK(6F9E50, TechnoClass_AI, 5)
{
	GET(TechnoClass*, pThis, ECX);

	// MindControlRangeLimit
	TechnoExt::ApplyMindControlRangeLimit(pThis);
	// Interceptor
	TechnoExt::ApplyInterceptor(pThis);
	// Powered.KillSpawns
	TechnoExt::ApplyPowered_KillSpawns(pThis);
	// Spawner.LimitRange & Spawner.ExtraLimitRange
	TechnoExt::ApplySpawn_LimitRange(pThis);
	//
	TechnoExt::ApplyCloak_Undeployed(pThis);

	return 0;
}

DEFINE_HOOK(5184FD, InfantryClass_TakeDamage_DeathWeaponFired_Check, 8)
{
	GET(InfantryClass*, pThis, ESI);
	GET(InfantryTypeClass*, pThisType, EAX);
	REF_STACK(args_ReceiveDamage const, ReceiveDamageArgs, STACK_OFFS(0xD0, -0x4));

	if (pThisType->NotHuman)
	{
		auto SeqInt = 11;
		auto TypeExt = TechnoTypeExt::ExtMap.Find(pThisType);

		if (TypeExt->NHumanUseAllDeathSeq.Get())
			SeqInt = ScenarioClass::Instance->Random.RandomRanged(11, 15);

		if (ReceiveDamageArgs.WH)
		{
			if (auto const WarheadExt = WarheadTypeExt::ExtMap.Find(ReceiveDamageArgs.WH))
			{
				auto WarheadSeq = WarheadExt->InfDeathSequence.Get();
				if (WarheadSeq > 0)
				{
					WarheadSeq = WarheadSeq > 5 ? 5 : WarheadSeq;
					SeqInt = 10 + WarheadSeq;
				}
			}
		}

		pThis->PlayAnim(static_cast<Sequence>(SeqInt), true, false);

		return 0x518515;
	}

	return 0x5185C8;
}