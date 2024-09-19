#include "Body.h"

#include <Ext/Aircraft/Body.h>
#include <Ext/Scenario/Body.h>
#include "Ext/Techno/Body.h"
#include "Ext/Building/Body.h"
#include <unordered_map>

DEFINE_HOOK(0x508C30, HouseClass_UpdatePower_UpdateCounter, 0x5)
{
	GET(HouseClass*, pThis, ECX);
	auto pHouseExt = HouseExt::ExtMap.Find(pThis);

	pHouseExt->PowerPlantEnhancers.clear();

	// This pre-iterating ensure our process to be done in O(NM) instead of O(N^2),
	// as M should be much less than N, this will be a great improvement. - secsome
	for (auto& pBld : pThis->Buildings)
	{
		if (TechnoExt::IsActive(pBld) && pBld->IsOnMap && pBld->HasPower)
		{
			const auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

			if (pExt->PowerPlantEnhancer_Buildings.size() &&
				(pExt->PowerPlantEnhancer_Amount != 0 || pExt->PowerPlantEnhancer_Factor != 1.0f))
			{
				++pHouseExt->PowerPlantEnhancers[pExt];
			}
		}
	}

	return 0;
}

// Power Plant Enhancer #131
DEFINE_HOOK(0x508CF2, HouseClass_UpdatePower_PowerOutput, 0x7)
{
	GET(HouseClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);

	pThis->PowerOutput += BuildingTypeExt::GetEnhancedPower(pBld, pThis);

	return 0x508D07;
}

DEFINE_HOOK(0x73E474, UnitClass_Unload_Storage, 0x6)
{
	GET(BuildingClass* const, pBuilding, EDI);
	GET(int const, idxTiberium, EBP);
	REF_STACK(float, amount, 0x1C);

	auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

	auto storageTiberiumIndex = RulesExt::Global()->Storage_TiberiumIndex;

	if (pTypeExt->Refinery_UseStorage && storageTiberiumIndex >= 0)
	{
		BuildingExt::StoreTiberium(pBuilding, amount, idxTiberium, storageTiberiumIndex);
		amount = 0.0f;
	}

	return 0;
}

namespace RecalcCenterTemp
{
	HouseExt::ExtData* pExtData;
}

DEFINE_HOOK(0x4FD166, HouseClass_RecalcCenter_SetContext, 0x5)
{
	GET(HouseClass* const, pThis, EDI);

	RecalcCenterTemp::pExtData = HouseExt::ExtMap.Find(pThis);

	return 0;
}

DEFINE_HOOK_AGAIN(0x4FD463, HouseClass_RecalcCenter_LimboDelivery, 0x6)
DEFINE_HOOK(0x4FD1CD, HouseClass_RecalcCenter_LimboDelivery, 0x6)
{
	enum { SkipBuilding1 = 0x4FD23B, SkipBuilding2 = 0x4FD4D5 };

	GET(BuildingClass* const, pBuilding, ESI);

	auto const pExt = RecalcCenterTemp::pExtData;

	if (!MapClass::Instance->CoordinatesLegal(pBuilding->GetMapCoords())
		|| (pExt && pExt->OwnsLimboDeliveredBuilding(pBuilding)))
	{
		return R->Origin() == 0x4FD1CD ? SkipBuilding1 : SkipBuilding2;
	}

	return 0;
}

DEFINE_HOOK(0x4AC534, DisplayClass_ComputeStartPosition_IllegalCoords, 0x6)
{
	enum { SkipTechno = 0x4AC55B };

	GET(TechnoClass* const, pTechno, ECX);

	if (!MapClass::Instance->CoordinatesLegal(pTechno->GetMapCoords()))
		return SkipTechno;

	return 0;
}

#pragma region LimboTracking

// These hooks handle tracking objects that are limboed e.g not physically on the map or engaged in game logic updates.
// The objects are manually updated once after pre-placed objects have been parsed, buildings are ignored as the limboed pre-placed buildings
// are not relevant (walls that will be converted into overlays etc), after which automatic update on limbo/unlimbo and uninit is enabled.

