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

DEFINE_HOOK(5184FD, InfantryClass_TakeDamage_NotHumanCheck, 8)
{
	GET(InfantryClass*, pVictim, ESI);
	GET(InfantryTypeClass*, Victim, EAX);
	GET_STACK(WarheadTypeClass *, Warhead, STACK_OFFS(0xD0, -0xC));

	if (Victim->NotHuman)
	{
		auto SeqInt = 11; //Default WWp Hardcode Sequence::Die1

		auto Ext = TechnoTypeExt::ExtMap.Find(pVictim->GetTechnoType());

		if (Ext->NHumanUseAllDeathSeq.Get())
			SeqInt = ScenarioClass::Instance->Random.RandomRanged(11, 15);

		if (Warhead)
		{
			if (auto const WHExt = WarheadTypeExt::ExtMap.Find(Warhead))
			{
				auto WHSeq = WHExt->InfDeathSequence.Get();
				if (WHSeq > 0)
				{
					WHSeq = WHSeq > 5 ? 5 : WHSeq;

					switch (WHSeq)
					{
					case 1:
						SeqInt = 11;
						break;
					case 2:
						SeqInt = 12;
						break;
					case 3:
						SeqInt = 13;
						break;
					case 4:
						SeqInt = 14;
						break;
					case 5:
						SeqInt = 15;
						break;
					default:
						SeqInt = SeqInt;
						break;
					}
				}
			}
		}

		pVictim->PlayAnim(static_cast<Sequence>(SeqInt), true, false);

		return 0x518515;
	}

	return 0x5185C8;
}