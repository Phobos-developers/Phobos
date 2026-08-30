#include "Body.h"

#include <GameOptionsClass.h>
#include <Ext/Anim/Body.h>
#include <Ext/House/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Side/Body.h>
#include <TacticalClass.h>
#include <PlanningTokenClass.h>

#pragma region Update

// After TechnoClass_AI
DEFINE_HOOK(0x43FE69, BuildingClass_AI, 0xA)
{
	GET(BuildingClass*, pThis, ESI);

	const auto pBuildingExt = BuildingExt::Fetch(pThis);
	pBuildingExt->DisplayIncomeString();

	TechnoExt* const pTechnoExt = pBuildingExt; // the building extension is a TechnoExt
	pTechnoExt->UpdateLaserTrails(); // Mainly for on turret trails

	// Force airstrike targets to redraw every frame to account for tint intensity fluctuations.
	if (pTechnoExt->AirstrikeTargetingMe)
		pThis->Mark(MarkType::Change);

	return 0;
}

DEFINE_HOOK(0x43FBEF, BuildingClass_AI_PoweredKillSpawns, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	BuildingExt::Fetch(pThis)->ApplyPoweredKillSpawns();

	return 0;
}

DEFINE_HOOK(0x4403D4, BuildingClass_AI_ChronoSparkle, 0x6)
{
	enum { SkipGameCode = 0x44055D };

	GET(BuildingClass*, pThis, ESI);

	if (RulesClass::Instance->ChronoSparkle1)
	{
		auto const displayPositions = RulesExt::Global()->ChronoSparkleBuildingDisplayPositions;
		auto const pType = pThis->Type;
		const bool displayOnBuilding = (displayPositions & ChronoSparkleDisplayPosition::Building) != ChronoSparkleDisplayPosition::None;
		const bool displayOnSlots = (displayPositions & ChronoSparkleDisplayPosition::OccupantSlots) != ChronoSparkleDisplayPosition::None;
		const bool displayOnOccupants = (displayPositions & ChronoSparkleDisplayPosition::Occupants) != ChronoSparkleDisplayPosition::None;
		const int occupantCount = displayOnSlots ? pType->MaxNumberOccupants : pThis->GetOccupantCount();
		const bool showOccupy = occupantCount && (displayOnOccupants || displayOnSlots);

		if (showOccupy)
		{
			auto const renderCoords = pThis->GetRenderCoords();

			for (int i = 0; i < occupantCount; i++)
			{
				if (!((Unsorted::CurrentFrame + i) % RulesExt::Global()->ChronoSparkleDisplayDelay))
				{
					auto const muzzleOffset = pType->MaxNumberOccupants <= 10 ? pType->MuzzleFlash[i] : BuildingTypeExt::Fetch(pType)->OccupierMuzzleFlashes.at(i);
					auto coords = CoordStruct::Empty;
					auto offset = TacticalClass::Instance->ApplyMatrix_Pixel(muzzleOffset);
					coords.X += offset.X;
					coords.Y += offset.Y;
					coords += renderCoords;

					GameCreate<AnimClass>(RulesClass::Instance->ChronoSparkle1, coords)->ZAdjust = -200;
				}
			}
		}

		if ((!showOccupy || displayOnBuilding) && !(Unsorted::CurrentFrame % RulesExt::Global()->ChronoSparkleDisplayDelay))
		{
			GameCreate<AnimClass>(RulesClass::Instance->ChronoSparkle1, pThis->GetCenterCoords());
		}

	}

	return SkipGameCode;
}

#pragma endregion

DEFINE_HOOK(0x443C81, BuildingClass_ExitObject_InitialClonedHealth, 0x7)
{
	GET(BuildingClass*, pBuilding, ESI);

	if (auto const pInf = abstract_cast<InfantryClass*>(R->EDI<FootClass*>()))
	{
		if (pBuilding && pBuilding->Type->Cloning)
		{
			const double percentage = GeneralUtils::GetRangedRandomOrSingleValue(BuildingTypeExt::Fetch(pBuilding->Type)->InitialStrength_Cloning);
			const int health = pInf->Type->Strength;
			const int strength = Math::clamp(static_cast<int>(health * percentage), 1, health);
			pInf->Health = strength;
			pInf->EstimatedHealth = strength;
		}
	}

	return 0;
}

DEFINE_HOOK(0x449ADA, BuildingClass_MissionConstruction_DeployToFireFix, 0x0)
{
	GET(BuildingClass*, pThis, ESI);

	auto const pExt = BuildingExt::Fetch(pThis);

	if (pExt->DeployedTechno && pThis->LastTarget)
	{
		pThis->Target = pThis->LastTarget;
		pThis->QueueMission(Mission::Attack, false);
	}
	else
	{
		pThis->QueueMission(Mission::Guard, false);
	}

	return 0x449AE8;
}

#pragma region EMPulseCannon

namespace EMPulseCannonTemp
{
	int weaponIndex = 0;
}

DEFINE_HOOK(0x44CEEC, BuildingClass_Mission_Missile_EMPulseSelectWeapon, 0x6)
{
	enum { SkipGameCode = 0x44CEF8 };

	GET(BuildingClass*, pThis, ESI);

	auto const pExt = BuildingExt::Fetch(pThis);

	if (!pExt->CurrentEMPulseSW)
		return 0;

	int weaponIndex = 0;
	auto const pSWExt = SWTypeExt::Fetch(pExt->CurrentEMPulseSW->Type);
	auto const pOwner = pThis->Owner;

	if (pSWExt->EMPulse_WeaponIndex >= 0)
	{
		weaponIndex = pSWExt->EMPulse_WeaponIndex;
	}
	else
	{
		auto const pCell = MapClass::Instance.TryGetCellAt(pOwner->EMPTarget);

		if (pCell)
		{
			AbstractClass* pTarget = pCell;

			if (auto const pObject = pCell->GetContent())
				pTarget = pObject;

			weaponIndex = pThis->SelectWeapon(pTarget);
		}
	}

	if (pSWExt->EMPulse_SuspendOthers)
	{
		auto const pHouseExt = HouseExt::Fetch(pOwner);
		const int index = pExt->CurrentEMPulseSW->Type->ArrayIndex;

		if (pHouseExt->SuspendedEMPulseSWs.count(index))
		{
			auto& supers = pOwner->Supers;

			for (auto const& swidx : pHouseExt->SuspendedEMPulseSWs[index])
			{
				auto const super = supers[swidx];
				super->IsSuspended = false;
			}

			pHouseExt->SuspendedEMPulseSWs[index].clear();
			pHouseExt->SuspendedEMPulseSWs.erase(index);
		}
	}

	pExt->CurrentEMPulseSW = nullptr;
	EMPulseCannonTemp::weaponIndex = weaponIndex;
	R->EAX(pThis->GetWeapon(weaponIndex));
	return SkipGameCode;
}

static CoordStruct* __fastcall BuildingClass_GetFireCoords_Wrapper(BuildingClass* pThis, void* _, CoordStruct* pCrd, int weaponIndex)
{
	auto coords = MapClass::Instance.GetCellAt(pThis->Owner->EMPTarget)->GetCellCoords();
	pCrd = pThis->GetFLH(&coords, EMPulseCannonTemp::weaponIndex, *pCrd);
	return pCrd;
}

