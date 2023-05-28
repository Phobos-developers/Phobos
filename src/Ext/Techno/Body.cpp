#include "Body.h"

#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Ext/House/Body.h>

template<> const DWORD Extension<TechnoClass>::Canary = 0x55555555;
TechnoExt::ExtContainer TechnoExt::ExtMap;

TechnoExt::ExtData::~ExtData()
{
	if (this->TypeExtData->AutoDeath_Behavior.isset())
	{
		auto pThis = this->OwnerObject();
		auto hExt = HouseExt::ExtMap.Find(pThis->Owner);
		auto it = std::find(hExt->OwnedTimedAutoDeathObjects.begin(), hExt->OwnedTimedAutoDeathObjects.end(), this);
		if (it != hExt->OwnedTimedAutoDeathObjects.end())
			hExt->OwnedTimedAutoDeathObjects.erase(it);
	}
}

bool TechnoExt::IsActive(TechnoClass* pThis)
{
	return
		pThis &&
		!pThis->TemporalTargetingMe &&
		!pThis->BeingWarpedOut &&
		!pThis->IsUnderEMP() &&
		pThis->IsAlive &&
		pThis->Health > 0 &&
		!pThis->InLimbo;
}

bool TechnoExt::IsHarvesting(TechnoClass* pThis)
{
	if (!TechnoExt::IsActive(pThis))
		return false;

	auto slave = pThis->SlaveManager;
	if (slave && slave->State != SlaveManagerStatus::Ready)
		return true;

	if (pThis->WhatAmI() == AbstractType::Building)
		return pThis->IsPowerOnline();

	if (TechnoExt::HasAvailableDock(pThis))
	{
		switch (pThis->GetCurrentMission())
		{
		case Mission::Harvest:
		case Mission::Unload:
		case Mission::Enter:
			return true;
		case Mission::Guard: // issue#603: not exactly correct, but idk how to do better
			if (auto pUnit = abstract_cast<UnitClass*>(pThis))
				return pUnit->IsHarvesting || pUnit->Locomotor->Is_Really_Moving_Now() || pUnit->HasAnyLink();
		default:
			return false;
		}
	}

	return false;
}

bool TechnoExt::HasAvailableDock(TechnoClass* pThis)
{
	for (auto pBld : pThis->GetTechnoType()->Dock)
	{
		if (pThis->Owner->CountOwnedAndPresent(pBld))
			return true;
	}

	return false;
}

void TechnoExt::SyncIronCurtainStatus(TechnoClass* pFrom, TechnoClass* pTo)
{
	if (pFrom->IsIronCurtained() && !pFrom->ForceShielded)
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pFrom->GetTechnoType());
		if (pTypeExt->IronCurtain_KeptOnDeploy.Get(RulesExt::Global()->IronCurtain_KeptOnDeploy))
		{
			pTo->IronCurtain(pFrom->IronCurtainTimer.GetTimeLeft(), pFrom->Owner, false);
			pTo->IronTintStage = pFrom->IronTintStage;
		}
	}
}

double TechnoExt::GetCurrentSpeedMultiplier(FootClass* pThis)
{
	double houseMultiplier = 1.0;

	if (pThis->WhatAmI() == AbstractType::Aircraft)
		houseMultiplier = pThis->Owner->Type->SpeedAircraftMult;
	else if (pThis->WhatAmI() == AbstractType::Infantry)
		houseMultiplier = pThis->Owner->Type->SpeedInfantryMult;
	else
		houseMultiplier = pThis->Owner->Type->SpeedUnitsMult;

	return pThis->SpeedMultiplier * houseMultiplier *
		(pThis->HasAbility(Ability::Faster) ? RulesClass::Instance->VeteranSpeed : 1.0);
}

CoordStruct TechnoExt::PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger, int maxAttempts = 1)
{
	if (!pThis || !pPassenger)
		return CoordStruct::Empty;

	if (maxAttempts < 1)
		maxAttempts = 1;

	CellClass* pCell;
	CellStruct placeCoords = CellStruct::Empty;
	auto pTypePassenger = pPassenger->GetTechnoType();
	CoordStruct finalLocation = CoordStruct::Empty;
	short extraDistanceX = 1;
	short extraDistanceY = 1;
	SpeedType speedType = pTypePassenger->SpeedType;
	MovementZone movementZone = pTypePassenger->MovementZone;

	if (pTypePassenger->WhatAmI() == AbstractType::AircraftType)
	{
		speedType = SpeedType::Track;
		movementZone = MovementZone::Normal;
	}

	do
	{
		placeCoords = pThis->GetCell()->MapCoords - CellStruct { (short)(extraDistanceX / 2), (short)(extraDistanceY / 2) };
		placeCoords = MapClass::Instance->NearByLocation(placeCoords, speedType, -1, movementZone, false, extraDistanceX, extraDistanceY, true, false, false, false, CellStruct::Empty, false, false);

		pCell = MapClass::Instance->GetCellAt(placeCoords);
		extraDistanceX += 1;
		extraDistanceY += 1;
	}
	while (extraDistanceX < maxAttempts && (pThis->IsCellOccupied(pCell, -1, -1, nullptr, false) != Move::OK) && pCell->MapCoords != CellStruct::Empty);

	pCell = MapClass::Instance->TryGetCellAt(placeCoords);
	if (pCell)
		finalLocation = pCell->GetCoordsWithBridge();

	return finalLocation;
}

