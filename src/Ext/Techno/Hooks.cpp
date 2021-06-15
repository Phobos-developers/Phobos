#include <InfantryClass.h>
#include <ScenarioClass.h>

#include "Body.h"

#include <Ext/TechnoType/Body.h>

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

	if (Victim->NotHuman)
	{
		auto SeqInt = 11; //Default WWp Hardcode Sequence::Die1

		auto Ext = TechnoTypeExt::ExtMap.Find(pVictim->GetTechnoType());
		if (Ext->NHumanUseAllDeathSeq.Get())
		{
			/* Land Type Checking is not working , i wonder why :s
			auto cell = MapClass::Instance->TryGetCellAt(pVictim->GetCoords());
			if (cell->LandType == LandType::Water || cell->LandType == LandType::Beach)
			{
			/
				SeqInt = ScenarioClass::Instance->Random.RandomRanged(20, 21);//WetDie1 & WetDie2
				Debug::Log(__FUNCTION__"NonHuman random on water play %d \n", SeqInt);
			}
			else
			{

				SeqInt = ScenarioClass::Instance->Random.RandomRanged(11, 15);
				Debug::Log(__FUNCTION__"NonHuman random on Land play %d \n", SeqInt);
			}*/

			SeqInt = ScenarioClass::Instance->Random.RandomRanged(11, 15);
		}

		pVictim->PlayAnim(static_cast<Sequence>(SeqInt), true, false);

		return 0x518515;
	}

	return 0x5185C8;
}