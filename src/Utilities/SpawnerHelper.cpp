#include "SpawnerHelper.h"
#include <Utilities/Debug.h>

bool SpawnerHelper::IsSaveGameEventHooked()
{
	return SaveGameHookStart == 0xE9;
}
