#include <Helpers/Macro.h>
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <HouseClass.h>
#include "../../Utilities/CanTargetFlags.h"
#include "Body.h"

bool CanUpgrade(BuildingClass* building, BuildingTypeClass* upgrade, HouseClass* upgradeOwner) {
	auto extUpgrade = BuildingTypeExt::ExtMap.Find(upgrade);

	if (CanTargetHouse(extUpgrade->PowersUp_Owner, upgradeOwner, building->Owner)) {
	for (int i = 0; i < extUpgrade->PowersUp_Buildings.Count; i++) {
			if (strcmp(building->Type->ID, extUpgrade->PowersUp_Buildings.GetItem(i)) == 0) {
				return true;
			}
		}
	}
	return false;
}

DEFINE_HOOK(452678, CanUpgrade_UpgradeAlliedBuildings, 8)
{
	GET(BuildingClass*, pBuilding, ECX);
	GET_STACK(BuildingTypeClass*, pUpgrade, 0x0C);
	GET(HouseClass*, upgradeOwner, EAX);

	if (CanUpgrade(pBuilding, pUpgrade, upgradeOwner)) {
		R->EAX(pBuilding->Type->PowersUpToLevel);
		return 0x4526A7;  // continue
	}

	return 0x4526B5;  // fail
}

DEFINE_HOOK(4408EB, Unlimbo_UpgradeAlliedBuildings, A)
{
	GET(BuildingClass*, buildingUnderMouse, EDI);
	GET(BuildingClass*, pUpgrade, ESI);
	HouseClass* upgradeOwner = pUpgrade->Owner;

	CanTargetFlags flags = BuildingTypeExt::ExtMap.Find(pUpgrade->Type)->PowersUp_Owner;

	if (CanUpgrade(buildingUnderMouse, pUpgrade->Type, pUpgrade->Owner)) {
		R->EBX(pUpgrade->Type);
		pUpgrade->Owner = buildingUnderMouse->Owner;
		return 0x440912; // continue
	}

	return 0x440926; // fail
}
