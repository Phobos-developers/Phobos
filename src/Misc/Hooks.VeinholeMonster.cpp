#include <Ext/Anim/Body.h>

#include <GameOptionsClass.h>
#include <IonBlastClass.h>
#include <OverlayTypeClass.h>
#include <ScenarioClass.h>
#include <UnitClass.h>
#include <VeinholeMonsterClass.h>


///
/// Veinhole Monster
///

// Loads the veinhole monster art
// Call removed from YR by WW
DEFINE_HOOK(0x4AD097, DisplayClass_ReadIni_LoadVeinholeArt, 0x6)
{
	int theater = static_cast<int>(ScenarioClass::Instance->Theater);
	VeinholeMonsterClass::LoadVeinholeArt(theater);

	return 0;
}

// Applies damage to the veinhole monster
DEFINE_HOOK(0x489671, Damage_at_Cell_Update_Veinhole, 0x6)
{
	GET(OverlayTypeClass*, pOverlay, EAX);
	GET(WarheadTypeClass*, pWH, ESI);
	GET_STACK(CellStruct, pCell, STACK_OFFSET(0xE0, -0x4C));
	GET_STACK(int, damage, STACK_OFFSET(0xE0, -0xBC));
	GET_STACK(ObjectClass*, pAttacker, STACK_OFFSET(0xE0, 0x8));
	GET_STACK(HouseClass*, pAttackingHouse, STACK_OFFSET(0xE0, 0x14));

	if (pOverlay->IsVeinholeMonster)
	{
		if (VeinholeMonsterClass* pVeinhole = VeinholeMonsterClass::GetVeinholeMonsterFrom(&pCell))
			pVeinhole->ReceiveDamage(&damage, 0, pWH, pAttacker, false, false, pAttackingHouse);
	}

	return 0;
}

DEFINE_HOOK(0x6D4656, TacticalClass_Draw_Veinhole, 0x5)
{
	enum { ContinueDraw = 0x6D465B };

	VeinholeMonsterClass::DrawAll();
	IonBlastClass::DrawAll();

	return ContinueDraw;
}

DEFINE_HOOK(0x5349A5, Map_ClearVectors_Veinhole, 0x5)
{
	VeinholeMonsterClass::DeleteAll();
	VeinholeMonsterClass::DeleteVeinholeGrowthData();
	return 0;
}

// DEFINE_HOOK(0x55B4E1, LogicClass_Update_Veinhole, 0x5) // Goto ScenarioExt

// Handles the veins' attack animation
DEFINE_HOOK(0x4243BC, AnimClass_Update_VeinholeAttack, 0x6)
{
	GET(AnimClass*, pAnim, ESI);

	if (pAnim->Type->IsVeins)
		AnimExt::VeinAttackAI(pAnim);

	return 0;
}

///
/// Weeder
///

// These 2 I am not sure, maybe they have smth to do with AI, maybe they are for the unit queue at the refinery
DEFINE_HOOK(0x736823, UnitClass_Update_WeederMissionMove, 0x6)
{
	enum
	{
		Continue = 0x736831,
		Skip = 0x736981
	};

	GET(UnitTypeClass*, pUnitType, EAX);

	if (pUnitType->Harvester || pUnitType->Weeder)
		return Continue;

	return Skip;
}

DEFINE_HOOK(0x7368C6, UnitClass_Update_WeederMissionMove2, 0x6)
{
	enum
	{
		Continue = 0x7368D4,
		Skip = 0x736981
	};

	GET(BuildingTypeClass*, pBuildingType, EDX);

	if (pBuildingType->Refinery || pBuildingType->Weeder)
		return Continue;

	return Skip;
}

// Not sure if necessary
/*
// These 2 have something to do with ZAdjustment when unloading
DEFINE_HOOK(0x7043E7, TechnoClass_Get_ZAdjustment_Weeder, 0x6)
{
	enum
	{
		Continue = 0x7043F1,
		Skip = 0x704421
	};

	GET(UnitTypeClass*, pUnitType, ECX);

	if (pUnitType->Harvester || pUnitType->Weeder)
		return Continue;

	return Skip;
}

DEFINE_HOOK(0x70440C, TechnoClass_Get_ZAdjustment_Weeder2, 0x6)
{
	enum
	{
		Continue = 0x704416,
		Skip = 0x704421
	};

	GET(BuildingTypeClass*, pBuildingType, EAX);

	if (pBuildingType->Refinery || pBuildingType->Weeder)
		return Continue;

	return Skip;
}

DEFINE_HOOK(0x741C32, UnitClass_SetDestination_SpecialAnim_Weeder, 0x6)
{
	enum
	{
		CheckSpecialAnimExists = 0x741C3C,
		Skip = 0x741C4F
	};

	GET(UnitTypeClass*, pUnitType, ECX);

	if (pUnitType->Harvester || pUnitType->Weeder)
		return CheckSpecialAnimExists;

	return Skip;
}
*/

