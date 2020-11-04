#include <Helpers/Macro.h>
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <HouseClass.h>

DEFINE_HOOK(452678, CanUpgrade_UpgradeAlliedBuildings, 8)
{
	GET(BuildingClass*, pThis, ECX);
	GET(BuildingTypeClass*, upgrade, EDI);
	GET(HouseClass*, upgradeOwner, EAX);

	if (upgradeOwner != pThis->Owner)
		return 0x4526B5; // can't upgrade
	
	return 0x452680; // continue checking
}

DEFINE_HOOK(4408EB, Unlimbo_UpgradeAlliedBuildings, A)
{
	GET(BuildingClass*, pThis, ESI);
	GET(BuildingClass*, buildingUnderMouse, EDI);

	R->EBX(pThis->Type);
	//pThis->Owner = buildingUnderMouse->Owner;

	if (pThis->Owner != buildingUnderMouse->Owner)
		return 0x440926;
	
	return 0x4408F5;
}