DEFINE_FUNCTION_JUMP(CALL6, 0x44D1F9, BuildingClass_GetFireCoords_Wrapper);

DEFINE_HOOK(0x44D455, BuildingClass_Mission_Missile_EMPulseBulletWeapon, 0x8)
{
	GET(WeaponTypeClass*, pWeapon, EBP);
	GET_STACK(BulletClass*, pBullet, STACK_OFFSET(0xF0, -0xA4));

	pBullet->SetWeaponType(pWeapon);

	return 0;
}

#pragma endregion

#pragma region KickOutStuckUnits

DEFINE_HOOK(0x44955D, BuildingClass_WeaponFactoryOutsideBusy_WeaponFactoryCell, 0x6)
{
	enum { NotBusy = 0x44969B };

	GET(BuildingClass* const, pThis, ESI);

	const auto pLink = pThis->GetNthLink();

	if (!pLink)
		return NotBusy;

	const auto pLinkType = pLink->GetTechnoType();

	if (pLinkType->JumpJet && pLinkType->BalloonHover)
		return NotBusy;

	return 0;
}

// Attempt to kick the stuck unit out again by setting the destination
DEFINE_HOOK(0x44E202, BuildingClass_Mission_Unload_CheckStuck, 0x6)
{
	enum { Waiting = 0x44E267, NextStatus = 0x44E20C };

	GET(BuildingClass*, pThis, EBP);

	if (!pThis->IsTether)
		return NextStatus;

	if (const auto pUnit = abstract_cast<UnitClass*>(pThis->GetNthLink()))
	{
		// Detecting movement status
		if (pUnit->Locomotor->Destination() == CoordStruct::Empty)
		{
			// Evacuate the congestion at the entrance
			reinterpret_cast<void(__thiscall*)(BuildingClass*)>(0x449540)(pThis);
			const auto pType = pThis->Type;
			const auto cell = pThis->GetMapCoords() + pType->FoundationOutside[10];
			const auto door = cell - CellStruct { 1, 0 };
			const auto pDest = MapClass::Instance.GetCellAt(door);

			// Hover units may stop one cell behind their destination, should forcing them to advance one more cell
			pUnit->SetDestination((pUnit->Destination != pDest ? pDest : MapClass::Instance.GetCellAt(cell)), true);
		}
	}

	return Waiting;
}

// Check for any stuck units inside after successful unload each time. If there is, kick it out
DEFINE_HOOK(0x44E260, BuildingClass_Mission_Unload_KickOutStuckUnits, 0x7)
{
	GET(BuildingClass*, pThis, EBP);

	BuildingExt::KickOutStuckUnits(pThis);

	return 0;
}

// Should not kick out units if the factory building is in construction process
DEFINE_HOOK(0x4444A0, BuildingClass_KickOutUnit_NoKickOutInConstruction, 0xA)
{
	enum { ThisIsOK = 0x444565, ThisIsNotOK = 0x4444B3 };

	GET(BuildingClass* const, pThis, ESI);

	const auto mission = pThis->GetCurrentMission();

	return (mission == Mission::Unload || mission == Mission::Construction) ? ThisIsNotOK : ThisIsOK;
}

#pragma endregion

// Ares didn't have something like 0x7397E4 in its UnitDelivery code
DEFINE_HOOK(0x44FBBF, CreateBuildingFromINIFile_AfterCTOR_BeforeUnlimbo, 0x8)
{
	GET(BuildingClass* const, pBld, ESI);

	if (auto const pExt = BuildingExt::TryFetch(pBld))
	{
		pExt->IsCreatedFromMapFile = true;

		GET_STACK(bool, hasPower, STACK_OFFSET(0xEC, -0xDC));

		if (hasPower)
			pExt->HasPowerFromMapFile = true;
	}

	return 0;
}

DEFINE_HOOK(0x44FDC5, CreateBuildingFromINIFile_AfterCTOR_AfterUnlimbo, 0xA)
{
	GET(BuildingClass* const, pBld, ESI);

	if (auto const pExt = BuildingExt::TryFetch(pBld))
		pExt->HasPowerFromMapFile = false;

	return 0x44FDD3;
}

DEFINE_HOOK(0x440B4F, BuildingClass_Unlimbo_SetShouldRebuild, 0x5)
{
	enum { ContinueCheck = 0x440B58, SkipSetShouldRebuild = 0x440B81 };

	GET(BuildingClass* const, pThis, ESI);

	if (BuildingTypeExt::Fetch(pThis->Type)->NewEvaVoice_Index.isset())
		SideExt::UpdateMainEvaVoice(pThis);

	if (SessionClass::IsCampaign())
	{
		// Preplaced structures are already managed before
		if (BuildingExt::Fetch(pThis)->IsCreatedFromMapFile)
			return SkipSetShouldRebuild;

		// Per-house dehardcoding: BaseNodes + SW-Delivery
		if (!HouseExt::Fetch(pThis->Owner)->RepairBaseNodes[GameOptionsClass::Instance.Difficulty].Get(RulesExt::Global()->RepairBaseNodes))
			return SkipSetShouldRebuild;
	}
	// Vanilla instruction: always repairable in other game modes
	return ContinueCheck;
}

DEFINE_HOOK(0x440EBB, BuildingClass_Unlimbo_NaturalParticleSystem_CampaignSkip, 0x5)
{
	enum { DoNotCreateParticle = 0x440F61 };
	GET(BuildingClass* const, pThis, ESI);
	return BuildingExt::Fetch(pThis)->IsCreatedFromMapFile ? DoNotCreateParticle : 0;
}

DEFINE_HOOK(0x4519A2, BuildingClass_UpdateAnim_SetParentBuilding, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EBP);

	// This runs while a savegame is loading too, where neither extension exists yet:
	// the building's is restored from the extension stream and attached once the load
	// settles, and an animation the game creates along the way gets one then as well.
	// The reference count only makes sense if both ends are there, so skip both.
	auto const pAnimExt = AnimExt::TryFetch(pAnim);
	auto const pExt = TechnoExt::TryFetch(pThis);

	if (pAnimExt && pExt)
	{
		pAnimExt->ParentBuilding = pThis;
		pExt->AnimRefCount++;
	}

	return 0;
}

DEFINE_HOOK(0x43D6E5, BuildingClass_Draw_ZShapePointMove, 0x5)
{
	enum { Apply = 0x43D6EF, Skip = 0x43D712 };

	GET(const Mission, mission, EAX);

	if ((mission != Mission::Selling && mission != Mission::Construction))
		return Apply;

	GET(BuildingClass*, pThis, ESI);

	if (BuildingTypeExt::Fetch(pThis->Type)->ZShapePointMove_OnBuildup)
		return Apply;

	return Skip;
}

DEFINE_HOOK(0x4511D6, BuildingClass_AnimationAI_SellBuildup, 0x7)
{
	enum { Skip = 0x4511E6, Continue = 0x4511DF };

	GET(BuildingClass*, pThis, ESI);

	return BuildingTypeExt::Fetch(pThis->Type)->SellBuildupLength == pThis->Animation.Value ? Continue : Skip;
}

