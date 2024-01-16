#include "CustomTheater.h"

#include <Ext/Scenario/Body.h>
#include <MapSeedClass.h>
#include <Randomizer.h>

// Writes the name of the theater to generated map
DEFINE_HOOK(0x5997AB, MapGeneratorClass_init, 0x9)
{
	R->ECX(ScenarioExt::Global()->CustomTheaterID.data());
	return 0x5997C6;
}

// Overrides the Ares hook RMG_EnableDesert on 0x5970EA
DEFINE_HOOK(0x5970AF, MapSeedClass_SetMenuOptions, 0x7)
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

	AddOption(TheaterProto::Temperate->UIName, 0);
	AddOption(TheaterProto::Snow->UIName, 1);
	AddOption(TheaterProto::Desert->UIName, 3);

	auto pTheater = &TheaterProto::Array[MapSeedClass::Instance->Theater];
	auto pUIName = StringTable::FetchString(pTheater->UIName);
	auto item = SendMessageA(hWnd, WW_CB_GETITEMINDEX, 0, reinterpret_cast<LPARAM>(pUIName));
	SendMessageA(hWnd, CB_SETCURSEL, item, 0);

	R->EBP(&MapSeedClass::Instance);
	return 0x59712A;
}
