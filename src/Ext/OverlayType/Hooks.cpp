#include "Body.h"

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
	REF_STACK(Point2D, pLocation, STACK_OFFSET(0x24, -0x10));

	const int wallOwnerIndex = pThis->WallOwnerIndex;
	int colorSchemeIndex = HouseClass::CurrentPlayer->ColorSchemeIndex;

	if (wallOwnerIndex >= 0)
		colorSchemeIndex = HouseClass::Array[wallOwnerIndex]->ColorSchemeIndex;

	LightConvertClass* pConvert = nullptr;
	auto const pTypeExt = OverlayTypeExt::Fetch(pOverlayType);

	if (pTypeExt->Palette)
		pConvert = pTypeExt->Palette->Items[colorSchemeIndex]->LightConvert;
	else
		pConvert = ColorScheme::Array[colorSchemeIndex]->LightConvert;

	DSurface::Temp->DrawSHP(pConvert, pShape, pThis->OverlayData, &pLocation, pBounds,
		BlitterFlags(0x4E00), 0, -2 - zAdjust, ZGradient::Deg90, pThis->Intensity_Normal, 0, 0, 0, 0, 0);

	return SkipGameCode;
}

#pragma region CanBeBuiltOn

DEFINE_HOOK(0x47C9A7, CellClass_IsClearToBuild_Overlays, 0x5)
{
	enum { ReturnFromFunction = 0x47C6D1, CheckTileLandType = 0x47C9CD };

	GET(CellClass*, pThis, EDI);
	GET_STACK(BuildingTypeClass*, pBuildingType, STACK_OFFSET(0x18, 0x8));

	const int overlayTypeIndex = pThis->OverlayTypeIndex;

	if (overlayTypeIndex != -1)
	{
		if (OverlayTypeExt::CanPlaceBuildingOnOverlay(overlayTypeIndex, pBuildingType, false))
			return CheckTileLandType;
	}

	return ReturnFromFunction;
}

DEFINE_HOOK(0x45EF11, BuildingTypeClass_FlushForPlacement_Overlays, 0x6)
{
	enum { Continue = 0x45EF2C };

	GET(BuildingTypeClass*, pThis, EBX);
	GET(const int, overlayTypeIndex, ECX);

	if (OverlayTypeExt::CanPlaceBuildingOnOverlay(overlayTypeIndex, pThis, false))
		return Continue;

	return 0;
}

#pragma endregion
