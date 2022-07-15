#include <Ext/Scenario/Body.h>
#include <Utilities/Macro.h>

#include "TheaterProto.h"
#include "CustomTheater.h"

//#include <Utilities/Debug.h>

/*
	To prevent log spam in debug.log when parsing Prerequisite.RequiredTheaters (Ares)
	I changed the return value of the Theater::FindIndex function to -2
	in case the theater is not found in the vanilla Theater::Array
*/
DEFINE_PATCH(0x48DC0A, 0xFE)

#pragma region CustomTheater Init

DEFINE_HOOK(0x5997AB, MapGeneratorClass_init, 0)
{
	R->ECX(ScenarioExt::Global()->CustomTheaterID.data());
	return 0x5997C6;
}

DEFINE_HOOK(0x687631, Read_Scenario_INI, 0)
{
	GET(CCINIClass*, pINI, EBP);
	ScenarioExt::Global()->CustomTheaterID.Read(pINI, "Map", "Theater", TheaterProto::Temperate->ID);

	R->EAX(0);
	return 0x687643;
}

DEFINE_HOOK(0x5349C9, Init_Theaters, 6)
{
	GET(int, ID, ECX);

	// For RMP
	if (ScenarioClass::Instance->Theater <= TheaterType::None)
		strcpy_s(ScenarioExt::Global()->CustomTheaterID.data(), TheaterProto::Array[ID].ID);

	CustomTheater::Instance->Init();

	R->EAX(FileSystem::LoadFile(CustomTheater::Instance->PaletteOverlay));
	R->EDI(0);
	R->EDX(0);
	return 0x534C09;
}

DEFINE_HOOK(0x534CA9, Init_Theaters_SetPaletteUnit, 0)
{
	R->ESI(FileSystem::LoadFile(CustomTheater::Instance->PaletteUnit));
	return 0x534CCA;
}

DEFINE_HOOK(0x54547F, IsometricTileTypeClass_ReadINI_SetPaletteISO, 0)
{
	R->ECX(CustomTheater::Instance->PaletteISO);
	return 0x5454A2;
}

DEFINE_HOOK(0x5454F0, IsometricTileTypeClass_ReadINI_SetControlFileName, 0)
{
	R->ECX(CustomTheater::Instance->ControlFileName);
	return 0x545513;
}

DEFINE_JUMP(LJMP, 0x6BE633, 0x6BE659)
DEFINE_HOOK(0x6BE5B9, Prog_End, 0)
{
	CustomTheater::Instance->UnloadMIXes();
	return 0x6BE607;
}

#pragma endregion CustomTheater Init

#pragma region RMG

#include <Randomizer.h>
#include <MapSeedClass.h>

// Overrides the Ares hook RMG_EnableDesert on 0x5970EA
DEFINE_HOOK(0x5970AF, MapSeedClass_SetMenuOptions, 0)
{
	GET(HWND, hWnd, EDI);

	auto AddOption = [hWnd](const char* label, int index)
	{
		auto ListItem = SendMessageA(hWnd, WW_CB_ADDITEM, 0,
			reinterpret_cast<LPARAM>(StringTable::FetchString(label))
		);

		// Set the item data (Theater ID)
		SendMessageA(hWnd, CB_SETITEMDATA, ListItem, index);
	};

	if (true)
	{
		AddOption(TheaterProto::Temperate->UIName, 0);
		AddOption(TheaterProto::Snow->UIName, 1);
		AddOption(TheaterProto::Desert->UIName, 3);
	}
	else
	{
		AddOption("NOSTR:Temperate", 0);
		AddOption("NOSTR:Snow", 1);
		AddOption("NOSTR:Urban", 2);
		AddOption("NOSTR:Desert", 3);
		AddOption("NOSTR:New Urban", 4);
		AddOption("NOSTR:Lunar", 5);
		AddOption("NOSTR:OMEGA", 6);
	}

	SendMessageA(hWnd, CB_SETCURSEL, MapSeedClass::Instance->Theater, 0);

	R->EBP(&MapSeedClass::Instance);
	return 0x59712A;
}

//DEFINE_HOOK(0x5967C1, MapSeedClass_DialogFunc_SurpriseMe, 9)
//{
//	GET(HWND, hDlg, EBP);
//
//	// selects theater / climate from all the items in the combobox
//	if(HWND hDlgItem = hDlgItem = GetDlgItem(hDlg, 0x407)) {
//		int index = Randomizer::Global->RandomRanged(0, 5);
//		Debug::Log("\t[SurpriseMe] = %d\n", index);
//		SendMessageA(hDlgItem, CB_GETITEMDATA, index, 0);
//		MapSeedClass::Instance->Theater = index;
//	}
//	return 0;
//}

#pragma endregion RMG

// ===============================================
DEFINE_HOOK(0x4A80D0, CD_AlwaysFindYR, 0)
{
	R->EAX(2);
	return 0x4A8265;
}

DEFINE_HOOK(0x4790E0, CD_AlwaysAvailable, 0)
{
	R->AL(1);
	return 0x479109;
}

DEFINE_HOOK(0x479110, CD_NeverAsk, 0)
{
	R->AL(1);
	return 0x4791EA;
}

DEFINE_HOOK(0x49F5C0, CopyProtection_IsLauncherRunning, 0)
{
	R->AL(1);
	return 0x49F61A;
}

DEFINE_HOOK(0x49F620, CopyProtection_NotifyLauncher, 0)
{
	R->AL(1);
	return 0x49F733;
}

DEFINE_HOOK(0x49F7A0, CopyProtection_CheckProtectedData, 0)
{
	R->AL(1);
	return 0x49F8A7;
}

DEFINE_HOOK(0x55CFDF, BlowMeUp, 0)
{
	return 0x55D059;
}
