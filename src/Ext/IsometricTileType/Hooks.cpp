#include "Body.h"

#include <CellClass.h>

CellClass* currentCell = nullptr;

inline void ReplacePalette(BytePalette** ppDest)
{
	if (currentCell) {
		auto pData = IsometricTileTypeExt::ExtMap.Find(currentCell->GetIsoTileType());

		if (pData && pData->Palette)
			*ppDest = pData->Palette;
	}
}

DEFINE_HOOK_AGAIN(484050, CellClass_Context_Set, 5)
DEFINE_HOOK(483E30, CellClass_Context_Set, 7)
{
	currentCell = R->ECX<CellClass*>();

	return 0;
}

DEFINE_HOOK_AGAIN(484150, CellClass_Context_Unset, 5)
DEFINE_HOOK(48403B, CellClass_Context_Unset, 6)
{
	currentCell = nullptr;

	return 0;
}

//DEFINE_HOOK_AGAIN(54508F, IsometricTileTypeClass_light_545000_CustomPalette, 5)
DEFINE_HOOK(544F32, IsometricTileTypeClass_SetupLightConvert_CustomPalette, 5)
{
	GET(int, shadeCount, ESI);

	GET_STACK(int, r, STACK_OFFS(0x18, +0x4));
	GET_STACK(int, g, STACK_OFFS(0x18, -0x8));
	GET_STACK(int, b, STACK_OFFS(0x18, -0x4));

	BytePalette* TilePalette = reinterpret_cast<BytePalette*>(0xABBED0);

	if (currentCell) {
		auto pData = IsometricTileTypeExt::ExtMap.Find(currentCell->GetIsoTileType());

		if (pData && pData->Palette) {
			TilePalette = pData->Palette;
		}
	}

	LightConvertClass* pLightConvert = GameCreate<LightConvertClass>(
		TilePalette, FileSystem::TEMPERAT_PAL, DSurface::Primary,
		r, g, b, LightConvertClass::Array->Count != 0, 0, shadeCount);
	
	R->ESI<LightConvertClass*>(pLightConvert);

	return 0x544F7F;
}

DEFINE_HOOK(544E94, IsometricTileTypeClass_SetupLightConvert_CustomPaletteCheck0, 6)
{
	if (currentCell) {
		auto pData = IsometricTileTypeExt::ExtMap.Find(currentCell->GetIsoTileType());

		if (pData && pData->Palette)
			return 0x544EC0;
	}

	return 0;
}

DEFINE_HOOK(544EF8, IsometricTileTypeClass_SetupLightConvert_CustomPaletteCheck1, 6)
{
	GET(LightConvertClass*, pLightConvert, EAX);

	if (currentCell) {
		auto pData = IsometricTileTypeExt::ExtMap.Find(currentCell->GetIsoTileType());

		if (pData && pData->Palette && pData->Palette != pLightConvert->UsedPalette1)
			return 0x544F14;
	}

	return 0;
}

/*
DEFINE_HOOK(545000, Skip11111, 0)
{
	return 0x545149;
}
*/
DEFINE_HOOK(546E81, Load_Tiles_Into_Memory, 6)
{
	GET(CellClass*, pCell, EDI);
	const int TileType = pCell->IsoTileTypeIndex;

	if (TileType != 0xFFFF && IsometricTileTypeClass::Array->ValidIndex(TileType))
	{
		auto pTile = IsometricTileTypeClass::Array->GetItem(TileType);

		char fileName[sizeof(pTile->FileName) + 1];
		strcpy_s(fileName, pTile->FileName);
		fileName[14] = 0;

		auto pTileExt = IsometricTileTypeExt::ExtMap.FindOrAllocate(pTile);

		Debug::Log("Load_Tiles_Into_Memory\n");
		Debug::Log("\t%s: %s\n", "FileName", fileName);
		Debug::Log("\t%s: %d\n", "TileSetNumber", pTileExt->TileSetNumber);
		Debug::Log("\n");
	}
	return 0;
}
