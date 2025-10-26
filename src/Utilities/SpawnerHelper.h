// SPDX-License-Identifier: GPL-3.0-or-later
// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
#pragma once

#include <Windows.h>

#include <Helpers/CompileTime.h>

class SpawnerHelper
{
private:
	DEFINE_REFERENCE(const unsigned char, SaveGameHookStart, 0x4C7A14u);

public:
	// Spawner hooks 0x4C7A14 and places an LJMP there. We check that memory address on whether it is a valid LJMP opcode
	// and assume (unreliable, but I am open for better ideas) that if it is, the save game event hook is active.
	//
	// This doesn't account for Spawner::Active, so in a case where the spawner is loaded but not active this will fail,
	// but oh well I am not engeneering a complicated solution just to fix that niche case which wouldn't happen 99% of the time.
	//
	// To read more about this mess and possibly engineer a better solution, look up the comments mentioning 0x4C7A14 in spawner.
	// - Kerbiter
	static bool IsSaveGameEventHooked();
};
