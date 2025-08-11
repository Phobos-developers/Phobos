#include "SpawnerHelper.h"

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

bool SpawnerHelper::IsSaveGameEventHooked()
{
	return SaveGameHookStart == LJMP_OPCODE;
}
