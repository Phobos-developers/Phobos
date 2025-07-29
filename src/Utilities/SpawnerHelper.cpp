#include "SpawnerHelper.h"

bool SpawnerHelper::SaveGameEventHooked()
{
	return SaveGameHookStart == 0xE9;
}