namespace LimboTrackingTemp
{
	bool Enabled = false;
	bool IsBeingDeleted = false;
}

DEFINE_HOOK(0x687B18, ScenarioClass_ReadINI_StartTracking, 0x7)
{
	for (auto const pTechno : *TechnoClass::Array())
	{
		auto const pType = pTechno->GetTechnoType();

		if (!pType->Insignificant && !pType->DontScore && pTechno->WhatAmI() != AbstractType::Building && pTechno->InLimbo)
		{
			auto const pOwnerExt = HouseExt::ExtMap.Find(pTechno->Owner);
			pOwnerExt->AddToLimboTracking(pType);
		}
	}

	LimboTrackingTemp::Enabled = true;

	return 0;
}

void __fastcall TechnoClass_UnInit_Wrapper(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();

	if (LimboTrackingTemp::Enabled && pThis->InLimbo && !pType->Insignificant && !pType->DontScore)
	{
		auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
		pOwnerExt->RemoveFromLimboTracking(pType);
	}

	LimboTrackingTemp::IsBeingDeleted = true;
	pThis->ObjectClass::UnInit();
	LimboTrackingTemp::IsBeingDeleted = false;
}

DEFINE_JUMP(CALL, 0x4DE60B, GET_OFFSET(TechnoClass_UnInit_Wrapper));   // FootClass
DEFINE_JUMP(VTABLE, 0x7E3FB4, GET_OFFSET(TechnoClass_UnInit_Wrapper)); // BuildingClass

DEFINE_HOOK(0x6F6BC9, TechnoClass_Limbo_AddTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();

	if (LimboTrackingTemp::Enabled && !pType->Insignificant && !pType->DontScore && !LimboTrackingTemp::IsBeingDeleted)
	{
		auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
		pOwnerExt->AddToLimboTracking(pType);
	}

	return 0;
}

DEFINE_HOOK(0x6F6D85, TechnoClass_Unlimbo_RemoveTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (LimboTrackingTemp::Enabled && !pType->Insignificant && !pType->DontScore && pExt->HasBeenPlacedOnMap)
	{
		auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
		pOwnerExt->RemoveFromLimboTracking(pType);
	}
	else if (!pExt->HasBeenPlacedOnMap)
	{
		pExt->HasBeenPlacedOnMap = true;

		if (pExt->TypeExtData->AutoDeath_Behavior.isset())
			ScenarioExt::Global()->AutoDeathObjects.push_back(pExt);
	}

	return 0;
}

DEFINE_HOOK(0x7015C9, TechnoClass_Captured_UpdateTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(HouseClass* const, pNewOwner, EBP);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
	auto const pNewOwnerExt = HouseExt::ExtMap.Find(pNewOwner);

	if (LimboTrackingTemp::Enabled && !pType->Insignificant && !pType->DontScore && pThis->InLimbo)
	{
		pOwnerExt->RemoveFromLimboTracking(pType);
		pNewOwnerExt->AddToLimboTracking(pType);
	}

	if (auto pMe = generic_cast<FootClass*>(pThis))
	{
		bool I_am_human = pThis->Owner->IsControlledByHuman();
		bool You_are_human = pNewOwner->IsControlledByHuman();
		auto pConvertTo = (I_am_human && !You_are_human) ? pExt->TypeExtData->Convert_HumanToComputer.Get() :
			(!I_am_human && You_are_human) ? pExt->TypeExtData->Convert_ComputerToHuman.Get() : nullptr;

		if (pConvertTo && pConvertTo->WhatAmI() == pType->WhatAmI())
			TechnoExt::ConvertToType(pMe, pConvertTo);

		for (auto& trail : pExt->LaserTrails)
		{
			if (trail.Type->IsHouseColor)
				trail.CurrentColor = pNewOwner->LaserColor;
		}

		if (!I_am_human && You_are_human)
			TechnoExt::ChangeOwnerMissionFix(pMe);
	}

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x65EB8D, HouseClass_SendSpyPlanes_PlaceAircraft, 0x6)
{
	enum { SkipGameCode = 0x65EBE5, SkipGameCodeNoSuccess = 0x65EC12 };

	GET(AircraftClass* const, pAircraft, ESI);
	GET(CellStruct const, edgeCell, EDI);

	bool result = AircraftExt::PlaceReinforcementAircraft(pAircraft, edgeCell);

	return result ? SkipGameCode : SkipGameCodeNoSuccess;
}

