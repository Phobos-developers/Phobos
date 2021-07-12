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
		sprintf_s(filename, "tab%02dpp.SHP", i);
		SidebarExt::TabProducingProgress[i] = ((SHPFile*)FileSystem::LoadWholeFileEx(filename, SidebarExt::TabProducingProgressParsed[i]));
	}
	return 0;
}

DEFINE_HOOK(0x6A5EA1, SidebarClass_UnloadShapes_AdditionalFiles, 0x5)
{
	for (int i = 0; i < 4; i++)
	{
		//if (SidebarExt::TabProducingProgressParsed[i])
		{
			GameDelete(SidebarExt::TabProducingProgress[i]);
			SidebarExt::TabProducingProgressParsed[i] = false;
		}
	}
	return 0;
}

DEFINE_HOOK(0x6A70B3, SidebarClass_DrawIt_ProducingProgress, 0xA)
{
	if (Phobos::UI::ShowProducingProgress)
	{
		auto pPlayer = HouseClass::Player();
		auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(HouseClass::Player->SideIndex));
		auto vSidebarRect = RectangleStruct{0,0,0,0};//DSurface::Sidebar()->GetRect();
		int XOffset = 0;
		int XBase = 0;
		int YBase = vSidebarRect.Y + 39;

		if (pSideExt->Sidebar_GDIPositions)
		{
			XOffset = 29;
			XBase = vSidebarRect.X + 26;
		}
		else
		{
			XOffset = 32;
			XBase = vSidebarRect.X + 20;
		}

		for (int i = 0; i < 4; i++)
		{
			if (!SidebarExt::TabProducingProgressParsed[i])
				continue;
			auto pSHP = SidebarExt::TabProducingProgress[i];
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
			DSurface::Sidebar()->DrawSHP(FileSystem::SIDEBAR_PAL, pSHP, idxFrame, &vPos,
				&vSidebarRect, BlitterFlags::bf_400, 0, 0, ZGradientDescIndex::Flat, 1000, 0, 0, 0, 0, 0);
		}
	}

	return 0;
}