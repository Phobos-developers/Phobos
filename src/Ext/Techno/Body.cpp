#include "Body.h"

#include <AircraftClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Ext/House/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Script/Body.h>

#include <Utilities/AresFunctions.h>

TechnoExt::ExtContainer TechnoExt::ExtMap;

TechnoExt::ExtData::~ExtData()
{
	auto const pTypeExt = this->TypeExtData;
	auto const pType = pTypeExt->OwnerObject();
	auto pThis = this->OwnerObject();

	if (pTypeExt->AutoDeath_Behavior.isset())
	{
		auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
		auto& vec = pOwnerExt->OwnedAutoDeathObjects;
		vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
	}

	if (pThis->WhatAmI() != AbstractType::Aircraft && pThis->WhatAmI() != AbstractType::Building
		&& pType->Ammo > 0 && pTypeExt->ReloadInTransport)
	{
		auto const pOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);
		auto& vec = pOwnerExt->OwnedTransportReloaders;
		vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
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
	while (extraDistanceX < maxAttempts && (pThis->IsCellOccupied(pCell, FacingType::None, -1, nullptr, false) != Move::OK) && pCell->MapCoords != CellStruct::Empty);

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
	int currentZone = useZone ? zone : MapClass::Instance->GetMovementZoneType(pThis->GetMapCoords(), mZone, pThis->OnBridge);

	if (currentZone != -1)
	{
		if (zoneScanType == TargetZoneScanType::Any)
			return true;

		int targetZone = MapClass::Instance->GetMovementZoneType(pTarget->GetMapCoords(), mZone, pTarget->OnBridge);

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

// Feature for common usage : TechnoType conversion -- Trsdy
// BTW, who said it was merely a Type pointer replacement and he could make a better one than Ares?
bool TechnoExt::ConvertToType(FootClass* pThis, TechnoTypeClass* pToType)
{
	if (IS_ARES_FUN_AVAILABLE(ConvertTypeTo))
		return AresFunctions::ConvertTypeTo(pThis, pToType);
	// In case not using Ares 3.0. Only update necessary vanilla properties
	AbstractType rtti;
	TechnoTypeClass** nowTypePtr;

	// Different types prohibited
	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry:
		nowTypePtr = reinterpret_cast<TechnoTypeClass**>(&(static_cast<InfantryClass*>(pThis)->Type));
		rtti = AbstractType::InfantryType;
		break;
	case AbstractType::Unit:
		nowTypePtr = reinterpret_cast<TechnoTypeClass**>(&(static_cast<UnitClass*>(pThis)->Type));
		rtti = AbstractType::UnitType;
		break;
	case AbstractType::Aircraft:
		nowTypePtr = reinterpret_cast<TechnoTypeClass**>(&(static_cast<AircraftClass*>(pThis)->Type));
		rtti = AbstractType::AircraftType;
		break;
	default:
		Debug::Log("%s is not FootClass, conversion not allowed\n", pToType->get_ID());
		return false;
	}

	if (pToType->WhatAmI() != rtti)
	{
		Debug::Log("Incompatible types between %s and %s\n", pThis->get_ID(), pToType->get_ID());
		return false;
	}

	// Detach CLEG targeting
	auto tempUsing = pThis->TemporalImUsing;
	if (tempUsing && tempUsing->Target)
		tempUsing->Detach();

	HouseClass* const pOwner = pThis->Owner;

	// Remove tracking of old techno
	if (!pThis->InLimbo)
		pOwner->RegisterLoss(pThis, false);
	pOwner->RemoveTracking(pThis);

	int oldHealth = pThis->Health;

	// Generic type-conversion
	TechnoTypeClass* prevType = *nowTypePtr;
	*nowTypePtr = pToType;

	// Readjust health according to percentage
	pThis->SetHealthPercentage((double)(oldHealth) / (double)prevType->Strength);
	pThis->EstimatedHealth = pThis->Health;

	// Add tracking of new techno
	pOwner->AddTracking(pThis);
	if (!pThis->InLimbo)
		pOwner->RegisterGain(pThis, false);
	pOwner->RecheckTechTree = true;

	// Update Ares AttachEffects -- skipped
	// Ares RecalculateStats -- skipped

	// Adjust ammo
	pThis->Ammo = Math::min(pThis->Ammo, pToType->Ammo);
	// Ares ResetSpotlights -- skipped

	// Adjust ROT
	if (rtti == AbstractType::AircraftType)
		pThis->SecondaryFacing.SetROT(pToType->ROT);
	else
		pThis->PrimaryFacing.SetROT(pToType->ROT);
	// Adjust Ares TurretROT -- skipped
	//  pThis->SecondaryFacing.SetROT(TechnoTypeExt::ExtMap.Find(pToType)->TurretROT.Get(pToType->ROT));

	// Locomotor change, referenced from Ares 0.A's abduction code, not sure if correct, untested
	CLSID nowLocoID;
	ILocomotion* iloco = pThis->Locomotor;
	const auto& toLoco = pToType->Locomotor;
	if ((SUCCEEDED(static_cast<LocomotionClass*>(iloco)->GetClassID(&nowLocoID)) && nowLocoID != toLoco))
	{
		// because we are throwing away the locomotor in a split second, piggybacking
		// has to be stopped. otherwise the object might remain in a weird state.
		while (LocomotionClass::End_Piggyback(pThis->Locomotor));
		// throw away the current locomotor and instantiate
		// a new one of the default type for this unit.
		if (auto newLoco = LocomotionClass::CreateInstance(toLoco))
		{
			newLoco->Link_To_Object(pThis);
			pThis->Locomotor = std::move(newLoco);
		}
	}

	// TODO : Jumpjet locomotor special treatement, some brainfart, must be uncorrect, HELP ME!
	const auto& jjLoco = LocomotionClass::CLSIDs::Jumpjet();
	if (pToType->BalloonHover && pToType->DeployToLand && prevType->Locomotor != jjLoco && toLoco == jjLoco)
		pThis->Locomotor->Move_To(pThis->Location);

	return true;
}

// Checks if vehicle can deploy into a building at its current location. If unit has no DeploysInto set returns noDeploysIntoDefaultValue (def = false) instead.
bool TechnoExt::CanDeployIntoBuilding(UnitClass* pThis, bool noDeploysIntoDefaultValue)
{
	if (!pThis)
		return false;

	auto const pDeployType = pThis->Type->DeploysInto;

	if (!pDeployType)
		return noDeploysIntoDefaultValue;

	bool canDeploy = true;
	auto mapCoords = CellClass::Coord2Cell(pThis->GetCoords());

	if (pDeployType->GetFoundationWidth() > 2 || pDeployType->GetFoundationHeight(false) > 2)
		mapCoords += CellStruct { -1, -1 };

	pThis->Mark(MarkType::Up);

	pThis->Locomotor->Mark_All_Occupation_Bits(MarkType::Up);

	if (!pDeployType->CanCreateHere(mapCoords, pThis->Owner))
		canDeploy = false;

	pThis->Locomotor->Mark_All_Occupation_Bits(MarkType::Down);
	pThis->Mark(MarkType::Down);

	return canDeploy;
}

bool TechnoExt::IsTypeImmune(TechnoClass* pThis, TechnoClass* pSource)
{
	if (!pThis || !pSource)
		return false;

	auto const pType = pThis->GetTechnoType();

	if (!pType->TypeImmune)
		return false;

	if (pType == pSource->GetTechnoType() && pThis->Owner == pSource->Owner)
		return true;

	return false;
}

bool TechnoExt::IsUnitAvailable(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed)
{
	if (!pTechno)
		return false;

	bool isAvailable = pTechno->IsAlive && pTechno->Health > 0 && !pTechno->InLimbo && pTechno->IsOnMap;

	if (checkIfInTransportOrAbsorbed)
		isAvailable &= !pTechno->Absorbed && !pTechno->Transporter;

	return isAvailable;
}

bool TechnoExt::UpdateRandomTarget(TechnoClass* pThis)
{
	if (!pThis)
		return false;

	int weaponIndex = pThis->SelectWeapon(pThis->Target);
	auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	if (!pWeapon)
		return false;

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	if (!pWeaponExt || pWeaponExt->RandomTarget <= 0.0)
		return false;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return false;

	if (pThis->GetCurrentMission() == Mission::Move)
	{
		pExt->CurrentRandomTarget = nullptr;
		pExt->OriginalTarget = nullptr;

		return false;
	}

	if (pExt->CurrentRandomTarget && IsUnitAvailable(pExt->CurrentRandomTarget, false) && pThis->SpawnManager)
		return false;

	if (!pThis->Target && !IsUnitAvailable(abstract_cast<TechnoClass*>(pExt->OriginalTarget), false))
	{
		pExt->OriginalTarget = nullptr;
		return false;
	}

	if (pExt->OriginalTarget && !IsUnitAvailable(abstract_cast<TechnoClass*>(pExt->OriginalTarget), false))
	{
		pExt->CurrentRandomTarget = nullptr;
		pExt->OriginalTarget = nullptr;
	}

	if (pThis->GetCurrentMission() != Mission::Attack)
	{
		pExt->OriginalTarget = nullptr;
		return false;
	}

	if (!pThis->Target)
		return false;

	if (pThis->DistanceFrom(pExt->OriginalTarget) > pWeapon->Range)
	{
		if (pThis->WhatAmI() == AbstractType::Building)
		{
			pThis->SetTarget(nullptr);
			pExt->CurrentRandomTarget = nullptr;
			pExt->OriginalTarget = nullptr;

			return false;
		}

		pThis->SetTarget(pExt->OriginalTarget);
	}

	if (pThis->DistanceFrom(pThis->Target) > pWeapon->Range)
	{
		pThis->SetTarget(pExt->OriginalTarget);
		return false;
	}

	auto pRandomTarget = FindRandomTarget(pThis);

	if (!pRandomTarget)
		return false;

	pExt->OriginalTarget = !pExt->OriginalTarget ? pThis->Target : pExt->OriginalTarget;
	pExt->CurrentRandomTarget = pRandomTarget;
	pThis->Target = pRandomTarget;

	if (pThis->SpawnManager)
	{
		bool isFirstSpawn = true;

		for (auto pSpawn : pThis->SpawnManager->SpawnedNodes)
		{
			if (!pSpawn->Unit)
				continue;

			TechnoClass* pSpawnTarget = nullptr;

			auto pSpawnExt = TechnoExt::ExtMap.Find(pSpawn->Unit);
			if (!pSpawnExt)
				continue;

			if (isFirstSpawn)
			{
				pSpawnTarget = pExt->CurrentRandomTarget;

				if (pWeaponExt->RandomTarget_Spawners_MultipleTargets)
					isFirstSpawn = false;
			}
			else
			{
				pSpawnTarget = FindRandomTarget(pThis);

				if (!pSpawnTarget)
					pSpawnTarget = abstract_cast<TechnoClass*>(pExt->OriginalTarget);
			}

			pSpawnExt->CurrentRandomTarget = pSpawnTarget;
			pSpawnExt->OriginalTarget = pExt->OriginalTarget;
		}
	}

	return true;
}

// Find all valid targets in the attacker's area and from the valid ones picks one randomly
TechnoClass* TechnoExt::FindRandomTarget(TechnoClass* pThis)
{
	TechnoClass* selection = nullptr;

	if (!pThis && !pThis->Target)
		return selection;

	int weaponIndex = pThis->SelectWeapon(pThis->Target);
	auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	if (!pWeapon)
		return selection;

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	if (!pWeaponExt || pWeaponExt->RandomTarget <= 0.0)
		return selection;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return selection;

	int retargetProbability = std::min((int)round(pWeaponExt->RandomTarget * 100), 100);
	int dice = ScenarioClass::Instance->Random.RandomRanged(1, 100);

	if (retargetProbability < dice)
		return selection;

	auto pThisType = pThis->GetTechnoType();
	int minimumRange = pWeapon->MinimumRange;
	int range = pWeapon->Range;
	int airRange = pWeapon->Range + pThisType->AirRangeBonus;
	bool omniFire = pWeapon->OmniFire;
	std::vector<TechnoClass*> candidates;
	auto originalTarget = abstract_cast<TechnoClass*>(!pExt->OriginalTarget ? pThis->Target : pExt->OriginalTarget);
	bool friendlyFire = pThis->Owner->IsAlliedWith(originalTarget);

	// Looking for all valid targeting candidates
	for (auto pCandidate : *TechnoClass::Array)
	{
		if (pCandidate == pThis
			|| !IsUnitAvailable(pCandidate, true)
			|| pThisType->Immune
			|| !EnumFunctions::IsTechnoEligible(pCandidate, pWeaponExt->CanTarget, true)
			|| (!pWeapon->Projectile->AA && pCandidate->IsInAir())
			|| (!pWeapon->Projectile->AG && !pCandidate->IsInAir())
			|| (!friendlyFire && (pThis->Owner->IsAlliedWith(pCandidate) || ScriptExt::IsUnitMindControlledFriendly(pThis->Owner, pCandidate)))
			|| pCandidate->TemporalTargetingMe
			|| pCandidate->BeingWarpedOut
			|| (pCandidate->GetTechnoType()->Underwater && pCandidate->GetTechnoType()->NavalTargeting == NavalTargetingType::Underwater_Never)
			|| (pCandidate->GetTechnoType()->Naval && pCandidate->GetTechnoType()->NavalTargeting == NavalTargetingType::Naval_None)
			|| (pCandidate->CloakState == CloakState::Cloaked && !pThisType->Naval)
			|| (pCandidate->InWhichLayer() == Layer::Underground))
		{
			continue;
		}

		int distanceFromAttacker = pThis->DistanceFrom(pCandidate);
		if (distanceFromAttacker < minimumRange)
			continue;

		if (omniFire)
		{
			if (pCandidate->IsInAir())
				range = airRange;

			if (distanceFromAttacker <= range)
				candidates.push_back(pCandidate);
		}
		else
		{
			int distanceFromOriginalTargetToCandidate = pCandidate->DistanceFrom(pThis->Target);
			int distanceFromOriginalTarget = pThis->DistanceFrom(pThis->Target);

			if (pCandidate->IsInAir())
				range = airRange;

			if (distanceFromAttacker <= range && distanceFromOriginalTargetToCandidate <= distanceFromOriginalTarget)
				candidates.push_back(pCandidate);
		}
	}

	if (candidates.size() == 0)
		return selection;

	// Pick one new target from the list of targets inside the weapon range
	dice = ScenarioClass::Instance->Random.RandomRanged(0, candidates.size() - 1);
	selection = candidates.at(dice);

	return selection;
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
		.Process(this->IsInTunnel)
		.Process(this->HasBeenPlacedOnMap)
		.Process(this->DeployFireTimer)
		.Process(this->ForceFullRearmDelay)
		.Process(this->WHAnimRemainingCreationInterval)
		.Process(this->OriginalTarget)
		.Process(this->CurrentRandomTarget)
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


// =============================
// container hooks

DEFINE_HOOK(0x6F3260, TechnoClass_CTOR, 0x5)
{
	GET(TechnoClass*, pItem, ESI);

	TechnoExt::ExtMap.TryAllocate(pItem);

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