bool TechnoExt::AllowedTargetByZone(TechnoClass* pThis, TechnoClass* pTarget, TargetZoneScanType zoneScanType, WeaponTypeClass* pWeapon, bool useZone, int zone)
{
	if (!pThis || !pTarget)
		return false;

	if (pThis->WhatAmI() == AbstractType::Aircraft)
		return true;

	MovementZone mZone = pThis->GetTechnoType()->MovementZone;
	int currentZone = useZone ? zone : MapClass::Instance->GetMovementZoneType(pThis->GetMapCoords(), mZone, pThis->IsOnBridge());

	if (currentZone != -1)
	{
		if (zoneScanType == TargetZoneScanType::Any)
			return true;

		int targetZone = MapClass::Instance->GetMovementZoneType(pTarget->GetMapCoords(), mZone, pTarget->IsOnBridge());

		if (zoneScanType == TargetZoneScanType::Same)
		{
			if (currentZone != targetZone)
				return false;
		}
		else
		{
			if (currentZone == targetZone)
				return true;

			auto const speedType = pThis->GetTechnoType()->SpeedType;
			auto cellStruct = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(pTarget->Location),
				speedType, -1, mZone, false, 1, 1, true,
				false, false, speedType != SpeedType::Float, CellStruct::Empty, false, false);
			auto const pCell = MapClass::Instance->GetCellAt(cellStruct);

			if (!pCell)
				return false;

			double distance = pCell->GetCoordsWithBridge().DistanceFrom(pTarget->GetCenterCoords());

			if (!pWeapon)
			{
				int weaponIndex = pThis->SelectWeapon(pTarget);

				if (weaponIndex < 0)
					return false;

				pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
			}

			if (distance > pWeapon->Range)
				return false;
		}
	}

	return true;
}

void TechnoExt::StartUniversalDeployAnim(TechnoClass* pThis)
{
	if (!pThis)
		return;

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return;

	if (pExt->DeployAnim)
	{
		pExt->DeployAnim->UnInit();
		pExt->DeployAnim = nullptr;
	}

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (!pTypeExt)
		return;

	if (pTypeExt->Convert_DeployingAnim.isset())
	{
		if (auto deployAnimType = pTypeExt->Convert_DeployingAnim.Get())
		{
			if (auto pAnim = GameCreate<AnimClass>(deployAnimType, pThis->Location))
			{
				pExt->DeployAnim = pAnim;
				pExt->DeployAnim->SetOwnerObject(pThis);

				pExt->Convert_UniversalDeploy_MakeInvisible = true;

				// Use the unit palette in the animation
				auto lcc = pThis->GetDrawer();
				pExt->DeployAnim->LightConvert = lcc;
			}
			else
			{
				Debug::Log("ERROR! [%s] Deploy animation %s can't be created.\n", pThis->GetTechnoType()->ID, deployAnimType->ID);
			}
		}
	}
}

void TechnoExt::UpdateUniversalDeploy(TechnoClass* pThis)
{
	if (!pThis)
		return;

	if (auto pExt = TechnoExt::ExtMap.Find(pThis))
	{
		if (!pExt->Convert_UniversalDeploy_InProgress)
			return;

		if (pThis->WhatAmI() == AbstractType::Building)
			return;

		auto pFoot = static_cast<FootClass*>(pThis);
		if (!pFoot)
			return;

		TechnoClass* deployed = nullptr;

		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
		if (!pTypeExt)
			return;

		bool deployToLand = pTypeExt->Convert_DeployToLand;

		if (pExt->Convert_UniversalDeploy_InProgress && deployToLand)
			pThis->IsFallingDown = true;

		if (!pThis->IsFallingDown && pFoot->Locomotor->Is_Really_Moving_Now())
			return;

		if (pThis->WhatAmI() != AbstractType::Infantry)
		{
			// Turn the unit to the right deploy facing
			if (!pExt->DeployAnim && pTypeExt->Convert_DeployDir >= 0)
			{
				DirType currentDir = pThis->PrimaryFacing.Current().GetDir(); // Returns current position in format [0 - 7] x 32
				DirType desiredDir = (static_cast<DirType>(pTypeExt->Convert_DeployDir * 32));
				DirStruct desiredFacing;
				desiredFacing.SetDir(desiredDir);

				if (currentDir != desiredDir)
				{
					pFoot->Locomotor->Move_To(CoordStruct::Empty);
					pThis->PrimaryFacing.SetDesired(desiredFacing);

					return;
				}
			}
		}
		
		AnimTypeClass* pDeployAnimType = pTypeExt->Convert_DeployingAnim.isset() ? pTypeExt->Convert_DeployingAnim.Get() : nullptr;
		bool hasValidDeployAnim = pDeployAnimType ? true : false;
		bool isDeployAnimPlaying = pExt->DeployAnim ? true : false;
		bool hasDeployAnimFinished = (isDeployAnimPlaying && (pExt->DeployAnim->Animation.Value >= (pDeployAnimType->End + pDeployAnimType->Start - 1))) ? true : false;
		int convert_DeploySoundIndex = pTypeExt->Convert_DeploySound.isset() ? pTypeExt->Convert_DeploySound.Get() : -1;
		AnimTypeClass* pAnimFXType = pTypeExt->Convert_AnimFX.isset() ? pTypeExt->Convert_AnimFX.Get() : nullptr;
		bool animFX_FollowDeployer = pTypeExt->Convert_AnimFX_FollowDeployer;

		// Update InAir information
		if (deployToLand && pThis->GetHeight() <= 20)
			pThis->SetHeight(0);

		pThis->InAir = (pThis->GetHeight() > 0);

		if (deployToLand)
		{
			if (pThis->InAir)
				return;

			pFoot->ParalysisTimer.Start(15);
		}

		CoordStruct deployLocation = pThis->GetCoords();

		// Before the conversion we need to play the full deploy animation
		if (hasValidDeployAnim && !isDeployAnimPlaying)
		{
			TechnoExt::StartUniversalDeployAnim(pThis);
			return;
		}

		// Hack for making the Voxel invisible during a deploy
		if (hasValidDeployAnim)
			pExt->Convert_UniversalDeploy_MakeInvisible = true;

		// The unit should remain paralyzed during a deploy animation
		if (isDeployAnimPlaying)
			pFoot->ParalysisTimer.Start(15);

		// Necessary for skipping the passengers ejection
		pFoot->CurrentMission = Mission::None;

		if (hasValidDeployAnim && !hasDeployAnimFinished)
			return;

		/*if (pThis->DeployAnim)
		{
			pThis->DeployAnim->Limbo();
			pThis->DeployAnim->UnInit();
			pThis->DeployAnim = nullptr;
		}*/

		// Time for the object conversion
		deployed = TechnoExt::UniversalConvert(pThis, nullptr);

		if (deployed)
		{
			if (convert_DeploySoundIndex >= 0)
				VocClass::PlayAt(convert_DeploySoundIndex, deployLocation);

			if (pAnimFXType)
			{
				if (auto const pAnim = GameCreate<AnimClass>(pAnimFXType, deployLocation))
				{
					if (animFX_FollowDeployer)
						pAnim->SetOwnerObject(deployed);

					pAnim->Owner = deployed->Owner;
				}
			}
		}
		else
		{
			pFoot->ParalysisTimer.Stop();
			pThis->IsFallingDown = false;
			pThis->ForceMission(Mission::Guard);
			pExt->Convert_UniversalDeploy_InProgress = false;
			pExt->Convert_UniversalDeploy_MakeInvisible = false;
		}
	}
}

