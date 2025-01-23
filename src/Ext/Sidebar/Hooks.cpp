#include "Body.h"

#include <HouseClass.h>
#include <FactoryClass.h>
#include <FileSystem.h>

#include <Ext/Side/Body.h>
#include <Ext/House/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Utilities/Macro.h>
#include <Utilities/ShapeTextPrinter.h>

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
		auto pPlayer = HouseClass::CurrentPlayer();
		auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(HouseClass::CurrentPlayer->SideIndex));
		int XOffset = pSideExt->Sidebar_GDIPositions ? 29 : 32;
		int XBase = (pSideExt->Sidebar_GDIPositions ? 26 : 20) + pSideExt->Sidebar_ProducingProgress_Offset.Get().X;
		int YBase = 197 + pSideExt->Sidebar_ProducingProgress_Offset.Get().Y;

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
					if (!pFactory || !pFactory->Object)
						pFactory = pPlayer->GetPrimaryFactory(AbstractType::UnitType, true, BuildCat::DontCare);
					if (!pFactory || !pFactory->Object)
						pFactory = pPlayer->GetPrimaryFactory(AbstractType::AircraftType, false, BuildCat::DontCare);
				}

				int idxFrame = pFactory
					? (int)(((double)pFactory->GetProgress() / 54) * (pSHP->Frames - 1))
					: -1;

				Point2D vPos = { XBase + i * XOffset, YBase };
				RectangleStruct sidebarRect = DSurface::Sidebar()->GetRect();

				if (idxFrame != -1)
				{
					DSurface::Sidebar()->DrawSHP(FileSystem::SIDEBAR_PAL, pSHP, idxFrame, &vPos,
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
		GET_STACK(int, width, STACK_OFFSET(0x18, -0x4));
		GET_STACK(int, height, STACK_OFFSET(0x18, -0x8));

		pRect->X = (width - 168 - pRect->Width) / 2;
		pRect->Y = (height - 32 - pRect->Height) / 2;

		R->EAX(pRect);
	}

	return 0;
}

DEFINE_HOOK(0x6A9BC5, StripClass_Draw_DrawGreyCameoExtraCover, 0x6)
{
	GET(const bool, greyCameo, EBX);
	GET(const int, destX, ESI);
	GET(const int, destY, EBP);
	GET_STACK(const RectangleStruct, boundingRect, STACK_OFFSET(0x48C, -0x3E0));
	GET_STACK(TechnoTypeClass* const, pType, STACK_OFFSET(0x48C, -0x458));

	const auto position = Point2D { destX + 30, destY + 24 };
	const auto pRulesExt = RulesExt::Global();
	const auto frames = pRulesExt->Cameo_OverlayFrames.Get();

	if (greyCameo) // Only draw extras over grey cameos
	{
		auto frame = frames.Y;
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (pTypeExt && pTypeExt->Cameo_AlwaysExist.Get(RulesExt::Global()->Cameo_AlwaysExist))
		{
			auto& vec = ScenarioExt::Global()->OwnedExistCameoTechnoTypes;

			if (std::find(vec.begin(), vec.end(), pTypeExt) != vec.end())
			{
				if (const auto CameoPCX = pTypeExt->GreyCameoPCX.GetSurface())
				{
					auto drawRect = RectangleStruct { destX, destY, 60, 48 };
					PCX::Instance->BlitToSurface(&drawRect, DSurface::Sidebar, CameoPCX);
				}

				frame = frames.Z;
			}
		}

		if (frame >= 0)
		{
			DSurface::Sidebar->DrawSHP(
				pRulesExt->Cameo_OverlayPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL),
				pRulesExt->Cameo_OverlayShapes,
				frame,
				&position,
				&boundingRect,
				BlitterFlags(0x600),
				0, 0,
				ZGradient::Ground,
				1000, 0, 0, 0, 0, 0);
		}
	}

	const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pType);

	if (pBuildingType) // Only count owned buildings
	{
		const auto pHouse = HouseClass::CurrentPlayer();
		auto count = BuildingTypeExt::GetUpgradesAmount(pBuildingType, pHouse);

		if (count == -1)
			count = pHouse->CountOwnedAndPresent(pBuildingType);

		if (count > 0)
		{
			if (frames.X >= 0)
			{
				DSurface::Sidebar->DrawSHP(
					pRulesExt->Cameo_OverlayPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL),
					pRulesExt->Cameo_OverlayShapes,
					frames.X,
					&position,
					&boundingRect,
					BlitterFlags(0x600),
					0, 0,
					ZGradient::Ground,
					1000, 0, 0, 0, 0, 0);
			}

			if (Phobos::Config::ShowBuildingStatistics)
			{
				GET_STACK(RectangleStruct, surfaceRect, STACK_OFFSET(0x48C, -0x438));

				const COLORREF color = Drawing::RGB_To_Int(Drawing::TooltipColor);
				const TextPrintType printType = TextPrintType::Background | TextPrintType::FullShadow | TextPrintType::Point8;
				auto textPosition = Point2D { destX, destY + 1 };

				wchar_t text[0x20];
				swprintf_s(text, L"%d", count);
				DSurface::Sidebar->DrawTextA(text, &surfaceRect, &textPosition, color, 0, printType);
			}
		}
	}

	return 0;
}
