#include <InfantryClass.h>

#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>

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

// Issue #271: Separate burst delay for weapon type
// Author: Starkku
DEFINE_HOOK(0x6FD05E, TechnoClass_Rearm_Delay_BurstDelays, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);

	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	int burstDelay = -1;

	if (pWeaponExt->Burst_Delays.size() >= (unsigned)pThis->CurrentBurstIndex)
	{
		burstDelay = pWeaponExt->Burst_Delays[pThis->CurrentBurstIndex - 1];
	}
	else if (pWeaponExt->Burst_Delays.size() > 0)
	{
		burstDelay = pWeaponExt->Burst_Delays[pWeaponExt->Burst_Delays.size() - 1];
	}

	if (burstDelay >= 0)
	{
		R->EAX(burstDelay);
		return 0x6FD099;
	}

	return 0;
}

DEFINE_HOOK(0x6F3B37, TechnoClass_Transform_6F3AD0_BurstFLH_1, 0x7)
{
	GET(TechnoClass*, pThis, EBX);
	GET_STACK(int, weaponIndex, STACK_OFFS(0xD8, -0x8));

	bool* FLHFound;
	CoordStruct FLH = TechnoExt::GetBurstFLH(pThis, weaponIndex, FLHFound);

	if (*FLHFound)
	{
		R->ECX(FLH.X);
		R->EBP(FLH.Y);
		R->EAX(FLH.Z);
	}

	return 0;
}

DEFINE_HOOK(0x6F3C88, TechnoClass_Transform_6F3AD0_BurstFLH_2, 0x6)
{
	GET(TechnoClass*, pThis, EBX);
	GET_STACK(int, weaponIndex, STACK_OFFS(0xD8, -0x8));

	bool* FLHFound;
	TechnoExt::GetBurstFLH(pThis, weaponIndex, FLHFound);

	if (*FLHFound)
		R->EAX(0);

	return 0;
}