#pragma region PowerPlantEnhancer

DEFINE_HOOK(0x441553, BuildingClass_Unlimbo_AddOwned, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	const auto pTypeExt = BuildingTypeExt::Fetch(pThis->Type);
	const auto pOwnerExt = HouseExt::Fetch(pThis->Owner);

	if (!pTypeExt->PowerPlantEnhancer_Buildings.empty() && (pTypeExt->PowerPlantEnhancer_Amount != 0 || pTypeExt->PowerPlantEnhancer_Factor != 1.0f))
		pOwnerExt->PowerPlantEnhancers.push_back(pThis);

	return 0;
}

DEFINE_HOOK(0x448A78, BuildingClass_SetOwningHouse_RemoveOwned, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(HouseClass*, pOwner, EBX);
	const auto pTypeExt = BuildingTypeExt::Fetch(pThis->Type);
	const auto pOwnerExt = HouseExt::Fetch(pOwner);

	if (!pTypeExt->PowerPlantEnhancer_Buildings.empty() && (pTypeExt->PowerPlantEnhancer_Amount != 0 || pTypeExt->PowerPlantEnhancer_Factor != 1.0f))
	{
		auto& vec = pOwnerExt->PowerPlantEnhancers;
		vec.erase(std::remove(vec.begin(), vec.end(), pThis), vec.end());
	}

	return 0;
}

DEFINE_HOOK(0x449197, BuildingClass_SetOwningHouse_AddOwned, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(HouseClass*, pNewOwner, EBP);
	const auto pTypeExt = BuildingTypeExt::Fetch(pThis->Type);
	const auto pNewOwnerExt = HouseExt::Fetch(pNewOwner);

	if (!pTypeExt->PowerPlantEnhancer_Buildings.empty() && (pTypeExt->PowerPlantEnhancer_Amount != 0 || pTypeExt->PowerPlantEnhancer_Factor != 1.0f))
		pNewOwnerExt->PowerPlantEnhancers.push_back(pThis);

	return 0;
}

#pragma endregion

#pragma region FactoryPlant

DEFINE_HOOK(0x441501, BuildingClass_Unlimbo_FactoryPlant, 0x6)
{
	enum { Skip = 0x441553 };

	GET(BuildingClass*, pThis, ESI);

	auto const pTypeExt = BuildingTypeExt::Fetch(pThis->Type);

	if (pTypeExt->FactoryPlant_AllowTypes.size() > 0 || pTypeExt->FactoryPlant_DisallowTypes.size() > 0)
	{
		auto const pHouseExt = HouseExt::Fetch(pThis->Owner);
		pHouseExt->RestrictedFactoryPlants.push_back(pThis);

		return Skip;
	}

	return 0;
}

DEFINE_HOOK(0x448A31, BuildingClass_Captured_FactoryPlant1, 0x6)
{
	enum { Skip = 0x448A78 };

	GET(BuildingClass*, pThis, ESI);

	auto const pTypeExt = BuildingTypeExt::Fetch(pThis->Type);

	if (pTypeExt->FactoryPlant_AllowTypes.size() > 0 || pTypeExt->FactoryPlant_DisallowTypes.size() > 0)
	{
		auto const pHouseExt = HouseExt::Fetch(pThis->Owner);

		if (!pHouseExt->RestrictedFactoryPlants.empty())
		{
			auto& vec = pHouseExt->RestrictedFactoryPlants;
			vec.erase(std::remove(vec.begin(), vec.end(), pThis), vec.end());
		}

		return Skip;
	}

	return 0;
}

DEFINE_HOOK(0x449149, BuildingClass_Captured_FactoryPlant2, 0x6)
{
	enum { Skip = 0x449197 };

	GET(BuildingClass*, pThis, ESI);

	auto const pTypeExt = BuildingTypeExt::Fetch(pThis->Type);

	if (pTypeExt->FactoryPlant_AllowTypes.size() > 0 || pTypeExt->FactoryPlant_DisallowTypes.size() > 0)
	{
		GET(HouseClass*, pNewOwner, EBP);

		auto const pHouseExt = HouseExt::Fetch(pNewOwner);
		pHouseExt->RestrictedFactoryPlants.push_back(pThis);

		return Skip;
	}

	return 0;
}

#pragma endregion

#pragma region DestroyableObstacle

template <bool remove = false>
static void RecalculateCells(BuildingClass* pThis)
{
	auto const cells = BuildingExt::GetFoundationCells(pThis, pThis->GetMapCoords());

	auto& map = MapClass::Instance;

	for (auto const& cell : cells)
	{
		if (auto const pCell = map.TryGetCellAt(cell))
		{
			pCell->RecalcAttributes(DWORD(-1));

			if constexpr (remove)
				map.ResetZones(cell);
			else
				map.RecalculateZones(cell);

			map.RecalculateSubZones(cell);

		}
	}
}

DEFINE_HOOK(0x440D01, BuildingClass_Unlimbo_DestroyableObstacle, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	if (BuildingTypeExt::Fetch(pThis->Type)->IsDestroyableObstacle)
		RecalculateCells(pThis);

	return 0;
}

DEFINE_HOOK(0x445D87, BuildingClass_Limbo_DestroyableObstacle, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	auto const pTypeExt = BuildingTypeExt::Fetch(pThis->Type);

	if (pTypeExt->NewEvaVoice_Index.isset() && pTypeExt->NewEvaVoice_RecheckOnDeath)
		SideExt::UpdateMainEvaVoice(pThis);

	if (pTypeExt->IsDestroyableObstacle)
		RecalculateCells<true>(pThis);

	// only remove animation when the building is destroyed or sold
	if (pThis->Health > 0 && pThis->IsAlive && pThis->GetCurrentMission() != Mission::Selling)
		return 0;

	for (auto& bAnim : pThis->Anims)
	{
		if (bAnim && VTable::Get(bAnim) == 0x7E3354)
		{
			bAnim->UnInit();
			bAnim = nullptr;
		}
	}

	return 0;
}

DEFINE_HOOK(0x483D8E, CellClass_CheckPassability_DestroyableObstacle, 0x6)
{
	enum { IsBlockage = 0x483CD4 };

	GET(BuildingClass*, pBuilding, ESI);

	if (BuildingTypeExt::Fetch(pBuilding->Type)->IsDestroyableObstacle)
		return IsBlockage;

	return 0;
}

#pragma endregion

#pragma region UnitRepair

namespace UnitRepairTemp
{
	bool SeparateRepair = false;
}

