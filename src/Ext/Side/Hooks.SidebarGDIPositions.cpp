#include <ScenarioClass.h>
#include "Body.h"

bool isNODSidebar = false;

DEFINE_HOOK(0x534FA7, Prep_For_Side, 0x5)
{
	GET(int, sideIndex, ECX);
	auto pSide = SideClass::Array.GetItem(sideIndex);
	auto pSideExt = pSide ? SideExt::ExtMap.Find(pSide) : nullptr;
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
