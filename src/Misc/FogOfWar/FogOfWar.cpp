#include "FogOfWar.h"

// issue #28 : Fix vanilla YR Fog of War bugs & issues
// Reimplement it would be nicer.

std::unordered_set<FoggedObject*> FogOfWar::FoggedObjects;

void FogOfWar::Reveal_DisplayClass_All_To_Look_Ground(TechnoClass* pTechno, DWORD dwUnk, DWORD dwUnk2)
{
#define _LOOK_ \
	{ \
		auto coords = pTechno->GetCoords(); \
		pTechno->See(0, dwUnk2); \
		if (pTechno->IsInAir()) \
			MapClass::Instance->RevealArea3(&coords, \
				pTechno->LastSightRange - 3, pTechno->LastSightRange + 3, false); \
		return; \
	}

	if (!dwUnk || pTechno->WhatAmI() != AbstractType::Building)
	{
		if (pTechno->GetTechnoType()->RevealToAll)
			_LOOK_;
		if (pTechno->DiscoveredByPlayer)
		{
			if (SessionClass::Instance && pTechno->Owner == HouseClass::Player)
				_LOOK_;
			if (pTechno->Owner->CurrentPlayer || pTechno->Owner->PlayerControl)
				_LOOK_;
		}
		auto const pHouse = pTechno->Owner;
		auto const pPlayer = HouseClass::Player;
		if (pPlayer)
		{
			if (pPlayer == pHouse)
				_LOOK_;
			if (pPlayer->ArrayIndex == pHouse->ArrayIndex || pPlayer->ArrayIndex != -1 && pHouse->IsAlliedWith(pPlayer))
				_LOOK_;
		}
	}

#undef _LOOK_
}

bool FogOfWar::MapClass_RevealFogShroud(MapClass* pMap, CellStruct* pCell_, HouseClass* pHouse)
{
	auto pCell = pMap->GetCellAt(*pCell_);
	bool bContainsBuilding = pCell->Flags & 2;
	bool bReturn = !bContainsBuilding || (pCell->CopyFlags & 8);
	bool bUnk = bReturn;
	pCell->Flags = pCell->Flags & 0xFFFFFFBF | 2;
	pCell->CopyFlags = pCell->CopyFlags & 0xFFFFFFDF | 8;
	char nOcclusion = TacticalClass::Instance->GetOcclusion(*pCell_, false);
	char nVisibility = pCell->Visibility;
	if (nOcclusion != nVisibility)
	{
		nVisibility = nOcclusion;
		bReturn = true;
		pCell->Visibility = nOcclusion;
	}
	if (nVisibility == -1)
		pCell->CopyFlags |= 0x10u;
	char nFoggedOcclusion = TacticalClass::Instance->GetOcclusion(*pCell_, true);
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
		pMap->reveal_check(pCell, pHouse, bUnk);
	}
	if (!bContainsBuilding && ScenarioClass::Instance->SpecialFlags.FogOfWar)
		pCell->CleanFog();
	return bReturn;
}

bool FogOfWar::IsLocationFogged(CoordStruct* pCoord)
{
	auto pCell = MapClass::Instance->GetCellAt(*pCoord);
	if (pCell->Flags & 2)
		return false;
	return ((pCell->GetNeighbourCell(3u)->Flags & 2) == 0);
}

void FogOfWar::ClearFoggedObjects(CellClass* pCell)
{
	auto const pExt = CellExt::ExtMap.Find(pCell);

	if (pExt->FoggedObjects.size())
	{
		for (auto pFoggedObject : pExt->FoggedObjects)
			if (pFoggedObject)
			{
				if (pFoggedObject->CoveredRTTIType == AbstractType::Building)
				{
					auto pRealCell = MapClass::Instance->GetCellAt(pFoggedObject->Location);
					if (pRealCell != pCell)
						if (auto const pRealExt = CellExt::ExtMap.Find(pRealCell))
							if (pRealExt->FoggedObjects.size())
							{
								auto itr = std::find(pRealExt->FoggedObjects.begin(), 
									pRealExt->FoggedObjects.end(), pFoggedObject);

								pRealExt->FoggedObjects.erase(itr);
							}
				}
				GameDelete(pFoggedObject);
			}

		pExt->FoggedObjects.clear();
	}
}

bool FogOfWar::DrawIfVisible(FoggedObject* pFoggedObject, RectangleStruct* pRect)
{
	if (!pFoggedObject->Translucent)
		return false;

	auto rect = pFoggedObject->Bound;
	rect.X += Drawing::SurfaceDimensions_Hidden.X - TacticalClass::Instance->TacticalPos0.X;
	rect.Y += Drawing::SurfaceDimensions_Hidden.Y - TacticalClass::Instance->TacticalPos0.Y;

	RectangleStruct ret = Drawing::Intersect(pRect, &rect, 0, 0);
	if (ret.Width <= 0 || ret.Height <= 0)
		return false;

	pFoggedObject->Draw(*pRect);

	return true;
}

void FogOfWar::UnionRectangle(RectangleStruct* rect1, RectangleStruct* rect2)
{
    RectangleStruct rect;
    rect.X = std::min(rect1->X, rect2->X);
    rect.Y = std::min(rect1->Y, rect2->Y);
    rect.Width = std::max(rect1->X + rect1->Width, rect2->X + rect2->Width) - rect.X;
    rect.Height = std::max(rect1->Y + rect1->Height, rect2->Y + rect2->Height) - rect.Y;
    *rect1 = rect;
}