DEFINE_HOOK(0x44C836, BuildingClass_Mission_Repair_UnitReload, 0x6)
{
	GET(BuildingClass*, pThis, EBP);

	if (pThis->Type->UnitReload)
	{
		auto const pTypeExt = BuildingTypeExt::Fetch(pThis->Type);

		if (pTypeExt->Units_RepairRate.isset())
		{
			const double repairRate = pTypeExt->Units_RepairRate.Get();

			if (repairRate < 0.0)
				return 0;

			const int rate = static_cast<int>(Math::max(repairRate * 900, 1));

			if (!(Unsorted::CurrentFrame % rate))
			{
				UnitRepairTemp::SeparateRepair = true;

				for (int i = 0; i < pThis->RadioLinks.Capacity; ++i)
				{
					if (auto const pLink = pThis->GetNthLink(i))
					{
						if (!pLink->IsInAir() && pLink->Health < pLink->GetType()->Strength && pThis->SendCommand(RadioCommand::QueryMoving, pLink) == RadioCommand::AnswerPositive)
							pThis->SendCommand(RadioCommand::RequestRepair, pLink);
					}
				}

				UnitRepairTemp::SeparateRepair = false;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x44B8F1, BuildingClass_Mission_Repair_Hospital, 0x6)
{
	enum { SkipGameCode = 0x44B8F7 };

	GET(BuildingClass*, pThis, EBP);

	auto const pTypeExt = BuildingTypeExt::Fetch(pThis->Type);
	const double repairRate = pTypeExt->Units_RepairRate.Get(RulesClass::Instance->IRepairRate);
	__asm { fld repairRate }

	return SkipGameCode;
}

DEFINE_HOOK(0x44BD38, BuildingClass_Mission_Repair_UnitRepair, 0x6)
{
	enum { SkipGameCode = 0x44BD3E };

	GET(BuildingClass*, pThis, EBP);

	auto const pTypeExt = BuildingTypeExt::Fetch(pThis->Type);
	const double repairRate = pTypeExt->Units_RepairRate.Get(RulesClass::Instance->URepairRate);
	__asm { fld repairRate }

	return SkipGameCode;
}

DEFINE_HOOK(0x6F4D1A, TechnoClass_ReceiveCommand_Repair, 0x5)
{
	enum { SkipEffects = 0x6F4DE5 };

	GET_STACK(TechnoClass*, pFrom, STACK_OFFSET(0x18, 0x4));
	GET(TechnoClass*, pThis, ESI);
	GET(int, repairStep, EAX);

	if (auto const pBuilding = abstract_cast<BuildingClass*>(pFrom))
	{
		auto const pTypeExt = BuildingTypeExt::Fetch(pBuilding->Type);

		if (pBuilding->Type->UnitReload && pTypeExt->Units_RepairRate.isset() && !UnitRepairTemp::SeparateRepair)
			return SkipEffects;

		repairStep = pTypeExt->Units_RepairStep.Get(repairStep);
		const double repairPercent = pTypeExt->Units_RepairPercent.Get(RulesClass::Instance->RepairPercent);
		int repairCost = 0;

		if (pTypeExt->Units_UseRepairCost.Get(pThis->WhatAmI() != AbstractType::Infantry))
		{
			auto const pType = pThis->GetTechnoType();
			repairCost = static_cast<int>((pType->GetCost() / (pType->Strength / static_cast<double>(repairStep))) * repairPercent);

			if (repairCost < 1)
				repairCost = 1;
		}

		R->EAX(repairStep);
		R->EBX(repairCost);
	}

	return 0;
}

#pragma endregion

#pragma region EnableBuildingProductionQueue

DEFINE_HOOK(0x6AB689, SelectClass_Action_SkipBuildingProductionCheck, 0x5)
{
	enum { SkipGameCode = 0x6AB6CE };
	return RulesExt::Global()->BuildingProductionQueue ? SkipGameCode : 0;
}

DEFINE_HOOK(0x4FA520, HouseClass_BeginProduction_SkipBuilding, 0x5)
{
	enum { SkipGameCode = 0x4FA553 };
	return RulesExt::Global()->BuildingProductionQueue ? SkipGameCode : 0;
}

DEFINE_HOOK(0x4FA612, HouseClass_BeginProduction_ForceRedrawStrip, 0x5)
{
	SidebarClass::Instance.SidebarBackgroundNeedsRedraw = true;
	return 0;
}

DEFINE_HOOK(0x4C9C7B, FactoryClass_QueueProduction_ForceCheckBuilding, 0x7)
{
	enum { SkipGameCode = 0x4C9C9E };
	return RulesExt::Global()->BuildingProductionQueue ? SkipGameCode : 0;
}

DEFINE_HOOK(0x4FAAD8, HouseClass_AbandonProduction_RewriteForBuilding, 0x8)
{
	enum { CheckSame = 0x4FAB3D, SkipCheck = 0x4FAB64, Return = 0x4FAC9B };

	GET_STACK(const bool, all, STACK_OFFSET(0x18, 0x10));
	GET(const int, index, EBX);
	GET(const BuildCat, buildCat, ECX);
	GET(const AbstractType, absType, EBP);
	GET(FactoryClass* const, pFactory, ESI);

	// After placing the building, the factory will be in this state
	if (buildCat != BuildCat::DontCare && !all && !pFactory->Object)
		return SkipCheck;

	const auto pType = TechnoTypeClass::GetByTypeAndIndex(absType, index);
	const auto firstRemoved = pFactory->RemoveOneFromQueue(pType);

	if (firstRemoved)
	{
		SidebarClass::Instance.SidebarBackgroundNeedsRedraw = true; // Added, force redraw strip
		SidebarClass::Instance.RepaintSidebar(SidebarClass::GetObjectTabIdx(absType, index, 0));

		if (all)
			while (pFactory->RemoveOneFromQueue(pType));
		else
			return Return;
	}

	return CheckSame;
}

DEFINE_HOOK(0x6A9C54, StripClass_DrawStrip_FindFactoryDehardCode, 0x6)
{
	GET(TechnoTypeClass* const, pType, ECX);
	LEA_STACK(BuildCat*, pBuildCat, STACK_OFFSET(0x490, -0x490));

	if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pType))
		*pBuildCat = pBuildingType->BuildCat;

	return 0;
}

DEFINE_HOOK(0x6A9789, StripClass_DrawStrip_NoGreyCameo, 0x6)
{
	enum { ContinueCheck = 0x6A9799, SkipGameCode = 0x6A97FB };

	GET(TechnoTypeClass* const, pType, EBX);
	GET_STACK(const bool, clicked, STACK_OFFSET(0x48C, -0x475));

	if (!RulesExt::Global()->BuildingProductionQueue)
	{
		if (pType->WhatAmI() == AbstractType::BuildingType && clicked)
			return SkipGameCode;
	}
	else if (const auto pBuildingType = abstract_cast<BuildingTypeClass*, true>(pType))
	{
		if (const auto pFactory = HouseClass::CurrentPlayer->GetPrimaryFactory(AbstractType::BuildingType, pType->Naval, pBuildingType->BuildCat))
		{
			if (const auto pProduct = abstract_cast<BuildingClass*>(pFactory->Object))
			{
				if (pFactory->IsDone() && pProduct->Type != pType && ((pProduct->Type->BuildCat != BuildCat::Combat) ^ (pBuildingType->BuildCat == BuildCat::Combat)))
					return SkipGameCode;
			}
		}
	}

	return ContinueCheck;
}

DEFINE_HOOK(0x6AA88D, StripClass_RecheckCameo_FindFactoryDehardCode, 0x6)
{
	GET(TechnoTypeClass* const, pType, EBX);
	LEA_STACK(BuildCat*, pBuildCat, STACK_OFFSET(0x158, -0x158));

	if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pType))
		*pBuildCat = pBuildingType->BuildCat;

	return 0;
}

