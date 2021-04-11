#pragma once

#include <Phobos.h>

#include <FoggedObjectClass.h>
#include <FootClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <DisplayClass.h>
#include <SessionClass.h>
#include <HouseClass.h>
#include <RulesClass.h>
#include <GameModeOptionsClass.h>
#include <ScenarioClass.h>
#include <TacticalClass.h>
#include <TerrainClass.h>

class FogOfWar
{
public:
	static void Reveal_DisplayClass_All_To_Look_Ground(TechnoClass* pTechno, DWORD dwUnk, DWORD dwUnk2);
	static bool MapClass_RevealFogShroud(MapClass* pMap, CellStruct* pCell_, HouseClass* pHouse);
	static bool IsLocationFogged(CoordStruct* pCoord);
};