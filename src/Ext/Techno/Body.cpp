#include "Body.h"

#include <AircraftClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <JumpjetLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/AresFunctions.h>

TechnoExt::ExtContainer TechnoExt::ExtMap;
UnitClass* TechnoExt::Deployer = nullptr;

TechnoExt::ExtData::~ExtData()
{
	auto const pTypeExt = this->TypeExtData;
	auto const pType = pTypeExt->OwnerObject();
	auto const pThis = this->OwnerObject();
	// Besides BuildingClass, calling pThis->WhatAmI() here will only result in AbstractType::None
	auto const whatAmI = pType->WhatAmI();

	if (pTypeExt->AutoDeath_Behavior.isset())
	{
		auto& vec = ScenarioExt::Global()->AutoDeathObjects;
		vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
	}

	if (whatAmI != AbstractType::AircraftType && whatAmI != AbstractType::BuildingType
		&& pType->Ammo > 0 && pTypeExt->ReloadInTransport)
	{
		auto& vec = ScenarioExt::Global()->TransportReloaders;
		vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
	}

	if (this->AnimRefCount > 0)
		AnimExt::InvalidateTechnoPointers(pThis);

	if (pTypeExt->Harvester_Counted)
	{
		auto& vec = HouseExt::ExtMap.Find(pThis->Owner)->OwnedCountedHarvesters;
		vec.erase(std::remove(vec.begin(), vec.end(), pThis), vec.end());
	}

	for (auto const pBolt : this->ElectricBolts)
	{
		pBolt->Owner = nullptr;
	}

	this->ElectricBolts.clear();
}

bool TechnoExt::IsActiveIgnoreEMP(TechnoClass* pThis)
{
	return pThis
		&& pThis->IsAlive
		&& pThis->Health > 0
		&& !pThis->InLimbo
		&& !pThis->TemporalTargetingMe
		&& !pThis->BeingWarpedOut
		;
}

bool TechnoExt::IsActive(TechnoClass* pThis)
{
	return TechnoExt::IsActiveIgnoreEMP(pThis)
		&& !pThis->Deactivated
		&& !pThis->IsUnderEMP()
		;
}

bool TechnoExt::IsHarvesting(TechnoClass* pThis)
{
	if (!TechnoExt::IsActive(pThis))
		return false;

	auto const pSlaveManager = pThis->SlaveManager;

	if (pSlaveManager && pSlaveManager->State != SlaveManagerStatus::Ready)
		return true;

	if (pThis->WhatAmI() == AbstractType::Building)
		return pThis->IsPowerOnline();

	if (TechnoExt::HasAvailableDock(pThis))
	{
		switch (pThis->GetCurrentMission())
		{
		case Mission::Harvest:
			if (auto const pUnit = abstract_cast<UnitClass*, true>(pThis))
			{
				if (pUnit->HasAnyLink() && !TechnoExt::HasRadioLinkWithDock(pUnit)) // Probably still in factory.
					return false;

				if (pUnit->IsUseless) // Harvesters currently sitting without purpose are idle even if they are on harvest mission.
					return false;
			}
			return true;
		case Mission::Unload:
			return true;
		case Mission::Enter:
			if (pThis->HasAnyLink())
			{
				auto const pLink = pThis->GetNthLink(0);

				if (pLink->WhatAmI() != AbstractType::Building) // Enter mission + non-building link = not trying to unload
					return false;
			}
			return true;
		case Mission::Guard:
			if (auto pUnit = abstract_cast<UnitClass*, true>(pThis))
			{
				if (pUnit->ArchiveTarget && pUnit->GetStoragePercentage() > 0.0 && pUnit->Locomotor->Is_Moving()) // Edge-case, waiting to be able to unload.
					return true;
			}
			return false;
		default:
			return false;
		}
	}

	return false;
}

bool TechnoExt::HasAvailableDock(TechnoClass* pThis)
{
	for (auto const pBld : pThis->GetTechnoType()->Dock)
	{
		if (pThis->Owner->CountOwnedAndPresent(pBld))
			return true;
	}

	return false;
}

