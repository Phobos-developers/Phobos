#include "FogOfWar.h"

#include <InfantryClass.h>

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
	GET_STACK(CellStruct*, pCell, 0x4);
	GET_STACK(HouseClass*, dwUnk, 0x8);
	// GET_STACK(bool, bUnk, 0xC);

	R->EAX(FogOfWar::MapClass_RevealFogShroud(pCell, dwUnk));

	return 0x4A9DC6;
}

DEFINE_HOOK(0x486BF0, CellClass_CleanFog, 0x9)
{
	GET(CellClass*, pCell_, ECX);

	auto pLocation = pCell_->MapCoords;
	for (int i = 1; i < 15; i += 2)
	{
		auto pCell = MapClass::Instance->GetCellAt(pLocation);
		if (pCell && pCell->Level == i - 1) // pCell->Level >= i - 2 && pCell->Level <= i
		{
			pCell->Flags &= ~cf_Fogged;
			FogOfWar::ClearFoggedObjects(pCell);
			++pLocation.X;
			++pLocation.Y;
		}
	}

	return 0x486C4C;
}

DEFINE_HOOK(0x486A70, CellClass_FogCell, 0x5)
{
	GET(CellClass*, pCell_, ECX);
	auto location = pCell_->MapCoords;
	if (ScenarioClass::Instance->SpecialFlags.FogOfWar)
	{
		for (int i = 1; i < 15; i += 2)
		{
			auto pCell = MapClass::Instance->GetCellAt(location);
			auto nLevel = pCell->Level;
			if (nLevel == i - 1) // if (nLevel >= i - 2 && nLevel <= i)
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
								if (pBld->IsBuildingFogged())
									GameCreate<FoggedBuilding>(pBld, pBld->IsStrange() || pBld->Translucency);
							break;
						case AbstractType::Terrain:
							if (auto pTer = abstract_cast<TerrainClass*>(pObject))
								GameCreate<FoggedTerrain>(pTer, pTer->Type->ArrayIndex);
							break;
						default:
							continue;
						}
					}
					if (pCell->OverlayTypeIndex != -1)
						GameCreate<FoggedOverlay>(pCell, pCell->OverlayTypeIndex, pCell->Powerup);
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

DEFINE_HOOK(0x440B8D, BuildingClass_Put_CheckFog, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	if (pBuilding->IsBuildingFogged())
		GameCreate<FoggedBuilding>(pBuilding, pBuilding->IsStrange() || pBuilding->Translucency);

	return 0x440C08;
}

DEFINE_HOOK(0x486C50, CellClass_ClearFoggedObjects, 0x6)
{
	GET(CellClass*, pCell, ECX);

	FogOfWar::ClearFoggedObjects(pCell);

	return 0x486D8A;
}

DEFINE_HOOK(0x70076E, TechnoClass_GetCursorOverCell_OverFog, 0x5)
{
	GET(CellClass*, pCell, EBP);

	auto pCellExt = CellExt::ExtMap.Find(pCell);

	if(pCellExt->FoggedObjects.size() == 0)
		return 0x700800;
	
	for (auto& fogged : pCellExt->FoggedObjects)
	{
		if (fogged->Translucent)
		{
			if (fogged->CoveredRTTIType == AbstractType::Overlay)
			{
				int nOverlayIdx = fogged->GetType();
				if (nOverlayIdx >= 0)
				{
					R->Stack(STACK_OFFS(0x2C, 0x14), nOverlayIdx);
					break;
				}
			}
			else if (fogged->CoveredRTTIType == AbstractType::Building)
			{
				auto pBld = static_cast<FoggedBuilding*>(fogged);
				if (!pBld->Owner || !HouseClass::Player->IsAlliedWith(pBld->Owner))
				{
					R->Stack(STACK_OFFS(0x2C, 0x19), true);
					break;
				}
			}
		}
	}

	return 0x700800;
}

// This function is the key to reduce lag I think
DEFINE_HOOK(0x6D3470, TacticalClass_DrawFoggedObject, 0x8)
{
	GET(TacticalClass*, pTactical, ECX);
	// auto const pTactical = TacticalClass::Instance;
	GET_STACK(RectangleStruct*, pRect1, 0x4);
	GET_STACK(RectangleStruct*, pRect2, 0x8);
	GET_STACK(bool, bUkn, 0xC);

	// Draw them

	RectangleStruct rect{ 0,0,0,0 };

	if (bUkn && DSurface::ViewBounds->Width > 0 && DSurface::ViewBounds->Height > 0)
		FogOfWar::UnionRectangle(&rect, &DSurface::ViewBounds());
	else
	{
		RectangleStruct buffer;

		if (pRect1->Width > 0 && pRect1->Height > 0)
			FogOfWar::UnionRectangle(&rect, pRect1);
		if (pRect2->Width > 0 && pRect2->Height > 0)
			FogOfWar::UnionRectangle(&rect, pRect2);

		if (auto& nVisibleCellCount = pTactical->VisibleCellCount)
		{
			buffer.Width = buffer.Height = 60;

			for (int i = 0; i < nVisibleCellCount; ++i)
			{
				auto const pCell = pTactical->VisibleCells[i];
				auto location = pCell->GetCoords();
				Point2D point;
				TacticalClass::Instance->CoordsToClient(location, &point);
				buffer.X = DSurface::ViewBounds->X + point.X - 30;
				buffer.Y = DSurface::ViewBounds->Y + point.Y;
				FogOfWar::UnionRectangle(&rect, &buffer);
			}
		}
		
		for (auto& dirty : Drawing::DirtyAreas())
		{
			buffer = dirty.Rect;
			buffer.Y += DSurface::ViewBounds->Y;
			if (buffer.Width > 0 && buffer.Height > 0)
				FogOfWar::UnionRectangle(&rect, &buffer);
		}
	}

	if (rect.Width > 0 && rect.Height > 0)
	{
		rect.X = std::max(rect.X, 0);
		rect.Y = std::max(rect.Y, 0);
		rect.Width = std::min(rect.Width, DSurface::ViewBounds->Width - rect.X);
		rect.Height = std::min(rect.Height, DSurface::ViewBounds->Height - rect.Y);

		for (auto pFoggedObj : FogOfWar::FoggedObjects)
			FogOfWar::DrawIfVisible(pFoggedObj, &rect);
	}

	return 0x6D3650;
}

DEFINE_HOOK(0x51F97C, InfantryClass_MouseOverCell_OverFog, 0x5)
{
	GET(InfantryClass*, pInf, EDI);
	GET(CellClass*, pCell, EAX);
	
	BuildingTypeClass* pBld = nullptr;
	CoordStruct* pCoord = nullptr;

	auto pCellExt = CellExt::ExtMap.Find(pCell);

	for (auto& fogged : pCellExt->FoggedObjects)
	{
		if (fogged->Translucent && fogged->CoveredRTTIType == AbstractType::Building)
		{
			pBld = fogged->GetBuildingType();
			pCoord = &fogged->Location;
			break;
		}
	}
	if (!pBld)
		return 0x51F9F4;

	R->EBP(pBld);

	if (pBld->BridgeRepairHut || pInf->Type->Engineer || pInf->Owner->ControlledByPlayer())
	{
		LEA_STACK(CellStruct*, pCellPos, STACK_OFFS(0x1C, -0xC));
		*pCellPos = CellClass::Coord2Cell(*pCoord);
		R->EDX(pCellPos);
		return 0x51FA6A;
	}

	return 0x51F9F4;
}