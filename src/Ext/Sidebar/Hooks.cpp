#include "Body.h"

#include <HouseClass.h>
#include <FactoryClass.h>
#include <FileSystem.h>
#include <Ext/Side/Body.h>

DEFINE_HOOK(0x6A593E, SidebarClass_InitForHouse_AdditionalFiles, 0x5)
{
	char filename[0x20];

	for (int i = 0; i < 4; i++)
	{
		sprintf_s(filename, "tab%02dpp.shp", i);
		bool parsed = false;
		SidebarExt::TabProducingProgress[i] = ((SHPFile*)FileSystem::LoadWholeFileEx(filename, parsed));
	}
	return 0;
}

DEFINE_HOOK(0x6A5EA1, SidebarClass_UnloadShapes_AdditionalFiles, 0x5)
{
	for (int i = 0; i < 4; i++)
		if (SidebarExt::TabProducingProgress[i])
			GameDelete(SidebarExt::TabProducingProgress[i]);
	
	return 0;
}

DEFINE_HOOK(0x6A70B3, SidebarClass_DrawIt_ProducingProgress, 0xA)
{
	if (Phobos::UI::ShowProducingProgress)
	{
		auto pPlayer = HouseClass::Player();
		auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(HouseClass::Player->SideIndex));
		int XOffset = pSideExt->Sidebar_GDIPositions ? 29 : 32;
		int XBase = pSideExt->Sidebar_GDIPositions ? 26 : 20;
		int YBase = 200;

		for (int i = 0; i < 4; i++)
		{
			if (auto pSHP = SidebarExt::TabProducingProgress[i])
			{
				auto rtti = i == 0 || i == 1 ? AbstractType::BuildingType : AbstractType::InfantryType;
				FactoryClass* pFactory = nullptr;
				if (i != 3)
				{
					pFactory = pPlayer->GetPrimaryFactory(rtti, false, i == 1 ? BuildCat::Combat : BuildCat::DontCare);
				}
				else
				{
					pFactory = pPlayer->GetPrimaryFactory(AbstractType::UnitType, false, BuildCat::DontCare);
				}
				int idxFrame = pFactory ? (int)(((double)pFactory->GetProgress() / 55) * pSHP->Frames) : -1;

				Point2D vPos = { XBase + i * XOffset, YBase };
				RectangleStruct sidebarRect = DSurface::Sidebar()->GetRect();

				DSurface::Sidebar()->DrawSHP(FileSystem::SIDEBAR_PAL, pSHP, idxFrame, &vPos,
					&sidebarRect, BlitterFlags::bf_400, 0, 0, ZGradientDescIndex::Flat, 1000, 0, 0, 0, 0, 0);
			}
		}
	}

	return 0;
}