bool TechnoExt::HasRadioLinkWithDock(TechnoClass* pThis)
{
	if (pThis->HasAnyLink())
	{
		auto const pLink = abstract_cast<BuildingClass*, true>(pThis->GetNthLink(0));

		if (pLink && pThis->GetTechnoType()->Dock.FindItemIndex(pLink->Type) >= 0)
			return true;
	}

	return false;
}

// Syncs Iron Curtain or Force Shield timer to another techno.
void TechnoExt::SyncInvulnerability(TechnoClass* pFrom, TechnoClass* pTo)
{
	if (pFrom->IsIronCurtained())
	{
		const auto pTypeExt = TechnoExt::ExtMap.Find(pFrom)->TypeExtData;
		const bool isForceShielded = pFrom->ForceShielded;
		const bool allowSyncing = !isForceShielded
			? pTypeExt->IronCurtain_KeptOnDeploy.Get(RulesExt::Global()->IronCurtain_KeptOnDeploy)
			: pTypeExt->ForceShield_KeptOnDeploy.Get(RulesExt::Global()->ForceShield_KeptOnDeploy);

		if (allowSyncing)
		{
			pTo->IronCurtainTimer = pFrom->IronCurtainTimer;
			pTo->IronTintStage = pFrom->IronTintStage;
			pTo->ForceShielded = isForceShielded;
		}
	}
}

double TechnoExt::GetCurrentSpeedMultiplier(FootClass* pThis)
{
	double houseMultiplier = 1.0;
	auto const whatAmI = pThis->WhatAmI();

	if (whatAmI == AbstractType::Aircraft)
		houseMultiplier = pThis->Owner->Type->SpeedAircraftMult;
	else if (whatAmI == AbstractType::Infantry)
		houseMultiplier = pThis->Owner->Type->SpeedInfantryMult;
	else
		houseMultiplier = pThis->Owner->Type->SpeedUnitsMult;

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	return pThis->SpeedMultiplier * houseMultiplier * pExt->AE.SpeedMultiplier *
		(pThis->HasAbility(Ability::Faster) ? RulesClass::Instance->VeteranSpeed : 1.0);
}

double TechnoExt::GetCurrentFirepowerMultiplier(TechnoClass* pThis)
{
	return pThis->FirepowerMultiplier * TechnoExt::ExtMap.Find(pThis)->AE.FirepowerMultiplier *
		(pThis->HasAbility(Ability::Firepower) ? RulesClass::Instance->VeteranCombat : 1.0);
}

CoordStruct TechnoExt::PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger, int maxAttempts = 1)
{
	if (!pThis || !pPassenger)
		return CoordStruct::Empty;

	if (maxAttempts < 1)
		maxAttempts = 1;

	const auto pTypePassenger = pPassenger->GetTechnoType();
	auto placeCoords = CellStruct::Empty;
	short extraDistance = 1;
	auto speedType = pTypePassenger->SpeedType;
	auto movementZone = pTypePassenger->MovementZone;

	if (pTypePassenger->WhatAmI() == AbstractType::AircraftType)
	{
		speedType = SpeedType::Track;
		movementZone = MovementZone::Normal;
	}
	do
	{
		placeCoords = pThis->GetMapCoords() - CellStruct { static_cast<short>(extraDistance / 2), static_cast<short>(extraDistance / 2) };
		placeCoords = MapClass::Instance.NearByLocation(placeCoords, speedType, -1, movementZone, false, extraDistance, extraDistance, true, false, false, false, CellStruct::Empty, false, false);

		if (placeCoords == CellStruct::Empty)
			return CoordStruct::Empty;

		const auto pCell = MapClass::Instance.GetCellAt(placeCoords);

		if (pThis->IsCellOccupied(pCell, FacingType::None, -1, nullptr, false) == Move::OK)
			break;

		extraDistance++;
	}
	while (extraDistance <= maxAttempts);

	if (const auto pCell = MapClass::Instance.TryGetCellAt(placeCoords))
		return pCell->GetCoordsWithBridge();

	return CoordStruct::Empty;
}

