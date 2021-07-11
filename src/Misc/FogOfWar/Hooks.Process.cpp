#include <Helpers/Macro.h>
#include <DisplayClass.h>
#include <FootClass.h>
#include <TacticalClass.h>
#include <BuildingClass.h>
#include <TerrainClass.h>
#include <Drawing.h>
#include <ScenarioClass.h>
#include <OverlayClass.h>
#include <SmudgeClass.h>

#include <Ext/Cell/Body.h>

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

DEFINE_HOOK(0x4ACE3C, MapClass_TryReshroudCell_SetCopyFlag, 0x6)
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

DEFINE_HOOK(0x4A9CA0, MapClass_RevealFogShroud, 0x7)
{
	// GET(MapClass*, pMap, ECX);
	auto const pMap = MapClass::Instance();
	GET_STACK(CellStruct*, pLocation, 0x4);
	GET_STACK(HouseClass*, pHouse, 0x8);
	// GET_STACK(bool, bUnk, 0xC);

	auto const pCell = pMap->GetCellAt(*pLocation);
	bool bFlag = pCell->Flags & 2;
	bool bReturn = !bFlag || (pCell->CopyFlags & cf2_NoShadow);
	bool bTemp = bReturn;

	pCell->Flags = pCell->Flags & 0xFFFFFFFF | 2;
	pCell->CopyFlags = pCell->CopyFlags & 0xFFFFFFDF | 8;

	char nOcclusion = TacticalClass::Instance->GetOcclusion(*pLocation, false);
	char nVisibility = pCell->Visibility;
	if (nOcclusion != nVisibility)
	{
		nVisibility = nOcclusion;
		bReturn = true;
		pCell->Visibility = nOcclusion;
	}
	if (nVisibility == -1)
		pCell->CopyFlags |= 0x10u;
	char nFoggedOcclusion = TacticalClass::Instance->GetOcclusion(*pLocation, true);
	char nFoggedness = pCell->Foggedness;
	if (nFoggedOcclusion != nFoggedness)
	{
		nFoggedness = nFoggedOcclusion;
		bReturn = true;
		pCell->Foggedness = nFoggedOcclusion;
	}
	if (nFoggedness == -1)
		pCell->Flags |= 1u;

	if (bReturn)
	{
		TacticalClass::Instance->RegisterCellAsVisible(pCell);
		pMap->RevealCheck(pCell, pHouse, bTemp);
	}
	if (!bFlag && ScenarioClass::Instance->SpecialFlags.FogOfWar)
		pCell->CleanFog();

	R->AL(bReturn);

	return 0x4A9DC6;
}

DEFINE_HOOK(0x486BF0, CellClass_CleanFog, 0x9)
{
	GET(CellClass*, pThis, ECX);

	auto pLocation = pThis->MapCoords;
	for (int i = 1; i < 15; i += 2)
	{
		auto pCell = MapClass::Instance->GetCellAt(pLocation);
		if (pCell && pCell->Level >= i - 2 && pCell->Level <= i)
		{
			pCell->Flags |= cf_Fogged;
			pCell->ClearFoggedObjects();
			++pLocation.X;
			++pLocation.Y;
		}
	}

	return 0x486C4C;
}

DEFINE_HOOK(0x486A70, CellClass_FogCell, 0x5)
{
	GET(CellClass*, pThis, ECX);
	auto location = pThis->MapCoords;
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
						{ // foots under the fog won't be drawn (I don't know how to draw either)
						case AbstractType::Unit:
						case AbstractType::Infantry:
						case AbstractType::Aircraft:
							pObject->Deselect();
							break;
						case AbstractType::Building:
							if (auto pBld = abstract_cast<BuildingClass*>(pObject))
								if (pBld->CheckFog())
									GameCreate<FoggedBuilding>(pBld);
							break;
						case AbstractType::Terrain:
							if (auto pTer = abstract_cast<TerrainClass*>(pObject))
								GameCreate<FoggedTerrain>(pTer);
							break;
						default:
							continue;
						}
					}

					if (pCell->OverlayTypeIndex != -1)
						GameCreate<FoggedOverlay>(pCell);
					if (pCell->SmudgeTypeIndex != -1 && !pCell->SmudgeData)
						GameCreate<FoggedSmudge>(pCell);
				}
			}

			++location.X;
			++location.Y;
		}
	}

	return 0x486BE6;
}

