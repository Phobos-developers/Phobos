#include <Utilities/AresHelper.h>
#include <Utilities/AresFunctions.h>
#include <Utilities/Helpers.Alex.h>

#include <Ext/Aircraft/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Sidebar/Body.h>
#include <Ext/EBolt/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Rules/Body.h>

#include <New/Entity/Ares/RadarJammerClass.h>
#include <Utilities/AresFunctions.h>

// Remember that we still don't fix Ares "issues" a priori. Extensions as well.
// Patches presented here are exceptions rather that the rule. They must be short, concise and correct.
// DO NOT POLLUTE ISSUEs and PRs.

static ObjectClass* __fastcall CreateInitialPayload(TechnoTypeClass* type, void*, HouseClass* owner)
{
	// temporarily reset the mutex since it's not part of the design
	const int mutex_old = std::exchange(Unsorted::ScenarioInit, 0);
	const auto instance = type->CreateObject(owner);
	Unsorted::ScenarioInit = mutex_old;
	return instance;
}

static void __fastcall InitialPayloadFix(TechnoClass* pThis)
{
	pThis->IsInPlayfield = true;
	ScenarioExt::Global()->RegisterAutoDeath(pThis);
	pThis->Limbo();
}

static void __fastcall InitialPayloadFix_Building(FootClass* pPassenger)
{
	ScenarioExt::Global()->RegisterAutoDeath(pPassenger);
	pPassenger->AbortMotion();
}

static void __fastcall UpdateThreatInCell_InitOccupant(TechnoClass* pBuilding, void*, CellClass* pCell)
{
	pBuilding->UpdateThreatInCell(pCell);

	if (auto* pBld = abstract_cast<BuildingClass*>(pBuilding))
	{
		const int count = pBld->GetOccupantCount();
		if (count > 0)
		{
			FootClass* pLast = pBld->Occupants.GetItem(count - 1);
			if (pLast)
				ScenarioExt::Global()->RegisterAutoDeath(pLast);
		}
	}
}

static void __fastcall LetGo(TemporalClass* pTemporal)
{
	pTemporal->LetGo();
}

static bool __stdcall ConvertToType(TechnoClass* pThis, TechnoTypeClass* pToType)
{
	if (const auto pFoot = abstract_cast<FootClass*, true>(pThis))
		return TechnoExt::ConvertToType(pFoot, pToType);

	return false;
}

// Technically this replaces GetTechnoType() call.
static TechnoTypeClass* __fastcall ShowPromoteAnim(TechnoClass* pThis)
{
	TechnoExt::ShowPromoteAnim(pThis);

	return pThis->GetTechnoType();
}

static WeaponStruct* __fastcall GetLaserWeapon(BuildingClass* pThis)
{
	return BuildingExt::GetLaserWeapon(pThis);
}

static EBolt* __stdcall CreateEBolt(WeaponTypeClass** pWeaponData)
{
	return EBoltExt::CreateEBolt(*pWeaponData);
}

static EBolt* __stdcall CreateEBolt2(WeaponTypeClass* pWeapon)
{
	return EBoltExt::CreateEBolt(pWeapon);
}

static bool __fastcall CameoIsVeteran(TechnoTypeClass** pTypeExt_Ares, void*, HouseClass* pHouse)
{
	return TechnoTypeExt::Fetch(*pTypeExt_Ares)->CameoIsVeteran(pHouse);
}

static bool __fastcall SW_IsAvailable(SuperWeaponTypeClass** pExt_Ares, void*, HouseClass* pHouse)
{
	return SWTypeExt::Fetch(*pExt_Ares)->IsAvailable(pHouse);
}

namespace PermaMCTemp
{
	WarheadTypeClass* Warhead = nullptr;
	bool Selected = false;
}

static bool __fastcall ApplyPermaMC_Wrapper(WarheadTypeClass** pExt_Ares, void*, HouseClass* pSourceHouse, AbstractClass* pTarget)
{
	PermaMCTemp::Warhead = *pExt_Ares;
	const bool result = AresFunctions::ApplyPermaMC(pExt_Ares, pSourceHouse, pTarget);
	PermaMCTemp::Warhead = nullptr;
	return result;
}

static bool __fastcall PermaMC_FreeUnit_SetContext(CaptureManagerClass* pManager, void*, TechnoClass* pTechno)
{
	PermaMCTemp::Selected = pTechno->IsSelected;
	return CaptureManagerExt::FreeUnit(pManager, pTechno, WarheadTypeExt::Fetch(PermaMCTemp::Warhead)->RemoveMindControl_Silent.Get(RulesExt::Global()->MindControl_Permanent_ReplaceSilent));
}

