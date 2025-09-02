#include "Body.h"

#include <BuildingClass.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

// TODO: Implement proper extended AircraftClass.

void AircraftExt::FireWeapon(AircraftClass* pThis, AbstractClass* pTarget)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	int weaponIndex = pExt->CurrentAircraftWeaponIndex;
	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	const int burstCount = pWeapon->Burst;
	const bool isStrafe = pThis->Is_Strafe();

	if (burstCount > 0)
	{
		int& bombDropCount = pExt->Strafe_BombsDroppedThisRound;
		int& currentBurstIndex = pThis->CurrentBurstIndex;
		const bool simulateBurst = pWeaponExt->Strafing_SimulateBurst;

		for (int i = 0; i < burstCount; i++)
		{
			if (isStrafe && burstCount < 2 && simulateBurst)
				currentBurstIndex = bombDropCount % 2 == 0;

			pThis->Fire(pTarget, weaponIndex);
		}

		if (isStrafe)
		{
			bombDropCount++;

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
	auto const pType = pThis->Type;
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
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
	const bool result = pThis->Unlimbo(coords, DirType::North);
	--Unsorted::ScenarioInit;

	pThis->SetHeight(pTypeExt->SpawnHeight.Get(pType->GetFlightLevel()));

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
	if (auto const pOwner = pThis->SpawnOwner)
		return pOwner->PrimaryFacing.Current().GetDir();

	auto const pType = pThis->Type;

	if (pDock || pThis->HasAnyLink())
	{
		auto const pLink = pThis->GetNthLink(0);

		if (auto const pBuilding = pDock ? pDock : abstract_cast<BuildingClass*, true>(pLink))
		{
			auto const pBuildingType = pBuilding->Type;
			auto const pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuildingType);
			const int docks = pBuildingType->NumberOfDocks;
			const int linkIndex = pBuilding->FindLinkIndex(pThis);

			if (docks > 0 && linkIndex >= 0 && linkIndex < docks)
			{
				if (pBuildingTypeExt->AircraftDockingDirs[linkIndex].has_value())
					return *pBuildingTypeExt->AircraftDockingDirs[linkIndex];
			}
			else if (docks > 0 && pBuildingTypeExt->AircraftDockingDirs[0].has_value())
				return *pBuildingTypeExt->AircraftDockingDirs[0];
		}
		else if (!pType->AirportBound)
			return pLink->PrimaryFacing.Current().GetDir();
	}

	const int landingDir = TechnoTypeExt::ExtMap.Find(pType)->LandingDir.Get((int)poseDir);

	if (!pType->AirportBound && landingDir < 0)
		return pThis->PrimaryFacing.Current().GetDir();

	return static_cast<DirType>(std::clamp(landingDir, 0, 255));
}