TechnoClass* TechnoExt::UniversalConvert(TechnoClass* pThis, TechnoTypeClass* pNewTechnoType = nullptr)
{
	if (!pThis)
		return nullptr;

	auto pOldTechnoType = pThis->GetTechnoType();
	if (!pOldTechnoType)
		return nullptr;

	auto pOldTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pOldTechnoType);
	if (!pOldTechnoTypeExt)
		return nullptr;

	auto newLocation = pThis->Location;
	auto pOldTechno = pThis;

	if (!pNewTechnoType)
	{
		if (pOldTechnoTypeExt->Convert_UniversalDeploy.size() > 0)
		{
			// TO-DO: having multiple deploy candidate IDs it should pick randomly from the list
			int index = 0; //ScenarioClass::Instance->Random.RandomRanged(0, pOldTechnoTypeExt->Convert_UniversalDeploy.size() - 1);
			pNewTechnoType = pOldTechnoTypeExt->Convert_UniversalDeploy.at(index);
		}
		else
		{
			return nullptr;
		}
	}

	// The "Infantry -> Vehicle" case we need to check if the cell is free of soldiers
	if (pOldTechnoType->WhatAmI() != AbstractType::Building && pNewTechnoType->WhatAmI() != AbstractType::Building)
	{
		int nObjects = 0;

		for (auto pObject = pOldTechno->GetCell()->FirstObject; pObject; pObject = pObject->NextObject)
		{
			auto const pItem = static_cast<TechnoClass*>(pObject);

			if (pItem && pItem != pOldTechno)
				nObjects++;
		}

		if (nObjects > 0)
		{
			CoordStruct loc = CoordStruct::Empty;
			pOldTechno->Scatter(loc, true, false);
			return nullptr;
		}
	}

	auto pOwner = pOldTechno->Owner;
	auto pNewTechno = static_cast<TechnoClass*>(pNewTechnoType->CreateObject(pOwner));

	if (!pNewTechno)
		return nullptr;

	// Transfer enemies target (part 1/2)
	DynamicVectorClass<TechnoClass*> enemiesTargetingMeList;
	for (auto pEnemy : *TechnoClass::Array)
	{
		if (pEnemy->Target == pOldTechno)
			enemiesTargetingMeList.AddItem(pEnemy);
	}

	// Transfer some stats from the old object to the new:
	// Health update
	double nHealthPercent = (double)(1.0 * pOldTechno->Health / pOldTechnoType->Strength);
	pNewTechno->Health = (int)round(pNewTechnoType->Strength * nHealthPercent);
	pNewTechno->EstimatedHealth = pNewTechno->Health;

	// Shield update
	auto pOldTechnoExt = TechnoExt::ExtMap.Find(pOldTechno);
	auto pNewTechnoExt = TechnoExt::ExtMap.Find(pNewTechno);

	if (pOldTechnoExt && pNewTechnoExt)
	{
		if (auto pOldShieldData = pOldTechnoExt->Shield.get())
		{
			if (auto pNewShieldData = pNewTechnoExt->Shield.get())
			{
				if (pOldTechnoExt->CurrentShieldType
					&& pOldShieldData
					&& pNewTechnoExt->CurrentShieldType
					&& pNewShieldData)
				{
					double nOldShieldHealthPercent = (double)(1.0 * pOldShieldData->GetHP() / pOldTechnoExt->CurrentShieldType->Strength);

					pNewShieldData->SetHP((int)(nOldShieldHealthPercent * pNewTechnoExt->CurrentShieldType->Strength));
				}
			}
		}
	}

	// Veterancy update
	if (pOldTechnoTypeExt->Convert_TransferVeterancy)
	{
		VeterancyStruct nVeterancy = pOldTechno->Veterancy;
		pNewTechno->Veterancy = nVeterancy;
	}

	// AI team update
	if (pOldTechno->BelongsToATeam())
	{
		auto pOldFoot = static_cast<FootClass*>(pOldTechno);
		auto pNewFoot = static_cast<FootClass*>(pNewTechno);

		if (pNewFoot)
			pNewFoot->Team = pOldFoot->Team;
	}

	// Ammo update
	auto nAmmo = pNewTechno->Ammo;

	if (nAmmo >= pOldTechno->Ammo)
		nAmmo = pOldTechno->Ammo;

	pNewTechno->Ammo = nAmmo;

	// If the object was selected it should remain selected
	bool isSelected = false;
	if (pOldTechno->IsSelected)
		isSelected = true;

	// Mind control update
	if (pOldTechno->IsMindControlled())
		TechnoExt::TransferMindControlOnDeploy(pOldTechno, pNewTechno);

	// Initial mission update
	pNewTechno->QueueMission(Mission::Guard, true);

	// Cloak update
	pNewTechno->CloakState = pOldTechno->CloakState;

	// Facing update
	DirStruct oldPrimaryFacing;
	DirStruct oldSecondaryFacing;

	if (pOldTechnoTypeExt->Convert_DeployDir >= 0)
	{
		DirType desiredDir = (static_cast<DirType>(pOldTechnoTypeExt->Convert_DeployDir * 32));
		oldPrimaryFacing.SetDir(desiredDir);
		oldSecondaryFacing.SetDir(desiredDir);
	}
	else
	{
		oldPrimaryFacing = pOldTechno->PrimaryFacing.Current();
		oldSecondaryFacing = pOldTechno->SecondaryFacing.Current();
	}

	DirType oldPrimaryFacingDir = oldPrimaryFacing.GetDir(); // Returns current position in format [0 - 7] x 32
	//DirType oldSecondaryFacingDir = oldSecondaryFacing.GetDir(); // Returns current position in format [0 - 7] x 32

	// Some vodoo magic
	pOldTechno->Limbo();

	if (!pNewTechno->Unlimbo(newLocation, oldPrimaryFacingDir))
	{
		// Abort operation, restoring old object
		pOldTechno->Unlimbo(newLocation, oldPrimaryFacingDir);
		pNewTechno->UnInit();

		if (isSelected)
			pOldTechno->Select();

		if (auto pExt = TechnoExt::ExtMap.Find(pOldTechno))
			pOldTechno->IsFallingDown = false;

		return nullptr;
	}

	// Recover turret direction
	if (pOldTechnoType->Turret && pNewTechnoType->Turret && !pNewTechnoType->TurretSpins)
		pNewTechno->SecondaryFacing.SetCurrent(oldSecondaryFacing);

	// Jumpjet tricks
	if (pNewTechnoType->JumpJet || pNewTechnoType->BalloonHover)
	{
		auto pFoot = static_cast<FootClass*>(pNewTechno);
		pFoot->Scatter(CoordStruct::Empty, true, false);
		//pFoot->Locomotor->Move_To(CoordStruct::Empty);
		//pFoot->Locomotor->Do_Turn(oldPrimaryFacing);
		//pNewTechno->PrimaryFacing.SetDesired(oldPrimaryFacing);
	}
	else
	{
		auto pNewTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pNewTechnoType);

		if (!pNewTechnoTypeExt->Convert_DeployToLand)
			pNewTechno->IsFallingDown = false;
		else
			pNewTechno->IsFallingDown = true;
	}

	// Transfer passengers/garrisoned units, if possible
	TechnoExt::PassengersTransfer(pOldTechno, pNewTechno);

	// Transfer Iron Courtain effect, if applied
	TechnoExt::SyncIronCurtainStatus(pOldTechno, pNewTechno);

	// Transfer enemies target (part 2/2)
	for (auto pEnemy : enemiesTargetingMeList)
	{
		pEnemy->Target = pNewTechno;
	}

	// Remember target if it was Autodeployed
	if (pOldTechnoExt->Convert_Autodeploy_RememberTarget)
	{
		pNewTechno->Target = pOldTechnoExt->Convert_Autodeploy_RememberTarget;
		pNewTechno->QueueMission(Mission::Attack, true);
	}

	if (pOldTechno->InLimbo)
		pOwner->RegisterLoss(pOldTechno, false);

	if (!pNewTechno->InLimbo)
		pOwner->RegisterGain(pNewTechno, true);

	pNewTechno->Owner->RecheckTechTree = true;

	// If the object was selected it should remain selected
	if (isSelected)
		pNewTechno->Select();

	return pNewTechno;
}