static bool __fastcall PermaMC_SetOwningHouse_Select(TechnoClass* pTechno, void*, HouseClass* pHouse, bool announce)
{
	const bool result = pTechno->SetOwningHouse(pHouse, announce);

	if (std::exchange(PermaMCTemp::Selected, false) && pTechno->Owner->IsCurrentPlayer())
	{
		const bool moveFeedBack = std::exchange(Unsorted::MoveFeedback, false);
		pTechno->Select();
		Unsorted::MoveFeedback = moveFeedBack;
	}

	return result;
}

namespace UnitDeliveryTemp
{
	bool Placing = false;
}

static void __fastcall UnitDeliveryStateMachine_Update_Wrapper(void* pThis)
{
	UnitDeliveryTemp::Placing = true;
	AresFunctions::UnitDeliveryStateMachine_Update(pThis);
	UnitDeliveryTemp::Placing = false;
}

#pragma region AresParadrop

namespace ParadropTemp
{
	AircraftTypeClass* pPlaneType = nullptr;
	CellClass* pDestination = nullptr;
}

static void SendPDPlane(HouseClass* pOwner, CellClass* pDestination, AircraftTypeClass* pPlaneType, Iterator<TechnoTypeClass*> Types, Iterator<int> Nums)
{
	ParadropTemp::pPlaneType = pPlaneType;
	ParadropTemp::pDestination = pDestination;
	AresFunctions::SendPDPlane(pOwner, pDestination, pPlaneType, Types, Nums);
}

static CellStruct* __fastcall ParadropPickCellOnEdge(MapClass* pThis, void* _, CellStruct& buffer, Edge edge,
	const CellStruct& waypointCell, const CellStruct& fallbackCell, SpeedType speedType, bool validate, MovementZone mZone)
{
	buffer = AircraftExt::PickEdgeCellForPlane(ParadropTemp::pPlaneType, ParadropTemp::pDestination->MapCoords, edge);
	return &buffer;
}

static bool __fastcall ParadropPlaneUnlimbo(AircraftClass* pThis, void* _, const CoordStruct& coords, DirType direction)
{
	return AircraftExt::PlaceReinforcementAircraft(pThis, coords);
}

#pragma endregion

#pragma region AresKeepAlive

struct AresHouseExt
{
	char _[0x18];
	int KeepAliveTechnos;
	int KeepAliveBuildings;
};

static bool __fastcall AresHouseExt_UpdateKeepAlive(AresHouseExt* pExt_Ares, void*, TechnoClass* const pTechno, const AbstractType rtti, const bool add)
{
	bool keepAlive = false;
	bool result = false;
	auto const pType = pTechno->GetTechnoType(); // can't use TypeExtData since it's not initialized here

	if (!pType->Insignificant && !pType->DontScore)
	{
		switch (rtti)
		{
		case AbstractType::Infantry:
		{
			keepAlive = RulesExt::Global()->KeepAlive_Infantry;
			break;
		}
		case AbstractType::Unit:
		{
			keepAlive = RulesExt::Global()->KeepAlive_Units;
			break;
		}
		case AbstractType::Aircraft:
		{
			keepAlive = RulesExt::Global()->KeepAlive_Aircraft;
			break;
		}
		case AbstractType::Building:
		{
			auto const pBuildingType = static_cast<BuildingTypeClass*>(pType);

			if (pBuildingType->BuildCat == BuildCat::Combat)
				keepAlive = RulesExt::Global()->KeepAlive_Defenses;
			else
				keepAlive = RulesExt::Global()->KeepAlive_Buildings;

			break;
		}
		default:
		{
			break;
		}
		}

		result = true;
	}

	if (TechnoTypeExt::Fetch(pType)->KeepAlive.Get(keepAlive))
	{
		const int number = add ? 1 : -1;
		pExt_Ares->KeepAliveTechnos += number;

		if (rtti == AbstractType::Building)
			pExt_Ares->KeepAliveBuildings += number;
	}

	return result;
}

#pragma endregion

DEFINE_HOOK(0x440580, BuildingClass_Unlimbo_UnitDeliveryFix, 0x5)
{
	if (UnitDeliveryTemp::Placing)
		R->Stack(0x8, DirType::North);

	return 0;
}

