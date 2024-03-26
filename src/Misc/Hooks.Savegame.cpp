#include <Helpers/Macro.h>
#include <LoadOptionsClass.h>
#include <Phobos.h>

DEFINE_HOOK(0x67D04E, GameSave_SavegameInformation, 0x7)
{
	REF_STACK(SavegameInformation, Info, STACK_OFFSET(0x4A4, -0x3F4));
	Info.InternalVersion = Info.InternalVersion + SAVEGAME_ID;
	return 0;
}

DEFINE_HOOK(0x559F29, LoadOptionsClass_GetFileInfo, 0x8)
{
	if (!R->BL()) return 0x55A03D; // vanilla overridden check

	REF_STACK(SavegameInformation, Info, STACK_OFFSET(0x400, -0x3F4));
	Info.InternalVersion = Info.InternalVersion - SAVEGAME_ID;
	return 0x559F29 + 0x8;
}

// Ares saves its things at the end of the save
// Phobos will save the things at the beginning of the save
// Considering how DTA gets the scenario name, I decided to save it after Rules - secsome

DEFINE_HOOK(0x67D32C, SaveGame_Phobos, 0x5)
{
	GET(IStream*, pStm, ESI);
	//UNREFERENCED_PARAMETER(pStm);
	Phobos::SaveGameData(pStm);
	return 0;
}

DEFINE_HOOK(0x67E826, LoadGame_Phobos, 0x6)
{
	GET(IStream*, pStm, ESI);
	//UNREFERENCED_PARAMETER(pStm);
	Phobos::LoadGameData(pStm);
	return 0;
}
