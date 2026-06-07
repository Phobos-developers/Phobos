#include <Utilities/AresHelper.h>
#include <Utilities/AresFunctions.h>
#include <Utilities/Helpers.Alex.h>

#include <Ext/Building/Body.h>
#include <Ext/Sidebar/Body.h>
#include <Ext/EBolt/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/CaptureManager/Body.h>

#include <New/Entity/Ares/RadarJammerClass.h>

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

static void __fastcall InitialPayload_OpenToppedFix(TechnoClass* pThis)
{
	pThis->IsInPlayfield = true;
	pThis->Limbo();
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
	return TechnoTypeExt::ExtMap.Find(*pTypeExt_Ares)->CameoIsVeteran(pHouse);
}

static bool __fastcall SW_IsAvailable(SuperWeaponTypeClass** pExt_Ares, void*, HouseClass* pHouse)
{
	return SWTypeExt::ExtMap.Find(*pExt_Ares)->IsAvailable(pHouse);
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
	return CaptureManagerExt::FreeUnit(pManager, pTechno, WarheadTypeExt::ExtMap.Find(PermaMCTemp::Warhead)->RemoveMindControl_Silent.Get(RulesExt::Global()->MindControl_Permanent_ReplaceSilent));
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

DEFINE_HOOK(0x440580, BuildingClass_Unlimbo_UnitDeliveryFix, 0x5)
{
	if (UnitDeliveryTemp::Placing)
		R->Stack(0x8, DirType::North);

	return 0;
}

_GET_FUNCTION_ADDRESS(RadarJammerClass::Update, AresRadarJammerClass_Update_GetAddr)

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

	// InitialPayload creation:
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x43D5D, &CreateInitialPayload);
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x43E4F, GET_OFFSET(InitialPayload_OpenToppedFix));

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

	// Redirect Ares's TechnoTypeExt::ExtData::CameoIsElite() to our implementation:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x3D800, &CameoIsVeteran);

	// Redirect Ares's SWTypeExt::ExtData::IsAvailable to our implementation:
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

	// InitialPayload creation:
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x4483D, &CreateInitialPayload);
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x4492F, GET_OFFSET(InitialPayload_OpenToppedFix));

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

	// Redirect Ares's TechnoTypeExt::ExtData::CameoIsElite() to our implementation:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x3E210, &CameoIsVeteran);

	// Redirect Ares's SWTypeExt::ExtData::IsAvailable to our implementation:
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
}
