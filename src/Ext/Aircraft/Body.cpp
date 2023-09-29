#include "Body.h"
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>

// TODO: Implement proper extended AircraftClass.

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, int shotNumber = 0)
{
	if (!pTarget) return;
	int weaponIndex = pThis->SelectWeapon(pTarget);
	auto weaponType = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(weaponType);

	if (weaponType->Burst > 0)
	{
		for (int i = 0; i < weaponType->Burst; i++)
		{
			if (weaponType->Burst < 2 && pWeaponTypeExt->Strafing_SimulateBurst)
				pThis->CurrentBurstIndex = shotNumber;

			pThis->Fire(pTarget, weaponIndex);
		}
	}
}

// Spy plane, airstrike etc.
bool AircraftExt::PlaceReinforcementAircraft(AircraftClass* pThis, CellStruct edgeCell)
{
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto coords = CellClass::Cell2Coord(edgeCell);
	coords.Z = 0;
	AbstractClass* pTarget = nullptr;

	if (pTypeExt->SpawnDistanceFromTarget.isset())
	{
		pTarget = pThis->Target ? pThis->Target : pThis->Destination;

		if (pTarget)
			coords = GeneralUtils::CalculateCoordsFromDistance(CellClass::Cell2Coord(edgeCell), pTarget->GetCoords(), pTypeExt->SpawnDistanceFromTarget.Get());
	}

	++Unsorted::IKnowWhatImDoing;
	bool result = pThis->Unlimbo(coords, DirType::North);
	--Unsorted::IKnowWhatImDoing;

	pThis->SetHeight(pTypeExt->SpawnHeight.Get(pThis->Type->GetFlightLevel()));

	if (pTarget)
		pThis->PrimaryFacing.SetDesired(pThis->GetTargetDirection(pTarget));

	return result;
}