static AresSWTargetResult* __stdcall PickSuperWeaponTarget(AresSWTargetResult* result, AresSWTargetInfo* info)
{
	SWTypeExt::CurrentAIEvaluatedSW = info->SW;
	auto const aiTargetingType = *(SuperWeaponAITargetingMode*)((char*)info->Ext + 0x210); // Ares' SW.AITargetingType

	if (SWTypeExt::HandleAITargetingOverrides(info->SW, aiTargetingType, result->TargetCell, result->WasSuccessful))
		return result;

	result = AresFunctions::PickSuperWeaponTarget(result, info);
	SWTypeExt::CurrentAIEvaluatedSW = nullptr;
	return result;
}

static bool __fastcall CanBePermaMindControlled(TechnoClass* pTechno)
{
	return SWTypeExt::EligibleTargetForPsyDomSW(pTechno);
}

_GET_FUNCTION_ADDRESS(RadarJammerClass::Update, AresRadarJammerClass_Update_GetAddr)

static DWORD _cdecl AresPreventScatter_Override(REGISTERS* R)
{
	GET(FootClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, 0xD0);

	if (!WarheadTypeExt::Fetch(pWarhead)->PreventScatter.Get(RulesExt::Global()->Warhead_PreventScatter))
		pThis->Scatter(CoordStruct::Empty, true, false);

	return 0x702D11;
}

void Apply_Ares3_0_Patches()
{
	// Abductor fix:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x54CDF, AresHelper::AresBaseAddress + 0x54D3C);

	// Amphibious enter fix:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x17536, AresHelper::AresBaseAddress + 0x1754D);

	// SpawnSurvivor fix:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x445E0, GET_OFFSET(TechnoExt::EjectRandomly));

	// KillDriver re-implementation and enhancement
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x456D0, GET_OFFSET(TechnoExt::ApplyKillDriver));

	// Redirect Ares' getCellSpreadItems to our implementation:
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x62267, &Helpers::Alex::getCellSpreadItems);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x528C8, &Helpers::Alex::getCellSpreadItems);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x5273A, &Helpers::Alex::getCellSpreadItems);

	// Redirect Ares's RemoveCameo to our implementation:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x02BDD0, GET_OFFSET(SidebarExt::AresTabCameo_RemoveCameo));

	// Remove Ares' WhatAmI() != AbstractType::Infantry check.
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x491B8, AresHelper::AresBaseAddress + 0x491C4);
	// InitialPayload creation:
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x43D5D, &CreateInitialPayload);
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x43E4F, GET_OFFSET(InitialPayloadFix));
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x43DBC, &UpdateThreatInCell_InitOccupant);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x43E33, &InitialPayloadFix_Building);

	// Replace the TemporalClass::Detach call by LetGo in convert function:
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x436DA, &LetGo);

	// SuperClass_Launch_SkipRelatedTags:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x3207C, AresHelper::AresBaseAddress + 0x320DF);

	// Convert ManagerFix:
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x039DAE, &ConvertToType);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x046C6D, &ConvertToType);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x04B397, &ConvertToType);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x04C099, &ConvertToType);

	// EBolt reimpl:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x550A0, GET_OFFSET(CreateEBolt));
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x550F0, GET_OFFSET(CreateEBolt2));
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x561F0, GET_OFFSET(EBoltExt::_EBolt_Draw_Colors));

	// Unit simple deployer fix:
	Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x4C0C6, { 0x5E }); // pop esi
	Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x4C0C7, { 0x33, 0xC0 }); // xor eax, eax
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x4C0A9, AresHelper::AresBaseAddress + 0x4C0C6);

	// Skip DeployDir parsing on Ares side cause we reimplement it and Ares' parser whines about -1.
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x3F38A, AresHelper::AresBaseAddress + 0x3F3A0);

	// Handle promote animations within Ares code if Ares is available.
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x46B44, &ShowPromoteAnim);

	// Apply laser weapon selection fix on Ares' laser fire replacement.
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x56415, &GetLaserWeapon);

	// Redirect Ares's RadarJammerClass::Update to our implementation
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x68500, AresRadarJammerClass_Update_GetAddr());

	// Redirect Ares's function to our implementation:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x112D0, &BuildingExt::KickOutClone);

	// Redirect Ares's TechnoTypeExt::CameoIsElite() to our implementation:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x3D800, &CameoIsVeteran);

	// Redirect Ares's SWTypeExt::IsAvailable to our implementation:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x32BE0, &SW_IsAvailable);
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x329E0, &SWTypeExt::IsSuperAvailable);

	// Remove Ares check for houses for Psychedelic=yes Warheads.
	Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x4AAAA, { 0x31, 0xC0, 0x90, 0x90, 0x90, 0x90 });

	// Get warhead of MindControl.Permanent
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x5385A, &ApplyPermaMC_Wrapper);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x717C3, &ApplyPermaMC_Wrapper);

	// Handle select of PsyDom
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x36107, &PermaMC_FreeUnit_SetContext);
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x36115, &PermaMC_SetOwningHouse_Select);
	// Handle select of MindControl.Permanent
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x45EAF, &PermaMC_FreeUnit_SetContext);
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x45EBE, &PermaMC_SetOwningHouse_Select);

	// Fix building direction of Ares's UnitDelivery
	Patch::Apply_VTABLE(AresHelper::AresBaseAddress + 0xA8D94, &UnitDeliveryStateMachine_Update_Wrapper);

	// Skip Ares' ProjectileRange handling - our replacement hooked at 0x467BA4 (BulletClass_AI_Ranged).
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x1ACA3, AresHelper::AresBaseAddress + 0x1AD20);

	// Replace Ares paradrop plane send function call with our wrapper.
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x745B8, &SendPDPlane);

	// Replace Ares paradrop plane edge cell picker with our wrapper.
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x74242, &ParadropPickCellOnEdge);

	// Replace Ares paradrop plane Unlimbo call with our wrapper.
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x742AC, &ParadropPlaneUnlimbo);

	// Replace Ares factory update logic with our wrapper.
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x13FA7, &BuildingExt::UpdateFactoryQueues);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x4CD9E, &BuildingExt::UpdateFactoryQueues);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x4CE84, &BuildingExt::UpdateFactoryQueues);

	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x4ADE0, GET_OFFSET(AresPreventScatter_Override));

	// Decouple SW.ShowCameo from SW.AutoFire - Ares' HouseClass_UpdateSuperWeaponsUnavailable
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x39635, AresHelper::AresBaseAddress + 0x39664);
	// Decouple SW.ManualFire from SW.AutoFire - Ares' SidebarClass_ProcessCameoClick_SuperWeapons
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x34DD0, AresHelper::AresBaseAddress + 0x34DD9);

	// Ares' `KeepAlive` adds global tags.
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x21F70, GET_OFFSET(AresHouseExt_UpdateKeepAlive));

	// Redirect Ares' SW target picker to our wrapper.
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x38356, &PickSuperWeaponTarget);

	// Redirect Ares' Psychic Dominator target evaluation checks to our wrapper.
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x38177, &CanBePermaMindControlled);

	// Skip checking house alliances in Ares' Psychic Dominator target evaluation checks, check them elsewhere later.
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x38137, AresHelper::AresBaseAddress + 0x38175);
}

