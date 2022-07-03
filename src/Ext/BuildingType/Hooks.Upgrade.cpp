#include <Helpers/Macro.h>
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <HouseClass.h>
#include <Utilities/EnumFunctions.h>
#include "Body.h"
#include <Ext/TechnoType/Body.h>
#include <FactoryClass.h>

bool BuildingTypeExt::CanUpgrade(BuildingClass* pBuilding, BuildingTypeClass* pUpgradeType, HouseClass* pUpgradeOwner)
{
	auto pUpgradeExt = BuildingTypeExt::ExtMap.Find(pUpgradeType);
	if (pUpgradeExt && EnumFunctions::CanTargetHouse(pUpgradeExt->PowersUp_Owner, pUpgradeOwner, pBuilding->Owner))
	{
		// PowersUpBuilding
		if (_stricmp(pBuilding->Type->ID, pUpgradeType->PowersUpBuilding) == 0)
			return true;

		// PowersUp.Buildings
		for (auto& pPowerUpBuilding : pUpgradeExt->PowersUp_Buildings)
		{
			if (_stricmp(pBuilding->Type->ID, pPowerUpBuilding->ID) == 0)
				return true;
		}
	}

	return false;
}

DEFINE_HOOK(0x452678, BuildingClass_CanUpgrade_UpgradeBuildings, 0x8)
{
	enum { Continue = 0x4526A7, ForbidUpgrade = 0x4526B5 };

	GET(BuildingClass*, pBuilding, ECX);
	GET_STACK(BuildingTypeClass*, pUpgrade, 0xC);
	GET(HouseClass*, pUpgradeOwner, EAX);

	if (BuildingTypeExt::CanUpgrade(pBuilding, pUpgrade, pUpgradeOwner))
	{
		R->EAX(pBuilding->Type->PowersUpToLevel);
		return Continue;
	}

	return ForbidUpgrade;
}

DEFINE_HOOK(0x4408EB, BuildingClass_Unlimbo_UpgradeBuildings, 0xA)
{
	enum { Continue = 0x440912, ForbidUpgrade = 0x440926 };

	GET(BuildingClass*, pBuilding, EDI);
	GET(BuildingClass*, pUpgrade, ESI);

	if (BuildingTypeExt::CanUpgrade(pBuilding, pUpgrade->Type, pUpgrade->Owner))
	{
		R->EBX(pUpgrade->Type);
		pUpgrade->SetOwningHouse(pBuilding->Owner, false);
		return Continue;
	}

	return ForbidUpgrade;
}

#pragma region UpgradesInteraction

int BuildLimitRemaining(HouseClass const* const pHouse, BuildingTypeClass const* const pItem)
{
	auto const BuildLimit = pItem->BuildLimit;

	if (BuildLimit >= 0)
		return BuildLimit - BuildingTypeExt::GetUpgradesAmount(const_cast<BuildingTypeClass*>(pItem), const_cast<HouseClass*>(pHouse));
	else
		return -BuildLimit - pHouse->CountOwnedEver(pItem);
}

int CheckBuildLimit(HouseClass const* const pHouse, BuildingTypeClass const* const pItem, bool const includeQueued)
{
	enum { NotReached = 1, ReachedPermanently = -1, ReachedTemporarily = 0 };

	int BuildLimit = pItem->BuildLimit;
	int Remaining = BuildLimitRemaining(pHouse, pItem);

	if (BuildLimit >= 0 && Remaining <= 0)
		return (includeQueued && FactoryClass::FindByOwnerAndProduct(pHouse, pItem)) ? NotReached : ReachedPermanently;

	return Remaining > 0 ? NotReached : ReachedTemporarily;

}

DEFINE_HOOK(0x4F8361, HouseClass_CanBuild_UpgradesInteraction, 0x3)
{
	GET(HouseClass const* const, pThis, ECX);
	GET_STACK(TechnoTypeClass const* const, pItem, 0x4);
	GET_STACK(bool const, includeInProduction, 0xC);
	GET(CanBuildResult const, resultOfAres, EAX);

	if (auto pBuilding = static_cast<BuildingTypeClass const*>(pItem))
	{
		if (auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pBuilding))
		{
			if (pBuildingExt->PowersUp_Buildings.size() > 0 && resultOfAres == CanBuildResult::Buildable)
				R->EAX(CheckBuildLimit(pThis, pBuilding, includeInProduction));
		}
	}

	return 0;
}

#pragma endregion
