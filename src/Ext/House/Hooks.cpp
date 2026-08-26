#include "Body.h"

#include <Ext/Aircraft/Body.h>
#include <Ext/Scenario/Body.h>
#include "Ext/Techno/Body.h"
#include "Ext/Building/Body.h"
#include <Ext/Event/Body.h>
#include <Ext/Tiberium/Body.h>
#include <New/Type/ResourceTypeClass.h>
#include <Misc/FlyingStrings.h>

#include <BeaconManagerClass.h>

#include <unordered_map>
#include <algorithm>
#include <utility>

// Trigger power recalculation on gain/loss of any techno, not just buildings.
DEFINE_HOOK_AGAIN(0x5025F0, HouseClass_RegisterGain, 0x5) // RegisterLoss
DEFINE_HOOK(0x502A80, HouseClass_RegisterGain, 0x8)
{
	if (!Phobos::Config::UnitPowerDrain)
		return 0;

	GET(HouseClass*, pThis, ECX);

	pThis->RecheckPower = true;

	return 0;
}

DEFINE_HOOK(0x508D8D, HouseClass_UpdatePower_AfterBuildings, 0x6)
{
	GET(HouseClass*, pThis, ESI);

	if (Phobos::Config::UnitPowerDrain)
	{
		auto updateDrainForThisType = [pThis](const TechnoTypeClass* pType)
			{
				const int count = pThis->CountOwnedAndPresent(pType);
				if (count == 0)
					return;
				const auto pExt = TechnoTypeExt::Fetch(pType);
				if (pExt->Power > 0)
					pThis->PowerOutput += pExt->Power * count;
				else
					pThis->PowerDrain -= pExt->Power * count;
			};

		for (const auto pType : InfantryTypeClass::Array)
			updateDrainForThisType(pType);
		for (const auto pType : UnitTypeClass::Array)
			updateDrainForThisType(pType);
		for (const auto pType : AircraftTypeClass::Array)
			updateDrainForThisType(pType);
		// Don't do this for buildings, they've already been counted.
	}

	HouseExt::CalculatePowerSurplus(pThis);

	return 0;
}

DEFINE_HOOK(0x73E474, UnitClass_Unload_Storage, 0x6)
{
	GET(BuildingClass* const, pBuilding, EDI);
	GET(int const, idxTiberium, EBP);
	REF_STACK(float, amount, 0x1C);

	auto const pTypeExt = BuildingTypeExt::Fetch(pBuilding->Type);

	auto const storageTiberiumIndex = RulesExt::Global()->Storage_TiberiumIndex;

	if (idxTiberium >= 0 && idxTiberium < TiberiumClass::Array.Count)
	{
		const auto pTiberium = TiberiumClass::Array.GetItem(idxTiberium);
		if (const auto pTibExt = TiberiumExt::TryFetch(pTiberium))
		{
			if (pTibExt->ResourceType.isset() && pTibExt->ResourceType >= 0)
			{
				const int resIdx = pTibExt->ResourceType.Get();
				const bool hasCustomResourceValue = pTibExt->ResourceValue.isset();
				const int unitVal = hasCustomResourceValue ? pTibExt->ResourceValue.Get() : pTiberium->Value;
				const float incomingPoints = amount * static_cast<float>(unitVal);

				if (incomingPoints > 0.0f && pBuilding && pBuilding->Owner)
				{
					if (const auto pHouseExt = HouseExt::TryFetch(pBuilding->Owner))
					{
						if (pHouseExt->IsResourceEnabled(resIdx))
						{
							if (const auto pBldExt = BuildingExt::TryFetch(pBuilding))
							{
								if (resIdx >= static_cast<int>(pBldExt->AccumulatedResources.size()))
									pBldExt->AccumulatedResources.resize(resIdx + 1, 0.0f);

								const int prevWhole = static_cast<int>(pBldExt->AccumulatedResources[resIdx]);
								pBldExt->AccumulatedResources[resIdx] += incomingPoints;
								const int newWhole = static_cast<int>(pBldExt->AccumulatedResources[resIdx]);
								const int pointsToGrant = newWhole - prevWhole;

								if (pointsToGrant > 0)
								{
									pHouseExt->UpdateResourceAmount(resIdx, pointsToGrant);
								}
							}
						}
					}
				}

				if (!hasCustomResourceValue)
				{
					amount = 0.0f; // Value was used for the custom resource, suppress money
				}
			}
		}
	}

	if (pTypeExt->Refinery_UseStorage && storageTiberiumIndex >= 0 && amount > 0.0f)
	{
		BuildingExt::StoreTiberium(pBuilding, amount, idxTiberium, storageTiberiumIndex);
		amount = 0.0f;
	}

	return 0;
}

