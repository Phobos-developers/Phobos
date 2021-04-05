#include "Body.h"
#include <EBolt.h>

PhobosMap<EBolt*, const WeaponTypeExt::ExtData*> BoltExt;
const WeaponTypeExt::ExtData* pThisWeapon = nullptr;

DEFINE_HOOK(6FD494, TechnoClass_FireEBolt_SetExtMap_AfterAres, 7)
{
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFS(0x30, -0x8));
	GET(EBolt*, EB, EAX);

	if (pWeapon) {
		BoltExt[EB] = WeaponTypeExt::ExtMap.FindOrAllocate(pWeapon);
	}

	return 0;
}

DEFINE_HOOK(4C2951, EBolt_DTOR, 5)
{
	GET(EBolt*, Bolt, ECX);
	BoltExt.erase(Bolt);
	return 0;
}

DEFINE_HOOK(4C24E4, Ebolt_DrawFist_Disable, 8)
{
	GET_STACK(EBolt*, Bolt, 0x40);
	pThisWeapon = BoltExt.get_or_default(Bolt);

	return (pThisWeapon && pThisWeapon->Bolt_Disable1) ? 0x4C2515 : 0;
}

DEFINE_HOOK(4C25FD, Ebolt_DrawSecond_Disable, A)
{
	return (pThisWeapon && pThisWeapon->Bolt_Disable2) ? 0x4C262A : 0;
}

DEFINE_HOOK(4C26EE, Ebolt_DrawThird_Disable, 8)
{
	return (pThisWeapon && pThisWeapon->Bolt_Disable3) ? 0x4C2710 : 0;
}