#pragma endregion

#pragma region BarracksExitCell

DEFINE_HOOK(0x44EFD8, BuildingClass_FindExitCell_BarracksExitCell, 0x6)
{
	enum { SkipGameCode = 0x44F13B, ReturnFromFunction = 0x44F037 };

	GET(BuildingClass*, pThis, EBX);
	REF_STACK(CellStruct, resultCell, STACK_OFFSET(0x30, -0x20));

	auto const pTypeExt = BuildingTypeExt::Fetch(pThis->Type);

	if (pTypeExt->BarracksExitCell.isset())
	{
		const Point2D offset = pTypeExt->BarracksExitCell.Get();
		auto exitCell = pThis->GetMapCoords();
		exitCell.X += (short)offset.X;
		exitCell.Y += (short)offset.Y;

		if (MapClass::Instance.CoordinatesLegal(exitCell))
		{
			GET(TechnoClass*, pTechno, ESI);
			auto const pCell = MapClass::Instance.GetCellAt(exitCell);

			if (pTechno->IsCellOccupied(pCell, FacingType::None, -1, nullptr, true) == Move::OK)
			{
				resultCell = exitCell;
				return ReturnFromFunction;
			}
		}

		return SkipGameCode;
	}

	return 0;
}

DEFINE_HOOK(0x444B83, BuildingClass_ExitObject_BarracksExitCell, 0x7)
{
	enum { SkipGameCode = 0x444C7C };

	GET(BuildingClass*, pThis, ESI);
	GET(const int, xCoord, EBP);
	GET(const int, yCoord, EDX);
	REF_STACK(CoordStruct, resultCoords, STACK_OFFSET(0x140, -0x108));

	auto const pType = pThis->Type;
	auto const pTypeExt = BuildingTypeExt::Fetch(pType);

	if (pTypeExt->BarracksExitCell.isset())
	{
		auto const exitCoords = pType->ExitCoord;
		resultCoords = CoordStruct { xCoord + exitCoords.X, yCoord + exitCoords.Y, exitCoords.Z };
		return SkipGameCode;
	}

	return 0;
}

DEFINE_HOOK(0x54BC99, JumpjetLocomotionClass_Ascending_BarracksExitCell, 0x6)
{
	enum { Continue = 0x54BCA3 };

	GET(BuildingTypeClass*, pType, EAX);

	auto const pTypeExt = BuildingTypeExt::Fetch(pType);

	if (pTypeExt->BarracksExitCell.isset())
		return Continue;

	return 0;
}

#pragma endregion

#pragma region BuildingFiring

DEFINE_HOOK(0x44B630, BuildingClass_MissionAttack_AnimDelayedFire, 0x6)
{
	enum { JustFire = 0x44B6C4, VanillaCheck = 0 };
	GET(BuildingClass* const, pThis, ESI);
	return (pThis->CurrentBurstIndex != 0 && !BuildingTypeExt::Fetch(pThis->Type)->IsAnimDelayedBurst) ? JustFire : VanillaCheck;
}

#pragma endregion

#pragma region BuildingWaypoints

static bool __fastcall BuildingTypeClass_CanUseWaypoint(BuildingTypeClass* pThis)
{
	return RulesExt::Global()->BuildingWaypoints;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4610, BuildingTypeClass_CanUseWaypoint)

DEFINE_HOOK(0x4AE95E, DisplayClass_sub_4AE750_DisallowBuildingNonAttackPlanning, 0x5)
{
	enum { SkipGameCode = 0x4AE982 };

	GET(ObjectClass* const, pObject, ECX);
	LEA_STACK(CellStruct*, pCell, STACK_OFFSET(0x20, 0x8));

	const auto action = pObject->MouseOverCell(pCell);

	if (!PlanningNodeClass::PlanningModeActive || pObject->WhatAmI() != AbstractType::Building || action == Action::Attack)
		pObject->CellClickedAction(action, pCell, pCell, false);

	return SkipGameCode;
}

#pragma endregion

DEFINE_HOOK(0x4400F9, BuildingClass_AI_UpdateOverpower, 0x6)
{
	enum { SkipGameCode = 0x44019D };

	GET(BuildingClass*, pThis, ESI);

	if (!pThis->Type->Overpowerable)
		return SkipGameCode;

	int overPower = 0;

	for (int idx = pThis->Overpowerers.Count - 1; idx >= 0; idx--)
	{
		const auto pCharger = pThis->Overpowerers[idx];

		if (pCharger->Target != pThis)
		{
			pThis->Overpowerers.RemoveItem(idx);
			continue;
		}

		const auto pWeapon = pCharger->GetWeapon(1)->WeaponType;

		if (!pWeapon || !pWeapon->Warhead || !pWeapon->Warhead->ElectricAssault)
		{
			pThis->Overpowerers.RemoveItem(idx);
			continue;
		}

		const auto pWHExt = WarheadTypeExt::Fetch(pWeapon->Warhead);
		overPower += pWHExt->ElectricAssaultLevel;
	}

	const auto pBuildingTypeExt = BuildingTypeExt::Fetch(pThis->Type);
	const int charge = pBuildingTypeExt->Overpower_ChargeWeapon;

	if (charge >= 0)
	{
		const int keepOnline = pBuildingTypeExt->Overpower_KeepOnline;
		pThis->IsOverpowered = overPower >= keepOnline + charge || (pThis->Owner->GetPowerPercentage() == 1.0 && pThis->HasPower && overPower >= charge);
	}
	else
	{
		pThis->IsOverpowered = false;
	}

	return SkipGameCode;
}

DEFINE_HOOK_AGAIN(0x45563B, BuildingClass_IsPowerOnline_Overpower, 0x6)
DEFINE_HOOK(0x4555E4, BuildingClass_IsPowerOnline_Overpower, 0x6)
{
	enum { LowPower = 0x4556BE, Continue1 = 0x4555F0, Continue2 = 0x455643 };

	GET(const int, threshold, EDI);

	// Battery.KeepOnline activated
	if (!threshold)
		return R->Origin() == 0x4555E4 ? Continue1 : Continue2;

	GET(BuildingClass*, pThis, ESI);
	const auto pBuildingTypeExt = BuildingTypeExt::Fetch(pThis->Type);
	const int keepOnline = pBuildingTypeExt->Overpower_KeepOnline;

	if (keepOnline < 0)
		return LowPower;

	int overPower = 0;

	for (const auto pCharger : pThis->Overpowerers)
	{
		const auto pWeapon = pCharger->GetWeapon(1)->WeaponType;

		if (pWeapon && pWeapon->Warhead)
		{
			const auto pWHExt = WarheadTypeExt::Fetch(pWeapon->Warhead);
			overPower += pWHExt->ElectricAssaultLevel;
		}
	}

	return overPower < keepOnline ? LowPower : (R->Origin() == 0x4555E4 ? Continue1 : Continue2);
}

#pragma region OwnerChangeBuildupFix

static void __fastcall BuildingClass_Place_Wrapper(BuildingClass* pThis, void*, bool captured)
{
	// Skip calling Place() here if we're in middle of buildup.
	if (pThis->CurrentMission != Mission::Construction || pThis->BState != (int)BStateType::Construction)
		pThis->Place(captured);
}