namespace RecalcCenterTemp
{
	HouseExt* pExtData;
}

DEFINE_HOOK(0x4FD166, HouseClass_RecalcCenter_SetContext, 0x5)
{
	GET(HouseClass* const, pThis, EDI);

	RecalcCenterTemp::pExtData = HouseExt::Fetch(pThis);

	return 0;
}

DEFINE_HOOK_AGAIN(0x4FD463, HouseClass_RecalcCenter_LimboDelivery, 0x6)
DEFINE_HOOK(0x4FD1CD, HouseClass_RecalcCenter_LimboDelivery, 0x6)
{
	enum { SkipBuilding1 = 0x4FD23B, SkipBuilding2 = 0x4FD4D5 };

	GET(BuildingClass* const, pBuilding, ESI);

	if (!MapClass::Instance.CoordinatesLegal(pBuilding->GetMapCoords())
		|| (RecalcCenterTemp::pExtData && RecalcCenterTemp::pExtData->OwnsLimboDeliveredBuilding(pBuilding))
		|| TechnoTypeExt::Fetch(pBuilding->Type)->IgnoreForBaseCenter)
	{
		return R->Origin() == 0x4FD1CD ? SkipBuilding1 : SkipBuilding2;
	}

	return 0;
}

DEFINE_HOOK(0x4AC534, DisplayClass_ComputeStartPosition_IllegalCoords, 0x6)
{
	enum { SkipTechno = 0x4AC55B };

	GET(TechnoClass* const, pTechno, ECX);

	if (!MapClass::Instance.CoordinatesLegal(pTechno->GetMapCoords()) || TechnoExt::Fetch(pTechno)->TypeExtData->IgnoreForBaseCenter)
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
	int IsBeingDeleted = 0;
}

DEFINE_HOOK(0x687B18, ScenarioClass_ReadINI_StartTracking, 0x7)
{
	for (auto const pTechno : TechnoClass::Array)
	{
		auto const pType = pTechno->GetTechnoType();

		if (!pType->Insignificant && !pType->DontScore && pTechno->WhatAmI() != AbstractType::Building && pTechno->InLimbo)
		{
			auto const pOwnerExt = HouseExt::Fetch(pTechno->Owner);
			pOwnerExt->AddToLimboTracking(pType);
		}
	}

	LimboTrackingTemp::Enabled = true;

	return 0;
}

static void __fastcall TechnoClass_UnInit_Wrapper(TechnoClass* pThis)
{

	if (LimboTrackingTemp::Enabled && pThis->InLimbo)
	{
		auto const pType = pThis->GetTechnoType();

		if (!pType->Insignificant && !pType->DontScore)
			HouseExt::Fetch(pThis->Owner)->RemoveFromLimboTracking(pType);
	}

	++LimboTrackingTemp::IsBeingDeleted;
	pThis->ObjectClass::UnInit();
	--LimboTrackingTemp::IsBeingDeleted;
}

DEFINE_FUNCTION_JUMP(CALL, 0x4DE60B, TechnoClass_UnInit_Wrapper);   // FootClass
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3FB4, TechnoClass_UnInit_Wrapper); // BuildingClass

DEFINE_HOOK(0x6F6BC9, TechnoClass_Limbo_AddTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();

	if (LimboTrackingTemp::Enabled && !pType->Insignificant && !pType->DontScore && !LimboTrackingTemp::IsBeingDeleted)
	{
		auto const pOwnerExt = HouseExt::Fetch(pThis->Owner);
		pOwnerExt->AddToLimboTracking(pType);
	}

	return 0;
}

