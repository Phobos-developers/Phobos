#pragma once
#include <Ext/Techno/Body.h>
#include <Ext/Foot/Body.h>
#include <AircraftClass.h>

// Concrete leaf extension for AircraftClass (empty; techno data lives in TechnoExt).
class AircraftExt : public FootExt
{
public:
	explicit AircraftExt(AircraftClass* const OwnerObject) : FootExt(OwnerObject)
	{ }

	AircraftClass* OwnerObject() const
	{
		return static_cast<AircraftClass*>(this->GetAttachedObject());
	}

	static void FireWeapon(AircraftClass* pThis, AbstractClass* pTarget);
	static bool PlaceReinforcementAircraft(AircraftClass* pThis, CoordStruct edgeCoords);
	static CellStruct PickEdgeCellForPlane(AircraftTypeClass* pPlaneType, CellStruct destCell, Edge edge, bool isOnRetreat = false);
	static DirType GetLandingDir(AircraftClass* pThis, BuildingClass* pDock = nullptr);
};
