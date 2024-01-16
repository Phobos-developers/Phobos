#include "CustomTheater.h"
#include "RulesClass.h"

#include <Ext/Scenario/Body.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x687631, Read_Scenario_INI, 0x8)
{
	GET(CCINIClass*, pMapINI, EBP);
	ScenarioExt::Global()->CustomTheaterID.Read(pMapINI, "Map", "Theater", TheaterProto::Temperate->ID);
	ScenarioExt::Global()->CustomTheaterID.Read(pMapINI, "Map", "CustomTheater");

	Theater::Init(TheaterType::None);

	if (auto pExtendedRulesINI = CCINIClass::LoadINIFile(CustomTheater::Instance->ExtendedRules))
	{
		RulesClass::Instance->Read_File(pExtendedRulesINI);
		CCINIClass::UnloadINIFile(pExtendedRulesINI);
	}

	return 0x687660;
}

DEFINE_HOOK(0x5349C9, Theater_Init, 0x6)
{
	GET(int, theaterIndex, ECX);
	LEA_STACK(TheaterType*, slot, STACK_OFFSET(0x6C, -0x54));

	// For RMP
	if (ScenarioClass::Instance->Theater <= TheaterType::None && theaterIndex >= 0)
		strcpy_s(ScenarioExt::Global()->CustomTheaterID.data(), TheaterProto::Array[theaterIndex].ID);

	CustomTheater::Instance->Init(ScenarioExt::Global()->CustomTheaterID);

	*slot = ScenarioClass::Instance->Theater;
	R->EAX(FileSystem::LoadFile(CustomTheater::Instance->PaletteOverlay));
	R->EDI(0);
	R->EDX(0);
	return 0x534C09;
}

DEFINE_HOOK(0x534CA9, Init_Theaters_SetPaletteUnit, 0x8)
{
	R->ESI(FileSystem::LoadFile(CustomTheater::Instance->PaletteUnit));
	return 0x534CCA;
}

DEFINE_HOOK(0x54547F, IsometricTileTypeClass_ReadINI_SetPaletteISO, 0x6)
{
	R->ECX<char*>(CustomTheater::Instance->PaletteISO);
	return 0x5454A2;
}

DEFINE_HOOK(0x5454F0, IsometricTileTypeClass_ReadINI_TerrainControl, 0x6)
{
	R->ECX<char*>(CustomTheater::Instance->TerrainControl);
	return 0x545513;
}

// Reinitialize forcibly, regardless of LastTheater value
DEFINE_JUMP(LJMP, 0x6268C7, 0x6268CF) // INIColors_6267A0

// Skip rereading Theater tag, in DisplayClass
DEFINE_JUMP(LJMP, 0x4ACFD8, 0x4ACFF6) // DisplayClass_ReadINI

// To prevent log spam in debug.log when parsing Prerequisite.RequiredTheaters (Ares)
// I changed the return value of the Theater::FindIndex function to -2
// in case the theater is not found in the vanilla Theater::Array
// https://github.com/Ares-Developers/Ares/blob/4f1d929920aca31924c6cd4d3dfa849daa65252a/src/Ext/TechnoType/Body.cpp#L137
DEFINE_PATCH(0x48DC0A, (byte)-2)

// Unload MIXes when process exits. For what ?
// Vanilla game does it, it's good practice to keep this behavior
DEFINE_JUMP(LJMP, 0x6BE633, 0x6BE659)
DEFINE_HOOK(0x6BE5B9, Prog_End, 0x6)
{
	CustomTheater::Instance->UnloadMIXes();
	return 0x6BE607;
}