DEFINE_HOOK(0x6F6D85, TechnoClass_Unlimbo_RemoveTracking, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoExt::Fetch(pThis);

	if (LimboTrackingTemp::Enabled && !pType->Insignificant && !pType->DontScore && pExt->HasBeenPlacedOnMap)
	{
		auto const pOwnerExt = HouseExt::Fetch(pThis->Owner);
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

	auto const pExt = TechnoExt::Fetch(pThis);
	auto const pTypeExt = pExt->TypeExtData;

	if (pTypeExt->AutoDeath_Behavior.isset())
	{
		const auto pFoot = generic_cast<FootClass*>(pThis);
		const bool IgnoreRevertOnExit = pFoot ? FootExt::Fetch(pFoot)->IsOwnerChangeFromRevertOnExit : false;
		const bool humanToComputer = pTypeExt->AutoDeath_OnOwnerChange_HumanToComputer.Get(pTypeExt->AutoDeath_OnOwnerChange);
		const bool computerToHuman = pTypeExt->AutoDeath_OnOwnerChange_ComputerToHuman.Get(pTypeExt->AutoDeath_OnOwnerChange);

		if (pTypeExt->AutoDeath_OnOwnerChange_IgnoreRevertOnExit.Get(RulesExt::Global()->AutoDeath_OnOwnerChange_IgnoreRevertOnExit) && IgnoreRevertOnExit)
			pExt->ShouldBeDead = false;
		else if (humanToComputer && computerToHuman)
		{
			pExt->ShouldBeDead = true;
		}
		else if (humanToComputer || computerToHuman)
		{
			const bool I_am_human = pThis->Owner->IsControlledByHuman();

			if (I_am_human != pNewOwner->IsControlledByHuman())
			{
				if ((I_am_human && humanToComputer) || (!I_am_human && computerToHuman))
					pExt->ShouldBeDead = true;
			}
		}
		if (pExt->ShouldBeDead && pThis->Transporter
		&& !IgnoreRevertOnExit
		&& !pTypeExt->AutoDeath_AllowLimboed.Get(RulesExt::Global()->AutoDeath_AllowLimboed))
			pExt->ShouldBeDead = false;
	}

	auto const pType = pTypeExt->OwnerObject();
	auto const pOwnerExt = HouseExt::Fetch(pThis->Owner);
	auto const pNewOwnerExt = HouseExt::Fetch(pNewOwner);

	if (LimboTrackingTemp::Enabled && !pType->Insignificant && !pType->DontScore && pThis->InLimbo)
	{
		pOwnerExt->RemoveFromLimboTracking(pType);
		pNewOwnerExt->AddToLimboTracking(pType);
	}

	if (pTypeExt->Harvester_Counted)
	{
		auto& vec = pOwnerExt->OwnedCountedHarvesters;
		vec.erase(std::remove(vec.begin(), vec.end(), pThis), vec.end());

		pNewOwnerExt->OwnedCountedHarvesters.push_back(pThis);
	}

	if (const auto pMe = generic_cast<FootClass*, true>(pThis))
	{
		const bool I_am_human = pThis->Owner->IsControlledByHuman();

		if (I_am_human != pNewOwner->IsControlledByHuman())
		{
			if (const auto pConvertTo = I_am_human
				? pTypeExt->Convert_HumanToComputer.Get()
				: pTypeExt->Convert_ComputerToHuman.Get())
			{
				if (pConvertTo->WhatAmI() == pType->WhatAmI())
					TechnoExt::ConvertToType(pMe, pConvertTo);
			}

			if (!I_am_human)
				TechnoExt::ChangeOwnerMissionFix(pMe);
		}

		pThis->Owner->RecheckTechTree = true;
		pNewOwner->RecheckTechTree = true;
	}

	for (const auto& pTrail : pExt->LaserTrails)
	{
		if (pTrail->Type->IsHouseColor)
			pTrail->CurrentColor = pNewOwner->LaserColor;
	}

	return 0;
}

#pragma endregion

#pragma region Reinforcement_Planes

DEFINE_HOOK(0x65EB2D, HouseClass_SendSpyPlane_PickEdgeCell, 0x6)
{
	enum { SkipGameCode = 0x65EB4D };

	GET(AircraftClass* const, pAircraft, ESI);
	GET(Edge const, edge, EAX);
	GET(AbstractClass* const, navCom, EBX);
	GET_STACK(AbstractClass* const, tarCom, STACK_OFFSET(0x28, 0xC));
	REF_STACK(CellStruct, edgeCell, STACK_OFFSET(0x28, -0x10));

	const auto pTarget = tarCom ? tarCom : navCom;
	const auto targetCell = pTarget ? CellClass::Coord2Cell(pTarget->GetCoords()) : CellStruct::Empty;
	edgeCell = AircraftExt::PickEdgeCellForPlane(pAircraft->Type, targetCell, edge);

	R->EAX(&edgeCell);
	return SkipGameCode;
}

DEFINE_HOOK(0x65EB8D, HouseClass_SendSpyPlane_PlaceAircraft, 0x6)
{
	enum { SkipGameCode = 0x65EBE5, SkipGameCodeNoSuccess = 0x65EC12 };

	GET(AircraftClass* const, pAircraft, ESI);
	GET(CellStruct const, edgeCell, EDI);

	const bool result = AircraftExt::PlaceReinforcementAircraft(pAircraft, CellClass::Cell2Coord(edgeCell));

	return result ? SkipGameCode : SkipGameCodeNoSuccess;
}

DEFINE_HOOK(0x65E881, HouseClass_SendAirstrike_PickEdgeCell, 0x5)
{
	enum { SkipGameCode = 0x65E8A0 };

	GET(AircraftTypeClass* const, pAircraftType, EBP);
	GET(Edge const, edge, EAX);
	GET_STACK(AbstractClass* const, navCom, STACK_OFFSET(0x38, 0x10));
	GET_STACK(AbstractClass* const, tarCom, STACK_OFFSET(0x38, 0xC));
	REF_STACK(CellStruct, edgeCell, STACK_OFFSET(0x38, -0x1C));

	const auto pTarget = tarCom ? tarCom : navCom;
	const auto targetCell = pTarget ? CellClass::Coord2Cell(pTarget->GetCoords()) : CellStruct::Empty;
	edgeCell = AircraftExt::PickEdgeCellForPlane(pAircraftType, targetCell, edge);

	R->EAX(&edgeCell);
	return SkipGameCode;
}

DEFINE_HOOK(0x65E997, HouseClass_SendAirstrike_PlaceAircraft, 0x6)
{
	enum { SkipGameCode = 0x65E9EE, SkipGameCodeNoSuccess = 0x65EA8B };

	GET(AircraftClass* const, pAircraft, ESI);
	GET(CellStruct const, edgeCell, EDI);

	const bool result = AircraftExt::PlaceReinforcementAircraft(pAircraft, CellClass::Cell2Coord(edgeCell));

	return result ? SkipGameCode : SkipGameCodeNoSuccess;
}

DEFINE_HOOK(0x65E6DB, HouseClass_SendParadrop_PickEdgeCell, 0x6)
{
	enum { SkipGameCode = 0x65E6FB };

	GET(AircraftClass* const, pAircraft, ESI);
	GET(Edge const, edge, EAX);
	GET_STACK(AbstractClass* const, navCom, STACK_OFFSET(0x30, 0x10));
	GET_STACK(AbstractClass* const, tarCom, STACK_OFFSET(0x30, 0xC));
	REF_STACK(CellStruct, edgeCell, STACK_OFFSET(0x30, -0x10));

	const auto pTarget = tarCom ? tarCom : navCom;
	const auto targetCell = pTarget ? CellClass::Coord2Cell(pTarget->GetCoords()) : CellStruct::Empty;
	edgeCell = AircraftExt::PickEdgeCellForPlane(pAircraft->Type, targetCell, edge);

	R->EAX(&edgeCell);
	return SkipGameCode;
}

DEFINE_HOOK(0x65E73A, HouseClass_SendParadrop_PlaceAircraft, 0x5)
{
	enum { SkipGameCode = 0x65E79B, SkipGameCodeNoSuccess = 0x65E82C };

	GET(AircraftClass* const, pAircraft, ESI);
	GET(CellStruct const, edgeCell, EDI);

	const bool result = AircraftExt::PlaceReinforcementAircraft(pAircraft, CellClass::Cell2Coord(edgeCell));

	return result ? SkipGameCode : SkipGameCodeNoSuccess;
}

DEFINE_HOOK(0x415A7A, AircraftClass_Mission_Retreat_PickEdgeCell, 0x6)
{
	enum { SkipGameCode = 0x415A9A };

	GET(AircraftClass* const, pThis, ESI);
	GET(Edge const, edge, EAX);
	REF_STACK(CellStruct, edgeCell, STACK_OFFSET(0xC, -0x4));

	edgeCell = AircraftExt::PickEdgeCellForPlane(pThis->Type, pThis->GetMapCoords(), edge, true);

	R->EAX(&edgeCell);
	return SkipGameCode;
}

#pragma endregion

// Vanilla and Ares all only hardcoded to find factory with BuildCat::DontCare...
static inline bool CheckShouldDisableDefensesCameo(HouseClass* pHouse, TechnoTypeClass* pType)
{
	if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pType))
	{
		if (pBuildingType->BuildCat == BuildCat::Combat)
		{
			auto count = 0;

			if (const auto pFactory = pHouse->Primary_ForDefenses)
			{
				count = pFactory->CountTotal(pBuildingType);

				if (pFactory->Object && pFactory->Object->GetType() == pBuildingType && pBuildingType->BuildLimit > 0)
					--count;
			}

			auto buildLimitRemaining = [](HouseClass* pHouse, BuildingTypeClass* pBldType)
			{
				const auto BuildLimit = pBldType->BuildLimit;

				if (BuildLimit >= 0)
					return BuildLimit - BuildingTypeExt::CountOwnedNowWithDeployOrUpgrade(pBldType, pHouse);
				else
					return -BuildLimit - pHouse->CountOwnedEver(pBldType);
			};

			if (buildLimitRemaining(pHouse, pBuildingType) - count <= 0)
				return true;
		}
	}

	return false;
}

