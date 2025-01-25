#include "Body.h"

#include <BulletClass.h>
#include <UnitClass.h>
#include <SuperClass.h>
#include <GameOptionsClass.h>
#include <Ext/Anim/Body.h>
#include <Ext/House/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <TacticalClass.h>

#pragma region Update

//After TechnoClass_AI?
DEFINE_HOOK(0x43FE69, BuildingClass_AI, 0xA)
{
	GET(BuildingClass*, pThis, ESI);

	// Do not search this up again in any functions called here because it is costly for performance - Starkku
	auto pExt = BuildingExt::ExtMap.Find(pThis);

	/*
	// Set only if unset or type has changed - Not currently useful as building type does not change.
	auto pType = pThis->Type;

	if (!pExt->TypeExtData || pExt->TypeExtData->OwnerObject() != pType)
		pExt->TypeExtData = BuildingTypeExt::ExtMap.Find(pType);
	*/

	pExt->DisplayIncomeString();
	pExt->ApplyPoweredKillSpawns();

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
		bool displayOnBuilding = (displayPositions & ChronoSparkleDisplayPosition::Building) != ChronoSparkleDisplayPosition::None;
		bool displayOnSlots = (displayPositions & ChronoSparkleDisplayPosition::OccupantSlots) != ChronoSparkleDisplayPosition::None;
		bool displayOnOccupants = (displayPositions & ChronoSparkleDisplayPosition::Occupants) != ChronoSparkleDisplayPosition::None;
		int occupantCount = displayOnSlots ? pType->MaxNumberOccupants : pThis->GetOccupantCount();
		bool showOccupy = occupantCount && (displayOnOccupants || displayOnSlots);

		if (showOccupy)
		{
			for (int i = 0; i < occupantCount; i++)
			{
				if (!((Unsorted::CurrentFrame + i) % RulesExt::Global()->ChronoSparkleDisplayDelay))
				{
					auto muzzleOffset = pType->MaxNumberOccupants <= 10 ? pType->MuzzleFlash[i] : BuildingTypeExt::ExtMap.Find(pType)->OccupierMuzzleFlashes.at(i);
					auto coords = CoordStruct::Empty;
					auto const renderCoords = pThis->GetRenderCoords();
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
			if (auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type))
			{
				double percentage = GeneralUtils::GetRangedRandomOrSingleValue(pTypeExt->InitialStrength_Cloning);
				int strength = Math::clamp(static_cast<int>(pInf->Type->Strength * percentage), 1, pInf->Type->Strength);

				pInf->Health = strength;
				pInf->EstimatedHealth = strength;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x449ADA, BuildingClass_MissionConstruction_DeployToFireFix, 0x0)
{
	GET(BuildingClass*, pThis, ESI);

	auto pExt = BuildingExt::ExtMap.Find(pThis);
	if (pExt && pExt->DeployedTechno && pThis->LastTarget)
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

	int weaponIndex = 0;
	auto const pExt = BuildingExt::ExtMap.Find(pThis);

	if (!pExt->EMPulseSW)
		return 0;

	auto const pSWExt = SWTypeExt::ExtMap.Find(pExt->EMPulseSW->Type);

	if (pSWExt->EMPulse_WeaponIndex >= 0)
	{
		weaponIndex = pSWExt->EMPulse_WeaponIndex;
	}
	else
	{
		auto const pCell = MapClass::Instance->TryGetCellAt(pThis->Owner->EMPTarget);

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
		auto const pHouseExt = HouseExt::ExtMap.Find(pThis->Owner);

		if (pHouseExt->SuspendedEMPulseSWs.count(pExt->EMPulseSW->Type->ArrayIndex))
		{
			for (auto const& swidx : pHouseExt->SuspendedEMPulseSWs[pExt->EMPulseSW->Type->ArrayIndex])
			{
				pThis->Owner->Supers[swidx]->IsSuspended = false;
			}

			pHouseExt->SuspendedEMPulseSWs[pExt->EMPulseSW->Type->ArrayIndex].clear();
			pHouseExt->SuspendedEMPulseSWs.erase(pExt->EMPulseSW->Type->ArrayIndex);
		}
	}

	pExt->EMPulseSW = nullptr;
	EMPulseCannonTemp::weaponIndex = weaponIndex;
	R->EAX(pThis->GetWeapon(weaponIndex));
	return SkipGameCode;
}

CoordStruct* __fastcall BuildingClass_GetFireCoords_Wrapper(BuildingClass* pThis, void* _, CoordStruct* pCrd, int weaponIndex)
{
	auto coords = MapClass::Instance->GetCellAt(pThis->Owner->EMPTarget)->GetCellCoords();
	pCrd = pThis->GetFLH(&coords, EMPulseCannonTemp::weaponIndex, *pCrd);
	return pCrd;
}

DEFINE_JUMP(CALL6, 0x44D1F9, GET_OFFSET(BuildingClass_GetFireCoords_Wrapper));

DEFINE_HOOK(0x44D455, BuildingClass_Mission_Missile_EMPulseBulletWeapon, 0x8)
{
	GET(WeaponTypeClass*, pWeapon, EBP);
	GET_STACK(BulletClass*, pBullet, STACK_OFFSET(0xF0, -0xA4));

	pBullet->SetWeaponType(pWeapon);

	return 0;
}

#pragma endregion

// Ares didn't have something like 0x7397E4 in its UnitDelivery code
DEFINE_HOOK(0x44FBBF, CreateBuildingFromINIFile_AfterCTOR_BeforeUnlimbo, 0x8)
{
	GET(BuildingClass* const, pBld, ESI);

	if (auto pExt = BuildingExt::ExtMap.Find(pBld))
		pExt->IsCreatedFromMapFile = true;

	return 0;
}

DEFINE_HOOK(0x440B4F, BuildingClass_Unlimbo_SetShouldRebuild, 0x5)
{
	enum { ContinueCheck = 0x440B58, SkipSetShouldRebuild = 0x440B81 };
	GET(BuildingClass* const, pThis, ESI);

	if (SessionClass::IsCampaign())
	{
		// Preplaced structures are already managed before
		if (BuildingExt::ExtMap.Find(pThis)->IsCreatedFromMapFile)
			return SkipSetShouldRebuild;

		// Per-house dehardcoding: BaseNodes + SW-Delivery
		if (!HouseExt::ExtMap.Find(pThis->Owner)->RepairBaseNodes[GameOptionsClass::Instance->Difficulty].Get(RulesExt::Global()->RepairBaseNodes))
			return SkipSetShouldRebuild;
	}
	// Vanilla instruction: always repairable in other game modes
	return ContinueCheck;
}

DEFINE_HOOK(0x440EBB, BuildingClass_Unlimbo_NaturalParticleSystem_CampaignSkip, 0x5)
{
	enum { DoNotCreateParticle = 0x440F61 };
	GET(BuildingClass* const, pThis, ESI);
	return BuildingExt::ExtMap.Find(pThis)->IsCreatedFromMapFile ? DoNotCreateParticle : 0;
}

DEFINE_HOOK(0x4519A2, BuildingClass_UpdateAnim_SetParentBuilding, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EBP);

	auto const pAnimExt = AnimExt::ExtMap.Find(pAnim);
	pAnimExt->ParentBuilding = pThis;

	return 0;
}

DEFINE_HOOK(0x43D6E5, BuildingClass_Draw_ZShapePointMove, 0x5)
{
	enum { Apply = 0x43D6EF, Skip = 0x43D712 };

	GET(BuildingClass*, pThis, ESI);
	GET(Mission, mission, EAX);

	if ((mission != Mission::Selling && mission != Mission::Construction) || BuildingTypeExt::ExtMap.Find(pThis->Type)->ZShapePointMove_OnBuildup)
		return Apply;

	return Skip;
}

DEFINE_HOOK(0x4511D6, BuildingClass_AnimationAI_SellBuildup, 0x7)
{
	enum { Skip = 0x4511E6, Continue = 0x4511DF };

	GET(BuildingClass*, pThis, ESI);

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	return pTypeExt->SellBuildupLength == pThis->Animation.Value ? Continue : Skip;
}

#pragma region FactoryPlant

DEFINE_HOOK(0x441501, BuildingClass_Unlimbo_FactoryPlant, 0x6)
{
	enum { Skip = 0x441553 };

	GET(BuildingClass*, pThis, ESI);

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->FactoryPlant_AllowTypes.size() > 0 || pTypeExt->FactoryPlant_DisallowTypes.size() > 0)
	{
		auto const pHouseExt = HouseExt::ExtMap.Find(pThis->Owner);
		pHouseExt->RestrictedFactoryPlants.push_back(pThis);

		return Skip;
	}

	return 0;
}

DEFINE_HOOK(0x448A31, BuildingClass_Captured_FactoryPlant1, 0x6)
{
	enum { Skip = 0x448A78 };

	GET(BuildingClass*, pThis, ESI);

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->FactoryPlant_AllowTypes.size() > 0 || pTypeExt->FactoryPlant_DisallowTypes.size() > 0)
	{
		auto const pHouseExt = HouseExt::ExtMap.Find(pThis->Owner);

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
	GET(HouseClass*, pNewOwner, EBP);

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->FactoryPlant_AllowTypes.size() > 0 || pTypeExt->FactoryPlant_DisallowTypes.size() > 0)
	{
		auto const pHouseExt = HouseExt::ExtMap.Find(pNewOwner);
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
		if (auto pCell = map->TryGetCellAt(cell))
		{
			pCell->RecalcAttributes(DWORD(-1));

			if constexpr (remove)
				map->ResetZones(cell);
			else
				map->RecalculateZones(cell);

			map->RecalculateSubZones(cell);

		}
	}
}

