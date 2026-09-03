#include "SpawnerHelper.h"

#include <Utilities/Macro.h>

bool SpawnerHelper::IsSaveGameEventHooked()
{
	return SaveGameHookStart == LJMP_OPCODE;
}