DEFINE_HOOK(0x50B669, HouseClass_ShouldDisableCameo_GreyCameo, 0x3)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(TechnoTypeClass*, pType, 0x4);
	GET(const bool, aresDisable, EAX);

	if (aresDisable || !pType)
		return 0;

	if (CheckShouldDisableDefensesCameo(pThis, pType) || HouseExt::ReachedBuildLimit(pThis, pType, false))
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

	const int delay = RulesExt::Global()->AISuperWeaponDelay.Get();

	if (delay > 0)
	{
		auto const pExt = HouseExt::Fetch(pThis);

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

	auto const pType = pBuilding->Type;

	if (BuildingTypeExt::Fetch(pType)->ExcludeFromMultipleFactoryBonus)
	{
		GET(HouseClass*, pThis, EDI);
		GET(const bool, isNaval, ECX);

		auto const pExt = HouseExt::Fetch(pThis);
		pExt->UpdateNonMFBFactoryCounts(pType->Factory, R->Origin() == 0x4FF9C9, isNaval);
	}

	return 0;
}

DEFINE_HOOK(0x500910, HouseClass_GetFactoryCount, 0x5)
{
	enum { SkipGameCode = 0x50095D };

	GET(HouseClass*, pThis, ECX);
	GET_STACK(AbstractType, rtti, 0x4);
	GET_STACK(const bool, isNaval, 0x8);

	auto const pExt = HouseExt::Fetch(pThis);
	R->EAX(pExt->GetFactoryCountWithoutNonMFB(rtti, isNaval));

	return SkipGameCode;
}