// Currently is a vehicle to vehicle. TO-DO: Building2Building and vehicle->Building & vice-versa
void TechnoExt::PassengersTransfer(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo = nullptr)
{
	if (!pTechnoFrom || (pTechnoFrom == pTechnoTo))
		return;

	// Without a target transport this method becomes an "eject passengers from transport"
	auto pTechnoTypeTo = pTechnoTo ? pTechnoTo->GetTechnoType() : nullptr;
	if (!pTechnoTypeTo && pTechnoTo)
		return;

	bool bTechnoToIsBuilding = (pTechnoTo && pTechnoTo->WhatAmI() == AbstractType::Building) ? true : false;
	DynamicVectorClass<FootClass*> passengersList;

	// From bunkered infantry
	if (auto pBuildingFrom = static_cast<BuildingClass*>(pTechnoFrom))
	{
		for (int i = 0; i < pBuildingFrom->Occupants.Count; i++)
		{
			InfantryClass* pPassenger = pBuildingFrom->Occupants.GetItem(i);
			auto pFooter = static_cast<FootClass*>(pPassenger);
			passengersList.AddItem(pFooter);
		}
	}

	// We'll get the passengers list in a more easy data structure
	while (pTechnoFrom->Passengers.NumPassengers > 0)
	{
		FootClass* pPassenger = pTechnoFrom->Passengers.RemoveFirstPassenger();

		pPassenger->Transporter = nullptr;
		pPassenger->ShouldEnterOccupiable = false;
		pPassenger->ShouldGarrisonStructure = false;
		pPassenger->InOpenToppedTransport = false;
		pPassenger->QueueMission(Mission::Guard, false);
		passengersList.AddItem(pPassenger);
	}

	if (passengersList.Count > 0)
	{
		double passengersSizeLimit = pTechnoTo ? pTechnoTypeTo->SizeLimit : 0;
		int passengersLimit = pTechnoTo ? pTechnoTypeTo->Passengers : 0;
		int numPassengers = pTechnoTo ? pTechnoTo->Passengers.NumPassengers : 0;
		TechnoClass* transportReference = pTechnoTo ? pTechnoTo : pTechnoFrom;

		while (passengersList.Count > 0)
		{
			bool forceEject = false;
			FootClass* pPassenger = nullptr;

			// The insertion order is different in buildings
			int passengerIndex = bTechnoToIsBuilding ? 0 : passengersList.Count - 1;

			pPassenger = passengersList.GetItem(passengerIndex);
			passengersList.RemoveItem(passengerIndex);
			auto pFromTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoFrom->GetTechnoType());
			
			if (!pTechnoTo || (pFromTypeExt && !pFromTypeExt->Convert_TransferPassengers))
				forceEject = true;

			// To bunkered infantry
			if (pTechnoTo->WhatAmI() == AbstractType::Building)
			{
				auto pBuildingTo = static_cast<BuildingClass*>(pTechnoTo);
				int nOccupants = pBuildingTo->Occupants.Count;
				int maxNumberOccupants = pTechnoTo ? pBuildingTo->Type->MaxNumberOccupants : 0;

				InfantryClass* infantry = static_cast<InfantryClass*>(pPassenger);
				bool isOccupier = infantry && (infantry->Type->Occupier || pFromTypeExt->Convert_TransferPassengers_IgnoreInvalidOccupiers) ? true : false;

				if (!forceEject && isOccupier && maxNumberOccupants > 0 && nOccupants < maxNumberOccupants)
				{
					pBuildingTo->Occupants.AddItem(infantry);
				}
				else
				{
					// Not enough space inside the new Bunker, eject the passenger
					CoordStruct passengerLoc = PassengerKickOutLocation(transportReference, pPassenger);
					CellClass* pCell = MapClass::Instance->TryGetCellAt(passengerLoc);
					pPassenger->LastMapCoords = pCell->MapCoords;
					pPassenger->Unlimbo(passengerLoc, DirType::North);
				}
			}
			else
			{
				double passengerSize = pPassenger->GetTechnoType()->Size;
				CoordStruct passengerLoc = PassengerKickOutLocation(transportReference, pPassenger);

				if (!forceEject && passengerSize > 0 && (passengersLimit - numPassengers - passengerSize >= 0) && passengerSize <= passengersSizeLimit)
				{
					if (pTechnoTo->GetTechnoType()->OpenTopped)
					{
						pPassenger->SetLocation(pTechnoTo->Location);
						pTechnoTo->EnteredOpenTopped(pPassenger);
					}

					pPassenger->Transporter = pTechnoTo;
					pTechnoTo->AddPassenger(pPassenger);
					numPassengers += (int)passengerSize;
				}
				else
				{
					// Not enough space inside the new transport, eject the passenger
					CellClass* pCell = MapClass::Instance->TryGetCellAt(passengerLoc);
					pPassenger->LastMapCoords = pCell->MapCoords;
					pPassenger->Unlimbo(passengerLoc, DirType::North);
				}
			}
		}
	}
}

