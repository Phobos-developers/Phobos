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

// FootClass_Draw_It does nothing. a whole function for Phobos weeeee - Kerbiter
DEFINE_HOOK(4DB250, FootClass_DrawIt, 3)
{
	GET(FootClass*, pThis, ECX);
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	for (auto const& trail: pExt->LaserTrails)
		trail->Draw(TechnoExt::GetFLHAbsoluteCoords(pThis, trail->FLH, trail->IsOnTurret));

	return 0;
}

DEFINE_HOOK(6F42F7, TechnoClass_Init_SetLaserTrails, 2)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const& pFoot = static_cast<FootClass*>(pThis))
		TechnoExt::InitializeLaserTrails(pFoot);

	return 0;
}