// Sell all and all in.
DEFINE_HOOK(0x4FD8F7, HouseClass_UpdateAI_OnLastLegs, 0x10)
{
	enum { ret = 0x4FD907 };

	GET(HouseClass*, pThis, EBX);

	if (RulesExt::Global()->AIFireSale)
	{
		auto const pExt = HouseExt::Fetch(pThis);

		if (RulesExt::Global()->AIFireSaleDelay <= 0 || pExt->AIFireSaleDelayTimer.Completed())
			pThis->Fire_Sale();
		else if (!pExt->AIFireSaleDelayTimer.HasStarted())
			pExt->AIFireSaleDelayTimer.Start(RulesExt::Global()->AIFireSaleDelay);
	}

	if (RulesExt::Global()->AIAllToHunt)
		pThis->All_To_Hunt();

	return ret;
}

DEFINE_HOOK(0x4F8ACC, HouseClass_Update_ResetTeamDelay, 0x6)
{
	enum { ResetTeamDelay = 0x4F8AD5 };

	GET(HouseClass*, pThis, ESI);

	const auto pHouseExt = HouseExt::Fetch(pThis);
	const int teamDelay = pHouseExt->TeamDelay;

	if (teamDelay >= 0)
	{
		R->ECX(teamDelay);
		return ResetTeamDelay;
	}

	const auto teamDelayType = RulesExt::Global()->TeamDelays_DynamicType;

	if (teamDelayType == DynamicTeamDelayType::None)
		return 0;

	int playerCount = ScenarioClass::Instance->NumberStartingPoints;

	if (playerCount >= 2 && !SessionClass::IsCampaign())
	{
		if (teamDelayType != DynamicTeamDelayType::StartingPoint)
		{
			playerCount = 0;
			const bool checkAlive = teamDelayType == DynamicTeamDelayType::AliveCount
				|| teamDelayType == DynamicTeamDelayType::AliveAllies
				|| teamDelayType == DynamicTeamDelayType::AliveEnemies;
			const bool checkAllies = teamDelayType == DynamicTeamDelayType::Allies
				|| teamDelayType == DynamicTeamDelayType::AliveAllies;
			const bool checkEnemies = teamDelayType == DynamicTeamDelayType::Enemies
				|| teamDelayType == DynamicTeamDelayType::AliveEnemies;

			for (auto const pHouse : HouseClass::Array)
			{
				if ((!checkAlive || !pHouse->Defeated)
					&& !pHouse->IsObserver()
					&& !pHouse->Type->MultiplayPassive
					&& (!checkAllies || (pThis != pHouse && pThis->IsAlliedWith(pHouse)))
					&& (!checkEnemies || !pThis->IsAlliedWith(pHouse)))
				{
					playerCount += 1;
				}
			}
		}

		if (playerCount < 1 || playerCount > 8)
			return 0;

		const int AIDifficulty = pThis->GetAIDifficultyIndex();
		int delay = 0;

		switch (AIDifficulty)
		{
		case 0:
			delay = RulesExt::Global()->TeamDelays_Count[playerCount - 1].Get().X;
			break;
		case 1:
			delay = RulesExt::Global()->TeamDelays_Count[playerCount - 1].Get().Y;
			break;
		case 2:
			delay = RulesExt::Global()->TeamDelays_Count[playerCount - 1].Get().Z;
			break;
		}

		if (delay > 0)
		{
			R->ECX(delay);
			return ResetTeamDelay;
		}
	}

	return 0;
}

