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
#include <ScenarioClass.h>
#include "Body.h"

bool isNODSidebar = false;

DEFINE_HOOK(0x534FA7, Prep_For_Side, 0x5)
{
	GET(const int, sideIndex, ECX);
	const auto pSide = SideClass::Array.GetItemOrDefault(sideIndex);
	const auto pSideExt = SideExt::ExtMap.TryFind(pSide);
	isNODSidebar = pSideExt ? !pSideExt->Sidebar_GDIPositions : sideIndex;

	return 0;
}

DEFINE_HOOK(0x652EAB, RadarClass_InitForHouse, 0x6)
{
	R->EAX(isNODSidebar);
	return 0x652EB7;
}

DEFINE_HOOK(0x6A5090, SidebarClass_InitPositions, 0x5)
{
	R->EAX(isNODSidebar);
	return 0x6A509B;
}

DEFINE_HOOK(0x6A51E9, SidebarClass_InitGUI, 0x6)
{
	DWORD& SidebarClass__OBJECT_HEIGHT = *reinterpret_cast<DWORD*>(0xB0B500);
	SidebarClass__OBJECT_HEIGHT = 0x32;

	R->ESI(isNODSidebar);
	R->EDX(isNODSidebar);
	return 0x6A5205;
}

// PowerBar Positions
DEFINE_HOOK(0x63FB5D, PowerClass_DrawIt, 0x6)
{
	R->EAX(isNODSidebar);
	return 0x63FB63;
}

// PowerBar Tooltip Positions
DEFINE_HOOK(0x6403DF, PowerClass_InitGUI, 0x6)
{
	R->ESI(isNODSidebar);
	return 0x6403E5;
}
