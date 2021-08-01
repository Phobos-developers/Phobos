#include <InfantryClass.h>

#include "Body.h"

#include <Ext/TechnoType/Body.h>

DEFINE_HOOK(0x6F9E50, TechnoClass_AI, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	TechnoExt::ApplyMindControlRangeLimit(pThis);
	TechnoExt::ApplyInterceptor(pThis);
	TechnoExt::ApplyPowered_KillSpawns(pThis);
	TechnoExt::ApplySpawn_LimitRange(pThis);
	TechnoExt::ApplyCloak_Undeployed(pThis);

	// LaserTrails update routine is in TechnoClass::AI hook because TechnoClass::Draw
	// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
	for (auto const& trail: pExt->LaserTrails)
		trail->Update(TechnoExt::GetFLHAbsoluteCoords(pThis, trail->FLH, trail->IsOnTurret));

	return 0;
}


DEFINE_HOOK(0x6F42F7, TechnoClass_Init_SetLaserTrails, 0x2)
{
	GET(TechnoClass*, pThis, ESI);

	TechnoExt::InitializeLaserTrails(pThis);

	return 0;
}

DEFINE_HOOK(0x702E4E, TechnoClass_Save_Killer_Techno, 0x6)
{
    GET(TechnoClass*, pKiller, EDI);
    GET(TechnoClass*, pVictim, ECX);

    if (pKiller && pVictim)
        TechnoExt::ObjectKilledBy(pVictim, pKiller);

    return 0;
}

DEFINE_HOOK(0x702E9D, TechnoClass_Save_Killer_Techno2, 0x6)
{
	GET(TechnoClass*, pKiller, EDI);
	GET(TechnoClass*, pVictim, ESI);

	if (pKiller && pVictim)
		TechnoExt::ObjectKilledBy(pVictim, pKiller);

	return 0;
}
