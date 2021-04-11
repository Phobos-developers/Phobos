#include "FogOfWar.h"

// ; process cell
// ;//4ACBC2 = MapClass_UpdateFogOnMap, 7
// ;//4A9D74 = MapClass_RevealFogShroud_RegisterCell, A
// 4ACE3C = MapClass_TryReshroudCell_SetCopyFlag, 6
// 4A9CA0 = MapClass_RevealFogShroud, 7
// 486BF0 = CellClass_CleanFog, 9
// 486A70 = CellClass_FogCell, 5
// ;//457AA0 = BuildingClass_FreezeInFog, 5
// 440B8D = BuildingClass_Put_CheckFog, 6
// 486C50 = CellClass_ClearFoggedObjects, 6
// 
// 70076E = TechnoClass_GetCursorOverCell_OverFog, 5
// 6D3470 = TacticalClass_DrawFoggedObject, 8
// 51F97C = InfantryClass_MouseOverCell_OverFog, 5

DEFINE_HOOK(4ACE3C, MapClass_TryReshroudCell_SetCopyFlag, 6)
{
	GET(CellClass*, pCell, EAX);

	auto oldfield = pCell->CopyFlags;
	pCell->CopyFlags = oldfield & 0xFFFFFFEF;
	auto nIndex = TacticalClass::Instance->GetOcclusion(pCell->MapCoords, false);
	if (((oldfield & 0x10) != 0 || pCell->Visibility != nIndex) && nIndex >= 0 && pCell->Visibility >= -1)
	{
		pCell->CopyFlags |= 8u;
		pCell->Visibility = nIndex;
	}
	TacticalClass::Instance->RegisterCellAsVisible(pCell);

	return 0x4ACE57;
}

DEFINE_HOOK(4A9CA0, MapClass_RevealFogShroud, 7)
{
	// GET(MapClass*, pMap, ECX);
	auto const pMap = MapClass::Global();
	GET_STACK(CellStruct*, pCell, 0x4);
	GET_STACK(HouseClass*, dwUnk, 0x8);
	// GET_STACK(bool, bUnk, 0xC);

	R->EAX(FogOfWar::MapClass_RevealFogShroud(pMap, pCell, dwUnk));

	return 0x4A9DC6;
}