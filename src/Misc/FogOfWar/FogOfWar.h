#pragma once

#include <GeneralDefinitions.h>

#include <set>

class TechnoClass;
class HouseClass;
class FoggedObject;
class FoggedBuilding;
struct RectangleStruct;
class MapClass;
class CellClass;

class FogOfWar
{
public:
	static void Reveal_DisplayClass_All_To_Look_Ground(TechnoClass* pTechno, DWORD dwUnk, DWORD dwUnk2);
	static bool MapClass_RevealFogShroud(CellStruct* pMapCoords, HouseClass* pHouse);
	static bool IsLocationFogged(CoordStruct* pCoord);
	static void ClearFoggedObjects(CellClass* pCell);

	static bool DrawIfVisible(FoggedObject* pFoggedObject, RectangleStruct* pRect);
	static bool DrawBldIfVisible(FoggedBuilding* pFoggedBuilding, RectangleStruct* pRect);
    // Result will be stored in rect1
	static void UnionRectangle(RectangleStruct* rect1, RectangleStruct* rect2);

	static void __fastcall MapClass_Reveal0(MapClass* pThis, void*_, CoordStruct* pCoord, int Radius,
		HouseClass* pHouse, bool OnlyOutline, bool a6, bool SkipReveal, bool AllowRevealByHeight, bool Add);
	static void __fastcall MapClass_Reveal2(MapClass* pThis, void*_, CoordStruct* Coords, int Height, int Radius, int SkipReveal);

	static std::set<FoggedObject*> FoggedObjects;
};