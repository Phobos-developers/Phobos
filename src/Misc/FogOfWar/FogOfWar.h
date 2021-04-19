#pragma once

#include <Phobos.h>

#include "BasicHeaders.h"

class FogOfWar
{
public:
	static void Reveal_DisplayClass_All_To_Look_Ground(TechnoClass* pTechno, DWORD dwUnk, DWORD dwUnk2);
	static bool MapClass_RevealFogShroud(MapClass* pMap, CellStruct* pCell_, HouseClass* pHouse);
	static bool IsLocationFogged(CoordStruct* pCoord);
	static void ClearFoggedObjects(CellClass* pCell);

	static void FogCell_Building(BuildingClass* pBld, DynamicVectorClass<FoggedObjectClass*>* pFoggedArray,
		CellClass* pCell, bool translucent);
	static void FogCell_Overlay(int index, DynamicVectorClass<FoggedObjectClass*>* pFoggedArray,
		CellClass* pCell, int powerup);
	static void FogCell_Smudge(int index, DynamicVectorClass<FoggedObjectClass*>* pFoggedArray,
		CellClass* pCell, int frameidx);
	static void FogCell_Terrain(TerrainClass* pTerrain, DynamicVectorClass<FoggedObjectClass*>* pFoggedArray);
};