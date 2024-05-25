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

int CheckBuildLimit(HouseClass const* const pHouse, BuildingTypeClass const* const pItem, bool const includeQueued)
{
	enum { NotReached = 1, ReachedPermanently = -1, ReachedTemporarily = 0 };

	int BuildLimit = pItem->BuildLimit;
	int Remaining = BuildLimitRemaining(pHouse, pItem);

	if (BuildLimit >= 0 && Remaining <= 0)
		return (includeQueued && FactoryClass::FindByOwnerAndProduct(pHouse, pItem)) ? NotReached : ReachedPermanently;

	return Remaining > 0 ? NotReached : ReachedTemporarily;

}

inline int QueuedNum(const HouseClass* pHouse, const TechnoTypeClass* pType)
{
	const AbstractType absType = pType->WhatAmI();
	const FactoryClass* pFactory = pHouse->GetPrimaryFactory(absType, pType->Naval, BuildCat::DontCare);
	int queued = 0;

	if (pFactory)
	{
		queued = pFactory->CountTotal(pType);

		if (const auto pObject = pFactory->Object)
		{
			if (pObject->GetType() == pType)
				--queued;
		}
	}

	return queued;
}

inline void RemoveProduction(const HouseClass* pHouse, const TechnoTypeClass* pType, int num)
{
	const AbstractType absType = pType->WhatAmI();
	FactoryClass* pFactory = pHouse->GetPrimaryFactory(absType, pType->Naval, BuildCat::DontCare);
	if (pFactory)
	{
		int queued = pFactory->CountTotal(pType);
		if (num >= 0)
			queued = Math::min(num, queued);

		for (int i = 0; i < queued; i ++)
		{
			pFactory->RemoveOneFromQueue(pType);
		}
	}
}

inline bool ReachedBuildLimit(const HouseClass* pHouse, const TechnoTypeClass* pType, bool ignoreQueued)
{
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->BuildLimitGroup_Types.empty() || pTypeExt->BuildLimitGroup_Nums.empty())
		return false;

	std::vector<int> limits = pTypeExt->BuildLimitGroup_Nums;

	if (!pTypeExt->BuildLimitGroup_ExtraLimit_Types.empty() && !pTypeExt->BuildLimitGroup_ExtraLimit_Nums.empty())
	{
		for (size_t i = 0; i < pTypeExt->BuildLimitGroup_ExtraLimit_Types.size(); i ++)
		{
			int count = pHouse->CountOwnedNow(pTypeExt->BuildLimitGroup_ExtraLimit_Types[i]);

			if (i < pTypeExt->BuildLimitGroup_ExtraLimit_MaxCount.size() && pTypeExt->BuildLimitGroup_ExtraLimit_MaxCount[i] > 0)
				count = Math::min(count, pTypeExt->BuildLimitGroup_ExtraLimit_MaxCount[i]);

			for (auto& limit : limits)
			{
				if (i < pTypeExt->BuildLimitGroup_ExtraLimit_Nums.size() && pTypeExt->BuildLimitGroup_ExtraLimit_Nums[i] > 0)
					limit += count * pTypeExt->BuildLimitGroup_ExtraLimit_Nums[i];
			}
		}
	}

	if (limits.size() == 1)
	{
		int count = 0;
		int queued = 0;
		bool inside = false;

		for (const TechnoTypeClass* pTmpType : pTypeExt->BuildLimitGroup_Types)
		{
			const auto pTmpTypeExt = TechnoTypeExt::ExtMap.Find(pTmpType);

			if (!ignoreQueued)
				queued += QueuedNum(pHouse, pTmpType) * pTmpTypeExt->BuildLimitGroup_Factor;

			count += pHouse->CountOwnedNow(pTmpType) * pTmpTypeExt->BuildLimitGroup_Factor;

			if (pTmpType == pType)
				inside = true;
		}

		int num = count - limits.back();

		if (num + queued >= 1 - pTypeExt->BuildLimitGroup_Factor)
		{
			if (inside)
				RemoveProduction(pHouse, pType, (num + queued + pTypeExt->BuildLimitGroup_Factor - 1) / pTypeExt->BuildLimitGroup_Factor);
			else if (num >= 1 - pTypeExt->BuildLimitGroup_Factor || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
				RemoveProduction(pHouse, pType, -1);

			return true;
		}
	}
	else
	{
		size_t size = Math::min(limits.size(), pTypeExt->BuildLimitGroup_Types.size());
		bool reached = true;
		bool realReached = true;

		for (size_t i = 0; i < size; i ++)
		{
			const TechnoTypeClass* pTmpType = pTypeExt->BuildLimitGroup_Types[i];
			const auto pTmpTypeExt = TechnoTypeExt::ExtMap.Find(pTmpType);
			int queued = ignoreQueued ? 0 : QueuedNum(pHouse, pTmpType) * pTmpTypeExt->BuildLimitGroup_Factor;
			int num = pHouse->CountOwnedNow(pTmpType) * pTmpTypeExt->BuildLimitGroup_Factor - limits[i];

			if (pType == pTmpType && num + queued >= 1 - pTypeExt->BuildLimitGroup_Factor)
			{
				if (pTypeExt->BuildLimitGroup_ContentIfAnyMatch)
				{
					if (num >= 1 - pTypeExt->BuildLimitGroup_Factor || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
						RemoveProduction(pHouse, pType, (num + queued + pTypeExt->BuildLimitGroup_Factor - 1) / pTypeExt->BuildLimitGroup_Factor);

					return true;
				}
				else if (num < 1 - pTypeExt->BuildLimitGroup_Factor)
				{
					realReached = false;
				}
			}
			else if (pType != pTmpType && num + queued >= 0)
			{
				if (pTypeExt->BuildLimitGroup_ContentIfAnyMatch)
				{
					if (num >= 0 || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
						RemoveProduction(pHouse, pType, -1);

					return true;
				}
				else if (num < 0)
				{
					realReached = false;
				}
			}
			else
			{
				reached = false;
			}
		}

		if (reached)
		{
			if (realReached || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
				RemoveProduction(pHouse, pType, -1);

			return true;
		}
	}

	return false;
}

DEFINE_HOOK(0x4F8361, HouseClass_CanBuild_UpgradesInteraction, 0x5)
{
	GET(HouseClass const* const, pThis, ECX);
	GET_STACK(TechnoTypeClass const* const, pItem, 0x4);
	GET_STACK(bool, buildLimitOnly, 0x8);
	GET_STACK(bool const, includeInProduction, 0xC);
	GET(CanBuildResult const, resultOfAres, EAX);

	if (auto const pBuilding = abstract_cast<BuildingTypeClass const* const>(pItem))
	{
		if (auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pBuilding))
		{
			if (pBuildingExt->PowersUp_Buildings.size() > 0 && resultOfAres == CanBuildResult::Buildable)
				R->EAX(CheckBuildLimit(pThis, pBuilding, includeInProduction));
		}
	}

	if (resultOfAres == CanBuildResult::Buildable)
		R->EAX(HouseExt::BuildLimitGroupCheck(pThis, pItem, buildLimitOnly, includeInProduction));
	else
		return 0;

	if (ReachedBuildLimit(pThis, pItem, true))
		R->EAX(CanBuildResult::TemporarilyUnbuildable);

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

DEFINE_HOOK(0x50B669, HouseClass_ShouldDisableCameo, 0x5)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(TechnoTypeClass*, pType, 0x4);
	GET(bool, aresDisable, EAX);

	if (aresDisable || !pType)
		return 0;

	if (ReachedBuildLimit(pThis, pType, false))
		R->EAX(true);

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
