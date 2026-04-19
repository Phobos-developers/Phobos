#include "Body.h"
#include <LightSourceClass.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x544E70, IsometricTileTypeClass_Init_Drawer, 0x8)
{
	enum { SkipGameCode = 0x544FDE };

	GET(CellClass* const, pCell, ESI); // Luckily, pCell is just ESI, so we don't need other hooks to set it
	GET(const int, red, ECX);
	GET(const int, green, EDX);
	GET_STACK(const int, blue, 0x4);

	LightConvertClass* pLightConvert = nullptr;
	const int isoTileTypeIndex = pCell->IsoTileTypeIndex;

	if (isoTileTypeIndex == 0xFFFF && IsometricTileTypeExt::InRender)
	{
		if (const auto pIsoTypeExt = IsometricTileTypeExt::ExtMap.Find(IsometricTileTypeClass::Array.Items[0]))
		{
			char paletteName[64];
			_snprintf(paletteName, sizeof(paletteName), pIsoTypeExt->PaletteName.data());
			_strupr(paletteName);

			pLightConvert = IsometricTileTypeExt::GetLightConvert(paletteName, red, green, blue, false);
		}
	}
	else if (isoTileTypeIndex >= 0 && isoTileTypeIndex < IsometricTileTypeClass::Array.Count)
	{
		if (const auto pIsoTypeExt = IsometricTileTypeExt::ExtMap.Find(IsometricTileTypeClass::Array.Items[isoTileTypeIndex]))
		{
			char paletteName[64];
			_snprintf(paletteName, sizeof(paletteName), pIsoTypeExt->PaletteName.data());
			_strupr(paletteName);

			pLightConvert = IsometricTileTypeExt::GetLightConvert(paletteName, red, green, blue, false);
		}
	}

	R->EAX(pLightConvert);
	return SkipGameCode;
}

DEFINE_HOOK(0x53ADD6, ScenarioClass_RecalcLighting_Reset, 0x5)
{
	enum { SkipGameCode = 0x53ADE0 };

	for (auto& entities : IsometricTileTypeExt::LightConvertEntities)
	{
		auto& vectors = entities.second;

		for (auto& item : vectors)
		{
			if (auto const pLightConvert = item.second)
			{
				LightConvertClass::Array.Remove(pLightConvert);
				GameDelete(pLightConvert);

				item.second = nullptr;
			}
		}

		vectors.clear();
	}

	IsometricTileTypeExt::LightConvertEntities.clear();
	IsometricTileTypeExt::InRender = true;

	MapClass::Instance.CellIteratorReset();

	for (CellClass* pCell = MapClass::Instance.CellIteratorNext(); pCell; pCell = MapClass::Instance.CellIteratorNext())
	{
		if (auto const pLightConvert = pCell->LightConvert)
		{
			const int index = LightConvertClass::Array.FindItemIndex(pLightConvert);

			if (index < 0)
				pCell->LightConvert = nullptr;
		}

		pCell->UpdateCellLighting();
	}

	IsometricTileTypeExt::InRender = false;

	return SkipGameCode;
}

static void __fastcall CellClass_InitLightConvert_CustomPalette(CellClass* pThis, void*,
	LightConvertClass* pDrawer,
	int Intensity,
	int Ambient,
	int Red,
	int Green,
	int Blue)
{
	IsometricTileTypeExt::InRender = true;
	pThis->InitLightConvert(pDrawer, Intensity, Ambient, Red, Green, Blue);
	IsometricTileTypeExt::InRender = false;
}

DEFINE_FUNCTION_JUMP(CALL, 0x480384, CellClass_InitLightConvert_CustomPalette)	// IsometricTileTypeClass_DrawIt
DEFINE_FUNCTION_JUMP(CALL, 0x423273, CellClass_InitLightConvert_CustomPalette)	// AnimClass::DrawIt
DEFINE_FUNCTION_JUMP(CALL, 0x47F748, CellClass_InitLightConvert_CustomPalette)	// CellClass::DrawOverlay
DEFINE_FUNCTION_JUMP(CALL, 0x47F5C9, CellClass_InitLightConvert_CustomPalette)	// CellClass::DrawOverlayShadow
DEFINE_FUNCTION_JUMP(CALL, 0x71C27B, CellClass_InitLightConvert_CustomPalette)	// TerrainClass::DrawIt
DEFINE_FUNCTION_JUMP(CALL, 0x71C40A, CellClass_InitLightConvert_CustomPalette)	// TerrainClass::DrawAgain
DEFINE_FUNCTION_JUMP(CALL, 0x705F42, CellClass_InitLightConvert_CustomPalette)	// TechnoClass::DrawShape
DEFINE_FUNCTION_JUMP(CALL, 0x7060F7, CellClass_InitLightConvert_CustomPalette)
DEFINE_FUNCTION_JUMP(CALL, 0x4D1BC9, CellClass_InitLightConvert_CustomPalette)	// sub_4D1890
DEFINE_FUNCTION_JUMP(CALL, 0x4D1EC9, CellClass_InitLightConvert_CustomPalette)

static void __fastcall LightSourceClass_UpdateLightConverts_CustomPalette(int value)
{
	IsometricTileTypeExt::InRender = true;
	LightSourceClass::UpdateLightConverts(value);
	IsometricTileTypeExt::InRender = false;
}

DEFINE_JUMP(CALL, 0x554B2E, GET_OFFSET(LightSourceClass_UpdateLightConverts_CustomPalette))
DEFINE_JUMP(CALL, 0x55B5F1, GET_OFFSET(LightSourceClass_UpdateLightConverts_CustomPalette))
