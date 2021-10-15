#include "Body.h"

#include <Helpers/Macro.h>

#include <CellClass.h>
#include <ConvertClass.h>

DEFINE_HOOK(0x544E70, IsometricTileTypeClass_Init_Drawer, 0x8)
{
	GET(CellClass*, pCell, ESI); // Luckily, pCell is just ESI, so we don't need other hooks to set it

	GET(int, Red, ECX);
	GET(int, Green, EDX);
	GET_STACK(int, Blue, 0x4);

	if (auto pData = IsometricTileTypeExt::ExtMap.Find(IsometricTileTypeClass::Array->GetItemOrDefault(pCell->IsoTileTypeIndex)))
	{
		R->EAX(IsometricTileTypeExt::InitDrawer(IsometricTileTypeExt::LoadedPalettesLookUp[pData->Tileset], Red, Green, Blue));
		return 0x544FDE;
	}

	return 0;
}