DEFINE_HOOK(0x440D01, BuildingClass_Unlimbo_DestroyableObstacle, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->IsDestroyableObstacle)
		RecalculateCells(pThis);

	return 0;
}

DEFINE_HOOK(0x445D87, BuildingClass_Limbo_DestroyableObstacle, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->IsDestroyableObstacle)
		RecalculateCells<true>(pThis);

	return 0;
}

DEFINE_HOOK(0x483D8E, CellClass_CheckPassability_DestroyableObstacle, 0x6)
{
	enum { IsBlockage = 0x483CD4 };

	GET(BuildingClass*, pBuilding, ESI);

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

	if (pTypeExt->IsDestroyableObstacle)
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
		auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

		if (pTypeExt->Units_RepairRate.isset())
		{
			double repairRate = pTypeExt->Units_RepairRate.Get();

			if (repairRate < 0.0)
				return 0;

			int rate = static_cast<int>(Math::max(repairRate * 900, 1));

			if (!(Unsorted::CurrentFrame % rate))
			{
				UnitRepairTemp::SeparateRepair = true;

				for (auto i = 0; i < pThis->RadioLinks.Capacity; ++i)
				{
					if (auto const pLink = pThis->GetNthLink(i))
					{
						if (!pLink->IsInAir() && pThis->SendCommand(RadioCommand::QueryMoving, pLink) == RadioCommand::AnswerPositive)
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

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
	double repairRate = pTypeExt->Units_RepairRate.Get(RulesClass::Instance->IRepairRate);
	__asm { fld repairRate }

	return SkipGameCode;
}

DEFINE_HOOK(0x44BD38, BuildingClass_Mission_Repair_UnitRepair, 0x6)
{
	enum { SkipGameCode = 0x44BD3E };

	GET(BuildingClass*, pThis, EBP);

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
	double repairRate = pTypeExt->Units_RepairRate.Get(RulesClass::Instance->URepairRate);
	__asm { fld repairRate }

	return SkipGameCode;
}

DEFINE_HOOK(0x6F4D1A, TechnoClass_ReceiveCommand_Repair, 0x5)
{
	enum { AnswerNegative = 0x6F4CB4 };

	GET(TechnoClass*, pThis, ESI);
	GET(int, repairStep, EAX);
	GET_STACK(TechnoClass*, pFrom, STACK_OFFSET(0x18, 0x4));

	if (auto const pBuilding = abstract_cast<BuildingClass*>(pFrom))
	{
		auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

		if (pBuilding->Type->UnitReload && pTypeExt->Units_RepairRate.isset() && !UnitRepairTemp::SeparateRepair)
			return AnswerNegative;

		repairStep = pTypeExt->Units_RepairStep.Get(repairStep);
		double repairPercent = pTypeExt->Units_RepairPercent.Get(RulesClass::Instance->RepairPercent);
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
	SidebarClass::Instance->SidebarBackgroundNeedsRedraw = true;
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

	if (buildCat == BuildCat::DontCare || all)
	{
		const auto pType = TechnoTypeClass::GetByTypeAndIndex(absType, index);
		const auto firstRemoved = pFactory->RemoveOneFromQueue(pType);

		if (firstRemoved)
		{
			SidebarClass::Instance->SidebarBackgroundNeedsRedraw = true; // Added, force redraw strip
			SidebarClass::Instance->RepaintSidebar(SidebarClass::GetObjectTabIdx(absType, index, 0));

			if (all)
				while (pFactory->RemoveOneFromQueue(pType));
			else
				return Return;
		}

		return CheckSame;
	}

	if (!pFactory->Object)
		return SkipCheck;

	if (!pFactory->RemoveOneFromQueue(TechnoTypeClass::GetByTypeAndIndex(absType, index)))
		return CheckSame;

	SidebarClass::Instance->SidebarBackgroundNeedsRedraw = true; // Added, force redraw strip
	SidebarClass::Instance->RepaintSidebar(SidebarClass::GetObjectTabIdx(absType, index, 0));

	return Return;
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
	GET_STACK(bool, clicked, STACK_OFFSET(0x48C, -0x475));

	return (!RulesExt::Global()->BuildingProductionQueue && pType->WhatAmI() == AbstractType::BuildingType && clicked) ? SkipGameCode : ContinueCheck;
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

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->BarracksExitCell.isset())
	{
		Point2D offset = pTypeExt->BarracksExitCell.Get();
		auto exitCell = pThis->GetMapCoords();
		exitCell.X += (short)offset.X;
		exitCell.Y += (short)offset.Y;

		if (MapClass::Instance->CoordinatesLegal(exitCell))
		{
			GET(TechnoClass*, pTechno, ESI);
			auto const pCell = MapClass::Instance->GetCellAt(exitCell);

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
	GET(int, xCoord, EBP);
	GET(int, yCoord, EDX);
	REF_STACK(CoordStruct, resultCoords, STACK_OFFSET(0x140, -0x108));

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->BarracksExitCell.isset())
	{
		auto const exitCoords = pThis->Type->ExitCoord;
		resultCoords = CoordStruct{ xCoord + exitCoords.X, yCoord + exitCoords.Y, exitCoords.Z };
		return SkipGameCode;
	}

	return 0;
}

#pragma endregion

