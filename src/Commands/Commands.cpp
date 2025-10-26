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


#include "Commands.h"

#include "ObjectInfo.h"
#include "NextIdleHarvester.h"
#include "QuickSave.h"
#include "DamageDisplay.h"
#include "FrameByFrame.h"
#include "FrameStep.h"
#include "ToggleDigitalDisplay.h"
#include "ToggleDesignatorRange.h"
#include "SaveVariablesToFile.h"
#include "ToggleSWSidebar.h"
#include "FireTacticalSW.h"
#include "ToggleMessageList.h"

#include <CCINIClass.h>

#include <Utilities/Macro.h>
#include <Ext/Sidebar/SWSidebar/SWSidebarClass.h>
#include <Misc/MessageColumn.h>

DEFINE_HOOK(0x533066, CommandClassCallback_Register, 0x6)
{
	// Load it after Ares'

	MakeCommand<NextIdleHarvesterCommandClass>();
	MakeCommand<QuickSaveCommandClass>();
	MakeCommand<ToggleDigitalDisplayCommandClass>();
	MakeCommand<ToggleDesignatorRangeCommandClass>();
	MakeCommand<ToggleMessageListCommandClass>();
	MakeCommand<ToggleSWSidebar>();

	if (Phobos::Config::SuperWeaponSidebarCommands)
	{
		SWSidebarClass::Commands[0] = MakeCommand<FireTacticalSWCommandClass<0>>();
		SWSidebarClass::Commands[1] = MakeCommand<FireTacticalSWCommandClass<1>>();
		SWSidebarClass::Commands[2] = MakeCommand<FireTacticalSWCommandClass<2>>();
		SWSidebarClass::Commands[3] = MakeCommand<FireTacticalSWCommandClass<3>>();
		SWSidebarClass::Commands[4] = MakeCommand<FireTacticalSWCommandClass<4>>();
		SWSidebarClass::Commands[5] = MakeCommand<FireTacticalSWCommandClass<5>>();
		SWSidebarClass::Commands[6] = MakeCommand<FireTacticalSWCommandClass<6>>();
		SWSidebarClass::Commands[7] = MakeCommand<FireTacticalSWCommandClass<7>>();
		SWSidebarClass::Commands[8] = MakeCommand<FireTacticalSWCommandClass<8>>();
		SWSidebarClass::Commands[9] = MakeCommand<FireTacticalSWCommandClass<9>>();
	}

	if (Phobos::Config::DevelopmentCommands)
	{
		MakeCommand<DamageDisplayCommandClass>();
		MakeCommand<SaveVariablesToFileCommandClass>();
		MakeCommand<ObjectInfoCommandClass>();
		MakeCommand<FrameByFrameCommandClass>();
		MakeCommand<FrameStepCommandClass<1>>(); // Single step in
		MakeCommand<FrameStepCommandClass<5>>(); // Speed 1
		MakeCommand<FrameStepCommandClass<10>>(); // Speed 2
		MakeCommand<FrameStepCommandClass<15>>(); // Speed 3
		MakeCommand<FrameStepCommandClass<30>>(); // Speed 4
		MakeCommand<FrameStepCommandClass<60>>(); // Speed 5
	}

	return 0;
}

static void MouseWheelDownCommand()
{
	if (MessageColumnClass::Instance.IsHovering())
		MessageColumnClass::Instance.ScrollDown();
}

static void MouseWheelUpCommand()
{
	if (MessageColumnClass::Instance.IsHovering())
		MessageColumnClass::Instance.ScrollUp();
}

DEFINE_HOOK(0x777998, Game_WndProc_ScrollMouseWheel, 0x6)
{
	GET(const WPARAM, WParam, ECX);

	if (WParam & 0x80000000u)
		MouseWheelDownCommand();
	else
		MouseWheelUpCommand();

	return 0;
}

static inline bool CheckSkipScrollSidebar()
{
	return MessageColumnClass::Instance.IsHovering();
}

DEFINE_HOOK(0x533F50, Game_ScrollSidebar_Skip, 0x5)
{
	enum { SkipScrollSidebar = 0x533FC3 };
	return CheckSkipScrollSidebar() ? SkipScrollSidebar : 0;
}