bool TechnoExt::AllowedTargetByZone(TechnoClass* pThis, TechnoClass* pTarget, TargetZoneScanType zoneScanType, WeaponTypeClass* pWeapon, bool useZone, int zone)
{
	if (!pThis || !pTarget)
		return false;

	if (pThis->WhatAmI() == AbstractType::Aircraft)
		return true;

	auto const pType = pThis->GetTechnoType();
	auto const mZone = pType->MovementZone;
	int currentZone = useZone ? zone : MapClass::Instance.GetMovementZoneType(pThis->GetMapCoords(), mZone, pThis->OnBridge);

	if (currentZone != -1)
	{
		if (zoneScanType == TargetZoneScanType::Any)
			return true;

		int targetZone = MapClass::Instance.GetMovementZoneType(pTarget->GetMapCoords(), mZone, pTarget->OnBridge);

		if (zoneScanType == TargetZoneScanType::Same)
		{
			if (currentZone != targetZone)
				return false;
		}
		else
		{
			if (currentZone == targetZone)
				return true;

			auto const speedType = pType->SpeedType;
			auto const cellStruct = MapClass::Instance.NearByLocation(CellClass::Coord2Cell(pTarget->Location),
				speedType, -1, mZone, false, 1, 1, true,
				false, false, speedType != SpeedType::Float, CellStruct::Empty, false, false);

			if (cellStruct == CellStruct::Empty)
				return false;

			auto const pCell = MapClass::Instance.TryGetCellAt(cellStruct);

			if (!pCell)
				return false;

			if (!pWeapon)
			{
				int weaponIndex = pThis->SelectWeapon(pTarget);

				if (weaponIndex < 0)
					return false;

				pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
			}

			const double distance = pCell->GetCoordsWithBridge().DistanceFrom(pTarget->GetCenterCoords());

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
	const auto pType = pThis->GetTechnoType();

	// It really should be at the beginning.
	if (pType == pToType || pType->WhatAmI() != pToType->WhatAmI())
	{
		Debug::Log("Incompatible types between %s and %s\n", pThis->get_ID(), pToType->get_ID());
		return false;
	}

	if (AresFunctions::ConvertTypeTo)
	{
		const int oldHealth = pThis->Health;

		if (AresFunctions::ConvertTypeTo(pThis, pToType))
		{
			// Fixed an issue where morphing could result in -1 health.
			double ratio = static_cast<double>(pToType->Strength) / pType->Strength;
			pThis->Health = static_cast<int>(oldHealth * ratio + 0.5);

			auto const pTypeExt = TechnoExt::ExtMap.Find(static_cast<TechnoClass*>(pThis));
			pTypeExt->UpdateTypeData(pToType);
			pTypeExt->UpdateTypeData_Foot();
			return true;
		}

		return false;
	}

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

	// Detach CLEG targeting
	auto const tempUsing = pThis->TemporalImUsing;
	if (tempUsing && tempUsing->Target)
		tempUsing->LetGo();

	auto const pOwner = pThis->Owner;

	// Remove tracking of old techno
	if (!pThis->InLimbo)
		pOwner->RegisterLoss(pThis, false);
	pOwner->RemoveTracking(pThis);

	const int oldHealth = pThis->Health;

	// Generic type-conversion
	auto const prevType = *nowTypePtr;
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
		if (auto const newLoco = LocomotionClass::CreateInstance(toLoco))
		{
			newLoco->Link_To_Object(pThis);
			pThis->Locomotor = std::move(newLoco);
		}
	}

	const auto& jjLoco = LocomotionClass::CLSIDs::Jumpjet;
	if (pToType->BalloonHover && pToType->DeployToLand && prevType->Locomotor != jjLoco && toLoco == jjLoco)
		pThis->Locomotor->Move_To(pThis->Location);

	auto const pTypeExt = TechnoExt::ExtMap.Find(static_cast<TechnoClass*>(pThis));
	pTypeExt->UpdateTypeData(pToType);
	pTypeExt->UpdateTypeData_Foot();
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

	auto mapCoords = CellClass::Coord2Cell(pThis->GetCoords());

	if (pDeployType->GetFoundationWidth() > 2 || pDeployType->GetFoundationHeight(false) > 2)
		mapCoords += CellStruct { -1, -1 };

	// The vanilla game used an inappropriate approach here, resulting in potential risk of desync.
	// Now, through additional checks, we can directly exclude the unit who want to deploy.
	TechnoExt::Deployer = pThis;
	const bool canDeploy = pDeployType->CanCreateHere(mapCoords, pThis->Owner);
	TechnoExt::Deployer = nullptr;

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

/// <summary>
/// Gets whether or not techno has listed AttachEffect types active on it
/// </summary>
/// <param name="attachEffectTypes">Attacheffect types.</param>
/// <param name="requireAll">Whether or not to require all listed types to be present or if only one will satisfy the check.</param>
/// <param name="ignoreSameSource">Ignore AttachEffects that come from set invoker and source.</param>
/// <param name="pInvoker">Invoker Techno used for same source check.</param>
/// <param name="pSource">Source AbstractClass instance used for same source check.</param>
/// <returns>True if techno has active AttachEffects that satisfy the source, false if not.</returns>
bool TechnoExt::ExtData::HasAttachedEffects(std::vector<AttachEffectTypeClass*> attachEffectTypes, bool requireAll, bool ignoreSameSource,
	TechnoClass* pInvoker, AbstractClass* pSource, std::vector<int> const* minCounts, std::vector<int> const* maxCounts) const
{
	unsigned int foundCount = 0;
	unsigned int typeCounter = 1;
	const bool checkSource = ignoreSameSource && pInvoker && pSource;

	for (auto const& type : attachEffectTypes)
	{
		for (auto const& attachEffect : this->AttachedEffects)
		{
			if (attachEffect->GetType() == type && attachEffect->IsActive())
			{
				if (checkSource && attachEffect->IsFromSource(pInvoker, pSource))
					continue;

				const unsigned int minSize = minCounts ? minCounts->size() : 0;
				const unsigned int maxSize = maxCounts ? maxCounts->size() : 0;

				if (type->Cumulative && (minSize > 0 || maxSize > 0))
				{
					const int cumulativeCount = this->GetAttachedEffectCumulativeCount(type, ignoreSameSource, pInvoker, pSource);

					if (minSize > 0)
					{
						if (cumulativeCount < minCounts->at(typeCounter - 1 >= minSize ? minSize - 1 : typeCounter - 1))
							continue;
					}
					if (maxSize > 0)
					{
						if (cumulativeCount > maxCounts->at(typeCounter - 1 >= maxSize ? maxSize - 1 : typeCounter - 1))
							continue;
					}
				}

				// Only need to find one match, can stop here.
				if (!requireAll)
					return true;

				foundCount++;
				break;
			}
		}

		// One of the required types was not found, can stop here.
		if (requireAll && foundCount < typeCounter)
			return false;

		typeCounter++;
	}

	if (requireAll && foundCount == attachEffectTypes.size())
		return true;

	return false;
}

/// <summary>
/// Gets how many counts of same cumulative AttachEffect type instance techno has active on it.
/// </summary>
/// <param name="pAttachEffectType">AttachEffect type.</param>
/// <param name="ignoreSameSource">Ignore AttachEffects that come from set invoker and source.</param>
/// <param name="pInvoker">Invoker Techno used for same source check.</param>
/// <param name="pSource">Source AbstractClass instance used for same source check.</param>
/// <returns>Number of active cumulative AttachEffect type instances on the techno. 0 if the AttachEffect type is not cumulative.</returns>
int TechnoExt::ExtData::GetAttachedEffectCumulativeCount(AttachEffectTypeClass* pAttachEffectType, bool ignoreSameSource, TechnoClass* pInvoker, AbstractClass* pSource) const
{
	if (!pAttachEffectType->Cumulative)
		return 0;

	unsigned int foundCount = 0;
	const bool checkSource = ignoreSameSource && pInvoker && pSource;

	for (auto const& attachEffect : this->AttachedEffects)
	{
		if (attachEffect->GetType() == pAttachEffectType && attachEffect->IsActive())
		{
			if (checkSource && attachEffect->IsFromSource(pInvoker, pSource))
				continue;

			foundCount++;
		}
	}

	return foundCount;
}

UnitTypeClass* TechnoExt::GetUnitTypeExtra(UnitClass* pUnit)
{
	if (pUnit->IsGreenHP())
	{
		return nullptr;
	}
	else if (pUnit->IsYellowHP())
	{
		auto const pData = TechnoTypeExt::ExtMap.Find(pUnit->Type);

		if (pUnit->GetCell()->LandType == LandType::Water && !pUnit->OnBridge)
		{
			if (auto const imageYellow = pData->WaterImage_ConditionYellow)
				return imageYellow;
		}
		else if (auto const imageYellow = pData->Image_ConditionYellow)
		{
			return abstract_cast<UnitTypeClass*, true>(imageYellow);
		}
	}
	else
	{
		auto const pData = TechnoTypeExt::ExtMap.Find(pUnit->Type);

		if (pUnit->GetCell()->LandType == LandType::Water && !pUnit->OnBridge)
		{
			if (auto const imageRed = pData->WaterImage_ConditionRed)
				return imageRed;
			else if (auto const imageYellow = pData->WaterImage_ConditionYellow)
				return imageYellow;
		}
		else if (auto const imageRed = pData->Image_ConditionRed)
		{
			return abstract_cast<UnitTypeClass*, true>(imageRed);
		}
		else if (auto const imageYellow = pData->Image_ConditionYellow)
		{
			return abstract_cast<UnitTypeClass*, true>(imageYellow);
		}
	}

	return nullptr;
}

AircraftTypeClass* TechnoExt::GetAircraftTypeExtra(AircraftClass* pAircraft)
{
	if (pAircraft->IsGreenHP())
	{
		return pAircraft->Type;
	}
	else if (pAircraft->IsYellowHP())
	{
		auto const pData = TechnoTypeExt::ExtMap.Find(pAircraft->Type);

		if (auto const imageYellow = pData->Image_ConditionYellow)
			return abstract_cast<AircraftTypeClass*, true>(imageYellow);
	}
	else
	{
		auto const pType = pAircraft->Type;
		auto const pData = TechnoTypeExt::ExtMap.Find(pType);

		if (auto const imageRed = pData->Image_ConditionRed)
			return abstract_cast<AircraftTypeClass*, true>(imageRed);
		else if (auto const imageYellow = pData->Image_ConditionYellow)
			return abstract_cast<AircraftTypeClass*, true>(imageYellow);
	}

	return pAircraft->Type;

}

void TechnoExt::ExtData::ResetDelayedFireTimer()
{
	this->DelayedFireTimer.Stop();
	this->DelayedFireWeaponIndex = -1;
	this->DelayedFireSequencePaused = false;

	if (this->CurrentDelayedFireAnim)
	{
		if (AnimExt::ExtMap.Find(this->CurrentDelayedFireAnim)->DelayedFireRemoveOnNoDelay)
			this->CurrentDelayedFireAnim->UnInit();
	}
}

void TechnoExt::CreateDelayedFireAnim(TechnoClass* pThis, AnimTypeClass* pAnimType, int weaponIndex, bool attach, bool center, bool removeOnNoDelay, bool onTurret, CoordStruct firingCoords)
{
	if (pAnimType)
	{
		auto coords = pThis->GetCenterCoords();

		if (!center)
			coords = TechnoExt::GetFLHAbsoluteCoords(pThis, firingCoords, onTurret);

		auto const pAnim = GameCreate<AnimClass>(pAnimType, coords);

		if (attach)
			pAnim->SetOwnerObject(pThis);

		auto const pAnimExt = AnimExt::ExtMap.Find(pAnim);
		pAnim->Owner = pThis->Owner;
		pAnimExt->SetInvoker(pThis);

		if (attach)
		{
			pAnimExt->DelayedFireRemoveOnNoDelay = removeOnNoDelay;
			TechnoExt::ExtMap.Find(pThis)->CurrentDelayedFireAnim = pAnim;
		}
	}
}

bool TechnoExt::HandleDelayedFireWithPauseSequence(TechnoClass* pThis, int weaponIndex, int firingFrame)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto& timer = pExt->DelayedFireTimer;
	auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pExt->DelayedFireWeaponIndex >= 0 && pExt->DelayedFireWeaponIndex != weaponIndex)
	{
		pExt->ResetDelayedFireTimer();
		pExt->DelayedFireSequencePaused = false;
	}

	if (pWeaponExt->DelayedFire_PauseFiringSequence && pWeaponExt->DelayedFire_Duration.isset() && (!pThis->Transporter || !pWeaponExt->DelayedFire_SkipInTransport))
	{
		if (pWeapon->Burst <= 1 || !pWeaponExt->DelayedFire_OnlyOnInitialBurst || pThis->CurrentBurstIndex == 0)
		{
			if (pThis->Animation.Value == firingFrame)
				pExt->DelayedFireSequencePaused = true;

			if (!timer.HasStarted())
			{
				pExt->DelayedFireWeaponIndex = weaponIndex;
				timer.Start(Math::max(GeneralUtils::GetRangedRandomOrSingleValue(pWeaponExt->DelayedFire_Duration), 0));
				auto pAnimType = pWeaponExt->DelayedFire_Animation;

				if (pThis->Transporter && pWeaponExt->DelayedFire_OpenToppedAnimation.isset())
					pAnimType = pWeaponExt->DelayedFire_OpenToppedAnimation;

				auto firingCoords = pThis->GetWeapon(weaponIndex)->FLH;

				if (pWeaponExt->DelayedFire_AnimOffset.isset())
					firingCoords = pWeaponExt->DelayedFire_AnimOffset;

				TechnoExt::CreateDelayedFireAnim(pThis, pAnimType, weaponIndex, pWeaponExt->DelayedFire_AnimIsAttached, pWeaponExt->DelayedFire_CenterAnimOnFirer,
					pWeaponExt->DelayedFire_RemoveAnimOnNoDelay, pWeaponExt->DelayedFire_AnimOnTurret, firingCoords);

				return true;
			}
			else if (timer.InProgress())
			{
				return true;
			}

			if (timer.Completed())
				pExt->ResetDelayedFireTimer();
		}

		pExt->DelayedFireSequencePaused = false;
	}

	return false;
}