DEFINE_FUNCTION_JUMP(CALL6, 0x448CEF, BuildingClass_Place_Wrapper);

DEFINE_HOOK(0x44939F, BuildingClass_Captured_BuildupFix, 0x7)
{
	GET(BuildingClass*, pThis, ESI);

	// If we're supposed to be playing buildup during/after owner change reset any changes to mission or BState made during owner change.
	if (pThis->CurrentMission == Mission::Construction && pThis->BState == (int)BStateType::Construction)
	{
		pThis->IsReadyToCommence = false;
		pThis->QueueBState = (int)BStateType::None;
		pThis->QueuedMission = Mission::None;
	}

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x4485DB, BuildingClass_SetOwningHouse_SyncLinkedOwner, 0x6)
{
	enum { SkipGameCode = 0x4486C8 };
	GET(BuildingClass*, pThis, ESI);
	return BuildingTypeExt::Fetch(pThis->Type)->BuildingRadioLink_SyncOwner.Get(RulesExt::Global()->BuildingRadioLink_SyncOwner) ? 0 : SkipGameCode;
}

#pragma region PrefiringMark

DEFINE_HOOK(0x440045, BuildingClass_UpdateDelayedFiring_PrefiringMark1, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	BuildingExt::Fetch(pThis)->IsFiringNow = (int)pThis->PrismStage && pThis->DelayBeforeFiring <= 1;
	return 0;
}

DEFINE_HOOK(0x4400F9, BuildingClass_UpdateDelayedFiring_PrefiringMar2, 0x7)
{
	GET(BuildingClass*, pThis, ESI);
	BuildingExt::Fetch(pThis)->IsFiringNow = false;
	return 0;
}

#pragma endregion

#pragma region ProductionAnim

static __inline bool AllowBuildingProductionAnim(BuildingTypeClass* pType)
{
	if (pType->ConstructionYard)
		return true;

	if (pType->Factory == AbstractType::BuildingType && GeneralUtils::IsValidString(pType->GetBuildingAnim(BuildingAnimSlot::Production).Anim))
		return true;

	return false;
}

static bool IsRoofExitTechno(TechnoTypeClass* pType)
{
	return TechnoTypeExt::Fetch(pType)->ExitThroughRoof.Get(pType->JumpJet || pType->BalloonHover);
}

static bool IsRoofExitBuildingUnit(BuildingClass* pBuilding)
{
	const auto pUnit = pBuilding->GetNthLink();
	return pUnit && IsRoofExitTechno(pUnit->GetTechnoType());
}

DEFINE_HOOK(0x43D0CF, BuildingClass_DrawDoorAnim_ExitThroughRoof, 0x6)
{
	enum { UseRoof = 0x43D0D9, UseGate = 0x43D0E7 };

	GET(TechnoTypeClass*, pType, EAX);

	return TechnoTypeExt::Fetch(pType)->ExitThroughRoof.Get(pType->JumpJet) ? UseRoof : UseGate; // 0x43D0CF, JumpJet Only
}

DEFINE_HOOK(0x43D363, BuildingClass_DrawUnderDoorAnim_ExitThroughRoof, 0x6)
{
	enum { MarkRoof = 0x43D381, Skip = 0x43D386 };

	GET(TechnoTypeClass*, pType, EAX);

	return IsRoofExitTechno(pType) ? MarkRoof : Skip;
}

static AnimTypeClass* PickRoofProductionAnim(BuildingTypeExt* pTypeExt, bool isDamaged, bool garrisoned)
{
	if (garrisoned && pTypeExt->RoofProductionAnimGarrisoned.Get() != nullptr)
		return pTypeExt->RoofProductionAnimGarrisoned.Get();

	if (isDamaged && pTypeExt->RoofProductionAnimDamaged.Get() != nullptr)
		return pTypeExt->RoofProductionAnimDamaged.Get();

	return pTypeExt->RoofProductionAnim.Get();
}

static void PlayRoofProductionAnim(BuildingClass* pBuilding, BuildingTypeExt* pTypeExt, bool isDamaged, bool garrisoned)
{
	const char* animName = nullptr;

	if (auto pAnimType = PickRoofProductionAnim(pTypeExt, isDamaged, garrisoned))
	{
		animName = pAnimType->get_ID();
	}
	else
	{
		// Fall back to the plain ProductionAnim only, so the roof line stays independent
		// of the ProductionAnimDamaged/Garrisoned branches.
		animName = pBuilding->Type->GetBuildingAnim(BuildingAnimSlot::Production).Anim;
	}

	if (!GeneralUtils::IsValidString(animName))
		return;

	// The roof anim needs its own placement and power values without affecting
	// the plain line, so apply them only for the duration of this PlayAnim
	// call.
	auto& prodAnim = pBuilding->Type->GetBuildingAnim(BuildingAnimSlot::Production);

	const Point2D savedPosition = prodAnim.Position;
	const int savedZAdjust = prodAnim.ZAdjust;
	const int savedYSort = prodAnim.YSort;
	const bool savedPowered = prodAnim.Powered;
	const bool savedPoweredLight = prodAnim.PoweredLight;
	const bool savedPoweredEffect = prodAnim.PoweredEffect;
	const bool savedPoweredSpecial = prodAnim.PoweredSpecial;

	prodAnim.Position.X = pTypeExt->RoofProductionAnimX.Get(savedPosition.X);
	prodAnim.Position.Y = pTypeExt->RoofProductionAnimY.Get(savedPosition.Y);
	prodAnim.ZAdjust = pTypeExt->RoofProductionAnimZAdjust.Get(savedZAdjust);
	prodAnim.YSort = pTypeExt->RoofProductionAnimYSort.Get(savedYSort);
	prodAnim.Powered = pTypeExt->RoofProductionAnimPowered.Get(savedPowered);
	prodAnim.PoweredLight = pTypeExt->RoofProductionAnimPoweredLight.Get(savedPoweredLight);
	prodAnim.PoweredEffect = pTypeExt->RoofProductionAnimPoweredEffect.Get(savedPoweredEffect);
	prodAnim.PoweredSpecial = pTypeExt->RoofProductionAnimPoweredSpecial.Get(savedPoweredSpecial);

	auto pExt = BuildingExt::Fetch(pBuilding);
	pExt->IsPlayingRoofProductionAnim = true;
	pBuilding->PlayAnim(animName, BuildingAnimSlot::Production, isDamaged, garrisoned, 0);
	pExt->IsPlayingRoofProductionAnim = false;

	prodAnim.Position = savedPosition;
	prodAnim.ZAdjust = savedZAdjust;
	prodAnim.YSort = savedYSort;
	prodAnim.Powered = savedPowered;
	prodAnim.PoweredLight = savedPoweredLight;
	prodAnim.PoweredEffect = savedPoweredEffect;
	prodAnim.PoweredSpecial = savedPoweredSpecial;
}

DEFINE_HOOK(0x43CC73, BuildingClass_ReceiveMessage_ProductionAnim, 0x6)
{
	enum { SkipGameCode = 0x43CC79 };

	GET(BuildingTypeClass*, pType, ECX);

	R->EAX(AllowBuildingProductionAnim(pType));

	return SkipGameCode;
}

