#include "Body.h"

#include <BuildingClass.h>

#include <Ext/BuildingType/Body.h>
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

DirType AircraftExt::GetLandingDir(AircraftClass* pThis, BuildingClass* pDock)
{
	auto const poseDir = static_cast<DirType>(RulesClass::Instance->PoseDir);

	if (!pThis)
		return poseDir;

	bool isAirportBound = true;

	if (pDock || pThis->HasAnyLink())
	{
		auto pBuilding = pDock;

		if (!pDock)
			pBuilding = abstract_cast<BuildingClass*>(pThis->GetNthLink(0));

		if (pBuilding)
		{
			auto const pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
			int docks = pBuilding->Type->NumberOfDocks;
			int linkIndex = pBuilding->FindLinkIndex(pThis);

			if (docks > 0 && linkIndex >= 0 && linkIndex < docks)
			{
				if (!pBuildingTypeExt->AircraftDockingDirs[linkIndex].empty())
					return pBuildingTypeExt->AircraftDockingDirs[linkIndex].get();
			}
			else if (docks > 0 && !pBuildingTypeExt->AircraftDockingDirs[0].empty())
				return pBuildingTypeExt->AircraftDockingDirs[0].get();
		}
	}
	else if (!pThis->Type->AirportBound)
	{
		isAirportBound = false;
	}

	int landingDir = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->LandingDir.Get((int)poseDir);

	if (isAirportBound)
		return static_cast<DirType>(Math::clamp(landingDir, 0, 255));
	else if (landingDir < 0)
		return pThis->PrimaryFacing.Current().GetDir();

	return poseDir;
}

