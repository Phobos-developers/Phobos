#include "SpawnerHelper.h"
#include <Utilities/Debug.h>

bool SpawnerHelper::IsSaveGameEventHooked()
{
	bool isHooked = SaveGameHookStart == 0xE9;

	if (isHooked)
		Debug::Log("IsSaveGameEventHooked: Spawner save game event hook is active.\n");
	else
		Debug::Log("IsSaveGameEventHooked: Spawner save game event hook is not active.\n");

	return isHooked;
}