DEFINE_HOOK(0x44B7AE, BuildingClass_Mission_Repair_ProductionAnim, 0x6)
{
	enum { SkipGameCode = 0x44B7B4 };

	GET(BuildingTypeClass*, pType, EAX);

	R->ECX(AllowBuildingProductionAnim(pType));

	return SkipGameCode;
}

// isRoofExit tells whether the produced unit leaves through the roof hatch;
// each caller decides it from its own reliable source.
static bool TryPlayRoofProductionAnim(BuildingClass* pBuilding, bool isDamaged, bool isRoofExit)
{
	if (!isRoofExit)
		return false;

	auto pTypeExt = BuildingTypeExt::Fetch(pBuilding->Type);
	const bool garrisoned = pBuilding->GetOccupantCount() > 0;

	const auto& prodAnim = pBuilding->Type->GetBuildingAnim(BuildingAnimSlot::Production);
	if (BuildingTypeExt::IsPoweredAnimBlocked(pBuilding,
		pTypeExt->RoofProductionAnimPowered.Get(prodAnim.Powered),
		pTypeExt->RoofProductionAnimPoweredLight.Get(prodAnim.PoweredLight),
		pTypeExt->RoofProductionAnimPoweredEffect.Get(prodAnim.PoweredEffect),
		pTypeExt->RoofProductionAnimPoweredSpecial.Get(prodAnim.PoweredSpecial)))
		return true;

	PlayRoofProductionAnim(pBuilding, pTypeExt, isDamaged, garrisoned);
	return true;
}

DEFINE_HOOK(0x44DDF0, BuildingClass_Unload_RoofProductionAnim, 0x6)
{
	enum { SkipGameCode = 0x44E267 };

	GET(BuildingClass*, pBuilding, EBP);

	return TryPlayRoofProductionAnim(pBuilding, false, IsRoofExitBuildingUnit(pBuilding)) ? SkipGameCode : 0;
}

DEFINE_HOOK(0x44DDDE, BuildingClass_Unload_RoofProductionAnim_Damaged, 0x6)
{
	enum { SkipGameCode = 0x44E267 };

	GET(BuildingClass*, pBuilding, EBP);

	return TryPlayRoofProductionAnim(pBuilding, true, IsRoofExitBuildingUnit(pBuilding)) ? SkipGameCode : 0;
}

DEFINE_HOOK(0x444D11, BuildingClass_ExitObject_ProductionAnimForInfantryFactory, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	// The roof check must use the unit that is actually leaving the factory.
	GET(TechnoClass*, pUnit, EDI);

	auto const pType = pThis->Type;

	if (pType->Factory == AbstractType::InfantryType)
	{
		const bool isDamaged = pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;

		if (pUnit && IsRoofExitTechno(pUnit->GetTechnoType()))
		{
			pThis->DestroyNthAnim(BuildingAnimSlot::Idle);
			TryPlayRoofProductionAnim(pThis, isDamaged, true);
			return 0;
		}

		auto anim = pType->GetBuildingAnim(BuildingAnimSlot::Production).Anim;

		if (isDamaged)
			anim = pType->GetBuildingAnim(BuildingAnimSlot::Production).Damaged;

		if (GeneralUtils::IsValidString(anim))
		{
			pThis->DestroyNthAnim(BuildingAnimSlot::Idle);
			pThis->PlayAnim(anim, BuildingAnimSlot::Production, isDamaged, false, 0);
		}
	}

	return 0;
}

DEFINE_HOOK(0x4501AF, AI_ConYard_CompleteProduction_ProductionAnim, 0x5)
{
	GET(BuildingClass*, pBuilding, ESI);
	GET(TechnoClass*, pObject, EDI);

	if (pBuilding->Owner->IsControlledByHuman())
		return 0;

	if (!pObject || pObject->WhatAmI() != AbstractType::Building)
		return 0;

	pBuilding->SendCommand(RadioCommand::RequestEndProduction, pBuilding);

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x45670D, BuildingClass_GetRadialIndicatorRange_Extras, 0x7)
{
	enum { ApplyRange = 0x45674B, ApplyTurretWeapon = 0x456714 };

	GET(BuildingClass*, pThis, ESI);
	const auto pTypeExt = BuildingTypeExt::Fetch(pThis->Type);

	if (!pTypeExt->PowerPlantEnhancer_Buildings.empty() && (pTypeExt->PowerPlantEnhancer_Amount != 0 || pTypeExt->PowerPlantEnhancer_Factor != 1.0f))
	{
		R->EAX(pTypeExt->PowerPlantEnhancer_Range.Get() / Unsorted::LeptonsPerCell);
		return ApplyRange;
	}

	R->EAX(pThis->TechnoClass::GetTurretWeapon());
	return ApplyTurretWeapon;
}

#pragma region Mission_Guard_Attack

static int HandleArmedBuildingGuard(BuildingClass* pThis)
{
	auto const pType = pThis->Type;
	pThis->IsReadyToCommence = true;

	// May 29, 2026 - Starkku: The EMPulseCannon and SW checks are most likely superfluous,
	// but kept them here just in case removing them would break something.
	if (pType->EMPulseCannon || pThis->FirstActiveSWIdx() >= 0 || (pType->CanBeOccupied && pThis->Occupants.Count <= 0) || !pThis->Target)
	{
		auto const pTypeExt = BuildingTypeExt::Fetch(pType);
		auto const& delay = pTypeExt->GuardRetryDelay.isset() ? pTypeExt->GuardRetryDelay : RulesExt::Global()->BuildingGuardRetryDelay;

		if (delay.isset())
			return GeneralUtils::GetRangedRandomOrSingleValue(delay);

		return static_cast<int>(MissionControlClass::Array[(int)pThis->CurrentMission].AARate * 900 + ScenarioClass::Instance->Random(0, 2));
	}
	else
	{
		pThis->QueueMission(Mission::Attack, false);
		pThis->NextMission();

		return 1;
	}
}

DEFINE_HOOK(0x4496FB, BuildingClass_Mission_Guard_Armed, 0x6)
{
	enum { ReturnFromFunction = 0x4497A7 };

	GET(BuildingClass*, pThis, ESI);

	R->EAX(HandleArmedBuildingGuard(pThis));
	return ReturnFromFunction;
}

#pragma endregion

#pragma region TurretAnim

