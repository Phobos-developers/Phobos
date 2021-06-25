#include "Body.h"

#include <Helpers\Macro.h>

#include <HouseClass.h>
#include <BuildingClass.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(6DD8B0, TActionClass_Execute, 6)
{
	GET(TActionClass*, pThis, ECX);
	GET_STACK(HouseClass*, pHouse, 0x4);
	GET_STACK(ObjectClass*, pObject, 0x8);
	GET_STACK(TriggerClass*, pTrigger, 0xC);
	GET_STACK(CellStruct const*, pLocation, 0x10);

	bool handled;

	R->AL(TActionExt::Execute(pThis, pHouse, pObject, pTrigger, *pLocation, handled));

	return handled ? 0x6DD910 : 0;
}

// Bugfix: TAction 125 Build At do not display the buildups
// Author: secsome
DEFINE_HOOK(6E427D, TActionClass_CreateBuildingAt, 9)
{
	GET(TActionClass*, pThis, ESI);
	GET(BuildingTypeClass*, pBldType, ECX);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(CoordStruct, coord, STACK_OFFS(0x24, 0x18));

	auto pBld = GameCreate<BuildingClass>(pBldType, pHouse);

	if (pThis->Bounds.X) // use this one for our flag: bPlayBuildUp
		pBld->QueueMission(Mission::Construction, true);

	if (pBld->Put(coord, Direction::North))
		pBld->IsReadyToCommence = true;
	else
		pBld->UnInit();

	return 0x6E42C1;
}