#include <Utilities/Macro.h>
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <HouseClass.h>
#include <Utilities/EnumFunctions.h>
#include "Body.h"
#include <Ext/TechnoType/Body.h>
#include <FactoryClass.h>
#include <Ext/House/Body.h>

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

CanBuildResult CheckBuildLimit(HouseClass const* const pHouse, BuildingTypeClass const* const pItem, bool const includeQueued)
{
	int BuildLimit = pItem->BuildLimit;
	int Remaining = BuildLimitRemaining(pHouse, pItem);

	if (BuildLimit >= 0 && Remaining <= 0)
		return (includeQueued && FactoryClass::FindByOwnerAndProduct(pHouse, pItem)) ? CanBuildResult::Buildable : CanBuildResult::Unbuildable;

	return Remaining > 0 ? CanBuildResult::Buildable : CanBuildResult::TemporarilyUnbuildable;

}

DEFINE_HOOK(0x4F8361, HouseClass_CanBuild_UpgradesInteraction, 0x5)
{
	GET(HouseClass const* const, pThis, ECX);
	GET_STACK(TechnoTypeClass const* const, pItem, 0x4);
	GET_STACK(bool, buildLimitOnly, 0x8);
	GET_STACK(bool const, includeInProduction, 0xC);
	GET(CanBuildResult const, resultOfAres, EAX);

	CanBuildResult canBuild = resultOfAres;

	if (canBuild == CanBuildResult::Buildable)
	{
		if (auto const pBuilding = abstract_cast<BuildingTypeClass const* const>(pItem))
		{
			if (auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pBuilding))
			{
				if (pBuildingExt->PowersUp_Buildings.size() > 0)
					canBuild = CheckBuildLimit(pThis, pBuilding, includeInProduction);
			}
		}
	}

	if (canBuild == CanBuildResult::Buildable)
	{
		canBuild = HouseExt::BuildLimitGroupCheck(pThis, pItem, buildLimitOnly, includeInProduction);

		if (HouseExt::ReachedBuildLimit(pThis, pItem, true))
			canBuild = CanBuildResult::TemporarilyUnbuildable;
	}

	if (!buildLimitOnly && includeInProduction && pThis->IsControlledByHuman()) // Eliminate any non-producible calls to change the list safely
		canBuild = BuildingTypeExt::CheckAlwaysExistCameo(pThis, pItem, canBuild);

	R->EAX(canBuild);
	return 0;
}

DEFINE_HOOK(0x4F7877, HouseClass_CanBuild_UpgradesInteraction_WithoutAres, 0x5)
{
	Debug::Log("Hook [HouseClass_CanBuild_UpgradesInteraction] disabled\n");

	Patch::Apply_RAW(0x4F8361, // Disable hook HouseClass_CanBuild_UpgradesInteraction
		{ 0xC2, 0x0C, 0x00, 0x6E, 0x7D }
	);

	Patch::Apply_RAW(0x4F7877, // Disable this hook
		{ 0x53, 0x55, 0x8B, 0xE9, 0x56 }
	);

	return 0;
}

#pragma endregion

#pragma region UpgradeAnimLogic

// Parse Powered(Light|Effect|Special) keys for upgrade anims.
DEFINE_HOOK(0x4648B3, BuildingTypeClass_ReadINI_PowerUpAnims, 0x5)
{
	GET(BuildingTypeClass*, pThis, EBP);
	GET(int, index, EBX);

	auto const pINI = &CCINIClass::INI_Art();
	auto const animData = &pThis->BuildingAnim[index - 1];

	char buffer[0x20];

	sprintf_s(buffer, "PowerUp%01dPowered", index);
	animData->Powered = pINI->ReadBool(pThis->ImageFile, buffer, animData->Powered);

	sprintf_s(buffer, "PowerUp%01dPoweredLight", index);
	animData->PoweredLight = pINI->ReadBool(pThis->ImageFile, buffer, animData->PoweredLight);

	sprintf_s(buffer, "PowerUp%01dPoweredEffect", index);
	animData->PoweredEffect = pINI->ReadBool(pThis->ImageFile, buffer, animData->PoweredEffect);

	sprintf_s(buffer, "PowerUp%01dPoweredSpecial", index);
	animData->PoweredSpecial = pINI->ReadBool(pThis->ImageFile, buffer, animData->PoweredSpecial);

	return 0;
}

// Don't allow upgrade anims to be created if building is not upgraded or they require power to be shown and the building isn't powered.
static __forceinline bool AllowUpgradeAnim(BuildingClass* pBuilding, BuildingAnimSlot anim)
{
	auto const pType = pBuilding->Type;

	if (pType->Upgrades != 0 && anim >= BuildingAnimSlot::Upgrade1 && anim <= BuildingAnimSlot::Upgrade3 && !pBuilding->Anims[int(anim)])
	{
		int upgradeLevel = pBuilding->UpgradeLevel - 1;

		if (upgradeLevel < 0 || (int)anim != upgradeLevel)
			return false;

		auto const animData = pType->BuildingAnim[int(anim)];

		if (((pType->Powered && pType->PowerDrain > 0 && (animData.PoweredLight || animData.PoweredEffect)) ||
			(pType->PoweredSpecial && animData.PoweredSpecial)) &&
			!(pBuilding->CurrentMission != Mission::Construction && pBuilding->CurrentMission != Mission::Selling && pBuilding->IsPowerOnline()))
		{
			return false;
		}
	}

	return true;
}

DEFINE_HOOK(0x45189D, BuildingClass_AnimUpdate_Upgrades, 0x6)
{
	enum { SkipAnim = 0x451B2C };

	GET(BuildingClass*, pThis, ESI);
	GET_STACK(BuildingAnimSlot, anim, STACK_OFFSET(0x34, 0x8));

	if (!AllowUpgradeAnim(pThis, anim))
		return SkipAnim;

	return 0;
}

#pragma endregion
