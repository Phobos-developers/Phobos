#include "FogOfWar.h"

#include "FoggedObject.h"

#include <Helpers/Macro.h>

#include <InfantryClass.h>
#include <TerrainClass.h>
#include <ScenarioClass.h>
#include <TacticalClass.h>
#include <HouseClass.h>
#include <Drawing.h>

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
	GET_STACK(CellStruct*, pCell, 0x4);
	GET_STACK(HouseClass*, dwUnk, 0x8);

	R->EAX(FogOfWar::MapClass_RevealFogShroud(pCell, dwUnk));

	return 0x4A9DC6;
}

DEFINE_HOOK(0x486BF0, CellClass_CleanFog, 0x9)
{
	GET(CellClass*, pCell_, ECX);

	auto location = pCell_->MapCoords;
	for (int i = 1; i < 15; i += 2)
	{
		auto pCell = MapClass::Instance->GetCellAt(location);
		if (pCell && pCell->Level >= i - 2 && pCell->Level <= i)
		{
			pCell->Flags &= ~cf_Fogged;
			FogOfWar::ClearFoggedObjects(pCell);

			RectangleStruct Dirty;
			pCell->GetContainingRect(&Dirty);
			TacticalClass::Instance->RegisterDirtyArea(Dirty, true);
		}
		++location.X;
		++location.Y;
	}

	return 0x486C4C;
}

DEFINE_HOOK(0x486A70, CellClass_FogCell, 0x5)
{
	if (HouseClass::Player->IsObserver() || HouseClass::Player->Defeated)
		return 0x486BE6;

	GET(CellClass*, pThis, ECX);

	if (ScenarioClass::Instance->SpecialFlags.FogOfWar)
	{
		auto location = pThis->MapCoords;
		for (int i = 1; i < 15; i += 2)
		{
			auto pCell = MapClass::Instance->GetCellAt(location);
			auto nLevel = pCell->Level;

			if (nLevel >= i - 2 && nLevel <= i)
			{
				if (!(pCell->Flags & cf_Fogged))
				{
					pCell->Flags |= cf_Fogged;

					for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
					{
						switch (pObject->WhatAmI())
						{
						case AbstractType::Unit:
						case AbstractType::Infantry:
						case AbstractType::Aircraft:
							pObject->Deselect();
							break;

						case AbstractType::Building:
							if (auto pBld = abstract_cast<BuildingClass*>(pObject))
								if (pBld->IsBuildingFogged())
									GameCreate<FoggedBuilding>(pBld, true);
							break;

						case AbstractType::Terrain:
							if (auto pTerrain = abstract_cast<TerrainClass*>(pObject))
							{
								auto& pType = pTerrain->Type;
								GameCreate<FoggedTerrain>(pTerrain, pType->ArrayIndex, pType->IsAnimated ? pTerrain->Animation.Value : 0);
							}
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
		GameCreate<FoggedBuilding>(pBuilding, false);

	return 0x440C08;
}

DEFINE_HOOK(0x486C50, CellClass_ClearFoggedObjects, 0x6)
{
	GET(CellClass*, pCell, ECX);

	FogOfWar::ClearFoggedObjects(pCell);

	return 0x486D8A;
}

DEFINE_HOOK(0x6D3470, TacticalClass_DrawFoggedObject, 0x8)
{
	GET_STACK(RectangleStruct*, pRect1, 0x4);
	GET_STACK(RectangleStruct*, pRect2, 0x8);
	GET_STACK(bool, bUkn, 0xC);

	// Draw them

	RectangleStruct rect { 0,0,0,0 };

	if (bUkn && DSurface::ViewBounds->Width > 0 && DSurface::ViewBounds->Height > 0)
		FogOfWar::UnionRectangle(&rect, &DSurface::ViewBounds());
	else
	{
		RectangleStruct buffer;

		if (pRect1->Width > 0 && pRect1->Height > 0)
			FogOfWar::UnionRectangle(&rect, pRect1);
		if (pRect2->Width > 0 && pRect2->Height > 0)
			FogOfWar::UnionRectangle(&rect, pRect2);

		if (auto& nVisibleCellCount = TacticalClass::Instance->VisibleCellCount)
		{
			buffer.Width = buffer.Height = 60;

			for (int i = 0; i < nVisibleCellCount; ++i)
			{
				auto const pCell = TacticalClass::Instance->VisibleCells[i];
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
			if (pFoggedObj->CoveredRTTIType == AbstractType::Building)
				FogOfWar::DrawBldIfVisible(static_cast<FoggedBuilding*>(pFoggedObj), &rect);
			else
				FogOfWar::DrawIfVisible(pFoggedObj, &rect);
	}

	return 0x6D3650;
}

DEFINE_HOOK(0x70076E, TechnoClass_GetCursorOverCell_OverFog, 0x5)
{
	GET(CellClass*, pCell, EBP);

	auto pCellExt = CellExt::ExtMap.Find(pCell);

	if(pCellExt->FoggedObjects.size() == 0)
		return 0x700800;
	
	for (auto& fogged : pCellExt->FoggedObjects)
	{
		if (fogged->Visible)
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

DEFINE_HOOK(0x51F97C, InfantryClass_MouseOverCell_OverFog, 0x5)
{
	GET(InfantryClass*, pInf, EDI);
	GET(CellClass*, pCell, EAX);
	
	BuildingTypeClass* pBld = nullptr;
	CoordStruct* pCoord = nullptr;

	auto pCellExt = CellExt::ExtMap.Find(pCell);

	for (auto& fogged : pCellExt->FoggedObjects)
	{
		if (fogged->Visible && fogged->CoveredRTTIType == AbstractType::Building)
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