DEFINE_HOOK(0x508E17, HouseClass_UpdateRadar_FreeRadar, 0x8)
{
	enum { ForceRadar = 0x508F2F, Continue = 0x508E4A };

	GET(HouseClass*, pThis, ECX);
	REF_STACK(bool, enableRadar, STACK_OFFSET(0x1C, -0xC));

	auto const pExt = HouseExt::Fetch(pThis);
	bool const freeRadar = pExt->FreeRadar;
	enableRadar = false;

	if (pExt->ForceRadar)
	{
		enableRadar = freeRadar;
		return ForceRadar;
	}
	else if (pThis->RadarBlackoutTimer.HasTimeLeft())
	{
		return ForceRadar;
	}
	else if (freeRadar)
	{
		enableRadar = true;
		return ForceRadar;
	}

	return Continue;
}

// WW's code set anger on every houses, even on the allies.
DEFINE_HOOK(0x4FD616, HouseClass_UpdateAI_DontAngerOnAlly, 0x9)
{
	enum { SkipCurrentHouse = 0x4FD6FE };

	GET(HouseClass*, pThis, EBX);
	GET(HouseClass*, pTargetHouse, ESI);

	return pThis->IsAlliedWith(pTargetHouse) ? SkipCurrentHouse : 0;
}

// WW calculates the distance from pThis to pThis ...
DEFINE_HOOK(0x4FD635, HouseClass_UpdateAI_DistCalcFix, 0x5)
{
	enum { SkipGameCode = 0x4FD657 };
	GET(HouseClass*, pTargetHouse, ESI);
	auto baseMapCrd = pTargetHouse->BaseCenter == CellStruct::Empty ? pTargetHouse->BaseSpawnCell : pTargetHouse->BaseCenter;
	R->EAX(*(int*)&baseMapCrd);
	return SkipGameCode;
}

// Replace game function.
DEFINE_HOOK(0x50BF60, HouseClass_CalculateCostMultipliers, 0x5)
{
	enum { SkipGameCode = 0x50C04A };

	GET(HouseClass*, pThis, ECX);

	std::unordered_map<int, int> counts;
	pThis->CostAircraftMult = 1.0f;
	pThis->CostBuildingsMult = 1.0f;
	pThis->CostDefensesMult = 1.0f;
	pThis->CostInfantryMult = 1.0f;
	pThis->CostUnitsMult = 1.0f;

	for (auto const& pBuilding : pThis->FactoryPlants)
	{
		auto const pType = pBuilding->Type;
		auto const pTypeExt = BuildingTypeExt::Fetch(pType);
		const int max = pTypeExt->FactoryPlant_MaxCount;

		if (max > -1 && counts[pType->ArrayIndex] >= max)
			continue;

		counts[pType->ArrayIndex]++;
		pThis->CostAircraftMult *= pType->AircraftCostBonus;
		pThis->CostBuildingsMult *= pType->BuildingsCostBonus;
		pThis->CostDefensesMult *= pType->DefensesCostBonus;
		pThis->CostInfantryMult *= pType->InfantryCostBonus;
		pThis->CostUnitsMult *= pType->UnitsCostBonus;
	}

	return SkipGameCode;
}

