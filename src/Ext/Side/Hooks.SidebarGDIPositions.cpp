#include <ScenarioClass.h>
#include "Body.h"

bool isNODSidebar()
{
	auto const PlayerSideIndex = ScenarioClass::Instance->PlayerSideIndex;
	if (auto const pSide = SideClass::Array->GetItemOrDefault(PlayerSideIndex)) {
		if (auto const pData = SideExt::ExtMap.Find(pSide)) {
			return !pData->Sidebar_GDIPositions;
		}
	}
	return PlayerSideIndex;
}

DEFINE_HOOK(6A5090, SidebarGDIPositions1, 5)
{
	R->EAX(isNODSidebar());
	return 0x6A509B;
}

DEFINE_HOOK(652EAB, SidebarGDIPositions2, 6)
{
	R->EAX(isNODSidebar());
	return 0x652EB7;
}

DEFINE_HOOK(6A51E9, SidebarGDIPositions3, 6)
{
	DWORD& SidebarClass__OBJECT_HEIGHT = *reinterpret_cast<DWORD*>(0xB0B500);
	SidebarClass__OBJECT_HEIGHT = 0x32;

	bool pos = isNODSidebar();
	R->ESI(pos);
	R->EDX(pos);
	return 0x6A5205;
}

// power bar
DEFINE_HOOK(63FB5D, SidebarGDIPositions4, 6)
{
	R->EAX(isNODSidebar());
	return 0x63FB63;
}

// power bar tooltip
DEFINE_HOOK(6403DF, SidebarGDIPositions5, 6)
{
	R->ESI(isNODSidebar());
	return 0x6403E5;
}
