#include "FogOfWar.h"

// ; process cell
// ;//4ACBC2 = MapClass_UpdateFogOnMap, 7
// ;//4A9D74 = MapClass_RevealFogShroud_RegisterCell, A
// 4ACE3C = MapClass_TryReshroudCell_SetCopyFlag, 6
// 4A9CA0 = MapClass_RevealFogShroud, 7
// 486BF0 = CleanFog, 9
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
	auto const pMap = MapClass::Instance;
	GET_STACK(CellStruct*, pCell, 0x4);
	GET_STACK(HouseClass*, dwUnk, 0x8);
	// GET_STACK(bool, bUnk, 0xC);

	R->EAX(FogOfWar::MapClass_RevealFogShroud(pMap, pCell, dwUnk));

	return 0x4A9DC6;
}

DEFINE_HOOK(486BF0, CellClass_CleanFog, 9)
{
	GET(CellClass*, pCell_, ECX);

	auto pLocation = pCell_->MapCoords;
	for (int i = 1; i < 15; i += 2)
	{
		auto pCell = MapClass::Instance->GetCellAt(pLocation);
		if (pCell && pCell->Level >= i - 2 && pCell->Level <= i)
		{
			pCell->Flags |= cf_Fogged;
			FogOfWar::ClearFoggedObjects(pCell);
			++pLocation.X;
			++pLocation.Y;
		}
	}

	return 0x486C4C;
}

DEFINE_HOOK(486A70, CellClass_FogCell, 5)
{
	GET(CellClass*, pCell_, ECX);
	auto location = pCell_->MapCoords;
	if (ScenarioClass::Instance->SpecialFlags.FogOfWar)
	{
		for (int i = 1; i < 15; i += 2)
		{
			auto pCell = MapClass::Instance->GetCellAt(location);
			auto nLevel = pCell->Level;
			if (nLevel >= i - 2 && nLevel <= i)
			{
				if ((pCell->Flags & cf_Fogged) == 0)
				{
					pCell->Flags |= cf_Fogged;
					for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
					{
						switch (pObject->WhatAmI())
						{ // foots under the fog won't be drawn
						case AbstractType::Unit:
						case AbstractType::Infantry:
						case AbstractType::Aircraft:
							pObject->Deselect();
							break;
						case AbstractType::Building:
							if (auto pBld = abstract_cast<BuildingClass*>(pObject))
								if (pBld->Is_Fogged())
									GameCreate<FoggedBuilding>(pBld, pBld->IsStrange() || pBld->Translucency);
							break;
						case AbstractType::Terrain:
							if (auto pTer = abstract_cast<TerrainClass*>(pObject))
								GameCreate<FoggedTerrain>(pTer, pTer->GetArrayIndex());
							break;
						default:
							continue;
						}
					}
					if (pCell->OverlayTypeIndex != -1)
						GameCreate<FoggedOverlay>(pCell, pCell->OverlayTypeIndex, pCell->OverlayData);
					if (pCell->SmudgeTypeIndex != -1 && !pCell->SmudgeData)
						GameCreate<FoggedSmudge>(pCell, pCell->SmudgeTypeIndex, pCell->SmudgeData);
				}
			}

			++location.X;
			++location.Y;
		}
	}

	return 0x486BE6;
}

DEFINE_HOOK(440B8D, BuildingClass_Put_CheckFog, 6)
{
	GET(BuildingClass*, pBuilding, ESI);

	if (pBuilding->Is_Fogged())
		GameCreate<FoggedBuilding>(pBuilding, pBuilding->IsStrange() || pBuilding->Translucency);

	return 0x440C08;
}

DEFINE_HOOK(486C50, CellClass_ClearFoggedObjects, 6)
{
	GET(CellClass*, pCell, ECX);

	FogOfWar::ClearFoggedObjects(pCell);

	return 0x486D8A;
}
//
//DEFINE_HOOK(70076E, TechnoClass_GetCursorOverCell_OverFog, 5)
//{
//	GET(CellClass*, pCell, EBP);
//
//	if (pCell->FoggedObjects && pCell->FoggedObjects->Count > 0)
//	{
//		int nOverlayIndex = -1;
//		for (auto pFoggedObject : *pCell->FoggedObjects)
//		{
//			if (pFoggedObject->Translucent)
//			{
//				if (pFoggedObject->CoveredAbstractType == AbstractType::Overlay)
//					nOverlayIndex = pFoggedObject->OverlayIndex;
//				else if (pFoggedObject->CoveredAbstractType == AbstractType::Building)
//					if (!pFoggedObject->Owner || !pFoggedObject->Owner->IsAlliedWith(HouseClass::Player))
//						if (pFoggedObject->DrawRecords.Count <= 0)
//							R->Stack<bool>(STACK_OFFS(0x2C, 0x19), true);
//			}
//		}
//		if (nOverlayIndex != -1)
//			R->Stack<OverlayTypeClass*>(STACK_OFFS(0x2C, 0x18), OverlayTypeClass::Array->GetItem(nOverlayIndex));
//	}
//	
//	return 0x700800;
//}

// TO BE IMPLEMENTED!
// This function is the key to reduce lag I think
DEFINE_HOOK(6D3470, TacticalClass_DrawFoggedObject, 8)
{
	// GET(TacticalClass*, pTactical, ECX);
	auto const pTactical = TacticalClass::Instance;
	GET_STACK(RectangleStruct*, pRect1, 0x4);
	GET_STACK(RectangleStruct*, pRect2, 0x8);
	GET_STACK(bool, bUkn, 0xC);

	// Draw them

	return 0x6D3650;
}

// TO BE IMPLEMENTED!
//DEFINE_HOOK(51F97C, InfantryClass_MouseOverCell_OverFog, 5)
//{
//	GET(InfantryClass*, pInf, EDI);
//	GET(CellClass*, pCell, EAX);
//
//	enum { DefaultAction = 0x51F9F4, NoMove = 0x51FA6A };
//
//	if (pCell->FoggedObjects && pCell->FoggedObjects->Count > 0)
//	{
//		for (auto pFoggedObject : *pCell->FoggedObjects)
//		{
//			if (pFoggedObject->Translucent && pFoggedObject->CoveredAbstractType == AbstractType::Building)
//			{
//				R->ESI<FoggedObjectClass*>(pFoggedObject);
//				break;
//			}
//		}
//		R->EBP()
//	}
//
//	return 0;
//}