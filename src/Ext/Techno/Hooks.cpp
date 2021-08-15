#include <InfantryClass.h>
#include <ScenarioClass.h>

#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>

DEFINE_HOOK(0x6F9E50, TechnoClass_AI, 0x5)
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

// Issue #237 NotHuman additional animations support
// Author: Otamaa
DEFINE_HOOK(0x518505, InfantryClass_TakeDamage_NotHuman, 0x4)
{
	GET(InfantryClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, nReceiveDamageArgs, STACK_OFFS(0xD0, -0x4));

	auto nSequenceint = 11;
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->NotHumanRandomDeathSequence.Get())
		nSequenceint = ScenarioClass::Instance->Random.RandomRanged(11, 15);

	if (nReceiveDamageArgs.WH)
	{
		if (auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(nReceiveDamageArgs.WH))
		{
			auto nWarheadsequence = pWarheadExt->NotHumanDeathSequence.Get();
			if (nWarheadsequence > 0)
				nSequenceint = 10 + Math::min(nWarheadsequence, 5);
		}
	}

	R->ECX(pThis);
	pThis->PlayAnim(static_cast<Sequence>(nSequenceint), true);

	return 0x518515;
}