DEFINE_HOOK(0x73D0DB, UnitClass_DrawAt_Weeder_Oregath, 0x6)
{
	enum
	{
		DrawOregath = 0x73D0E9,
		Skip = 0x73D298
	};

	GET(UnitClass*, pUnit, ESI);

	if (pUnit->Type->Harvester || pUnit->Type->Weeder || pUnit->IsHarvesting)
		return DrawOregath;

	return Skip;
}
/*
DEFINE_HOOK(0x73D2A6, UnitClass_DrawAt_Weeder_UnloadingClass, 0x6)
{
	enum
	{
		ShowUnloadingClass = 0x73D2B0,
		Skip = 0x73D2CA
	};

	GET(UnitTypeClass*, pUnitType, EAX);

	if (pUnitType->Harvester || pUnitType->Weeder)
		return ShowUnloadingClass;

	return Skip;
}
*/

// Enables the weeder to harvest veins
DEFINE_HOOK(0x73D49E, UnitClass_Harvesting_Weeder, 0x7)
{
	enum
	{
		Harvest = 0x73D4DA,
		Skip = 0x73D5FE
	};

	GET(UnitClass*, pUnit, ESI);
	GET(CellClass*, pCell, EBP);
	constexpr unsigned char weedOverlayData = 0x30;

	bool harvesterCanHarvest = pUnit->Type->Harvester && pCell->LandType == LandType::Tiberium;
	bool weederCanWeed = pUnit->Type->Weeder && pCell->LandType == LandType::Weeds && pCell->OverlayData >= weedOverlayData;


	if ((harvesterCanHarvest || weederCanWeed) && pUnit->GetStoragePercentage() < 1.0)
		return Harvest;

	return Skip;
}

// Not sure if necessary
/*
DEFINE_HOOK(0x73E005, UnitClass_Unload_WeederAnim, 0x6)
{
	enum
	{
		ProceedWithAnim = 0x73E013,
		Skip = 0x73E093
	};

	GET(UnitTypeClass*, pUnitType, ECX);

	if (pUnitType->Harvester || pUnitType->Weeder)
		return ProceedWithAnim;

	return Skip;
}
*/

// This lets the weeder actually enter the waste facility and unload
// WW removed weeders from this check in YR
DEFINE_HOOK(0x43C788, BuildingClass_ReceivedRadioCommand_Weeder_CompleteEnter, 0x6)
{
	enum
	{
		CompleteEnter = 0x43C796,
		Skip = 0x43CE43
	};

	GET(BuildingTypeClass*, pBuildingType, EAX);

	if (pBuildingType->DockUnload || pBuildingType->Weeder)
		return CompleteEnter;

	return Skip;
}

// This assigns the weeder to the "Harvest" mission when it is granted as a free unit
// Ares made the weeder receive the "Guard" command instead
DEFINE_HOOK(0x446EAD, BuildingClass_GrandOpening_FreeWeeder_Mission, 0x6)
{
	GET(UnitClass*, pUnit, EDI);

	if (pUnit->Type->Weeder)
		pUnit->ForceMission(Mission::Harvest);

	pUnit->NextMission();

	return 0x446EB7;
}

// Teleport cooldown for weeders
// DEFINE_HOOK(0x719580, TeleportLocomotion_Weeder, 0x6) //Goto Hooks.Teleport.cpp

// DockUnload bypass for Weeders when teleporting
DEFINE_HOOK(0x7424BD, UnitClass_AssignDestination_Weeder_Teleport, 0x6)
{
	GET(BuildingTypeClass*, pDestination, ECX);

	return pDestination->DockUnload || pDestination->Weeder ? 0x7424CB : 0x7425DB;
}

//// Skip check for Weeder so that weeders go through teleport stuff
//DEFINE_JUMP(LJMP, 0x73E844, 0x73E793)
//
//
//DEFINE_HOOK(0x73E84A, UnitClass_Mission_Harvest, 0x6)
//{
//	GET(UnitClass*, pUnit, EBP);
//
//	bool isOnTiberium;
//	if (pUnit->Type->Weeder)
//		isOnTiberium = pUnit->MoveToWeed(RulesClass::Instance->TiberiumLongScan / Unsorted::LeptonsPerCell);
//	else
//		isOnTiberium = pUnit->MoveToTiberium(RulesClass::Instance->TiberiumLongScan / Unsorted::LeptonsPerCell);
//
//	R->EBX(isOnTiberium);
//	return 0x73E86B;
//}

DEFINE_HOOK(0x73E9A0, UnitClass_Weeder_StopHarvesting, 0x6)
{
	enum
	{
		StopHarvesting = 0x73E9CA,
		Skip = 0x73EA8D
	};

	GET(UnitClass*, pUnit, EBP);

	if ((pUnit->Type->Harvester || pUnit->Type->Weeder) && pUnit->GetStoragePercentage() == 1.0)
	{
		return StopHarvesting;	
	}

	return Skip;
}
