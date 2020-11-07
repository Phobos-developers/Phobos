#include <Helpers/Macro.h>
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <HouseClass.h>
#include "Utilities/CanTargetFlags.h"
#include "Ext/BuildingType/Body.h"

DEFINE_HOOK(452678, CanUpgrade_UpgradeAlliedBuildings, 8)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(BuildingTypeClass*, upgrade, 0x0C);
	GET(HouseClass*, upgradeOwner, EAX);

	CanTargetFlags flags = BuildingTypeExt::ExtMap.Find(upgrade)->canTargetFlags;

	if (!CanTargetHouse(flags, upgradeOwner, pThis->Owner))
		return 0x4526B5; // fail
	
	return 0x452680; // continue
}

DEFINE_HOOK(4408EB, Unlimbo_UpgradeAlliedBuildings, A)
{
	GET(BuildingClass*, pThis, ESI);
	GET(BuildingClass*, buildingUnderMouse, EDI);

	CanTargetFlags flags = BuildingTypeExt::ExtMap.Find(pThis->Type)->canTargetFlags;

	R->EBX(pThis->Type);

	if (!CanTargetHouse(flags, pThis->Owner, buildingUnderMouse->Owner))
		return 0x440926; // fail
	
	pThis->Owner = buildingUnderMouse->Owner;
	return 0x4408F5; //continue
}