DEFINE_HOOK(0x451242, BuildingClass_AnimationAI_TurretAnim, 0xA)
{
	enum { SkipGameCode = 0x451296 };

	GET(BuildingClass*, pThis, ESI);

	if (auto const pAnim = pThis->Anims[(int)BuildingAnimSlot::Turret])
	{
		pAnim->Animation.Value = BuildingExt::GetTurretFrame(pThis);
		pAnim->Animation.Step = 0;
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x44B6C7, BuildingClass_Mission_Attack_TurretAnim, 0x6)
{
	enum { SkipFiring = 0x44B6FE };

	GET(BuildingClass*, pThis, ESI);

	if (pThis->HasTurret())
	{
		if (auto const pAnim = pThis->Anims[(int)BuildingAnimSlot::Turret])
		{
			auto const pExt = BuildingExt::Fetch(pThis);
			auto const pTypeExt = pExt->GetTypeExtData();
			const bool isLowPower = !pThis->StuffEnabled || !pThis->IsPowerOnline();
			const int firingFrames = isLowPower ? pTypeExt->TurretAnim_LowPowerFiringFrames : pTypeExt->TurretAnim_FiringFrames;

			if (firingFrames > 0 && pExt->TurretAnimFiringFrame == -1)
			{
				pExt->TurretAnimFiringFrame = 0;
				pExt->TurretAnimRateTick = 0;
			}
		}
	}

	return 0;
}

#pragma endregion

#pragma region TankBunker

// Jun 23, 2026 - Starkku: Vanilla tank bunker code assumes
// even-sized foundation. The approach used for even-sized
// foundations and those that have discrete center cell are
// mutually exclusive due to pathfinding constraints.

// Handle docking offset calculations.
DEFINE_HOOK(0x447BE3, BuildingClass_DockingCoord_TankBunker, 0x6)
{
	enum { SkipGameCode = 0x447CE1 };

	GET(BuildingClass*, pThis, ESI);

	// Discrete center cell: Docking coord is building center instead of cell closest to approach.
	if (pThis->Type->GetFoundationWidth() % 2)
	{
		const auto coords = pThis->GetCenterCoords();

		// Cleaner hook return at function return is causing problems
		// due to stack offsets, which is why we're doing it like this.
		R->ECX(coords.X);
		R->EDX(coords.Y);
		R->EAX(&coords);

		return SkipGameCode;
	}

	return 0;
}

// Remove now unnecessary (and in fact interfering) rotation code in BuildingClass::UpdateTankBunker().
DEFINE_HOOK(0x459069, BuildingClass_UpdateTankBunker_CheckOccupants, 0x7)
{
	enum { SkipGameCode = 0x4590EF };

	GET(BuildingClass*, pThis, ESI);

	// Discrete center cell: No need to change facing at this stage.
	if (pThis->Type->GetFoundationWidth() % 2)
		return SkipGameCode;

	return 0;
}

// Handle force moving unit to center of bunker.
DEFINE_HOOK(0x459101, BuildingClass_UpdateTankBunker_RotateToTrack, 0x6)
{
	enum { ReturnFromFunction = 0x4591CE };

	GET(BuildingClass*, pThis, ESI);
	GET(UnitClass*, pUnit, EBP);

	// Discrete center cell: Skip straight to rotating in bunker once finished moving instead of forcing track to center.
	if (pThis->Type->GetFoundationWidth() % 2)
	{
		if (!pUnit->Locomotor->Is_Moving())
		{
			pUnit->PrimaryFacing.SetDesired(DirStruct(DirType::South));
			pThis->TankBunkerState = TankBunkerState::RotateInBunker;
		}

		return ReturnFromFunction;
	}

	return 0;
}

inline void EjectBunkeredUnit(BuildingClass* pThis, UnitClass* pUnit)
{
	auto const pType = pUnit->Type;
	auto const cell = pThis->GetMapCoords() + CellStruct(-1, 1);
	auto const pNearbyCell = MapClass::Instance.NearByLocation(cell, pType->SpeedType, -1, pType->MovementZone, false, 1, 1, false, false, false, true, cell, false, false);
	pUnit->SetDestination(MapClass::Instance.GetCellAt(pNearbyCell), false);
	pUnit->QueueMission(Mission::Move, false);
}

// Bunker was destroyed, sold or warped away.
DEFINE_HOOK(0x4593C7, BuildingClass_DestroyTankBunker, 0x6)
{
	enum { SkipGameCode = 0x459450 };

	GET(BuildingClass*, pThis, EDI);
	GET(UnitClass*, pUnit, ESI);

	// Discrete center cell: Send unit out to a nearby cell without forcing drive track.
	if (pThis->Type->GetFoundationWidth() % 2)
	{
		EjectBunkeredUnit(pThis, pUnit);
		return SkipGameCode;
	}

	return 0;
}

// Manual unload of the bunker.
DEFINE_HOOK(0x4596EC, BuildingClass_UnloadTankBunker, 0x6)
{
	enum { SkipGameCode = 0x45980D };

	GET(BuildingClass*, pThis, EDI);
	GET(UnitClass*, pUnit, ESI);

	// Discrete center cell: Send unit out to a nearby cell without forcing drive track.
	if (pThis->Type->GetFoundationWidth() % 2)
	{
		EjectBunkeredUnit(pThis, pUnit);
		return SkipGameCode;
	}

	return 0;
}

// Add customization for tank bunker logic update delay.
DEFINE_HOOK(0x44C976, BuildingClass_Mission_Repair_TankBunker, 0x5)
{
	GET(BuildingClass*, pThis, EBP);

	auto const pType = pThis->Type;

	if (pType->Bunker && (pThis->TankBunkerState > TankBunkerState::Idle && pThis->TankBunkerState < TankBunkerState::Bunkered))
		R->EAX(BuildingTypeExt::Fetch(pType)->BunkerStateUpdateDelay.Get(RulesExt::Global()->BunkerStateUpdateDelay));

	return 0;
}

#pragma endregion

#pragma region BuildingStartFacing

static int GetBuildingStartFacing(BuildingClass* pThis)
{
	auto const pTypeExt = BuildingTypeExt::Fetch(pThis->Type);

	if (pTypeExt->StartFacing_Random.Get(RulesExt::Global()->StartFacing_Random))
	{
		auto pExt = BuildingExt::Fetch(pThis);
		if (pExt->ConstructionStartFacing < 0)
			pExt->ConstructionStartFacing = ScenarioClass::Instance->Random.RandomRanged(0, 255);
		return pExt->ConstructionStartFacing;
	}

	return pTypeExt->StartFacing.Get(RulesExt::Global()->StartFacing);
}

DEFINE_HOOK(0x449AFE, BuildingClass_Mission_Construction_StartFacing, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	int facing = GetBuildingStartFacing(pThis);
	R->CH(static_cast<BYTE>(facing));
	return 0x449B04;
}

DEFINE_HOOK(0x449DAA, BuildingClass_Mission_Selling_StartFacing_Compare, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	int facing = GetBuildingStartFacing(pThis);
	R->EBX(facing);
	return 0x449DB0;
}

DEFINE_HOOK(0x449DE9, BuildingClass_Mission_Selling_StartFacing_Set, 0x6)
{
	GET(BuildingClass*, pThis, EBP);
	int facing = GetBuildingStartFacing(pThis);
	R->CH(static_cast<BYTE>(facing));
	return 0x449DEF;
}

DEFINE_HOOK(0x6F6D9E, TechnoClass_Unlimbo_BuildingStartFacing, 0x7)
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis->AbstractFlags & AbstractFlags::Foot)
		return 0;

	const auto pBuilding = static_cast<BuildingClass*>(pThis);

	if (pBuilding->Type->LaserFence || BuildingExt::Fetch(pBuilding)->IsCreatedFromMapFile)
		return 0;

	R->AH(static_cast<BYTE>(GetBuildingStartFacing(pBuilding)));
	return 0;
}

#pragma endregion
