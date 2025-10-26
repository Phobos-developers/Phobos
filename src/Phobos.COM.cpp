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
#include "Phobos.COM.h"

#include <Helpers/Macro.h>

#include <Locomotion/TestLocomotionClass.h>


#ifdef CUSTOM_LOCO_EXAMPLE_ENABLED // Register the loco
DEFINE_HOOK(0x6BD68D, WinMain_PhobosRegistrations, 0x6)
{
	Debug::Log("Starting COM registration...\n");

	// Add new classes to be COM-registered below
	RegisterFactoryForClass<TestLocomotionClass>();

	Debug::Log("COM registration done!\n");

	return 0;
}
#endif
