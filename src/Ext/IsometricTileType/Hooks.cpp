#include "Body.h"

#include <Helpers/Macro.h>

#include <CellClass.h>
#include <ConvertClass.h>

DEFINE_HOOK(544E70, IsometricTileTypeClass_Init_Drawer, 8)
{
	GET(CellClass*, pCell, ESI); // Luckily, pCell is just ESI, so we don't need other hooks to set it

	GET(int, Red, ECX);
	GET(int, Green, EDX);
	GET_STACK(int, Blue, 0x4);
	
	TintStruct tint { Red,Green,Blue };
	auto const pItem = IsometricTileTypeClass::Array->GetItemOrDefault(pCell->IsoTileTypeIndex);

	R->EAX(IsometricTileTypeExt::InitDrawer(pItem, tint));
	return 0x544FDE;	
}