#pragma region PlayerAutoRepair

DEFINE_HOOK(0x6A5395, SidebarClass_InitIO_InitRepairButton, 0x6)
{
	if (!RulesExt::Global()->ExtendedPlayerRepair)
		return 0;

	if (HouseExt::Fetch(HouseClass::CurrentPlayer)->PlayerAutoRepair)
	{
		SidebarClass::Instance.SidebarNeedsRedraw = true;
		SidebarClass::ToggleRepairButton.IsOn = true;
	}

	return 0;
}

DEFINE_HOOK(0x536FA0, ToggleRepariModeCommandClass_Execute_PlayerAutoRepair, 0x7)
{
	if (!RulesExt::Global()->ExtendedPlayerRepair)
		return 0;

	EventExt::RaiseTogglePlayerAutoRepair();
	return 0x536FAC;
}

DEFINE_HOOK(0x6A78F6, SidebarClass_Update_ToggleRepair, 0x9)
{
	if (!RulesExt::Global()->ExtendedPlayerRepair)
		MapClass::Instance.SetRepairMode(-1);
	else
		EventExt::RaiseTogglePlayerAutoRepair();
	return 0x6A78FF;
}

DEFINE_HOOK(0x6A7AE1, SidebarClass_Update_RepairButton, 0x6)
{
	if (!RulesExt::Global()->ExtendedPlayerRepair)
		return 0;

	R->AL(HouseExt::Fetch(HouseClass::CurrentPlayer)->PlayerAutoRepair);
	return 0x6A7AE7;
}

DEFINE_HOOK(0x45063F, BuildingClass_UpdateRepairSell_PlayerAutoRepair, 0x6)
{
	enum { CanAutoRepair = 0x450659, CanNotAutoRepair = 0x450813 };

	if (!RulesExt::Global()->ExtendedPlayerRepair)
		return 0;

	GET(BuildingClass*, pThis, ESI);

	if (!pThis->Owner->IsControlledByHuman())
		return 0;

	if (HouseExt::Fetch(pThis->Owner)->PlayerAutoRepair)
	{
		return CanAutoRepair;
	}
	else
	{
		if (pThis->IsBeingRepaired)
			pThis->SetRepairState(0);
		return CanNotAutoRepair;
	}
}

#pragma endregion

#pragma region BeaconOrder

