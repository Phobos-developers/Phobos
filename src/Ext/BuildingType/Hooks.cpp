#include "Body.h"

#include <TacticalClass.h>

#include <BuildingClass.h>
#include <HouseClass.h>
#include <Ext/Rules/Body.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x460285, BuildingTypeClass_LoadFromINI_Muzzle, 0x6)
{
	enum { Skip = 0x460388, Read = 0x460299 };

	GET(BuildingTypeClass*, pThis, EBP);

	// Restore overriden instructions
	R->Stack(STACK_OFFS(0x368, 0x358), 0);
	R->EDX(0);

	// Disable Vanilla Muzzle flash when MaxNumberOccupants is 0 or more than 10
	return !pThis->MaxNumberOccupants || pThis->MaxNumberOccupants > 10
		? Skip : Read;
}

DEFINE_HOOK(0x44043D, BuildingClass_AI_Temporaled_Chronosparkle_MuzzleFix, 0x8)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if (pType->MaxNumberOccupants > 10)
	{
		GET(int, nFiringIndex, EBX);
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->EAX(&pTypeExt->OccupierMuzzleFlashes[nFiringIndex]);
	}

	return 0;
}

DEFINE_HOOK(0x45387A, BuildingClass_FireOffset_Replace_MuzzleFix, 0xA)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if (pType->MaxNumberOccupants > 10)
	{
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->EDX(&pTypeExt->OccupierMuzzleFlashes[pThis->FiringOccupantIndex]);
	}

	return 0;
}

DEFINE_HOOK(0x458623, BuildingClass_KillOccupiers_Replace_MuzzleFix, 0x7)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if (pType->MaxNumberOccupants > 10)
	{
		GET(int, nFiringIndex, EDI);
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->ECX(&pTypeExt->OccupierMuzzleFlashes[nFiringIndex]);
	}

	return 0;
}

DEFINE_HOOK(0x6D528A, TacticalClass_DrawPlacement_PlacementPreview, 0x6)
{
	ObjectClass* pObject = Make_Global<ObjectClass*>(0x88098C);

	if (auto const pBuilding = specific_cast<BuildingClass*>(pObject))
	{
		if (auto const pType = pBuilding->Type)
		{
			auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pType);

			if (pTypeExt->PlacementPreview_Show.Get(RulesExt::Global()->Building_PlacementPreview.Get()))
			{
				auto pImage = pTypeExt->PlacementPreview_Shape.Get(pType->GetImage());

				if (!pImage)
					return 0x0;

				auto const nFrame = Math::clamp(pTypeExt->PlacementPreview_ShapeFrame.Get(), 0, (int)pImage->Frames);
				CellStruct const nDisplayCell = Make_Global<CellStruct>(0x88095C);
				CellStruct const nDisplayCell_Offset = Make_Global<CellStruct>(0x880960);
				auto const pCell = MapClass::Instance->TryGetCellAt(nDisplayCell + nDisplayCell_Offset);

				if (!pCell)
					return 0x0;

				auto const nHeight = pCell->GetFloorHeight({ 0,0 });
				Point2D nPoint{ 0,0 };
				TacticalClass::Instance->CoordsToClient(CellClass::Cell2Coord(pCell->MapCoords, nHeight),&nPoint);

				auto nFlag = BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass;
				auto const nFlag_ = Math::clamp(pTypeExt->PlacementPreview_Transculency.Get(), 0, 3);

				switch (nFlag_)
				{
				case 1:
					nFlag = nFlag | BlitterFlags::TransLucent25;
					break;
				case 2:
					nFlag = nFlag | BlitterFlags::TransLucent50;
					break;
				case 3:
					nFlag = nFlag | BlitterFlags::TransLucent75;
					break;
				default:
					break;
				}

				auto const nREct = DSurface::Temp()->GetRect();
				auto const nScheme = ColorScheme::Array()->GetItem(HouseClass::Player->ColorSchemeIndex);
				auto const nPalette = pTypeExt->PlacementPreview_Remap.Get() ? nScheme->LightConvert : pTypeExt->PlacementPreview_Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL());
				Point2D nDefault{ 0 , -15 };
				nPoint += pTypeExt->PlacementPreview_Offset.Get(nDefault);

				DSurface::Temp()->DrawSHP(
					nPalette,
					pImage,
					nFrame,
					&nPoint,
					&nREct,
					nFlag,
					0,
					0, ZGradient::Ground ,
					 1000, 0, nullptr, 0, 0, 0

				);
			}
		}
	}

	return 0x0;
}

//Make Building placement Grid tranparent
static void __fastcall CellClass_Draw_It_Shape(Surface* Surface, ConvertClass* Palette, SHPStruct* SHP, int FrameIndex,
	const Point2D* const Position, const RectangleStruct* const Bounds, BlitterFlags Flags,
	int Remap,
	int ZAdjust, // + 1 = sqrt(3.0) pixels away from screen
	ZGradient ZGradientDescIndex,
	int Brightness, // 0~2000. Final color = saturate(OriginalColor * Brightness / 1000.0f)
	int TintColor, SHPStruct* ZShape, int ZShapeFrame, int XOffset, int YOffset)
{
	auto const nFlag_ = Math::clamp(RulesExt::Global()->PlacementGrid_Transculency.Get(), 0, 3);

	switch (nFlag_)
	{
	case 1:
		Flags = Flags | BlitterFlags::TransLucent25;
		break;
	case 2:
		Flags = Flags | BlitterFlags::TransLucent50;
		break;
	case 3:
		Flags = Flags | BlitterFlags::TransLucent75;
		break;
	default:
		break;
	}

	CC_Draw_Shape(Surface, Palette, SHP, FrameIndex, Position, Bounds, Flags, Remap, ZAdjust,
		ZGradientDescIndex, Brightness, TintColor, ZShape, ZShapeFrame, XOffset, YOffset);
}

DEFINE_POINTER_CALL(0x47EFB4, &CellClass_Draw_It_Shape);
