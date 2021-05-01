#pragma once

#include <Phobos.h>

#include "BasicHeaders.h"
#include "FoggedObject.h"

#include "../../Ext/Cell/Body.h"

class FogOfWar
{
public:
	static void Reveal_DisplayClass_All_To_Look_Ground(TechnoClass* pTechno, DWORD dwUnk, DWORD dwUnk2);
	static bool MapClass_RevealFogShroud(MapClass* pMap, CellStruct* pCell_, HouseClass* pHouse);
	static bool IsLocationFogged(CoordStruct* pCoord);
	static void ClearFoggedObjects(CellClass* pCell);

	static bool DrawIfVisible(FoggedObject* pFoggedObject, RectangleStruct* pRect);
    // Result will be stored in rect1
	static void UnionRectangle(RectangleStruct* rect1, RectangleStruct* rect2);

	static std::unordered_set<FoggedObject*> FoggedObjects;
};