#include "Body.h"
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

	for (auto& item : IsometricTileTypeExt::LightConvertEntities)
	{
		auto& vectors = item.second;

		for (int index = int(vectors.size()) - 1; index > 0; --index)
		{
			auto const pLightConvert = vectors[index];
			LightConvertClass::Array.Remove(pLightConvert);
			GameDelete(pLightConvert);

			vectors.erase(vectors.begin() + index);
		}
	}

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
	
	return 0x53ADE0;
}

static void __fastcall IsometricTileTypeClass_DrawIt_InitLightConvert(CellClass* pThis, void*,
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

DEFINE_FUNCTION_JUMP(CALL, 0x480384, IsometricTileTypeClass_DrawIt_InitLightConvert)
