#include "Body.h"

#include <Helpers/Macro.h>

#include <CellClass.h>
#include <ConvertClass.h>

DEFINE_HOOK(0x544E70, IsometricTileTypeClass_Init_Drawer, 0x8)
{
	GET(CellClass*, pCell, ESI); // Luckily, pCell is just ESI, so we don't need other hooks to set it

	GET(int, red, ECX);
	GET(int, green, EDX);
	GET_STACK(int, blue, 0x4);

	int isoTileTypeIndex = pCell->IsoTileTypeIndex;

	if (0 <= isoTileTypeIndex && isoTileTypeIndex < IsometricTileTypeClass::Array->Count)
	{
		const auto pIsoTypeExt = IsometricTileTypeExt::ExtMap.Find(IsometricTileTypeClass::Array->GetItem(pCell->IsoTileTypeIndex));

		if (pIsoTypeExt != nullptr)
		{
			LightConvertClass* pLightConvert = pIsoTypeExt->GetLightConvert(red, green, blue);

			//LightConvertClass* tmp = IsometricTileTypeExt::InitDrawer(IsometricTileTypeExt::LoadedPalettesLookUp[pData->Tileset], Red, Green, Blue);

			R->EAX(pLightConvert);
			return 0x544FDE;
		}
	}

	return 0;
}
