#include <InfantryClass.h>

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

DEFINE_HOOK(702E4E, TechnoClass_Save_Killer_Techno, 6)
{
    GET(TechnoClass*, pKiller, EDI);
    GET(TechnoClass*, pVictim, ECX);

    if (pKiller && pVictim)
        TechnoExt::ObjectKilledBy(pVictim, pKiller);

    return 0;
}

DEFINE_HOOK(702E9D, TechnoClass_Save_Killer_Techno2, 6)
{
	GET(TechnoClass*, pKiller, EDI);
	GET(TechnoClass*, pVictim, ESI);

	if (pKiller && pVictim)
		TechnoExt::ObjectKilledBy(pVictim, pKiller);

	return 0;
}
