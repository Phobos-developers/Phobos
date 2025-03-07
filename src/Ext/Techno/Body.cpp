#include "Body.h"

#include <AircraftClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/AresFunctions.h>

TechnoExt::ExtContainer TechnoExt::ExtMap;

TechnoExt::ExtData::~ExtData()
{
	auto const pTypeExt = this->TypeExtData;
	auto const pType = pTypeExt->OwnerObject();
	auto pThis = this->OwnerObject();

	if (pTypeExt->AutoDeath_Behavior.isset())
	{
		auto& vec = ScenarioExt::Global()->AutoDeathObjects;
		vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
	}

	if (pThis->WhatAmI() != AbstractType::Aircraft && pThis->WhatAmI() != AbstractType::Building
		&& pType->Ammo > 0 && pTypeExt->ReloadInTransport)
	{
		auto& vec = ScenarioExt::Global()->TransportReloaders;
		vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
	}

	if (this->AnimRefCount > 0)
		AnimExt::InvalidateTechnoPointers(pThis);

	if (this->TypeExtData->Harvester_Counted)
	{
		auto& vec = HouseExt::ExtMap.Find(pThis->Owner)->OwnedCountedHarvesters;
		vec.erase(std::remove(vec.begin(), vec.end(), pThis), vec.end());
	}
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
			if (auto const pUnit = abstract_cast<UnitClass*>(pThis))
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
			if (auto pUnit = abstract_cast<UnitClass*>(pThis))
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
	for (auto pBld : pThis->GetTechnoType()->Dock)
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
		auto const pLink = abstract_cast<BuildingClass*>(pThis->GetNthLink(0));

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
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pFrom->GetTechnoType());
		bool isForceShielded = pFrom->ForceShielded;
		bool allowSyncing = !isForceShielded ? pTypeExt->IronCurtain_KeptOnDeploy.Get(RulesExt::Global()->IronCurtain_KeptOnDeploy) :
			pTypeExt->ForceShield_KeptOnDeploy.Get(RulesExt::Global()->ForceShield_KeptOnDeploy);

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

	if (pThis->WhatAmI() == AbstractType::Aircraft)
		houseMultiplier = pThis->Owner->Type->SpeedAircraftMult;
	else if (pThis->WhatAmI() == AbstractType::Infantry)
		houseMultiplier = pThis->Owner->Type->SpeedInfantryMult;
	else
		houseMultiplier = pThis->Owner->Type->SpeedUnitsMult;

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	return pThis->SpeedMultiplier * houseMultiplier * pExt->AE.SpeedMultiplier *
		(pThis->HasAbility(Ability::Faster) ? RulesClass::Instance->VeteranSpeed : 1.0);
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
		placeCoords = MapClass::Instance->NearByLocation(placeCoords, speedType, -1, movementZone, false, extraDistance, extraDistance, true, false, false, false, CellStruct::Empty, false, false);

		if (placeCoords == CellStruct::Empty)
			return CoordStruct::Empty;

		const auto pCell = MapClass::Instance->GetCellAt(placeCoords);

		if (pThis->IsCellOccupied(pCell, FacingType::None, -1, nullptr, false) == Move::OK)
			break;

		extraDistance++;
	}
	while (extraDistance <= maxAttempts);

	if (const auto pCell = MapClass::Instance->TryGetCellAt(placeCoords))
		return pCell->GetCoordsWithBridge();

	return CoordStruct::Empty;
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

			if (cellStruct == CellStruct::Empty)
				return false;

			auto const pCell = MapClass::Instance->TryGetCellAt(cellStruct);

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
	if (AresFunctions::ConvertTypeTo)
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
		tempUsing->LetGo();

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

	for (auto const& type : attachEffectTypes)
	{
		for (auto const& attachEffect : this->AttachedEffects)
		{
			if (attachEffect->GetType() == type && attachEffect->IsActive())
			{
				if (ignoreSameSource && pInvoker && pSource && attachEffect->IsFromSource(pInvoker, pSource))
					continue;

				unsigned int minSize = minCounts ? minCounts->size() : 0;
				unsigned int maxSize = maxCounts ? maxCounts->size() : 0;

				if (type->Cumulative && (minSize > 0 || maxSize > 0))
				{
					int cumulativeCount = this->GetAttachedEffectCumulativeCount(type, ignoreSameSource, pInvoker, pSource);

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

	for (auto const& attachEffect : this->AttachedEffects)
	{
		if (attachEffect->GetType() == pAttachEffectType && attachEffect->IsActive())
		{
			if (ignoreSameSource && pInvoker && pSource && attachEffect->IsFromSource(pInvoker, pSource))
				continue;

			foundCount++;
		}
	}

	return foundCount;
}

int TechnoExt::CalculateBlockDamage(TechnoClass* pThis, args_ReceiveDamage* args)
{
	int damage = *args->Damage;
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(args->WH);

	if (pWHExt->ImmuneToBlock)
		return damage;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (!pTypeExt->CanBlock)
		return damage;

	const auto pBlockType = pWHExt->Block_BasedOnWarhead ? pWHExt->BlockType.get() : pTypeExt->BlockType.get();
	const auto pOtherBlock = !pWHExt->Block_BasedOnWarhead ? pWHExt->BlockType.get() : pTypeExt->BlockType.get();
	std::vector<double> blockChances = pBlockType->Block_Chances;
	std::vector<double> blockDamageMultipliers = pBlockType->Block_DamageMultipliers;

	if (pWHExt->Block_AllowOverride)
	{
		blockChances = !pOtherBlock->Block_Chances.empty() ? pOtherBlock->Block_Chances : blockChances;
		blockDamageMultipliers = !pOtherBlock->Block_DamageMultipliers.empty() ? pOtherBlock->Block_DamageMultipliers : blockDamageMultipliers;
	}

	if (!pWHExt->Block_IgnoreAttachEffect)
	{
		std::pair<std::vector<double>, std::vector<double>> blockPair = TechnoExt::GetBlockChanceAndDamageMult(pThis, blockChances, blockDamageMultipliers);
		blockChances = blockPair.first;
		blockDamageMultipliers = blockPair.second;
	}

	if ((blockChances.size() == 1 && blockChances[0] + pWHExt->Block_ExtraChance > 0.0) || blockChances.size() > 1)
	{
		// handle block conditions first
		auto blockAffectBelowPercents = pBlockType->Block_AffectBelowPercents;
		auto blockAffectsHouses = pBlockType->Block_AffectsHouses.Get(AffectedHouse::All);
		bool blockCanActiveZeroDamage = pBlockType->Block_CanActive_ZeroDamage.Get(false);
		bool blockCanActiveNegativeDamage = pBlockType->Block_CanActive_NegativeDamage.Get(false);
		bool blockCanActivePowered = pBlockType->Block_CanActive_Powered.Get(false);
		bool blockCanActiveNoFirer = pBlockType->Block_CanActive_NoFirer.Get(true);
		bool blockCanActiveShieldActive = pBlockType->Block_CanActive_ShieldActive.Get(true);
		bool blockCanActiveShieldInactive = pBlockType->Block_CanActive_ShieldInactive.Get(true);
		bool blockCanActiveMove = pBlockType->Block_CanActive_Move.Get(true);
		bool blockCanActiveStationary = pBlockType->Block_CanActive_Stationary.Get(true);

		if (pWHExt->Block_AllowOverride)
		{
			blockAffectBelowPercents = !pOtherBlock->Block_AffectBelowPercents.empty() ? pOtherBlock->Block_AffectBelowPercents : blockAffectBelowPercents;
			blockAffectsHouses = pOtherBlock->Block_AffectsHouses.isset() ? pOtherBlock->Block_AffectsHouses.Get() : blockAffectsHouses;
			blockCanActiveZeroDamage = pOtherBlock->Block_CanActive_ZeroDamage.isset() ? pOtherBlock->Block_CanActive_ZeroDamage : blockCanActiveZeroDamage;
			blockCanActiveNegativeDamage = pOtherBlock->Block_CanActive_NegativeDamage.isset() ? pOtherBlock->Block_CanActive_NegativeDamage : blockCanActiveNegativeDamage;
			blockCanActivePowered = pOtherBlock->Block_CanActive_Powered.isset() ? pOtherBlock->Block_CanActive_Powered : blockCanActivePowered;
			blockCanActiveNoFirer = pOtherBlock->Block_CanActive_NoFirer.isset() ? pOtherBlock->Block_CanActive_NoFirer : blockCanActiveNoFirer;
			blockCanActiveShieldActive = pOtherBlock->Block_CanActive_ShieldActive.isset() ? pOtherBlock->Block_CanActive_ShieldActive : blockCanActiveShieldActive;
			blockCanActiveShieldInactive = pOtherBlock->Block_CanActive_ShieldInactive.isset() ? pOtherBlock->Block_CanActive_ShieldInactive : blockCanActiveShieldInactive;
			blockCanActiveMove = pOtherBlock->Block_CanActive_Move.isset() ? pOtherBlock->Block_CanActive_Move : blockCanActiveMove;
			blockCanActiveStationary = pOtherBlock->Block_CanActive_Stationary.isset() ? pOtherBlock->Block_CanActive_Stationary : blockCanActiveStationary;
		}

		if (blockAffectBelowPercents.size() > 0 && pThis->GetHealthPercentage() > blockAffectBelowPercents[0])
			return damage;

		if (damage == 0 && !blockCanActiveZeroDamage)
			return 0;
		else if (damage < 0 && !blockCanActiveNegativeDamage)
			return damage;

		unsigned int level = 0;

		if (blockAffectBelowPercents.size() > 0)
		{
			for (; level < blockAffectBelowPercents.size() - 1; level++)
			{
				if (pThis->GetHealthPercentage() > blockAffectBelowPercents[level + 1])
					break;
			}
		}

		double dice = ScenarioClass::Instance->Random.RandomDouble();

		if (blockChances.size() == 1)
		{
			if (blockChances[0] * pWHExt->Block_ChanceMultiplier + pWHExt->Block_ExtraChance < dice)
				return damage;
		}
		else if (blockChances.size() <= level || blockChances[level] * pWHExt->Block_ChanceMultiplier + pWHExt->Block_ExtraChance < dice)
		{
			return damage;
		}

		if (blockCanActivePowered)
		{
			bool isActive = !(pThis->Deactivated || pThis->IsUnderEMP());

			if (isActive && pThis->WhatAmI() == AbstractType::Building)
			{
				auto const pBuilding = static_cast<BuildingClass const*>(pThis);
				isActive = pBuilding->IsPowerOnline();
			}

			if (!isActive)
				return damage;
		}

		if (auto const pFoot = abstract_cast<FootClass*>(pThis))
		{
			if (pFoot->Locomotor->Is_Moving())
			{
				if (!blockCanActiveMove)
					return damage;
			}
			else if (!blockCanActiveStationary)
			{
				return damage;
			}
		}

		const auto pFirer = args->Attacker;

		if (pFirer)
		{
			if (pFirer->Owner && !EnumFunctions::CanTargetHouse(blockAffectsHouses, pFirer->Owner, pThis->Owner))
				return damage;
		}
		else if (!blockCanActiveNoFirer)
		{
			return damage;
		}

		const auto pShieldData = pExt->Shield.get();

		if (pShieldData && pShieldData->IsActive())
		{
			if (!blockCanActiveShieldActive || !pShieldData->GetType()->CanBlock)
				return damage;
		}
		else if (!blockCanActiveShieldInactive)
		{
			return damage;
		}

		// a block is triggered
		auto blockAnims = pBlockType->Block_Anims;
		auto blockWeapon = pBlockType->Block_Weapon.Get();
		bool blockFlash = pBlockType->Block_Flash.Get(false);
		bool blockReflectDamage = pBlockType->Block_ReflectDamage.Get(false);
		double blockReflectDamageChance = pBlockType->Block_ReflectDamage_Chance.Get(1.0);

		if (pWHExt->Block_AllowOverride)
		{
			blockAnims = !pOtherBlock->Block_Anims.empty() ? pOtherBlock->Block_Anims : blockAnims;
			blockWeapon = pOtherBlock->Block_Weapon.isset() ? pOtherBlock->Block_Weapon.Get() : blockWeapon;
			blockFlash = pOtherBlock->Block_Flash.isset() ? pOtherBlock->Block_Flash.Get() : blockFlash;
			blockReflectDamage = pOtherBlock->Block_ReflectDamage.isset() ? pOtherBlock->Block_ReflectDamage.Get() : blockReflectDamage;
			blockReflectDamageChance = pOtherBlock->Block_ReflectDamage_Chance.isset() ? pOtherBlock->Block_ReflectDamage_Chance.Get() : blockReflectDamageChance;
		}

		if (blockAnims.size() > 0)
		{
			int idx = ScenarioClass::Instance->Random.RandomRanged(0, blockAnims.size() - 1);
			GameCreate<AnimClass>(blockAnims[idx], pThis->Location);
		}

		if (blockFlash)
		{
			int size = pBlockType->Block_Flash_FixedSize.Get(damage * 2);
			SpotlightFlags flags = SpotlightFlags::NoColor;
			bool blockFlashRed = pBlockType->Block_Flash_Red.Get(true);
			bool blockFlashGreen = pBlockType->Block_Flash_Green.Get(true);
			bool blockFlashBlue = pBlockType->Block_Flash_Blue.Get(true);
			bool blockFlashBlack = pBlockType->Block_Flash_Black.Get(false);

			if (pWHExt->Block_AllowOverride)
			{
				size = pOtherBlock->Block_Flash_FixedSize.isset() ? pOtherBlock->Block_Flash_FixedSize.Get() : size;
				blockFlashRed = pOtherBlock->Block_Flash_Red.isset() ? pOtherBlock->Block_Flash_Red.Get() : blockFlashRed;
				blockFlashGreen = pOtherBlock->Block_Flash_Green.isset() ? pOtherBlock->Block_Flash_Green.Get() : blockFlashGreen;
				blockFlashBlue = pOtherBlock->Block_Flash_Blue.isset() ? pOtherBlock->Block_Flash_Blue.Get() : blockFlashBlue;
				blockFlashBlack = pOtherBlock->Block_Flash_Black.isset() ? pOtherBlock->Block_Flash_Black.Get() : blockFlashBlack;
			}

			if (blockFlashBlack)
			{
				flags = SpotlightFlags::NoColor;
			}
			else
			{
				if (!blockFlashRed)
					flags = SpotlightFlags::NoRed;
				if (!blockFlashGreen)
					flags |= SpotlightFlags::NoGreen;
				if (!blockFlashBlue)
					flags |= SpotlightFlags::NoBlue;
			}

			MapClass::FlashbangWarheadAt(size, args->WH, pThis->Location, true, flags);
		}

		if (blockReflectDamage && blockReflectDamageChance >= ScenarioClass::Instance->Random.RandomDouble()
			&& damage > 0 && pFirer && !pWHExt->SuppressReflectDamage && !pWHExt->Reflected)
		{
			auto pWHRef = pBlockType->Block_ReflectDamage_Warhead.Get(RulesClass::Instance->C4Warhead);
			auto blockReflectDamageAffectsHouses = pBlockType->Block_ReflectDamage_AffectsHouses.Get(blockAffectsHouses);
			Nullable<int> blockReflectDamageOverride = pBlockType->Block_ReflectDamage_Override;
			double blockReflectDamageMultiplier = pBlockType->Block_ReflectDamage_Multiplier.Get(1.0);
			bool blockReflectDamageWHDetonate = pBlockType->Block_ReflectDamage_Warhead_Detonate.Get(false);

			if (pWHExt->Block_AllowOverride)
			{
				pWHRef = pOtherBlock->Block_ReflectDamage_Warhead.isset() ? pOtherBlock->Block_ReflectDamage_Warhead.Get() : pWHRef;
				blockReflectDamageOverride = pOtherBlock->Block_ReflectDamage_Override.isset() ? pOtherBlock->Block_ReflectDamage_Override : blockReflectDamageOverride;
				blockReflectDamageAffectsHouses = pOtherBlock->Block_ReflectDamage_AffectsHouses.isset() ? pOtherBlock->Block_ReflectDamage_AffectsHouses.Get() : blockReflectDamageAffectsHouses;
				blockReflectDamageMultiplier = pOtherBlock->Block_ReflectDamage_Multiplier.isset() ? pOtherBlock->Block_ReflectDamage_Multiplier.Get() : blockReflectDamageMultiplier;
				blockReflectDamageWHDetonate = pOtherBlock->Block_ReflectDamage_Warhead_Detonate.isset() ? pOtherBlock->Block_ReflectDamage_Warhead_Detonate.Get() : blockReflectDamageWHDetonate;
			}

			int damageRef = blockReflectDamageOverride.Get(static_cast<int>(damage * blockReflectDamageMultiplier));

			if (EnumFunctions::CanTargetHouse(blockReflectDamageAffectsHouses, pThis->Owner, pFirer->Owner))
			{
				auto const pWHExtRef = WarheadTypeExt::ExtMap.Find(pWHRef);
				pWHExtRef->Reflected = true;

				if (blockReflectDamageWHDetonate)
					WarheadTypeExt::DetonateAt(pWHRef, pFirer, pThis, damageRef, pThis->Owner);
				else
					pFirer->ReceiveDamage(&damage, 0, pWHRef, pThis, false, false, pThis->Owner);

				pWHExtRef->Reflected = false;
			}
		}

		if (blockDamageMultipliers.size() == 1)
			damage = static_cast<int>(damage * (blockDamageMultipliers[0] * pWHExt->Block_ChanceMultiplier + pWHExt->Block_ExtraChance));
		else if (blockDamageMultipliers.size() > level)
			damage = static_cast<int>(damage * (blockDamageMultipliers[level] * pWHExt->Block_ChanceMultiplier + pWHExt->Block_ExtraChance));

		if (blockWeapon)
			TechnoExt::FireWeaponAtSelf(pThis, blockWeapon);
	}

	return damage;
}

std::pair<std::vector<double>, std::vector<double>> TechnoExt::GetBlockChanceAndDamageMult(TechnoClass* pThis, std::vector<double> blockChance, std::vector<double> blockDamageMult)
{
	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (blockChance.size() == 0)
		blockChance.push_back(0.0);

	if (blockDamageMult.size() == 0)
		blockDamageMult.push_back(0.0);

	double extraChance = 0.0;
	double extraDamageMult = 0.0;

	for (auto& attachEffect : pExt->AttachedEffects)
	{
		if (!attachEffect->IsActive())
			continue;

		auto const pType = attachEffect->GetType();

		if (pType->Block_ChanceMultiplier == 1.0 && pType->Block_ExtraChance == 0.0)
			continue;

		for (auto& chance : blockChance)
		{
			chance = chance * Math::max(pType->Block_ChanceMultiplier, 0);
		}

		for (auto& extraDamage : blockDamageMult)
		{
			extraDamage = static_cast<int>(extraDamage * pType->Block_DamageMult_Multiplier);
		}

		extraChance += pType->Block_ExtraChance;
		extraDamageMult += pType->Block_DamageMult_Bonus;
	}

	for (auto& chance : blockChance)
	{
		chance += extraChance;
	}

	for (auto& extraDamage : blockDamageMult)
	{
		extraDamage += extraDamageMult;
	}

	return std::pair<std::vector<double>, std::vector<double>>(blockChance, blockDamageMult);
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
		.Process(this->AnimRefCount)
		.Process(this->ReceiveDamage)
		.Process(this->PassengerDeletionTimer)
		.Process(this->CurrentShieldType)
		.Process(this->LastWarpDistance)
		.Process(this->ChargeTurretTimer)
		.Process(this->AutoDeathTimer)
		.Process(this->MindControlRingAnimType)
		.Process(this->Strafe_BombsDroppedThisRound)
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
		.Process(this->LastTargetID)
		.Process(this->AccumulatedGattlingValue)
		.Process(this->ShouldUpdateGattlingValue)
		.Process(this->OriginalPassengerOwner)
		.Process(this->HasRemainingWarpInDelay)
		.Process(this->LastWarpInDelay)
		.Process(this->IsBeingChronoSphered)
		.Process(this->KeepTargetOnMove)
		.Process(this->LastSensorsMapCoords)
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
