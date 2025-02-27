#pragma once
#include <AircraftClass.h>

// TODO: Implement proper extended AircraftClass.

class AircraftExt
{
public:
	static void FireWeapon(AircraftClass* pThis, AbstractClass* pTarget, int shotNumber);
	static bool PlaceReinforcementAircraft(AircraftClass* pThis, CellStruct edgeCell);
	static DirType GetLandingDir(AircraftClass* pThis, BuildingClass* pDock = nullptr);
};