bool TechnoExt::IsHealthInThreshold(TechnoClass* pObject, double min, double max)
{
	const double hp = pObject->GetHealthPercentage();
	return hp <= max && hp >= min;
}

bool TechnoExt::CannotMove(UnitClass* pThis)
{
	const auto pType = pThis->Type;

	if (pType->Speed == 0)
		return true;

	if (!locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
	{
		LandType landType = pThis->GetCell()->LandType;
		const LandType movementRestrictedTo = pType->MovementRestrictedTo;

		if (pThis->OnBridge
			&& (landType == LandType::Water || landType == LandType::Beach))
		{
			landType = LandType::Road;
		}

		if (movementRestrictedTo != LandType::None
			&& movementRestrictedTo != landType
			&& landType != LandType::Tunnel)
		{
			return true;
		}
	}

	return false;
}

bool TechnoExt::HasAmmoToDeploy(TechnoClass* pThis)
{
	const auto pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;

	const int min = pTypeExt->Ammo_DeployUnlockMinimumAmount;
	const int max = pTypeExt->Ammo_DeployUnlockMaximumAmount;

	if (min < 0 && max < 0)
		return true;

	const int ammo = pThis->Ammo;

	if ((min < 0 || ammo >= min) && (max < 0 || ammo <= max))
		return true;

	return false;
}

void TechnoExt::HandleOnDeployAmmoChange(TechnoClass* pThis, int maxAmmoOverride)
{
	const auto pTypeExt = TechnoExt::ExtMap.Find(pThis)->TypeExtData;
	int add = pTypeExt->Ammo_AddOnDeploy;

	if (add != 0)
	{
		int maxAmmo = pTypeExt->OwnerObject()->Ammo;

		if (maxAmmoOverride >= 0)
			maxAmmo = maxAmmoOverride;

		int originalAmmo = pThis->Ammo;
		pThis->Ammo = std::clamp(pThis->Ammo + add, 0, maxAmmo);

		if (originalAmmo != pThis->Ammo)
		{
			pThis->StartReloading();
			pThis->Mark(MarkType::Change);
		}
	}
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
		.Process(this->AttachedEffects)
		.Process(this->AE)
		.Process(this->PreviousType)
		.Process(this->AnimRefCount)
		.Process(this->ReceiveDamage)
		.Process(this->LastKillWasTeamTarget)
		.Process(this->PassengerDeletionTimer)
		.Process(this->CurrentShieldType)
		.Process(this->LastWarpDistance)
		.Process(this->ChargeTurretTimer)
		.Process(this->AutoDeathTimer)
		.Process(this->MindControlRingAnimType)
		.Process(this->DamageNumberOffset)
		.Process(this->Strafe_BombsDroppedThisRound)
		.Process(this->Strafe_TargetCell)
		.Process(this->CurrentAircraftWeaponIndex)
		.Process(this->IsInTunnel)
		.Process(this->IsBurrowed)
		.Process(this->HasBeenPlacedOnMap)
		.Process(this->DeployFireTimer)
		.Process(this->SkipTargetChangeResetSequence)
		.Process(this->ForceFullRearmDelay)
		.Process(this->LastRearmWasFullDelay)
		.Process(this->CanCloakDuringRearm)
		.Process(this->WHAnimRemainingCreationInterval)
		.Process(this->LastWeaponType)
		.Process(this->FiringObstacleCell)
		.Process(this->IsDetachingForCloak)
		.Process(this->BeControlledThreatFrame)
		.Process(this->LastTargetID)
		.Process(this->AccumulatedGattlingValue)
		.Process(this->ShouldUpdateGattlingValue)
		.Process(this->OriginalPassengerOwner)
		.Process(this->HasRemainingWarpInDelay)
		.Process(this->LastWarpInDelay)
		.Process(this->IsBeingChronoSphered)
		.Process(this->KeepTargetOnMove)
		.Process(this->LastSensorsMapCoords)
		.Process(this->TiberiumEater_Timer)
		.Process(this->AirstrikeTargetingMe)
		.Process(this->FiringAnimationTimer)
		.Process(this->SimpleDeployerAnimationTimer)
		.Process(this->DelayedFireSequencePaused)
		.Process(this->DelayedFireTimer)
		.Process(this->DelayedFireWeaponIndex)
		.Process(this->CurrentDelayedFireAnim)
		.Process(this->AttachedEffectInvokerCount)
		.Process(this->TintColorOwner)
		.Process(this->TintColorAllies)
		.Process(this->TintColorEnemies)
		.Process(this->TintIntensityOwner)
		.Process(this->TintIntensityAllies)
		.Process(this->TintIntensityEnemies)
		.Process(this->AttackMoveFollowerTempCount)
		;
}

void TechnoExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(this->AirstrikeTargetingMe, ptr);
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

DEFINE_HOOK(0x710415, TechnoClass_DetachAnim, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	GET(AbstractClass*, pTarget, EAX);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->CurrentDelayedFireAnim == pTarget)
		pExt->CurrentDelayedFireAnim = nullptr;

	return 0;
}
