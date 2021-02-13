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
