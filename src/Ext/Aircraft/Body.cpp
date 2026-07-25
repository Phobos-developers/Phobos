#include "Body.h"

#include <Ext/AircraftType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/WeaponType/Body.h>

AircraftExt::ExtContainer AircraftExt::ExtMap;

void AircraftExt::FireWeapon(AircraftClass* pThis, AbstractClass* pTarget)
{
	auto const pExt = AircraftExt::Fetch(pThis);
	const int weaponIndex = pExt->CurrentAircraftWeaponIndex;
	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExt::Fetch(pWeapon);
	const int burstCount = pWeapon->Burst;
	const bool isStrafe = pThis->Is_Strafe();

	if (burstCount > 0)
	{
		int& bombDropCount = pExt->Strafe_BombsDroppedThisRound;
		int& currentBurstIndex = pThis->CurrentBurstIndex;
		const bool simulateBurst = pWeaponExt->Strafing_SimulateBurst.Get(RulesExt::Global()->Strafing_SimulateBurst);

		for (int i = 0; i < burstCount; i++)
		{
			if (isStrafe && burstCount < 2 && simulateBurst)
				currentBurstIndex = bombDropCount % 2 == 0;

			pThis->Fire(pTarget, weaponIndex);
		}

		if (isStrafe)
		{
			bombDropCount++;

			if (pWeaponExt->Strafing_UseAmmoPerShot.Get(RulesExt::Global()->Strafing_UseAmmoPerShot))
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

// Paradrop, spy plane, airstrike.
bool AircraftExt::PlaceReinforcementAircraft(AircraftClass* pThis, CoordStruct edgeCoords)
{
	auto const pType = pThis->Type;
	auto const pTypeExt = AircraftTypeExt::Fetch(pType);
	auto dir = DirType::North;
	auto coords = edgeCoords;
	coords.Z = 0;
	AbstractClass* pTarget = pThis->Target ? pThis->Target : pThis->Destination;

	if (pTarget)
	{
		auto const pTargetCoords = pTarget->GetCoords();

		if (pTypeExt->SpawnDistanceFromTarget.isset())
			coords = GeneralUtils::CalculateCoordsFromDistance(edgeCoords, pTargetCoords, pTypeExt->SpawnDistanceFromTarget.Get());

		dir = GeneralUtils::GetDirectionBetweenCoords(coords, pTargetCoords).GetDir();
	}

	bool result = false;

	++Unsorted::ScenarioInit;
	result = pThis->Unlimbo(coords, dir);
	--Unsorted::ScenarioInit;

	pThis->SetHeight(pTypeExt->SpawnHeight.isset() ? pTypeExt->SpawnHeight.Get() : pType->GetFlightLevel());

	if (pTarget)
		pThis->PrimaryFacing.SetDesired(pThis->GetTargetDirection(pTarget));

	return result;
}

CellStruct AircraftExt::PickEdgeCellForPlane(AircraftTypeClass* pPlaneType, CellStruct destCell, Edge edge, bool isOnRetreat)
{
	auto const pTypeExt = AircraftTypeExt::Fetch(pPlaneType);
	auto const edgeMode = !isOnRetreat ? pTypeExt->SpawnFromEdge.Get(RulesExt::Global()->AircraftSpawnFromEdge)
		: pTypeExt->RetreatToEdge.Get(RulesExt::Global()->AircraftRetreatToEdge);
	auto spawnEdge = edge;
	auto refCell = CellStruct::Empty;

	switch (edgeMode)
	{
	case EdgeType::Closest:
	{
		if (destCell != CellStruct::Empty)
		{
			spawnEdge = Edge::None;
			refCell = destCell;

			// Scatter the coords a bit to randomize spawn cell a little - otherwise multiple planes sent at same target
			// from same source might end up overlapping - still a possibility, just less likely.
			// The edge cell picking function itself will do no randomization on Edge::None + waypoint cell set mode.
			int const randomRange = 5;
			short const randomX = static_cast<short>(ScenarioClass::Instance->Random.RandomRanged(-randomRange, randomRange));
			short const randomY = static_cast<short>(ScenarioClass::Instance->Random.RandomRanged(-randomRange, randomRange));
			refCell += CellStruct { randomX, randomY };
		}
		break;
	}
	case EdgeType::Random:
	{
		int const min = static_cast<int>(Edge::North);
		int const max = static_cast<int>(Edge::West);
		spawnEdge = static_cast<Edge>(ScenarioClass::Instance->Random.RandomRanged(min, max));
		break;
	}
	default:
	{
		break;
	}
	}

	return MapClass::Instance.PickCellOnEdge(spawnEdge, refCell, CellStruct::Empty, SpeedType::Winged, true, MovementZone::Normal);
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
			auto const pBuildingTypeExt = BuildingTypeExt::Fetch(pBuildingType);
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

	const int landingDir = AircraftTypeExt::Fetch(pType)->LandingDir.Get((int)poseDir);

	if (!pType->AirportBound && landingDir < 0)
		return pThis->PrimaryFacing.Current().GetDir();

	return static_cast<DirType>(std::clamp(landingDir, 0, 255));
}

AircraftTypeClass* AircraftExt::GetAircraftTypeExtra(AircraftClass* pAircraft)
{
	auto const pType = pAircraft->Type;
	auto const pData = AircraftTypeExt::Fetch(pType);

	if (!pData->NeedDamagedImage || pAircraft->IsGreenHP())
	{
		return pType;
	}
	else if (pAircraft->IsYellowHP())
	{
		if (auto const imageYellow = pData->Image_ConditionYellow)
			return abstract_cast<AircraftTypeClass*, true>(imageYellow);
	}
	else
	{
		if (auto const imageRed = pData->Image_ConditionRed)
			return abstract_cast<AircraftTypeClass*, true>(imageRed);
		else if (auto const imageYellow = pData->Image_ConditionYellow)
			return abstract_cast<AircraftTypeClass*, true>(imageYellow);
	}

	return pType;
}

// =============================
// load / save

template <typename T>
void AircraftExt::Serialize(T& Stm)
{
	Stm
		.Process(this->Strafe_BombsDroppedThisRound)
		.Process(this->Strafe_TargetCell)
		.Process(this->CurrentAircraftWeaponIndex)
		;
}

void AircraftExt::LoadFromStream(PhobosStreamReader& Stm)
{
	FootExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AircraftExt::SaveToStream(PhobosStreamWriter& Stm)
{
	FootExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

AircraftExt::ExtContainer::ExtContainer() : Container("AircraftClass") { }
AircraftExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x413D30, AircraftClass_CTOR, 0x7)
{
	GET(AircraftClass*, pItem, ESI);

	AircraftExt::ExtMap.Allocate(pItem);

	return 0;
}

// Late in every destructor body of the class, right before it chains into the
// base destructor: the last point where the extension is no longer used.
DEFINE_HOOK_AGAIN(0x41426D, AircraftClass_DTOR, 0x9)
DEFINE_HOOK(0x4141FA, AircraftClass_DTOR, 0x9)
{
	GET(AircraftClass*, pItem, EDI);

	AircraftExt::ExtMap.Remove(pItem);

	return 0;
}
