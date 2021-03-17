#include <Helpers/Macro.h>
#include <LoadOptionsClass.h>
#include "../Phobos.version.h"

DEFINE_HOOK(67D04E, Game_Save_SavegameInformation, 7)
{
	REF_STACK(SavegameInformation, Info, STACK_OFFS(0x4A4, 0x3F4));
	Info.Version = Info.Version + SAVEGAME_ID;
	return 0;
}

DEFINE_HOOK(559F27, LoadOptionsClass_GetFileInfo, A)
{
	REF_STACK(SavegameInformation, Info, STACK_OFFS(0x400, 0x3F4));
	Info.Version = Info.Version - SAVEGAME_ID;
	return 0;
}

// Ares saves its things at the end of the save
// Phobos will save the things at the beginning of the save
// Considering how DTA gets the scenario name, I decided to save it after Rules.

DEFINE_HOOK(67D32C, SaveGame_Phobos, 5)
{
	GET(IStream*, pStm, ESI);
	UNREFERENCED_PARAMETER(pStm);
	// Phobos::SaveGameData(pStm);
	return 0;
}

DEFINE_HOOK(67E826, LoadGame_Phobos, 6)
{
	GET(IStream*, pStm, ESI);
	UNREFERENCED_PARAMETER(pStm);
	// Phobos::LoadGameData(pStm);
	return 0;
}