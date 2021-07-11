#include "FogOfWar.h"
#include"FoggedObject.h"

#include <MapClass.h>
#include <SessionClass.h>
#include <HouseClass.h>
#include <FootClass.h>
#include <BuildingClass.h>
#include <TacticalClass.h>
#include <ScenarioClass.h>

// issue #28 : Fix vanilla YR Fog of War bugs & issues
// Reimplement it would be nicer.

bool FogOfWar::IsLocationFogged(CoordStruct* pCoord)
{
	auto pCell = MapClass::Instance->GetCellAt(*pCoord);
	if (pCell->Flags & 2)
		return false;
	return ((pCell->GetNeighbourCell(3u)->Flags & 2) == 0);
}
