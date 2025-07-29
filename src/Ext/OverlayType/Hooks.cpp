#include "Body.h"
#include <HouseClass.h>

#include <Ext/BuildingType/Body.h>

DEFINE_HOOK(0x47F71D, CellClass_DrawOverlay_ZAdjust, 0x5)
{
	GET(int, zAdjust, EDI);
	GET_STACK(OverlayTypeClass*, pOverlayType, STACK_OFFSET(0x24, -0x14));

	auto const pTypeExt = OverlayTypeExt::ExtMap.Find(pOverlayType);

	if (pTypeExt->ZAdjust != 0)
		R->EDI(zAdjust - pTypeExt->ZAdjust);

	return 0;
}

// Replaces an Ares hook at 0x47F9A4
DEFINE_HOOK(0x47F974, CellClass_DrawOverlay_Walls, 0x5)
{
	enum { SkipGameCode = 0x47FB86 };

	GET(CellClass*, pThis, ESI);
	GET(SHPStruct*, pShape, EAX);
	GET(RectangleStruct*, pBounds, EBP);
	GET(int, zAdjust, EDI);
	GET_STACK(OverlayTypeClass*, pOverlayType, STACK_OFFSET(0x24, -0x14));
	REF_STACK(Point2D, location, STACK_OFFSET(0x24, -0x10));

	int wallOwnerIndex = pThis->WallOwnerIndex;
	int colorSchemeIndex = HouseClass::CurrentPlayer->ColorSchemeIndex;

	if (wallOwnerIndex >= 0)
		colorSchemeIndex = HouseClass::Array[wallOwnerIndex]->ColorSchemeIndex;

	LightConvertClass* pConvert = nullptr;
	auto const pTypeExt = OverlayTypeExt::ExtMap.Find(pOverlayType);

	if (pTypeExt->Palette)
		pConvert = pTypeExt->Palette->Items[colorSchemeIndex]->LightConvert;
	else
		pConvert = ColorScheme::Array[colorSchemeIndex]->LightConvert;

	DSurface::Temp->DrawSHP(pConvert, pShape, pThis->OverlayData, &location, pBounds,
		BlitterFlags(0x4E00), 0, -2 - zAdjust, ZGradient::Deg90, pThis->Intensity_Normal, 0, 0, 0, 0, 0);

	return SkipGameCode;
}

DEFINE_HOOK(0x47FAFD, CellClass_DrawOverlay_Rubble, 0x5)
{
	enum { SkipGameCode = 0x47FB86 };

	GET(CellClass*, pCell, ESI);
	GET(RectangleStruct*, pBounds, EBP);
	GET_STACK(SHPStruct*, pRubbleShape, STACK_OFFSET(0x24, -0x14));
	GET_STACK(int, frameIdx, STACK_OFFSET(0x24, 0x8));
	REF_STACK(const Point2D, location, STACK_OFFSET(0x24, -0x10));

	const auto pRubbleTypeExt = BuildingTypeExt::ExtMap.Find(pCell->Rubble);
	const auto pRubblePalette = pRubbleTypeExt->RubblePalette.GetOrDefaultConvert(pCell->LightConvert);

	const auto blitterFlags = BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered;
	const int zAdjust = R->EDI<int>() - R->Stack<int>(STACK_OFFSET(0x24, 0x4)) - 2;
	const auto zGradient = R->BL() ? ZGradient::Ground : ZGradient::Deg90;

	DSurface::Temp->DrawSHP(pRubblePalette, pRubbleShape, frameIdx, &location, pBounds, blitterFlags, 0, zAdjust, zGradient, pCell->Intensity_Terrain, 0, nullptr, 0, 0, 0);
	return SkipGameCode;
}