DEFINE_HOOK(0x65E997, HouseClass_SendAirstrike_PlaceAircraft, 0x6)
{
	enum { SkipGameCode = 0x65E9EE, SkipGameCodeNoSuccess = 0x65EA8B };

	GET(AircraftClass* const, pAircraft, ESI);
	GET(CellStruct const, edgeCell, EDI);

	bool result = AircraftExt::PlaceReinforcementAircraft(pAircraft, edgeCell);

	return result ? SkipGameCode : SkipGameCodeNoSuccess;
}

DEFINE_HOOK(0x50B669, HouseClass_ShouldDisableCameo, 0x5)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(TechnoTypeClass*, pType, 0x4);
	GET(bool, aresDisable, EAX);

	if (aresDisable || !pType)
		return 0;

	if (HouseExt::ReachedBuildLimit(pThis, pType, false))
		R->EAX(true);

	return 0;
}

DEFINE_HOOK(0x4FD77C, HouseClass_ExpertAI_Superweapons, 0x5)
{
	enum { SkipSWProcess = 0x4FD7A0 };

	if (RulesExt::Global()->AISuperWeaponDelay.isset())
		return SkipSWProcess;

	return 0;
}

DEFINE_HOOK(0x4F9038, HouseClass_AI_Superweapons, 0x5)
{
	GET(HouseClass*, pThis, ESI);

	if (!RulesExt::Global()->AISuperWeaponDelay.isset() || pThis->IsControlledByHuman() || pThis->Type->MultiplayPassive)
		return 0;

	int delay = RulesExt::Global()->AISuperWeaponDelay.Get();

	if (delay > 0)
	{
		auto const pExt = HouseExt::ExtMap.Find(pThis);

		if (pExt->AISuperWeaponDelayTimer.HasTimeLeft())
			return 0;

		pExt->AISuperWeaponDelayTimer.Start(delay);
	}

	if (!SessionClass::IsCampaign() || pThis->IQLevel2 >= RulesClass::Instance->SuperWeapons)
		pThis->AI_TryFireSW();

	return 0;
}

DEFINE_HOOK_AGAIN(0x4FFA99, HouseClass_ExcludeFromMultipleFactoryBonus, 0x6)
DEFINE_HOOK(0x4FF9C9, HouseClass_ExcludeFromMultipleFactoryBonus, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	if (BuildingTypeExt::ExtMap.Find(pBuilding->Type)->ExcludeFromMultipleFactoryBonus)
	{
		GET(HouseClass*, pThis, EDI);
		GET(bool, isNaval, ECX);

		auto const pExt = HouseExt::ExtMap.Find(pThis);
		pExt->UpdateNonMFBFactoryCounts(pBuilding->Type->Factory, R->Origin() == 0x4FF9C9, isNaval);
	}

	return 0;
}

DEFINE_HOOK(0x500910, HouseClass_GetFactoryCount, 0x5)
{
	enum { SkipGameCode = 0x50095D };

	GET(HouseClass*, pThis, ECX);
	GET_STACK(AbstractType, rtti, 0x4);
	GET_STACK(bool, isNaval, 0x8);

	auto const pExt = HouseExt::ExtMap.Find(pThis);
	R->EAX(pExt->GetFactoryCountWithoutNonMFB(rtti, isNaval));

	return SkipGameCode;
}