DEFINE_HOOK(0x440B8D, BuildingClass_Put_CheckFog, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	if (pBuilding->CheckFog())
		GameCreate<FoggedBuilding>(pBuilding);

	return 0x440C08;
}

DEFINE_HOOK(0x486C50, CellClass_ClearFoggedObjects, 0x6)
{
	GET(CellClass*, pThis, ECX);
	auto const pExt = CellExt::ExtMap.Find(pThis);

	for (auto const pFoggedObject : pExt->FoggedObjects)
		if (pFoggedObject)
		{
			auto const pCell = pFoggedObject->AttachedCell;
			if (pThis != pCell)
			{
				auto pAttachedExt = CellExt::ExtMap.Find(pThis);
				auto itr = std::find(pAttachedExt->FoggedObjects.begin(), pAttachedExt->FoggedObjects.end(), pFoggedObject);
				if (itr != pAttachedExt->FoggedObjects.end())
					pAttachedExt->FoggedObjects.erase(itr);
			}
			GameDelete(pFoggedObject);
		}
	pExt->FoggedObjects.clear();

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
DEFINE_HOOK(0x6D3470, TacticalClass_DrawFoggedObject, 0x8)
{
	GET(TacticalClass*, pTactical, ECX);
	// auto const pTactical = TacticalClass::Instance;
	GET_STACK(RectangleStruct*, pRect1, 0x4);
	GET_STACK(RectangleStruct*, pRect2, 0x8);
	GET_STACK(bool, bUkn, 0xC);

	// Draw them

	RectangleStruct rect { 0,0,0,0 };

	if (bUkn && DSurface::ViewBounds().Width > 0 && DSurface::ViewBounds().Height > 0)
		rect = Drawing::Union(rect, DSurface::ViewBounds());
	else
	{
		RectangleStruct buffer;

		if (pRect1->Width > 0 && pRect1->Height > 0)
			rect = Drawing::Union(rect, *pRect1);
		if (pRect2->Width > 0 && pRect2->Height > 0)
			rect = Drawing::Union(rect, *pRect2);

		if (auto& nVisibleCellCount = pTactical->VisibleCellCount)
		{
			buffer.Width = buffer.Height = 60;

			for (int i = 0; i < nVisibleCellCount; ++i)
			{
				auto const pCell = pTactical->VisibleCells[i];
				auto location = pCell->GetCoords();
				Point2D point;
				TacticalClass::Instance->CoordsToClient(location, &point);
				buffer.X = DSurface::ViewBounds().X + point.X - 30;
				buffer.Y = DSurface::ViewBounds().Y + point.Y;
				rect = Drawing::Union(rect, buffer);
			}
		}

		for (auto const& dirty : Drawing::DirtyAreas())
		{
			buffer = dirty.Rect;
			buffer.Y += DSurface::ViewBounds().Y;
			if (buffer.Width > 0 && buffer.Height > 0)
				rect = Drawing::Union(rect, buffer);
		}
	}

	if (rect.Width > 0 && rect.Height > 0)
	{
		rect.X = std::max(rect.X, 0);
		rect.Y = std::max(rect.Y, 0);
		rect.Width = std::min(rect.Width, DSurface::ViewBounds().Width - rect.X);
		rect.Height = std::min(rect.Height, DSurface::ViewBounds().Height - rect.Y);

		// Draw if visible
		for (auto const pFoggedObj : FoggedObject::Instances)
			pFoggedObj->DrawIt(DSurface::ViewBounds());
	}

	return 0x6D3650;
}

// TO BE IMPLEMENTED!
//DEFINE_HOOK(51F97C, InfantryClass_MouseOverCell_OverFog, 0x5)
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