void Apply_Ares3_0p1_Patches()
{
	// Abductor fix:
	// Issue: moving vehicles leave permanent occupation stats on terrain
	// What's done here: Skip Mark_Occupation_Bits cuz pFoot->Remove/Limbo() will do it.
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x5598F, AresHelper::AresBaseAddress + 0x559EC);

	// Amphibious enter fix:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x17C26, AresHelper::AresBaseAddress + 0x17C3D);

	// SpawnSurvivor fix:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x450C0, GET_OFFSET(TechnoExt::EjectRandomly));

	// KillDriver re-implementation and enhancement
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x46240, GET_OFFSET(TechnoExt::ApplyKillDriver));

	// Redirect Ares' getCellSpreadItems to our implementation:
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x62FB7, &Helpers::Alex::getCellSpreadItems);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x53578, &Helpers::Alex::getCellSpreadItems);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x533EA, &Helpers::Alex::getCellSpreadItems);

	// Redirect Ares's RemoveCameo to our implementation:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x02C910, GET_OFFSET(SidebarExt::AresTabCameo_RemoveCameo));

	// Remove Ares' WhatAmI() != AbstractType::Infantry check.
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x49E08, AresHelper::AresBaseAddress + 0x49E14);
	// InitialPayload creation:
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x4483D, &CreateInitialPayload);
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x4492F, GET_OFFSET(InitialPayloadFix));
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x4489C, &UpdateThreatInCell_InitOccupant);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x44913, &InitialPayloadFix_Building);

	// Replace the TemporalClass::Detach call by LetGo in convert function:
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x441BA, &LetGo);

	// SuperClass_Launch_SkipRelatedTags:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x32A5C, AresHelper::AresBaseAddress + 0x32ABF);

	// Convert ManagerFix:
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x3A82E, &ConvertToType);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x4780D, &ConvertToType);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x4BFF7, &ConvertToType);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x4CCF9, &ConvertToType);

	// EBolt reimpl:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x55D50, GET_OFFSET(CreateEBolt));
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x55DA0, GET_OFFSET(CreateEBolt2));
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x56EA0, GET_OFFSET(EBoltExt::_EBolt_Draw_Colors));

	// Unit simple deployer fix:
	Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x4CD26, { 0x5E }); // pop esi
	Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x4CD27, { 0x33, 0xC0 }); // xor eax, eax
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x4CD09, AresHelper::AresBaseAddress + 0x4CD26);

	// Skip DeployDir parsing on Ares side cause we reimplement it and Ares' parser whines about -1.
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x3FFEA, AresHelper::AresBaseAddress + 0x40000);

	// Handle promote animations within Ares code if Ares is available.
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x476E4, &ShowPromoteAnim);

	// Apply laser weapon selection fix on Ares' laser fire replacement.
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x570C5, &GetLaserWeapon);

	// Redirect Ares's RadarJammerClass::Update to our implementation
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x69470, AresRadarJammerClass_Update_GetAddr());

	// Redirect Ares's function to our implementation:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x11860, &BuildingExt::KickOutClone);

	// Redirect Ares's TechnoTypeExt::CameoIsElite() to our implementation:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x3E210, &CameoIsVeteran);

	// Redirect Ares's SWTypeExt::IsAvailable to our implementation:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x335E0, &SW_IsAvailable);
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x333E0, &SWTypeExt::IsSuperAvailable);

	// Remove Ares check for houses for Psychedelic=yes Warheads.
	Patch::Apply_RAW(AresHelper::AresBaseAddress + 0x4B70A, { 0x31, 0xC0, 0x90, 0x90, 0x90, 0x90 });

	// Get warhead of MindControl.Permanent
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x5450A, &ApplyPermaMC_Wrapper);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x727E3, &ApplyPermaMC_Wrapper);

	// Handle select of PsyDom
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x36BA7, &PermaMC_FreeUnit_SetContext);
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x36BB5, &PermaMC_SetOwningHouse_Select);
	// Handle select of MindControl.Permanent
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x46A1F, &PermaMC_FreeUnit_SetContext);
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x46A2E, &PermaMC_SetOwningHouse_Select);

	// Fix building direction of Ares's UnitDelivery
	Patch::Apply_VTABLE(AresHelper::AresBaseAddress + 0xA9F28, &UnitDeliveryStateMachine_Update_Wrapper);

	// Skip Ares' ProjectileRange handling - our replacement hooked at 0x467BA4 (BulletClass_AI_Ranged).
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x1B393, AresHelper::AresBaseAddress + 0x1B410);

	// Replace Ares paradrop plane send function call with our wrapper.
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x75668, &SendPDPlane);

	// Replace Ares paradrop plane edge cell picker with our wrapper.
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x752F2, &ParadropPickCellOnEdge);

	// Replace Ares paradrop plane Unlimbo call with our wrapper.
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x7535C, &ParadropPlaneUnlimbo);

	// Replace Ares factory update logic with our wrapper.
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x14537, &BuildingExt::UpdateFactoryQueues);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x4DA0E, &BuildingExt::UpdateFactoryQueues);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x4DAF4, &BuildingExt::UpdateFactoryQueues);

	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x4BA40, GET_OFFSET(AresPreventScatter_Override));

	// Decouple SW.ShowCameo from SW.AutoFire - Ares' HouseClass_UpdateSuperWeaponsUnavailable
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x3A0B5, AresHelper::AresBaseAddress + 0x3A0E4);
	// Decouple SW.ManualFire from SW.AutoFire - Ares' SidebarClass_ProcessCameoClick_SuperWeapons
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x35810, AresHelper::AresBaseAddress + 0x35819);

	// Ares' `KeepAlive` adds global tags.
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x229F0, GET_OFFSET(AresHouseExt_UpdateKeepAlive));

	// Redirect Ares' SW target picker to our wrapper.
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x38DF6, &PickSuperWeaponTarget);

	// Redirect Ares' Psychic Dominator target evaluation checks to our wrapper.
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x38C17, &CanBePermaMindControlled);

	// Skip checks in Ares' Psychic Dominator target evaluation checks, check them elsewhere later.
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x38BD7, AresHelper::AresBaseAddress + 0x38C15);
}
