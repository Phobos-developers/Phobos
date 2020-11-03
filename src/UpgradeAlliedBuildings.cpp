#include <Helpers/Macro.h>
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <HouseClass.h>

DEFINE_HOOK(452678, UpgradeAlliedBuildings, 8)
{
	GET(BuildingClass*, pThis, ECX);
	GET_BASE(BuildingTypeClass*, upgrade, 0x4);
	GET_BASE(HouseClass*, upgradeOwner, 0x8);

	if (upgradeOwner == pThis->Owner)
		return 0x452680; // continue checking
	
	return 0x4526B5; // can't upgrade
}