bool TechnoExt::AutoDeploy(TechnoClass* pTechnoFrom)
{
	if (!pTechnoFrom->Target)
		return false;
	
	auto pTechnoTypeFrom = pTechnoFrom->GetTechnoType();
	if (!pTechnoTypeFrom)
		return false;
	
	auto pTechnoTypeFromExt = TechnoTypeExt::ExtMap.Find(pTechnoTypeFrom);
	if (!pTechnoTypeFromExt)
		return false;

	
	if (pTechnoTypeFromExt->Convert_UniversalDeploy.size() == 0)
		return false;

	auto pNewTechnoType = pTechnoTypeFromExt->Convert_UniversalDeploy.at(0); // TO-DO

	auto pTechnoFromExt = TechnoExt::ExtMap.Find(pTechnoFrom);
	if (!pTechnoFromExt)
		return false;	

	if (pTechnoFromExt->Convert_AutodeployTimerCountDown <= 0)
	{
		// Prepare the timer
		int startTimer = (pTechnoTypeFromExt->Convert_Autodeploy_Rate.Get() > 0 ? pTechnoTypeFromExt->Convert_Autodeploy_Rate.Get() : 5) * 15;
		pTechnoFromExt->Convert_AutodeployTimerCountDown = startTimer;
		pTechnoFromExt->Convert_AutodeployTimer.Start(startTimer);

		return false;
	}
	else
	{
		// Check if the timer finished
		if (pTechnoFromExt->Convert_AutodeployTimer.Completed())
			pTechnoFromExt->Convert_AutodeployTimerCountDown = -1;
		else
			return false;
	}

	bool forceDeploy = false;
	int targetDistance = pTechnoFrom->DistanceFrom(pTechnoFrom->Target); // Remember: this and weapon ranges in leptons

	// Load weapons data from original techno and deployed techno.
	int weaponIndexFrom = pTechnoFrom->SelectWeapon(pTechnoFrom->Target);
	auto pTechnoToTmp = static_cast<TechnoClass*>(pNewTechnoType->CreateObject(pTechnoFrom->Owner)); // Dummy object
	int weaponIndexTo = pTechnoToTmp->SelectWeapon(pTechnoFrom->Target);

	WeaponTypeClass* weaponTypeFrom = nullptr;
	int weaponMinimumRangeFrom = 0;
	int weaponRangeFrom = 0;

	if (weaponIndexFrom >= 0)
	{
		weaponTypeFrom = pTechnoFrom->GetWeapon(weaponIndexFrom)->WeaponType;
		weaponMinimumRangeFrom = weaponTypeFrom->MinimumRange;
		weaponRangeFrom = weaponTypeFrom->Range;
	}

	WeaponTypeClass* weaponTypeTo = nullptr;
	int weaponMinimumRangeTo = 0;
	int weaponRangeTo = 0;

	if (weaponIndexTo >= 0)
	{
		weaponTypeTo = pTechnoToTmp->GetWeapon(weaponIndexTo)->WeaponType;
		weaponMinimumRangeTo = weaponTypeTo->MinimumRange;
		weaponRangeTo = weaponTypeTo->Range;
	}

	// Load warheads data
	WarheadTypeClass* weaponWarheadFrom = nullptr;

	if (weaponTypeFrom && weaponTypeFrom->Warhead)
		weaponWarheadFrom = weaponTypeFrom->Warhead;

	WarheadTypeClass* weaponWarheadTo = nullptr;

	if (weaponTypeTo && weaponTypeTo->Warhead)
		weaponWarheadTo = weaponTypeTo->Warhead;

	// Check if weapons are in range (if this is a requisite)
	bool checkRangeFrom = pTechnoTypeFromExt->Convert_Autodeploy_InNondeployedRange.Get();
	bool checkRangeTo = pTechnoTypeFromExt->Convert_Autodeploy_InDeployedRange.Get();
	bool isInRangeTo = targetDistance <= weaponRangeTo && targetDistance >= weaponMinimumRangeTo;
	bool isInRangeFrom = targetDistance <= weaponRangeFrom && targetDistance >= weaponMinimumRangeFrom;
	bool isInValidRange = false;

	if ((!checkRangeFrom && !checkRangeTo) || (checkRangeFrom && isInRangeFrom && checkRangeTo && isInRangeTo))
	{
		// Feature deactivated or all checks are valid
		isInValidRange = true;
	}
	else
	{
		if ((checkRangeFrom && isInRangeFrom) || (checkRangeTo && isInRangeTo))
			isInValidRange = true;
	}

	// If the unit is below of the HP threshold and the Autodeploy setting is set then the object must deploy
	if (!forceDeploy && isInValidRange && pTechnoTypeFromExt->Convert_Autodeploy_HP_Threshold.isset())
	{
		bool canBeEvaluated = false;
		double nHealthPercent = (double)(1.0 * pTechnoFrom->Health / pTechnoTypeFrom->Strength);
		double HPThreshold = pTechnoTypeFromExt->Convert_Autodeploy_HP_Threshold.Get();
		HPThreshold = HPThreshold < 0.0 ? 0.0 : HPThreshold;
		HPThreshold = HPThreshold > 1.0 ? 1.0 : HPThreshold;

		if (pTechnoTypeFromExt->Convert_Autodeploy_HP_Threshold_Inverted.Get())
			canBeEvaluated = nHealthPercent >= HPThreshold;
		else
			canBeEvaluated = nHealthPercent < HPThreshold;

		if (canBeEvaluated)
		{
			// Throw the dice and see if there is a valid chance
			int thresholdChance = (int)(pTechnoTypeFromExt->Convert_Autodeploy_HP_Threshold_Chance.Get() * 100);
			thresholdChance = thresholdChance > 100 ? 100 : thresholdChance;
			thresholdChance = thresholdChance < 0 ? 0 : thresholdChance;
			int dice = ScenarioClass::Instance->Random.RandomRanged(0, 99);
			forceDeploy = dice < thresholdChance;
		}
	}

	// If the unit is below of the Shield HP threshold and the Autodeploy setting is set then the object must deploy
	if (!forceDeploy && isInValidRange && pTechnoTypeFromExt->Convert_Autodeploy_SHP_Threshold.isset())
	{
		bool canBeEvaluated = false;
		double SHPThreshold = pTechnoTypeFromExt->Convert_Autodeploy_SHP_Threshold.Get();
		SHPThreshold = SHPThreshold < 0.0 ? 0.0 : SHPThreshold;
		SHPThreshold = SHPThreshold > 1.0 ? 1.0 : SHPThreshold;
		
		if (auto pShieldData = pTechnoFromExt->Shield.get())
		{
			if (pTechnoFromExt->CurrentShieldType && pShieldData)
			{
				double nShieldHealthPercent = (double)(pShieldData->GetHP() / (pTechnoFromExt->CurrentShieldType->Strength.Get() * 1.0));

				if (pTechnoTypeFromExt->Convert_Autodeploy_SHP_Threshold_Inverted.Get())
					canBeEvaluated = nShieldHealthPercent >= SHPThreshold;
				else
					canBeEvaluated = nShieldHealthPercent < SHPThreshold;

				if (canBeEvaluated)
				{
					// Throw the dice and see if there is a valid chance
					int thresholdChance = (int)(pTechnoTypeFromExt->Convert_Autodeploy_SHP_Threshold_Chance.Get() * 100);
					thresholdChance = thresholdChance > 100 ? 100 : thresholdChance;
					thresholdChance = thresholdChance < 0 ? 0 : thresholdChance;
					int dice = ScenarioClass::Instance->Random.RandomRanged(0, 99);
					forceDeploy = dice < thresholdChance ? true : forceDeploy;
				}
			}
		}
	}

	// Special case when the target is out of the weapon range of the deployed object
	if (!forceDeploy && !isInRangeFrom && targetDistance >= weaponMinimumRangeTo && pTechnoTypeFromExt->Convert_Autodeploy_TargetOutOfRange_Chance.Get() > 0.0)
	{
		int chance = pTechnoTypeFromExt->Convert_Autodeploy_TargetOutOfRange_Chance.Get() > 1.0 ? 100 : (int)(pTechnoTypeFromExt->Convert_Autodeploy_TargetOutOfRange_Chance.Get() * 100);
		int dice = ScenarioClass::Instance->Random.RandomRanged(0, 99);

		if (dice < chance)
			forceDeploy = true;
	}

	// Throw the dice and see if there is a chance for deploy automatically and end here
	if (!forceDeploy && isInValidRange && pTechnoTypeFromExt->Convert_Autodeploy_WhileTargeting_Chance.Get() > 0.0)
	{
		int chance = pTechnoTypeFromExt->Convert_Autodeploy_WhileTargeting_Chance.Get() > 1.0 ? 100 : (int)(pTechnoTypeFromExt->Convert_Autodeploy_WhileTargeting_Chance.Get() * 100);
		int dice = ScenarioClass::Instance->Random.RandomRanged(0, 99);

		if (dice < chance)
		{
			if (checkRangeFrom && isInRangeFrom && targetDistance >= weaponMinimumRangeFrom)
				forceDeploy = true;

			if (checkRangeTo && isInRangeTo && targetDistance >= weaponMinimumRangeTo)
				forceDeploy = true;
		}
	}

	// If the deployed object has better weapon range the object must deploy
	if (!forceDeploy && isInValidRange && pTechnoTypeFromExt->Convert_Autodeploy_ByWeaponRange_Chance.Get() > 0.0)
	{
		int chance = pTechnoTypeFromExt->Convert_Autodeploy_ByWeaponRange_Chance.Get() > 1.0 ? 100 : (int)(pTechnoTypeFromExt->Convert_Autodeploy_ByWeaponRange_Chance.Get() * 100);
		int dice = ScenarioClass::Instance->Random.RandomRanged(0, 99);
		
		if (dice < chance)
		{
			// If the objective is inside the MinimumRange of the current object. Is better if the object deploys
			if (weaponMinimumRangeFrom > 0 && targetDistance < weaponMinimumRangeFrom && targetDistance > weaponMinimumRangeTo)
				forceDeploy = true;

			// Useful for structures that can undeploy?
			if (!forceDeploy && (pTechnoTypeFrom->Speed == 0 && targetDistance > weaponRangeFrom && pTechnoToTmp->GetTechnoType()->Speed != 0))
				forceDeploy = true;

			if (!forceDeploy && (isInRangeFrom || isInRangeTo))
			{
				if (pTechnoTypeFromExt->Convert_Autodeploy_ByWeaponRange_Inverted.Get())
				{
					// Range from nondeployed state takes priority in range checks
					if (weaponRangeFrom >= weaponRangeTo && targetDistance <= weaponRangeFrom)
						forceDeploy = true;
				}
				else
				{
					// Range from deployed state takes priority in range checks
					if (weaponRangeTo >= weaponRangeFrom && targetDistance <= weaponRangeTo)
						forceDeploy = true;
				}
			}
		}
	}
	
	auto pTarget = static_cast<TechnoClass*>(pTechnoFrom->Target);
	bool isValidTarget = pTarget->WhatAmI() == AbstractType::Unit
		|| pTarget->WhatAmI() == AbstractType::Building
		|| pTarget->WhatAmI() == AbstractType::Infantry
		|| pTarget->WhatAmI() == AbstractType::Aircraft;

	// If the deployed object has better weapon damage the object must deploy
	if (!forceDeploy && isInValidRange && isValidTarget && pTechnoTypeFromExt->Convert_Autodeploy_ByWeaponDamage_Chance.Get() > 0.0)
	{
		int chance = pTechnoTypeFromExt->Convert_Autodeploy_ByWeaponDamage_Chance.Get() > 1.0 ? 100 : (int)(pTechnoTypeFromExt->Convert_Autodeploy_ByWeaponDamage_Chance.Get() * 100);
		int dice = ScenarioClass::Instance->Random.RandomRanged(0, 99);

		if (dice < chance)
		{
			int weaponDamageFrom = 0;
			int weaponDamageTo = 0;

			if (weaponTypeFrom && weaponWarheadFrom)
			{
				if (weaponTypeFrom->AmbientDamage > 0)
				{
					weaponDamageFrom = MapClass::GetTotalDamage(weaponTypeFrom->AmbientDamage, weaponTypeFrom->Warhead, pTarget->GetTechnoType()->Armor, 0) + MapClass::GetTotalDamage(weaponTypeFrom->Damage, weaponWarheadFrom, pTarget->GetTechnoType()->Armor, 0);
				}
				else
				{
					weaponDamageFrom = MapClass::GetTotalDamage(weaponTypeFrom->Damage, weaponWarheadFrom, pTarget->GetTechnoType()->Armor, 0);
				}
			}

			if (weaponTypeTo && weaponWarheadTo)
			{
				if (weaponTypeTo->AmbientDamage > 0)
				{
					weaponDamageTo = MapClass::GetTotalDamage(weaponTypeTo->AmbientDamage, weaponTypeTo->Warhead, pTarget->GetTechnoType()->Armor, 0) + MapClass::GetTotalDamage(weaponTypeTo->Damage, weaponTypeTo->Warhead, pTarget->GetTechnoType()->Armor, 0);
				}
				else
				{
					weaponDamageTo = MapClass::GetTotalDamage(weaponTypeTo->Damage, weaponTypeTo->Warhead, pTarget->GetTechnoType()->Armor, 0);
				}
			}

			if (pTechnoTypeFromExt->Convert_Autodeploy_ByWeaponDamage_CompareDPS.Get())
			{
				// Damage Per Second

				double dpsFrom = (weaponDamageFrom / (weaponTypeFrom->ROF / 15.0));
				double dpsTo = (weaponDamageTo / (weaponTypeTo->ROF / 15.0));

				if (dpsTo > dpsFrom)
					forceDeploy = true;
			}
			else
			{
				// RAW Damage
				if (weaponDamageTo > weaponDamageFrom)
					forceDeploy = true;
			}
		}
	}

	// We got the weapons data. Not necessary anymore
	if (pTechnoToTmp)
	{
		pTechnoToTmp->Limbo();
		pTechnoToTmp->UnInit();
	}

	if (!forceDeploy)
	{
		// Unit not ready for autodeploying
		int startTimer = (pTechnoTypeFromExt->Convert_Autodeploy_Rate > 0 ? pTechnoTypeFromExt->Convert_Autodeploy_Rate.Get() : 5) * 15;
		pTechnoFromExt->Convert_AutodeployTimerCountDown = startTimer;
		pTechnoFromExt->Convert_AutodeployTimer.Start(startTimer);

		return false;
	}

	pTechnoFromExt->Convert_Autodeploy_RememberTarget = pTechnoFrom->Target;
	pTechnoFromExt->Convert_UniversalDeploy_InProgress = true;
	//Action::Self_Deploy
	//pTechnoFrom->ObjectClickedAction(Action::Self_Deploy, pTechnoFrom, false);
	pTechnoFrom->QueueMission(Mission::Unload, true);

	return false;
}

