#include "FogOfWar.h"

// issue #28 : Fix vanilla YR Fog of War bugs & issues
// Reimplement it would be nicer.

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
	auto pCell = MapClass::Global()->GetCellAt(*pCoord);
	if (pCell->Flags & 2)
		return false;
	return ((pCell->GetNeighbourCell(3u)->Flags & 2) == 0);
}

void FogOfWar::ClearFoggedObjects(CellClass* pCell)
{
	if (pCell->FoggedObjects)
	{
		for (auto pFoggedObject : *pCell->FoggedObjects)
			if (pFoggedObject)
			{
				if (pFoggedObject->CoveredAbstractType == AbstractType::Building)
				{
					auto cell = pFoggedObject->GetMapCoords();
					auto pRealCell = MapClass::Instance->GetCellAt(cell);
					if (pRealCell != pCell)
						if (pRealCell->FoggedObjects)
							pRealCell->FoggedObjects->Remove(pFoggedObject);
				}
				GameDelete(pFoggedObject);
			}

		pCell->FoggedObjects->Clear();
		GameDelete(pCell->FoggedObjects);

		pCell->FoggedObjects = nullptr;
	}
			
}

void FogOfWar::FogCell_Building(BuildingClass* pBld, DynamicVectorClass<FoggedObjectClass*>* pFoggedArray, 
	CellClass* pCell, bool translucent)
{
	auto pFoggedBld = GameCreate<FoggedObjectClass>(pBld, translucent);
	auto pFoundationData = pBld->GetFoundationData();
	auto coordCell = pBld->GetMapCoords();

	while (pFoundationData->X != 0x7FFF || pFoundationData->Y != 0x7FFF)
	{
		coordCell.X += pFoundationData->X;
		coordCell.Y += pFoundationData->Y;
		auto pRealCell = MapClass::Instance->GetCellAt(coordCell);
		if (pCell && pCell == pRealCell)
			pFoggedArray->AddItem(pFoggedBld);
		else
		{
			if (pRealCell->FoggedObjects)
				pFoggedArray = pRealCell->FoggedObjects;
			else
			{
				auto pNewFoggedObjects = GameCreate<DynamicVectorClass<FoggedObjectClass*>>();
				pRealCell->FoggedObjects = pNewFoggedObjects;
				pNewFoggedObjects->SetCapacity(1);
				pNewFoggedObjects->CapacityIncrement = 1;
				pFoggedArray = pNewFoggedObjects;
			}
		}
		++pFoundationData;
	}
}

void FogOfWar::FogCell_Overlay(int index, DynamicVectorClass<FoggedObjectClass*>* pFoggedArray, CellClass* pCell, int powerup)
{
	auto coord = pCell->GetCoords();
	auto pFoggedOvl = GameCreate<FoggedObjectClass>(coord, index, powerup);
	pFoggedArray->AddItem(pFoggedOvl);
}

void FogOfWar::FogCell_Smudge(int index, DynamicVectorClass<FoggedObjectClass*>* pFoggedArray, CellClass* pCell, int frameidx)
{
	auto coord = pCell->GetCoords();
	auto pFoggedSmu = GameCreate<FoggedObjectClass>(&coord, index, frameidx);
	pFoggedArray->AddItem(pFoggedSmu);
}

void FogOfWar::FogCell_Terrain(TerrainClass* pTerrain, DynamicVectorClass<FoggedObjectClass*>* pFoggedArray)
{
	auto pFoggedTer = GameCreate<FoggedObjectClass>(pTerrain);
	pFoggedArray->AddItem(pFoggedTer);
}
