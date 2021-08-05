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

DEFINE_HOOK(0x452678, CanUpgrade_UpgradeBuildings, 0x8)
{
	GET(BuildingClass*, pBuilding, ECX);
	GET_STACK(BuildingTypeClass*, pUpgrade, 0x0C);
	GET(HouseClass*, upgradeOwner, EAX);

	if (BuildingTypeExt::CanUpgrade(pBuilding, pUpgrade, upgradeOwner))
	{
		R->EAX(pBuilding->Type->PowersUpToLevel);
		return 0x4526A7;  // continue
	}

	return 0x4526B5;  // fail
}

DEFINE_HOOK(0x4408EB, Unlimbo_UpgradeBuildings, 0xA)
{
	GET(BuildingClass*, buildingUnderMouse, EDI);
	GET(BuildingClass*, pUpgrade, ESI);

	if (BuildingTypeExt::CanUpgrade(buildingUnderMouse, pUpgrade->Type, pUpgrade->Owner))
	{
		R->EBX(pUpgrade->Type);
		pUpgrade->Owner = buildingUnderMouse->Owner;

		return 0x440912; // continue
	}

	return 0x440926; // fail
}

#pragma region UpgradesInteraction

int CountOwnedNowTotal(
	HouseClass const* const pHouse, TechnoTypeClass const* const pItem)
{
	int index = -1;
	int sum = 0;
	const BuildingTypeClass* pBType = nullptr;
	const char* pPowersUp = nullptr;

	auto checkUpgrade = [pHouse, pBType, &sum](BuildingTypeClass* pTPowersUp)
	{
		for (auto const& pBld : pHouse->Buildings)
		{
			if (pBld->Type == pTPowersUp)
			{
				for (auto const& pUpgrade : pBld->Upgrades)
				{
					if (pUpgrade == pBType)
					{
						++sum;
					}
				}
			}
		}
	};

	switch (pItem->WhatAmI())
	{
	case AbstractType::BuildingType:
		pBType = static_cast<BuildingTypeClass const*>(pItem);
		pPowersUp = pBType->PowersUpBuilding;
		if (pPowersUp[0])
		{
			if (auto const pTPowersUp = BuildingTypeClass::Find(pPowersUp))
				checkUpgrade(pTPowersUp);
		}

		if (auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pBType))
		{
			for (auto pTPowersUp : pBuildingExt->PowersUp_Buildings)
				checkUpgrade(pTPowersUp);
		}

		break;
	default:
		__assume(0);
	}

	return sum;
}

int BuildLimitRemaining(
	HouseClass const* const pHouse, TechnoTypeClass const* const pItem)
{
	auto const BuildLimit = pItem->BuildLimit;
	if (BuildLimit >= 0)
	{
		return BuildLimit - CountOwnedNowTotal(pHouse, pItem);
	}
	else
	{
		return -BuildLimit - pHouse->CountOwnedEver(pItem);
	}
}

int CheckBuildLimit(
	HouseClass const* const pHouse, TechnoTypeClass const* const pItem,
	bool const includeQueued)
{
	enum { NotReached = 1, ReachedPermanently = -1, ReachedTemporarily = 0 };

	auto const pData = TechnoTypeExt::ExtMap.Find(pItem);
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

	if (auto pBuilding = abstract_cast<BuildingTypeClass*>(pItem))
		if (auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pBuilding))
			if (pBuildingExt->PowersUp_Buildings.size() > 0)
				if (resultOfAres == CanBuildResult::Buildable)
					R->EAX(CheckBuildLimit(pThis, pItem, includeInProduction));

	return 0;
}

#pragma endregion