DEFINE_HOOK(0x43131B, BeaconManagerClass_DeleteBeacon_RecordOrder, 0x5)
{
	if (!RulesExt::Global()->AutoRemoveEarliestBeacon)
		return 0;

	GET(const int, beaconIdx, EBX);
	GET(const int, houseIdx, ECX);

	const auto pHouse = HouseClass::Array.GetItem(houseIdx);
	const auto pExt = HouseExt::Fetch(pHouse);

	const int oldValue = std::exchange(pExt->BeaconsPlacedOrder[beaconIdx], 0);

	if (oldValue != 0)
	{
		for (int i = 0; i < 3; ++i)
		{
			if (i != beaconIdx && pExt->BeaconsPlacedOrder[i] > oldValue)
				--pExt->BeaconsPlacedOrder[i];
		}
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x430E5D, BeaconManagerClass_PlaceBeacon_RecordOrder, 0x5)
DEFINE_HOOK(0x430C64, BeaconManagerClass_PlaceBeacon_RecordOrder, 0x5)
{
	if (!RulesExt::Global()->AutoRemoveEarliestBeacon)
		return 0;

	GET(const int, beaconIdx, EAX);
	GET(const int, houseIdx, EBX);

	const auto pHouse = HouseClass::Array.GetItem(houseIdx);
	const auto pExt = HouseExt::Fetch(pHouse);

	const int maxVal = std::max({ pExt->BeaconsPlacedOrder[0], pExt->BeaconsPlacedOrder[1], pExt->BeaconsPlacedOrder[2] });
	pExt->BeaconsPlacedOrder[beaconIdx] = maxVal + 1;

	return 0;
}

DEFINE_HOOK(0x4AC9B2, MouseClass_ToggleBeaconMode_AllUsed, 0x6)
{
	enum { RET = 0x4AC9B8 };

	GET(const bool, canPlace, EAX);

	if (canPlace)
		return RET;

	if (!RulesExt::Global()->AutoRemoveEarliestBeacon)
	{
		R->BL(0);
		return RET;
	}

	const auto pHouse = HouseClass::CurrentPlayer;
	const auto pExt = HouseExt::Fetch(pHouse);

	for (int i = 0; i < 3; ++i)
	{
		if (pExt->BeaconsPlacedOrder[i] == 1)
		{
			const auto pManager = &BeaconManagerClass::Instance;
			const auto pBeacon = pManager->Beacons[pHouse->ArrayIndex][i];
			// Select and delete beacon.
			// If you don't select the beacon, the game will not send the IPX packet.
			MapClass::UnselectAll();
			pBeacon->Bitfield |= 2;
			pManager->DeleteBeacon(-1, -1);
			break;
		}
	}

	return RET;
}

#pragma endregion

#pragma region ResourceCosts

DEFINE_HOOK(0x4FD590, HouseClass_CanBuild_CheckResources, 0x6)
{
	GET(HouseClass*, pHouse, ECX);
	GET_STACK(TechnoTypeClass*, pItem, 0x4);
	GET_STACK(bool, check_money, 0x8);

	if (check_money && pItem && pHouse)
	{
		if (const auto pTypeExt = TechnoTypeExt::TryFetch(pItem))
		{
			if (const auto pHouseExt = HouseExt::TryFetch(pHouse))
			{
				for (size_t i = 0; i < pTypeExt->ResourceCosts.size(); ++i)
				{
					const int cost = pTypeExt->ResourceCosts[i];
					if (cost > 0 && !pHouseExt->CanAffordResource(static_cast<int>(i), cost))
					{
						R->AL(0);
						return 0x4FD605;
					}
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x4FA4B0, HouseClass_BeginProduction_CheckAndDeductResources, 0x6)
{
	GET(HouseClass*, pHouse, ECX);
	GET_STACK(AbstractType, absType, 0x4);
	GET_STACK(int, index, 0x8);
	GET_STACK(int, count, 0xC);

	if (pHouse && count > 0)
	{
		if (const auto pType = TechnoTypeClass::GetByTypeAndIndex(absType, index))
		{
			if (const auto pTypeExt = TechnoTypeExt::TryFetch(pType))
			{
				if (const auto pHouseExt = HouseExt::TryFetch(pHouse))
				{
					for (size_t i = 0; i < pTypeExt->ResourceCosts.size(); ++i)
					{
						const int cost = pTypeExt->ResourceCosts[i];
						if (cost > 0 && !pHouseExt->CanAffordResource(static_cast<int>(i), cost * count))
						{
							R->AL(0);
							return 0x4FA66C;
						}
					}

					for (size_t i = 0; i < pTypeExt->ResourceCosts.size(); ++i)
					{
						const int cost = pTypeExt->ResourceCosts[i];
						if (cost > 0)
						{
							pHouseExt->UpdateResourceAmount(static_cast<int>(i), -(cost * count));
						}
					}
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x4FAA20, HouseClass_AbandonProduction_RefundResources, 0x6)
{
	GET(HouseClass*, pHouse, ECX);
	GET_STACK(AbstractType, absType, 0x4);
	GET_STACK(int, index, 0x8);
	GET_STACK(int, count, 0xC);

	if (pHouse)
	{
		if (const auto pType = TechnoTypeClass::GetByTypeAndIndex(absType, index))
		{
			if (const auto pTypeExt = TechnoTypeExt::TryFetch(pType))
			{
				if (const auto pHouseExt = HouseExt::TryFetch(pHouse))
				{
					const int refundCount = (count > 0) ? count : 1;
					for (size_t i = 0; i < pTypeExt->ResourceCosts.size(); ++i)
					{
						const int cost = pTypeExt->ResourceCosts[i];
						if (cost > 0)
						{
							pHouseExt->UpdateResourceAmount(static_cast<int>(i), cost * refundCount);
						}
					}
				}
			}
		}
	}

	return 0;
}

#pragma endregion