// =============================
// load / save

template <typename T>
void TechnoExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->TypeExtData)
		.Process(this->Shield)
		.Process(this->LaserTrails)
		.Process(this->ReceiveDamage)
		.Process(this->PassengerDeletionTimer)
		.Process(this->CurrentShieldType)
		.Process(this->LastWarpDistance)
		.Process(this->AutoDeathTimer)
		.Process(this->MindControlRingAnimType)
		.Process(this->OriginalPassengerOwner)
		.Process(this->CurrentLaserWeaponIndex)
		.Process(this->IsInTunnel)
		.Process(this->DeployFireTimer)
		.Process(this->ForceFullRearmDelay)
		.Process(this->DeployAnim)
		.Process(this->Convert_UniversalDeploy_InProgress)
		.Process(this->Convert_UniversalDeploy_MakeInvisible)
		.Process(this->Convert_AutodeployTimerCountDown)
		.Process(this->Convert_AutodeployTimer)
		.Process(this->Convert_AutodeployInProgress)
		.Process(this->Convert_Autodeploy_RememberTarget)
		;
}

void TechnoExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TechnoClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TechnoExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TechnoClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TechnoExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TechnoExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

TechnoExt::ExtContainer::ExtContainer() : Container("TechnoClass") { }

TechnoExt::ExtContainer::~ExtContainer() = default;

void TechnoExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

// =============================
// container hooks

DEFINE_HOOK(0x6F3260, TechnoClass_CTOR, 0x5)
{
	GET(TechnoClass*, pItem, ESI);

	TechnoExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x6F4500, TechnoClass_DTOR, 0x5)
{
	GET(TechnoClass*, pItem, ECX);

	TechnoExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x70C250, TechnoClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x70BF50, TechnoClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TechnoClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TechnoExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x70C249, TechnoClass_Load_Suffix, 0x5)
{
	TechnoExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x70C264, TechnoClass_Save_Suffix, 0x5)
{
	TechnoExt::ExtMap.SaveStatic();

	return 0;
}
