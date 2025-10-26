// SPDX-License-Identifier: GPL-3.0-or-later
#include "SpawnerHelper.h"

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

bool SpawnerHelper::IsSaveGameEventHooked()
{
	return SaveGameHookStart == LJMP_OPCODE;
}
