#include "Body.h"

#include <RadarClass.h>

bool isNODSidebar = false;

DEFINE_HOOK(0x534FA7, Prep_For_Side, 0x5)
{
	GET(const int, sideIndex, ECX);
	const auto pSide = SideClass::Array.GetItemOrDefault(sideIndex);
	const auto pSideExt = SideExt::TryFetch(pSide);
	isNODSidebar = pSideExt ? !pSideExt->Sidebar_GDIPositions : sideIndex;

	return 0;
}

DEFINE_HOOK(0x652EAB, RadarClass_InitForHouse, 0x6)
{
	R->EAX(isNODSidebar);
	return 0x652EB7;
}

DEFINE_HOOK(0x652F4F, RadarClass_InitForHouse_RadarOffset, 0x6)
{
	// Vanilla uses 16 as the radar X origin, while the visible aperture in
	// the sidebar shape starts at 13, causing a 3 pixel horizontal offset.
	GET(RadarClass*, pThis, ESI);
	pThis->RadarX -= 3;
	R->EDX(pThis->RadarX);

	return 0x652F55;
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
