#include "Body.h"
#include "SWSidebar/SWSidebarClass.h"

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
		SidebarExt::TabProducingProgress[i] = GameCreate<SHPReference>(filename);
	}

	return 0;
}

DEFINE_HOOK(0x6A5EA1, SidebarClass_UnloadShapes_AdditionalFiles, 0x5)
{
	for (int i = 0; i < 4; i++)
	{
		if (SidebarExt::TabProducingProgress[i])
		{
			GameDelete(SidebarExt::TabProducingProgress[i]);
			SidebarExt::TabProducingProgress[i] = nullptr;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6A6EB1, SidebarClass_DrawIt_ProducingProgress, 0x6)
{
	if (Phobos::UI::ProducingProgress_Show)
	{
		const auto pPlayer = HouseClass::CurrentPlayer;
		const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array.GetItem(HouseClass::CurrentPlayer->SideIndex));
		const int XOffset = pSideExt->Sidebar_GDIPositions ? 29 : 32;
		const int XBase = (pSideExt->Sidebar_GDIPositions ? 26 : 20) + pSideExt->Sidebar_ProducingProgress_Offset.Get().X;
		const int YBase = 197 + pSideExt->Sidebar_ProducingProgress_Offset.Get().Y;

		for (int i = 0; i < 4; i++)
		{
			if (const auto pSHP = SidebarExt::TabProducingProgress[i])
			{
				const auto rtti = i == 0 || i == 1 ? AbstractType::BuildingType : AbstractType::InfantryType;
				FactoryClass* pFactory = nullptr;

				if (i != 3)
				{
					pFactory = pPlayer->GetPrimaryFactory(rtti, false, i == 1 ? BuildCat::Combat : BuildCat::DontCare);
				}
				else
				{
					pFactory = pPlayer->GetPrimaryFactory(AbstractType::UnitType, false, BuildCat::DontCare);
					if (!pFactory || !pFactory->Object)
						pFactory = pPlayer->GetPrimaryFactory(AbstractType::UnitType, true, BuildCat::DontCare);
					if (!pFactory || !pFactory->Object)
						pFactory = pPlayer->GetPrimaryFactory(AbstractType::AircraftType, false, BuildCat::DontCare);
				}

				const int idxFrame = pFactory
					? (int)(((double)pFactory->GetProgress() / 54) * (pSHP->Frames - 1))
					: -1;

				Point2D vPos = { XBase + i * XOffset, YBase };
				RectangleStruct sidebarRect = DSurface::Sidebar->GetRect();

				if (idxFrame != -1)
				{
					DSurface::Sidebar->DrawSHP(FileSystem::SIDEBAR_PAL, pSHP, idxFrame, &vPos,
						&sidebarRect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x72FCB5, InitSideRectangles_CenterBackground, 0x5)
{
	if (Phobos::UI::CenterPauseMenuBackground)
	{
		GET(RectangleStruct*, pRect, EAX);
		GET_STACK(const int, width, STACK_OFFSET(0x18, -0x4));
		GET_STACK(const int, height, STACK_OFFSET(0x18, -0x8));

		pRect->X = (width - 168 - pRect->Width) / 2;
		pRect->Y = (height - 32 - pRect->Height) / 2;

		R->EAX(pRect);
	}

	return 0;
}

#pragma region SWSidebarButtonsRelated

DEFINE_HOOK(0x692419, DisplayClass_ProcessClickCoords_SWSidebar, 0x7)
{
	enum { DoNothing = 0x6925FC };

	if (SWSidebarClass::IsEnabled() && SWSidebarClass::Instance.CurrentColumn)
		return DoNothing;

	const auto toggleButton = SWSidebarClass::Instance.ToggleButton;

	return toggleButton && toggleButton->IsHovering ? DoNothing : 0;
}

DEFINE_HOOK(0x6A5082, SidebarClass_InitClear_InitializeSWSidebar, 0x5)
{
	SWSidebarClass::Instance.InitClear();
	return 0;
}

DEFINE_HOOK(0x6A5839, SidebarClass_InitIO_InitializeSWSidebar, 0x5)
{
	SWSidebarClass::Instance.InitIO();
	return 0;
}

DEFINE_HOOK_AGAIN(0x4E13B2, GadgetClass_DTOR_ClearCurrentOverGadget, 0x6)
DEFINE_HOOK(0x4E1A84, GadgetClass_DTOR_ClearCurrentOverGadget, 0x6)
{
	GadgetClass* const pThis = (R->Origin() == 0x4E1A84) ? R->ESI<GadgetClass*>() : R->ECX<GadgetClass*>();
	AnnounceInvalidPointer(Make_Global<GadgetClass*>(0x8B3E94), pThis);
	return 0;
}

#pragma endregion
