#include "Body.h"

#include <BuildingClass.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

// TODO: Implement proper extended AircraftClass.

void AircraftExt::FireWeapon(AircraftClass* pThis, AbstractClass* pTarget)
{
	auto weaponIndex = TechnoExt::ExtMap.Find(pThis)->CurrentAircraftWeaponIndex;

	if (weaponIndex < 0)
		weaponIndex = pThis->SelectWeapon(pTarget);

	bool isStrafe = pThis->Is_Strafe();
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWeapon->Burst > 0)
	{
		for (int i = 0; i < pWeapon->Burst; i++)
		{
			if (isStrafe && pWeapon->Burst < 2 && pWeaponExt->Strafing_SimulateBurst)
				pThis->CurrentBurstIndex = pExt->Strafe_BombsDroppedThisRound % 2 == 0;

			pThis->Fire(pTarget, weaponIndex);
		}

		if (isStrafe)
		{
			pExt->Strafe_BombsDroppedThisRound++;

			if (pWeaponExt->Strafing_UseAmmoPerShot)
			{
				pThis->Ammo--;
				pThis->ShouldLoseAmmo = false;

				if (!pThis->Ammo)
				{
					pThis->SetTarget(nullptr);
					pThis->SetDestination(nullptr, true);
				}
			}
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

	++Unsorted::ScenarioInit;
	bool result = pThis->Unlimbo(coords, DirType::North);
	--Unsorted::ScenarioInit;

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

	// If this is a spawnee, use the spawner's facing.
	if (auto pOwner = pThis->SpawnOwner)
		return pOwner->PrimaryFacing.Current().GetDir();

	if (pDock || pThis->HasAnyLink())
	{
		auto pLink = pThis->GetNthLink(0);

		if (auto pBuilding = pDock ? pDock : abstract_cast<BuildingClass*>(pLink))
		{
			auto const pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
			int docks = pBuilding->Type->NumberOfDocks;
			int linkIndex = pBuilding->FindLinkIndex(pThis);

			if (docks > 0 && linkIndex >= 0 && linkIndex < docks)
			{
				if (pBuildingTypeExt->AircraftDockingDirs[linkIndex].has_value())
					return *pBuildingTypeExt->AircraftDockingDirs[linkIndex];
			}
			else if (docks > 0 && pBuildingTypeExt->AircraftDockingDirs[0].has_value())
				return *pBuildingTypeExt->AircraftDockingDirs[0];
		}
		else if (!pThis->Type->AirportBound)
			return pLink->PrimaryFacing.Current().GetDir();
	}

	int landingDir = TechnoTypeExt::ExtMap.Find(pThis->Type)->LandingDir.Get((int)poseDir);

	if (!pThis->Type->AirportBound && landingDir < 0)
		return pThis->PrimaryFacing.Current().GetDir();

	return static_cast<DirType>(std::clamp(landingDir, 0, 255));
}
