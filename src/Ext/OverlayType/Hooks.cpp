#include "Body.h"

#include <Ext/BuildingType/Body.h>

DEFINE_HOOK(0x47F71D, CellClass_DrawOverlay_ZAdjust, 0x5)
{
	GET(const int, zAdjust, EDI);
	GET_STACK(OverlayTypeClass*, pOverlayType, STACK_OFFSET(0x24, -0x14));

	auto const pTypeExt = OverlayTypeExt::Fetch(pOverlayType);

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
	GET(const int, zAdjust, EDI);
	GET_STACK(OverlayTypeClass*, pOverlayType, STACK_OFFSET(0x24, -0x14));
	REF_STACK(Point2D, location, STACK_OFFSET(0x24, -0x10));

	const int wallOwnerIndex = pThis->WallOwnerIndex;
	int colorSchemeIndex = HouseClass::CurrentPlayer->ColorSchemeIndex;

	if (wallOwnerIndex >= 0)
		colorSchemeIndex = HouseClass::Array[wallOwnerIndex]->ColorSchemeIndex;

	auto const pTypeExt = OverlayTypeExt::Fetch(pOverlayType);
	const auto pPalettes = pTypeExt->Palette;
	const auto pConvert = (pPalettes ? pPalettes->Items[colorSchemeIndex] : ColorScheme::Array[colorSchemeIndex])->LightConvert;

	DSurface::Temp->DrawSHP(pConvert, pShape, pThis->OverlayData, &location, pBounds,
		BlitterFlags(0x4E00), 0, -2 - zAdjust, ZGradient::Deg90, pThis->Intensity_Normal, 0, 0, 0, 0, 0);

	return SkipGameCode;
}

DEFINE_HOOK(0x47FAFD, CellClass_DrawOverlay_Rubble, 0x5)
{
	enum { SkipGameCode = 0x47FB86 };

	GET(CellClass*, pThis, ESI);
	GET(RectangleStruct*, pBounds, EBP);
	GET_STACK(SHPStruct*, pRubbleShape, STACK_OFFSET(0x24, -0x14));
	GET_STACK(int, frameIdx, STACK_OFFSET(0x24, 0x8));
	REF_STACK(const Point2D, location, STACK_OFFSET(0x24, -0x10));

	const auto pRubbleTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Rubble);
	LightConvertClass* pRubbleConvert = nullptr;

	if (const auto pPalettes = pRubbleTypeExt->RubblePalette)
	{
		const int wallOwnerIdx = pThis->WallOwnerIndex;
		const int colorSchemeIdx = (wallOwnerIdx != -1 ? HouseClass::Array[wallOwnerIdx] : HouseClass::CurrentPlayer)->ColorSchemeIndex;
		pRubbleConvert = pPalettes->Items[colorSchemeIdx]->LightConvert;
	}
	else
	{
		pRubbleConvert = pThis->LightConvert;
	}

	const auto blitterFlags = BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered;
	const int zAdjust = R->EDI<int>() - R->Stack<int>(STACK_OFFSET(0x24, 0x4)) - 2;
	const auto zGradient = R->BL() ? ZGradient::Ground : ZGradient::Deg90;

	DSurface::Temp->DrawSHP(pRubbleConvert, pRubbleShape, frameIdx, &location, pBounds,
		blitterFlags, 0, zAdjust, zGradient, pThis->Intensity_Terrain, 0, nullptr, 0, 0, 0);

	return SkipGameCode;
}
