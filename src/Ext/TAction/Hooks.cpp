#include "Body.h"

#include <Helpers\Macro.h>

#include <HouseClass.h>
#include <BuildingClass.h>

// Bugfix : TAction 125 Build At do not display the buildups
DEFINE_HOOK(6E427D, TActionClass_CreateBuildingAt, 9)
{
	GET(BuildingTypeClass*, pBldType, ECX);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(CoordStruct, coord, STACK_OFFS(0x24, 0x18));

	auto pBld = GameCreate<BuildingClass>(pBldType, pHouse);

	pBld->QueueMission(Mission::Construction, true);
	if (!pBld->Put(coord, Direction::North))
		pBld->UnInit();

	return 0x6E42C1;
}