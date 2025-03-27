#include <BuildingClass.h>
#include <FootClass.h>

#include <Utilities/Macro.h>
#include <Utilities/AresHelper.h>
#include <Utilities/Helpers.Alex.h>

#include <Ext/Sidebar/Body.h>

// Remember that we still don't fix Ares "issues" a priori. Extensions as well.
// Patches presented here are exceptions rather that the rule. They must be short, concise and correct.
// DO NOT POLLUTE ISSUEs and PRs.

ObjectClass* __fastcall CreateInitialPayload(TechnoTypeClass* type, void*, HouseClass* owner)
{
	// temporarily reset the mutex since it's not part of the design
	int mutex_old = std::exchange(Unsorted::ScenarioInit, 0);
	auto instance = type->CreateObject(owner);
	Unsorted::ScenarioInit = mutex_old;
	return instance;
}

void __fastcall LetGo(TemporalClass* pTemporal)
{
	pTemporal->LetGo();
}

void Apply_Ares3_0_Patches()
{
	// Abductor fix:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x54CDF, AresHelper::AresBaseAddress + 0x54D3C);

	// Redirect Ares' getCellSpreadItems to our implementation:
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x62267, &Helpers::Alex::getCellSpreadItems);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x528C8, &Helpers::Alex::getCellSpreadItems);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x5273A, &Helpers::Alex::getCellSpreadItems);

	// Redirect Ares's RemoveCameo to our implementation:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x02BDD0, GET_OFFSET(SidebarExt::AresTabCameo_RemoveCameo));

	// InitialPayload creation:
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x43D5D, &CreateInitialPayload);

	// Replace the TemporalClass::Detach call by LetGo in convert function:
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x436DA, &LetGo);
}

void Apply_Ares3_0p1_Patches()
{
	// Abductor fix:
	// Issue: moving vehicles leave permanent occupation stats on terrain
	// What's done here: Skip Mark_Occupation_Bits cuz pFoot->Remove/Limbo() will do it.
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x5598F, AresHelper::AresBaseAddress + 0x559EC);

	// Redirect Ares' getCellSpreadItems to our implementation:
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x62FB7, &Helpers::Alex::getCellSpreadItems);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x53578, &Helpers::Alex::getCellSpreadItems);
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x533EA, &Helpers::Alex::getCellSpreadItems);

	// Redirect Ares's RemoveCameo to our implementation:
	Patch::Apply_LJMP(AresHelper::AresBaseAddress + 0x02C910, GET_OFFSET(SidebarExt::AresTabCameo_RemoveCameo));

	// InitialPayload creation:
	Patch::Apply_CALL6(AresHelper::AresBaseAddress + 0x4483D, &CreateInitialPayload);

	// Replace the TemporalClass::Detach call by LetGo in convert function:
	Patch::Apply_CALL(AresHelper::AresBaseAddress + 0x441BA, &LetGo);
}
