#include "Body.h"
#include <EBolt.h>

namespace BoltTemp
{
	PhobosMap<EBolt*, const WeaponTypeExt::ExtData*> boltWeaponTypeExt;
	const WeaponTypeExt::ExtData* pType = nullptr;
}

DEFINE_HOOK(6FD494, TechnoClass_FireEBolt_SetExtMap_AfterAres, 7)
{
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFS(0x30, -0x8));
	GET(EBolt*, pBolt, EAX);

	if (pWeapon)
		BoltTemp::boltWeaponTypeExt[pBolt] = WeaponTypeExt::ExtMap.FindOrAllocate(pWeapon);

	return 0;
}

DEFINE_HOOK(4C2951, EBolt_DTOR, 5)
{
	GET(EBolt*, pBolt, ECX);

	BoltTemp::boltWeaponTypeExt.erase(pBolt);

	return 0;
}

DEFINE_HOOK(4C24E4, Ebolt_DrawFist_Disable, 8)
{
	GET_STACK(EBolt*, pBolt, 0x40);
	BoltTemp::pType = BoltTemp::boltWeaponTypeExt.get_or_default(pBolt);

	return (BoltTemp::pType && BoltTemp::pType->Bolt_Disable1) ? 0x4C2515 : 0;
}

DEFINE_HOOK(4C25FD, Ebolt_DrawSecond_Disable, A)
{
	return (BoltTemp::pType && BoltTemp::pType->Bolt_Disable2) ? 0x4C262A : 0;
}

DEFINE_HOOK(4C26EE, Ebolt_DrawThird_Disable, 8)
{
	return (BoltTemp::pType && BoltTemp::pType->Bolt_Disable3) ? 0x4C2710 : 0;
}
