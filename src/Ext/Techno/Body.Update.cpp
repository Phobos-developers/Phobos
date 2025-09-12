// methods used in TechnoClass_AI hooks or anything similar
#include "Body.h"

#include <SessionClass.h>
#include <SpawnManagerClass.h>
#include <ParticleSystemClass.h>
#include <Conversions.h>
#include <SlaveManagerClass.h>
#include <AirstrikeClass.h>
#include <Kamikaze.h>
#include <JumpjetLocomotionClass.h>
#include <FlyLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/AresFunctions.h>


// TechnoClass_AI_0x6F9E50
// It's not recommended to do anything more here it could have a better place for performance consideration
void TechnoExt::ExtData::OnEarlyUpdate()
{
	auto const pType = this->OwnerObject()->GetTechnoType();

	// Set only if unset or type is changed
	// Notice that Ares may handle type conversion in the same hook here, which is executed right before this one thankfully
	if (!this->TypeExtData || this->TypeExtData->OwnerObject() != pType)
		this->UpdateTypeData(pType);

	if (this->CheckDeathConditions())
		return;

	this->UpdateShield();
	this->UpdateAttachEffects();
	this->ApplyInterceptor();
	this->EatPassengers();
	this->ApplySpawnLimitRange();
	this->ApplyMindControlRangeLimit();
	this->UpdateRecountBurst();
	this->UpdateRearmInEMPState();

	if (this->AttackMoveFollowerTempCount)
		this->AttackMoveFollowerTempCount--;
}

void TechnoExt::ExtData::ApplyInterceptor()
{
	const auto pThis = this->OwnerObject();
	const auto pTypeExt = this->TypeExtData;
	const auto pInterceptorType = pTypeExt->InterceptorType.get();

	if (!pInterceptorType)
		return;

	const auto pTarget = pThis->Target;

	if (pTarget)
	{
		if (pTarget->WhatAmI() != AbstractType::Bullet)
			return;

		const auto pTargetExt = BulletExt::ExtMap.Find(static_cast<BulletClass*>(pTarget));

		if ((pTargetExt->InterceptedStatus & InterceptedStatus::Locked) == InterceptedStatus::None)
			return;
	}

	const int count = BulletClass::Array.Count;

	if (this->IsBurrowed || !count)
		return;

	BulletClass* pTargetBullet = nullptr;
	const double guardRange = pInterceptorType->GuardRange.Get(pThis);
	const double guardRangeSq = guardRange * guardRange;
	const double minguardRange = pInterceptorType->MinimumGuardRange.Get(pThis);
	const double minguardRangeSq = minguardRange * minguardRange;
	const auto pOwner = pThis->Owner;
	const auto location = pThis->Location;
	const auto canTargetHouses = pInterceptorType->CanTargetHouses;
	// Interceptor weapon is always fixed
	const auto pWeapon = pThis->GetWeapon(pInterceptorType->Weapon)->WeaponType;
	const auto pWH = pWeapon->Warhead;

	// DO NOT iterate BulletExt::ExtMap here, the order of items is not deterministic
	// so it can differ across players throwing target management out of sync.
	int i = 0;

	for ( ; i < count; ++i)
	{
		const auto& pBullet = BulletClass::Array.GetItem(i);
		const auto pBulletExt = BulletExt::ExtMap.Find(pBullet);
		const auto pBulletTypeExt = pBulletExt->TypeExtData;

		if (!pBulletTypeExt->Interceptable || pBullet->SpawnNextAnim)
			continue;

		const auto distanceSq = pBullet->Location.DistanceFromSquared(location);

		if (distanceSq > guardRangeSq || distanceSq < minguardRangeSq)
			continue;

		if (pBulletTypeExt->Armor.isset())
		{
			const double versus = GeneralUtils::GetWarheadVersusArmor(pWH, pBulletTypeExt->Armor.Get());

			if (versus == 0.0)
				continue;
		}

		const auto bulletOwner = pBullet->Owner ? pBullet->Owner->Owner : pBulletExt->FirerHouse;

		if (!EnumFunctions::CanTargetHouse(canTargetHouses, pOwner, bulletOwner))
			continue;

		if (pBulletExt->InterceptedStatus & (InterceptedStatus::Targeted | InterceptedStatus::Locked))
		{
			// Set as optional target
			pTargetBullet = pBullet;
			break;
		}

		// Establish target
		pThis->SetTarget(pBullet);
		return;
	}

	// Loop ends and there is no target
	if (!pTargetBullet)
		return;

	// There is an optional target, but it is still possible to continue checking for more suitable target
	for ( ; i < count; ++i)
	{
		const auto& pBullet = BulletClass::Array.GetItem(i);
		const auto pBulletExt = BulletExt::ExtMap.Find(pBullet);

		if (pBulletExt->InterceptedStatus & (InterceptedStatus::Targeted | InterceptedStatus::Locked))
			continue;

		const auto pBulletTypeExt = pBulletExt->TypeExtData;

		if (!pBulletTypeExt->Interceptable || pBullet->SpawnNextAnim)
			continue;

		const auto distanceSq = pBullet->Location.DistanceFromSquared(location);

		if (distanceSq > guardRangeSq || distanceSq < minguardRangeSq)
			continue;

		if (pBulletTypeExt->Armor.isset())
		{
			const double versus = GeneralUtils::GetWarheadVersusArmor(pWH, pBulletTypeExt->Armor.Get());

			if (versus == 0.0)
				continue;
		}

		const auto bulletOwner = pBullet->Owner ? pBullet->Owner->Owner : pBulletExt->FirerHouse;

		if (!EnumFunctions::CanTargetHouse(canTargetHouses, pOwner, bulletOwner))
			continue;

		// Establish target
		pThis->SetTarget(pBullet);
		return;
	}

	// There is no more suitable target, establish optional target
	if (pTargetBullet)
		pThis->SetTarget(pTargetBullet);
}

void TechnoExt::ExtData::DepletedAmmoActions()
{
	auto const pThis = this->OwnerObject();
	auto const pTypeExt = this->TypeExtData;
	auto const pType = pTypeExt->OwnerObject();

	if (pType->Ammo <= 0)
		return;

	auto const rtti = pThis->WhatAmI();
	UnitClass* pUnit = nullptr;

	if (rtti == AbstractType::Unit)
	{
		pUnit = static_cast<UnitClass*>(pThis);
		auto const pUnitType = pUnit->Type;

		if (!pUnitType->IsSimpleDeployer && !pUnitType->DeploysInto && !pUnitType->DeployFire
			&& pUnitType->Passengers < 1 && pUnit->Passengers.NumPassengers < 1)
		{
			return;
		}
	}

	int const min = pTypeExt->Ammo_AutoDeployMinimumAmount;
	int const max = pTypeExt->Ammo_AutoDeployMaximumAmount;

	if (min < 0 && max < 0)
		return;

	int const ammo = pThis->Ammo;
	bool canDeploy = TechnoExt::HasAmmoToDeploy(pThis) && (min < 0 || ammo >= min) && (max < 0 || ammo <= max);
	bool isDeploying = pThis->CurrentMission == Mission::Unload || pThis->QueuedMission == Mission::Unload;

	if (canDeploy && !isDeploying)
	{
		pThis->QueueMission(Mission::Unload, true);
	}
	else if (!canDeploy && isDeploying)
	{
		pThis->QueueMission(Mission::Guard, true);

		if (pUnit && pUnit->Type->IsSimpleDeployer && pThis->InAir)
		{
			if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pUnit->Locomotor))
				pJJLoco->State = JumpjetLocomotionClass::State::Ascending;
		}
	}
}

// TODO : Merge into new AttachEffects
bool TechnoExt::ExtData::CheckDeathConditions(bool isInLimbo)
{
	auto const pTypeExt = this->TypeExtData;

	if (!pTypeExt->AutoDeath_Behavior.isset())
		return false;

	auto const pThis = this->OwnerObject();
	auto const pType = pThis->GetTechnoType();

	// Self-destruction must be enabled
	const auto howToDie = pTypeExt->AutoDeath_Behavior.Get();

	// Death if no ammo
	if (pType->Ammo > 0 && pThis->Ammo <= 0 && pTypeExt->AutoDeath_OnAmmoDepletion)
	{
		TechnoExt::KillSelf(pThis, howToDie, pTypeExt->AutoDeath_VanishAnimation, isInLimbo);
		return true;
	}

	// Death if countdown ends
	if (pTypeExt->AutoDeath_AfterDelay > 0)
	{
		if (!this->AutoDeathTimer.HasStarted())
		{
			this->AutoDeathTimer.Start(pTypeExt->AutoDeath_AfterDelay);
		}
		else if (this->AutoDeathTimer.Completed())
		{
			TechnoExt::KillSelf(pThis, howToDie, pTypeExt->AutoDeath_VanishAnimation, isInLimbo);
			return true;
		}
	}

	auto const pOwner = pThis->Owner;

	auto existTechnoTypes = [pOwner](const ValueableVector<TechnoTypeClass*>& vTypes, AffectedHouse affectedHouse, bool any, bool allowLimbo)
		{
			auto existSingleType = [pOwner, affectedHouse, allowLimbo](TechnoTypeClass* pType)
				{
					if (affectedHouse == AffectedHouse::Owner)
						return allowLimbo ? HouseExt::ExtMap.Find(pOwner)->CountOwnedPresentAndLimboed(pType) > 0 : pOwner->CountOwnedAndPresent(pType) > 0;

					for (auto const pHouse : HouseClass::Array)
					{
						if (EnumFunctions::CanTargetHouse(affectedHouse, pOwner, pHouse)
							&& (allowLimbo ? HouseExt::ExtMap.Find(pHouse)->CountOwnedPresentAndLimboed(pType) > 0 : pHouse->CountOwnedAndPresent(pType) > 0))
							return true;
					}

					return false;
				};

			return any
				? std::any_of(vTypes.begin(), vTypes.end(), existSingleType)
				: std::all_of(vTypes.begin(), vTypes.end(), existSingleType);
		};

	// death if listed technos don't exist
	if (!pTypeExt->AutoDeath_TechnosDontExist.empty())
	{
		if (!existTechnoTypes(pTypeExt->AutoDeath_TechnosDontExist, pTypeExt->AutoDeath_TechnosDontExist_Houses, !pTypeExt->AutoDeath_TechnosDontExist_Any, pTypeExt->AutoDeath_TechnosDontExist_AllowLimboed))
		{
			TechnoExt::KillSelf(pThis, howToDie, pTypeExt->AutoDeath_VanishAnimation, isInLimbo);

			return true;
		}
	}

	// death if listed technos exist
	if (!pTypeExt->AutoDeath_TechnosExist.empty())
	{
		if (existTechnoTypes(pTypeExt->AutoDeath_TechnosExist, pTypeExt->AutoDeath_TechnosExist_Houses, pTypeExt->AutoDeath_TechnosExist_Any, pTypeExt->AutoDeath_TechnosExist_AllowLimboed))
		{
			TechnoExt::KillSelf(pThis, howToDie, pTypeExt->AutoDeath_VanishAnimation, isInLimbo);

			return true;
		}
	}

	return false;
}

void TechnoExt::ExtData::EatPassengers()
{
	auto const pThis = this->OwnerObject();
	auto const pTypeExt = this->TypeExtData;

	if (!pTypeExt->PassengerDeletionType || !TechnoExt::IsActiveIgnoreEMP(pThis))
		return;

	auto const pDelType = pTypeExt->PassengerDeletionType.get();

	if (!pDelType->UnderEMP && (pThis->Deactivated || pThis->IsUnderEMP()))
		return;

	if (pDelType->Rate > 0 || pDelType->UseCostAsRate)
	{
		if (pThis->Passengers.NumPassengers > 0)
		{
			// Passengers / CargoClass is essentially a stack, last in, first out (LIFO) kind of data structure
			FootClass* pPassenger = nullptr;          // Passenger to potentially delete
			FootClass* pPreviousPassenger = nullptr;  // Passenger immediately prior to the deleted one in the stack
			ObjectClass* pLastPassenger = nullptr;    // Passenger that is last in the stack
			auto pCurrentPassenger = pThis->Passengers.GetFirstPassenger();
			const auto allowedHouses = pDelType->AllowedHouses;
			const auto pOwner = pThis->Owner;

			// Find the first entered passenger that is eligible for deletion.
			while (pCurrentPassenger)
			{
				if (EnumFunctions::CanTargetHouse(allowedHouses, pOwner, pCurrentPassenger->Owner))
				{
					pPreviousPassenger = abstract_cast<FootClass*>(pLastPassenger);
					pPassenger = pCurrentPassenger;
				}

				pLastPassenger = pCurrentPassenger;
				pCurrentPassenger = abstract_cast<FootClass*>(pCurrentPassenger->NextObject);
			}

			if (!pPassenger)
			{
				this->PassengerDeletionTimer.Stop();
				return;
			}

			if (!this->PassengerDeletionTimer.IsTicking()) // Execute only if timer has been stopped or not started
			{
				int timerLength = 0;

				if (pDelType->UseCostAsRate)
				{
					// Use passenger cost as countdown.
					timerLength = (int)(pPassenger->GetTechnoType()->Cost * pDelType->CostMultiplier);

					if (pDelType->CostRateCap.isset())
						timerLength = std::min(timerLength, pDelType->CostRateCap.Get());
				}
				else
				{
					// Use explicit rate optionally multiplied by unit size as countdown.
					timerLength = pDelType->Rate;
					const double size = (double)pPassenger->GetTechnoType()->Size;

					if (pDelType->Rate_SizeMultiply && size > 1.0)
						timerLength *= (int)(size + 0.5);
				}

				this->PassengerDeletionTimer.Start(timerLength);
			}
			else if (this->PassengerDeletionTimer.Completed()) // Execute only if timer has ran out after being started
			{
				--pThis->Passengers.NumPassengers;

				if (pLastPassenger)
					pLastPassenger->NextObject = nullptr;

				if (pPreviousPassenger)
					pPreviousPassenger->NextObject = pPassenger->NextObject;

				if (pThis->Passengers.NumPassengers <= 0)
					pThis->Passengers.FirstPassenger = nullptr;

				if (pDelType->ReportSound >= 0)
					VocClass::PlayAt(pDelType->ReportSound.Get(), pThis->GetCoords(), nullptr);

				AnimExt::CreateRandomAnim(pDelType->Anim, pThis->Location, pThis, nullptr, true, true);

				// Check if there is money refund
				if (pDelType->Soylent
					&& EnumFunctions::CanTargetHouse(pDelType->SoylentAllowedHouses, pOwner, pPassenger->Owner))
				{
					const int nMoneyToGive = (int)(pPassenger->GetTechnoType()->GetRefund(pPassenger->Owner, true) * pDelType->SoylentMultiplier);

					if (nMoneyToGive > 0)
					{
						pOwner->GiveMoney(nMoneyToGive);

						if (pDelType->DisplaySoylent)
						{
							FlyingStrings::AddMoneyString(nMoneyToGive, pOwner,
								pDelType->DisplaySoylentToHouses, pThis->Location, pDelType->DisplaySoylentOffset);
						}
					}
				}

				// Handle gunner change.
				auto const pTransportType = pThis->GetTechnoType();

				if (pTransportType->Gunner)
				{
					if (auto const pFoot = abstract_cast<FootClass*, true>(pThis))
					{
						pFoot->RemoveGunner(pPassenger);

						if (auto pGunner = pFoot->Passengers.GetFirstPassenger())
						{
							for (auto pNext = abstract_cast<FootClass*>(pGunner->NextObject); pNext; pNext = abstract_cast<FootClass*>(pNext->NextObject))
								pGunner = pNext;

							pFoot->ReceiveGunner(pGunner);
						}
					}
				}

				auto const pSource = pDelType->DontScore ? nullptr : pThis;
				pPassenger->KillPassengers(pSource);
				pPassenger->RegisterDestruction(pSource);
				pPassenger->UnInit();

				// Handle extra power
				if (auto const pBldType = abstract_cast<BuildingTypeClass*, true>(pTransportType))
				{
					if (pBldType->ExtraPowerBonus || pBldType->ExtraPowerDrain)
						pOwner->RecheckPower = true;
				}

				this->PassengerDeletionTimer.Stop();
			}
		}
		else
		{
			this->PassengerDeletionTimer.Stop();
		}
	}
}

void TechnoExt::ExtData::UpdateTiberiumEater()
{
	const auto pEaterType = this->TypeExtData->TiberiumEaterType.get();

	if (!pEaterType)
		return;

	const int transDelay = pEaterType->TransDelay;

	if (transDelay && this->TiberiumEater_Timer.InProgress())
		return;

	const auto pThis = this->OwnerObject();
	const auto pOwner = pThis->Owner;
	bool active = false;
	const bool displayCash = pEaterType->Display && pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer);
	int facing = pThis->PrimaryFacing.Current().GetFacing<8>();

	if (facing >= 7)
		facing = 0;
	else
		facing++;

	const int cellCount = static_cast<int>(pEaterType->Cells.size());
	const int locationZ = pThis->Location.Z;
	const int numOrePurifiers = pOwner->NumOrePurifiers;
	const float cashMultiplier = pEaterType->CashMultiplier;
	const float purifierBonus = RulesClass::Instance->PurifierBonus;
	const bool animMove = pEaterType->AnimMove;
	const auto displayToHouse = pEaterType->DisplayToHouse;
	const auto amountPerCell = pEaterType->AmountPerCell;
	const auto displayOffset = pEaterType->DisplayOffset;
	const auto& animsAll = pEaterType->Anims;
	auto* scenarioRandom = &ScenarioClass::Instance->Random;

	for (int idx = 0; idx < cellCount; idx++)
	{
		const auto& cellOffset = pEaterType->Cells[idx];
		const auto pos = TechnoExt::GetFLHAbsoluteCoords(pThis, CoordStruct { cellOffset.X, cellOffset.Y, 0 }, false);
		const auto pCell = MapClass::Instance.TryGetCellAt(pos);

		if (!pCell)
			continue;

		if (const int contained = pCell->GetContainedTiberiumValue())
		{
			const int tiberiumIdx = pCell->GetContainedTiberiumIndex();
			const int tiberiumValue = TiberiumClass::Array[tiberiumIdx]->Value;
			const int tiberiumAmount = static_cast<int>(static_cast<double>(contained) / tiberiumValue);
			const int amount = amountPerCell > 0 ? std::min(amountPerCell.Get(), tiberiumAmount) : tiberiumAmount;
			pCell->ReduceTiberium(amount);
			const float multiplier = cashMultiplier * (1.0f + numOrePurifiers * purifierBonus);
			const int value = static_cast<int>(std::round(amount * tiberiumValue * multiplier));
			pOwner->TransactMoney(value);
			active = true;

			if (displayCash)
			{
				auto cellCoords = pCell->GetCoords();
				cellCoords.Z = std::max(locationZ, cellCoords.Z);
				FlyingStrings::AddMoneyString(value, pOwner, displayToHouse, cellCoords, displayOffset);
			}

			const auto& anims = pEaterType->Anims_Tiberiums[tiberiumIdx].GetElements(animsAll);
			const int animCount = static_cast<int>(anims.size());

			if (animCount == 0)
				continue;

			AnimTypeClass* pAnimType = nullptr;

			switch (animCount)
			{
			case 1:
				pAnimType = anims[0];
				break;

			case 8:
				pAnimType = anims[facing];
				break;

			default:
				pAnimType = anims[scenarioRandom->RandomRanged(0, animCount - 1)];
				break;
			}

			if (pAnimType)
			{
				const auto pAnim = GameCreate<AnimClass>(pAnimType, pos);
				AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->Owner, nullptr, false, true);

				if (animMove)
					pAnim->SetOwnerObject(pThis);
			}
		}
	}

	if (active && transDelay)
		this->TiberiumEater_Timer.Start(pEaterType->TransDelay);
}

void TechnoExt::ExtData::UpdateShield()
{
	// Set current shield type if it is not set.
	if (!this->CurrentShieldType || this->CurrentShieldType->Strength <= 0)
	{
		auto const pTypeExt = this->TypeExtData;

		if (pTypeExt->ShieldType && pTypeExt->ShieldType->Strength > 0)
			this->CurrentShieldType = pTypeExt->ShieldType;
	}

	// Create shield class instance if it does not exist.
	if (this->CurrentShieldType && this->CurrentShieldType->Strength > 0 && !this->Shield)
	{
		this->Shield = std::make_unique<ShieldClass>(this->OwnerObject());
		this->Shield->UpdateTint();
	}

	if (const auto pShieldData = this->Shield.get())
		pShieldData->AI();
}

void TechnoExt::ExtData::UpdateOnTunnelEnter()
{
	if (!this->IsInTunnel)
	{
		if (const auto pShieldData = this->Shield.get())
			pShieldData->SetAnimationVisibility(false);

		for (const auto& pTrail : this->LaserTrails)
		{
			pTrail->Visible = false;
			pTrail->LastLocation = { };
		}

		this->IsInTunnel = true;
	}
}

void TechnoExt::ExtData::UpdateOnTunnelExit()
{
	this->IsInTunnel = false;

	if (const auto pShieldData = this->Shield.get())
		pShieldData->SetAnimationVisibility(true);
}

void TechnoExt::ExtData::ApplySpawnLimitRange()
{
	auto const pTypeExt = this->TypeExtData;

	if (pTypeExt->Spawner_LimitRange)
	{
		auto const pThis = this->OwnerObject();

		if (auto const pManager = pThis->SpawnManager)
		{
			const int weaponRange = pThis->Veterancy.IsElite() ? pTypeExt->EliteSpawnerRange : pTypeExt->SpawnerRange;

			if (pManager->Target && (pThis->DistanceFrom(pManager->Target) > weaponRange))
				pManager->ResetTarget();
		}
	}
}

void TechnoExt::ExtData::UpdateTypeData(TechnoTypeClass* pCurrentType)
{
	auto const pThis = this->OwnerObject();
	auto const pOldType = this->TypeExtData->OwnerObject();
	auto const pOldTypeExt = TechnoTypeExt::ExtMap.Find(pOldType);
	auto const pOwner = pThis->Owner;
	auto& pSlaveManager = pThis->SlaveManager;
	auto& pSpawnManager = pThis->SpawnManager;
	auto& pCaptureManager = pThis->CaptureManager;
	auto& pTemporalImUsing = pThis->TemporalImUsing;
	auto& pAirstrike = pThis->Airstrike;

	// Cache the new type data
	this->PreviousType = pOldType;
	auto const pNewTypeExt = TechnoTypeExt::ExtMap.Find(pCurrentType);
	this->TypeExtData = pNewTypeExt;

	this->UpdateSelfOwnedAttachEffects();

	// Recreate Laser Trails
	if (const size_t trailCount = this->LaserTrails.size())
	{
		std::vector<std::unique_ptr<LaserTrailClass>> addition;
		addition.reserve(trailCount);

		for (auto& pTrail : this->LaserTrails)
		{
			if (!pTrail->Intrinsic)
				addition.emplace_back(std::move(pTrail));
		}

		this->LaserTrails.clear();
		this->LaserTrails.reserve(this->TypeExtData->LaserTrailData.size() + addition.size());

		for (const auto& entry : this->TypeExtData->LaserTrailData)
			this->LaserTrails.emplace_back(std::make_unique<LaserTrailClass>(entry.GetType(), pOwner, entry.FLH, entry.IsOnTurret));

		for (auto& pTrail : addition)
			this->LaserTrails.emplace_back(std::move(pTrail));
	}
	else if (const size_t trailSize = pNewTypeExt->LaserTrailData.size())
	{
		this->LaserTrails.reserve(trailSize);

		for (const auto& entry : pNewTypeExt->LaserTrailData)
			this->LaserTrails.emplace_back(std::make_unique<LaserTrailClass>(entry.GetType(), pOwner, entry.FLH, entry.IsOnTurret));
	}

	// Reset AutoDeath Timer
	if (this->AutoDeathTimer.HasStarted())
		this->AutoDeathTimer.Stop();

	// Reset PassengerDeletion Timer
	if (this->PassengerDeletionTimer.IsTicking() && pNewTypeExt->PassengerDeletionType && pNewTypeExt->PassengerDeletionType->Rate <= 0)
		this->PassengerDeletionTimer.Stop();

	// Remove from tracked AutoDeath objects if no longer has AutoDeath
	if (pOldTypeExt->AutoDeath_Behavior.isset() && !pNewTypeExt->AutoDeath_Behavior.isset())
	{
		auto& vec = ScenarioExt::Global()->AutoDeathObjects;
		vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
	}

	// Remove from harvesters list if no longer a harvester.
	if (pOldTypeExt->Harvester_Counted && !pNewTypeExt->Harvester_Counted)
	{
		auto& vec = HouseExt::ExtMap.Find(pOwner)->OwnedCountedHarvesters;
		vec.erase(std::remove(vec.begin(), vec.end(), pThis), vec.end());
	}

	// Remove from limbo reloaders if no longer applicable
	if (pOldType->Ammo > 0 && pOldTypeExt->ReloadInTransport && !pNewTypeExt->ReloadInTransport)
	{
		auto& vec = ScenarioExt::Global()->TransportReloaders;
		vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
	}

	// Powered by ststl-s, Fly-Star
	if (pCurrentType->Enslaves && pCurrentType->SlavesNumber > 0)
	{
		// SlaveManager does not exist or they have different slaves.
		if (!pSlaveManager || pSlaveManager->SlaveType != pCurrentType->Enslaves)
		{
			if (pSlaveManager)
			{
				// Slaves are not the same, so clear out.
				pSlaveManager->Killed(nullptr);
				GameDelete(pSlaveManager);
				pSlaveManager = nullptr;
			}

			pSlaveManager = GameCreate<SlaveManagerClass>(pThis, pCurrentType->Enslaves, pCurrentType->SlavesNumber, pCurrentType->SlaveRegenRate, pCurrentType->SlaveReloadRate);
		}
		else if (pSlaveManager->SlaveCount != pCurrentType->SlavesNumber)
		{
			// Additions/deletions made when quantities are inconsistent.
			if (pSlaveManager->SlaveCount < pCurrentType->SlavesNumber)
			{
				// There are too few slaves here. More are needed.
				const int count = pCurrentType->SlavesNumber - pSlaveManager->SlaveCount;

				for (int i = 0; i < count; i++)
				{
					const auto pSlaveNode = GameCreate<SlaveManagerClass::SlaveControl>();
					pSlaveNode->Slave = nullptr;
					pSlaveNode->State = SlaveControlStatus::Dead;
					pSlaveNode->RespawnTimer.Start(pCurrentType->SlaveRegenRate);
					pSlaveManager->SlaveNodes.AddItem(pSlaveNode);
				}
			}
			else
			{
				// Remove excess slaves
				for (int i = pSlaveManager->SlaveCount - 1; i >= pCurrentType->SlavesNumber; --i)
				{
					if (const auto pSlaveNode = pSlaveManager->SlaveNodes.GetItem(i))
					{
						if (const auto pSlave = pSlaveNode->Slave)
						{
							if (pSlave->InLimbo)
							{
								// He wasn't killed, just erased.
								pSlave->RegisterDestruction(pThis);
								pSlave->UnInit();
							}
							else
							{
								// Oh, my God, he's been killed.
								pSlave->ReceiveDamage(&pSlave->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
							}
						}

						// Unlink
						pSlaveNode->Slave = nullptr;
						pSlaveNode->State = SlaveControlStatus::Dead;
						GameDelete(pSlaveNode);
					}

					// Remove it
					pSlaveManager->SlaveNodes.RemoveItem(i);
				}
			}

			pSlaveManager->SlaveCount = pCurrentType->SlavesNumber;
		}
	}
	else if (pSlaveManager)
	{
		pSlaveManager->Killed(nullptr);
		GameDelete(pSlaveManager);
		pSlaveManager = nullptr;
	}

	if (pCurrentType->Spawns && pCurrentType->SpawnsNumber > 0)
	{
		// No SpawnManager exists, or their SpawnType is inconsistent.
		if (!pSpawnManager || pCurrentType->Spawns != pSpawnManager->SpawnType)
		{
			if (pSpawnManager)
			{
				// It may be odd that AircraftType is different, I chose to reset it.
				pSpawnManager->KillNodes();
				GameDelete(pSpawnManager);
			}

			pSpawnManager = GameCreate<SpawnManagerClass>(pThis, pCurrentType->Spawns, pCurrentType->SpawnsNumber, pCurrentType->SpawnRegenRate, pCurrentType->SpawnReloadRate);
		}
		else if (pSpawnManager->SpawnCount != pCurrentType->SpawnsNumber)
		{
			// Additions/deletions made when quantities are inconsistent.
			if (pSpawnManager->SpawnCount < pCurrentType->SpawnsNumber)
			{
				const int count = pCurrentType->SpawnsNumber - pSpawnManager->SpawnCount;

				// Add the missing Spawns, but don't intend for them to be born right away.
				for (int i = 0; i < count; i++)
				{
					const auto pSpawnNode = GameCreate<SpawnControl>();
					pSpawnNode->Unit = nullptr;
					pSpawnNode->Status = SpawnNodeStatus::Dead;
					pSpawnNode->SpawnTimer.Start(pCurrentType->SpawnRegenRate);
					pSpawnNode->IsSpawnMissile = false;
					pSpawnManager->SpawnedNodes.AddItem(pSpawnNode);
				}
			}
			else
			{
				// Remove excess spawns
				for (int i = pSpawnManager->SpawnCount - 1; i >= pCurrentType->SpawnsNumber; --i)
				{
					if (const auto pSpawnNode = pSpawnManager->SpawnedNodes.GetItem(i))
					{
						auto& pStatus = pSpawnNode->Status;

						// Spawns that don't die get killed.
						if (const auto pAircraft = pSpawnNode->Unit)
						{
							pAircraft->SpawnOwner = nullptr;

							if (pAircraft->InLimbo
								|| pStatus == SpawnNodeStatus::Idle
								|| pStatus == SpawnNodeStatus::Reloading
								|| pStatus == SpawnNodeStatus::TakeOff)
							{
								if (pStatus == SpawnNodeStatus::TakeOff)
									Kamikaze::Instance.Remove(pAircraft);

								pAircraft->UnInit();
							}
							else if (pSpawnNode->IsSpawnMissile)
							{
								pAircraft->ReceiveDamage(&pAircraft->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
							}
							else
							{
								pAircraft->Crash(nullptr);
							}
						}

						// Unlink
						pSpawnNode->Unit = nullptr;
						pStatus = SpawnNodeStatus::Dead;
						GameDelete(pSpawnNode);
					}

					// Remove it
					pSpawnManager->SpawnedNodes.RemoveItem(i);
				}
			}

			pSpawnManager->SpawnCount = pCurrentType->SpawnsNumber;
		}
	}
	else if (pSpawnManager)
	{
		// Reset the target.
		pSpawnManager->ResetTarget();

		// pSpawnManager->KillNodes() kills all Spawns, but it is not necessary to kill the parts that are not performing tasks.
		for (const auto pSpawnNode : pSpawnManager->SpawnedNodes)
		{
			const auto pAircraft = pSpawnNode->Unit;
			auto& pStatus = pSpawnNode->Status;

			// A dead or idle Spawn is not killed.
			if (!pAircraft
				|| pStatus == SpawnNodeStatus::Dead
				|| pStatus == SpawnNodeStatus::Idle
				|| pStatus == SpawnNodeStatus::Reloading)
			{
				continue;
			}

			pAircraft->SpawnOwner = nullptr;

			if (pStatus == SpawnNodeStatus::TakeOff)
			{
				Kamikaze::Instance.Remove(pAircraft);
				pAircraft->UnInit();
			}
			else if (pSpawnNode->IsSpawnMissile)
			{
				pAircraft->ReceiveDamage(&pAircraft->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
			}
			else
			{
				pAircraft->Crash(nullptr);
			}

			pSpawnNode->Unit = nullptr;
			pStatus = SpawnNodeStatus::Dead;
			pSpawnNode->IsSpawnMissile = false;
			pSpawnNode->SpawnTimer.Start(pSpawnManager->RegenRate);
		}
	}

	// Prepare the variables.
	int maxCapture = 0;
	bool infiniteCapture = false;
	bool hasTemporal = false;
	bool hasAirstrike = false;
	bool hasLocomotor = false;
	bool hasParasite = false;

	auto checkWeapon = [&maxCapture, &infiniteCapture, &hasTemporal,
		&hasAirstrike, &hasLocomotor, &hasParasite](WeaponTypeClass* pWeaponType)
	{
		if (!pWeaponType)
			return;

		const auto pWH = pWeaponType->Warhead;

		if (pWH->MindControl)
		{
			if (pWeaponType->Damage > maxCapture)
				maxCapture = pWeaponType->Damage;

			if (pWeaponType->InfiniteMindControl)
				infiniteCapture = true;
		}

		if (pWH->Temporal)
			hasTemporal = true;

		if (pWH->Airstrike)
			hasAirstrike = true;

		if (pWH->IsLocomotor)
			hasLocomotor = true;

		if (pWH->Parasite)
			hasParasite = true;
	};

	for (int i = 0; i < TechnoTypeClass::MaxWeapons; i++)
	{
		checkWeapon(pThis->GetWeapon(i)->WeaponType);
	}

	if (maxCapture > 0)
	{
		if (!pCaptureManager)
		{
			// Rebuild a CaptureManager
			pCaptureManager = GameCreate<CaptureManagerClass>(pThis, maxCapture, infiniteCapture);
		}
		else if (pOldTypeExt->Convert_ResetMindControl)
		{
			if (!infiniteCapture && pCaptureManager->ControlNodes.Count > maxCapture)
			{
				// Remove excess nodes.
				for (int i = pCaptureManager->ControlNodes.Count - 1; i >= maxCapture; --i)
				{
					auto const pControlNode = pCaptureManager->ControlNodes.GetItem(i);
					pCaptureManager->FreeUnit(pControlNode->Unit);
				}
			}

			pCaptureManager->MaxControlNodes = maxCapture;
			pCaptureManager->InfiniteMindControl = infiniteCapture;
		}
	}
	else if (pCaptureManager && pOldTypeExt->Convert_ResetMindControl)
	{
		// Remove CaptureManager completely
		pCaptureManager->FreeAll();
		GameDelete(pCaptureManager);
		pCaptureManager = nullptr;
	}

	if (hasTemporal)
	{
		if (!pTemporalImUsing)
		{
			// Rebuild a TemporalClass
			pTemporalImUsing = GameCreate<TemporalClass>(pThis);
		}
	}
	else if (pTemporalImUsing)
	{
		if (pTemporalImUsing->Target)
		{
			// Free this afflicted man.
			pTemporalImUsing->LetGo();
		}

		// Delete it
		GameDelete(pTemporalImUsing);
		pTemporalImUsing = nullptr;
	}

	if (hasAirstrike && pCurrentType->AirstrikeTeam > 0)
	{
		if (!pAirstrike)
		{
			// Rebuild a AirstrikeClass
			pAirstrike = GameCreate<AirstrikeClass>(pThis);
		}
		else
		{
			// Modify the parameters of AirstrikeClass.
			pAirstrike->AirstrikeTeam = pCurrentType->AirstrikeTeam;
			pAirstrike->EliteAirstrikeTeam = pCurrentType->EliteAirstrikeTeam;
			pAirstrike->AirstrikeTeamType = pCurrentType->AirstrikeTeamType;
			pAirstrike->EliteAirstrikeTeamType = pCurrentType->EliteAirstrikeTeamType;
			pAirstrike->AirstrikeRechargeTime = pCurrentType->AirstrikeRechargeTime;
			pAirstrike->EliteAirstrikeRechargeTime = pCurrentType->EliteAirstrikeRechargeTime;
		}
	}
	else if (pAirstrike)
	{
		pAirstrike->InvalidatePointer(pThis);
		GameDelete(pAirstrike);
		pAirstrike = nullptr;
	}

	if (!hasLocomotor && pThis->LocomotorTarget)
	{
		pThis->ReleaseLocomotor(pThis->Target == pThis->LocomotorTarget);
		pThis->LocomotorTarget->LocomotorSource = nullptr;
		pThis->LocomotorTarget = nullptr;
	}

	// FireAngle
	pThis->BarrelFacing.SetCurrent(DirStruct(0x4000 - (pCurrentType->FireAngle << 8)));

	// Reset recoil data
	{
		auto& turretRecoil = pThis->TurretRecoil.Turret;
		const auto& turretAnimData = pCurrentType->TurretAnimData;
		turretRecoil.Travel = turretAnimData.Travel;
		turretRecoil.CompressFrames = turretAnimData.CompressFrames;
		turretRecoil.RecoverFrames = turretAnimData.RecoverFrames;
		turretRecoil.HoldFrames = turretAnimData.HoldFrames;
		auto& barrelRecoil = pThis->BarrelRecoil.Turret;
		const auto& barrelAnimData = pCurrentType->BarrelAnimData;
		barrelRecoil.Travel = barrelAnimData.Travel;
		barrelRecoil.CompressFrames = barrelAnimData.CompressFrames;
		barrelRecoil.RecoverFrames = barrelAnimData.RecoverFrames;
		barrelRecoil.HoldFrames = barrelAnimData.HoldFrames;
	}

	// Only FootClass* can use this.
	if (const auto pFoot = abstract_cast<FootClass*, true>(pThis))
	{
		auto& pParasiteImUsing = pFoot->ParasiteImUsing;

		if (hasParasite)
		{
			if (!pParasiteImUsing)
			{
				// Rebuild a ParasiteClass
				pParasiteImUsing = GameCreate<ParasiteClass>(pFoot);
			}
		}
		else if (pParasiteImUsing)
		{
			if (pParasiteImUsing->Victim)
			{
				// Release of victims.
				pParasiteImUsing->ExitUnit();
			}

			// Delete it
			GameDelete(pParasiteImUsing);
			pParasiteImUsing = nullptr;
		}
	}
}

void TechnoExt::ExtData::UpdateTypeData_Foot()
{
	auto const pThis = static_cast<FootClass*>(this->OwnerObject());
	auto const pOldType = this->PreviousType;
	auto const pCurrentType = this->TypeExtData->OwnerObject();
	auto const abs = pThis->WhatAmI();
	//auto const pOldTypeExt = TechnoTypeExt::ExtMap.Find(pOldType);

	// Update movement sound if still moving while type changed.
	if (pThis->IsMoveSoundPlaying && pThis->Locomotor->Is_Moving())
	{
		if (pCurrentType->MoveSound != pOldType->MoveSound)
		{
			// End the old sound.
			pThis->MoveSoundAudioController.End();

			if (auto const count = pCurrentType->MoveSound.Count)
			{
				// Play a new sound.
				const int soundIndex = pCurrentType->MoveSound[Randomizer::Global.Random() % count];
				VocClass::PlayAt(soundIndex, pThis->Location, &pThis->MoveSoundAudioController);
				pThis->IsMoveSoundPlaying = true;
			}
			else
			{
				pThis->IsMoveSoundPlaying = false;
			}

			pThis->MoveSoundDelay = 0;
		}
	}

	if (abs == AbstractType::Infantry)
	{
		auto const pInf = static_cast<InfantryClass*>(pThis);

		// It's still not recommended to have such idea, please avoid using this
		if (static_cast<InfantryTypeClass*>(pOldType)->Deployer && !static_cast<InfantryTypeClass*>(pCurrentType)->Deployer)
		{
			switch (pInf->SequenceAnim)
			{
			case Sequence::Deploy:
			case Sequence::Deployed:
			case Sequence::DeployedIdle:
				pInf->PlayAnim(Sequence::Ready, true);
				break;
			case Sequence::DeployedFire:
				pInf->PlayAnim(Sequence::FireUp, true);
				break;
			default:
				break;
			}
		}
	}

	if (pOldType->Locomotor == LocomotionClass::CLSIDs::Teleport && pCurrentType->Locomotor != LocomotionClass::CLSIDs::Teleport && pThis->WarpingOut)
		this->HasRemainingWarpInDelay = true;

	// Update open topped state of potential passengers if transport's OpenTopped value changes.
	// OpenTopped does not work properly with buildings to begin with which is why this is here rather than in the Techno update one.
	if (pThis->Passengers.NumPassengers > 0)
	{
		const bool toOpenTopped = pCurrentType->OpenTopped;
		FootClass* pFirstPassenger = pThis->Passengers.GetFirstPassenger();

		while (true)
		{
			if (toOpenTopped)
			{
				// Add passengers to the logic layer.
				pThis->EnteredOpenTopped(pFirstPassenger);
			}
			else
			{
				// Lose target & destination
				pFirstPassenger->SetTarget(nullptr);
				pFirstPassenger->SetCurrentWeaponStage(0);
				pFirstPassenger->AbortMotion();
				pThis->ExitedOpenTopped(pFirstPassenger);

				// OpenTopped adds passengers to logic layer when enabled. Under normal conditions this does not need to be removed since
				// OpenTopped state does not change while passengers are still in transport but in case of type conversion that can happen.
				LogicClass::Instance.RemoveObject(pFirstPassenger);
			}

			pFirstPassenger->Transporter = pThis;

			if (const auto pNextPassenger = abstract_cast<FootClass*>(pFirstPassenger->NextObject))
				pFirstPassenger = pNextPassenger;
			else
				break;
		}

		if (pCurrentType->Gunner)
			pThis->ReceiveGunner(pFirstPassenger);
	}
	else if (pCurrentType->Gunner)
	{
		pThis->RemoveGunner(nullptr);
	}

	if (!pCurrentType->CanDisguise || (!pThis->Disguise && pCurrentType->PermaDisguise))
	{
		// When it can't disguise or has lost its disguise, update its disguise.
		pThis->ClearDisguise();
	}

	if (abs != AbstractType::Aircraft)
	{
		auto const pLocomotorType = pCurrentType->Locomotor;

		// The Hover movement pattern allows for self-landing.
		if (pLocomotorType != LocomotionClass::CLSIDs::Fly && pLocomotorType != LocomotionClass::CLSIDs::Hover)
		{
			const bool isinAir = pThis->IsInAir() && !pThis->LocomotorSource;

			if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
			{
				const int turnrate = pCurrentType->JumpjetTurnRate >= 127 ? 127 : pCurrentType->JumpjetTurnRate;
				pJJLoco->Speed = pCurrentType->JumpjetSpeed;
				pJJLoco->Accel = pCurrentType->JumpjetAccel;
				pJJLoco->Crash = pCurrentType->JumpjetCrash;
				pJJLoco->Deviation = pCurrentType->JumpjetDeviation;
				pJJLoco->NoWobbles = pCurrentType->JumpjetNoWobbles;
				pJJLoco->Wobbles = pCurrentType->JumpjetWobbles;
				pJJLoco->TurnRate = turnrate;
				pJJLoco->CurrentHeight = pCurrentType->JumpjetHeight;
				pJJLoco->Height = pCurrentType->JumpjetHeight;
				pJJLoco->LocomotionFacing.SetROT(turnrate);

				if (isinAir)
				{
					const bool inMove = pJJLoco->Is_Really_Moving_Now();

					if (pCurrentType->BalloonHover)
					{
						// Makes the jumpjet think it is hovering without actually moving.
						pJJLoco->State = JumpjetLocomotionClass::State::Hovering;
						pJJLoco->IsMoving = true;

						if (!inMove)
							pJJLoco->DestinationCoords = pThis->Location;
					}
					else if (!inMove)
					{
						pJJLoco->Move_To(pThis->Location);
					}
				}
			}
			else if (isinAir)
			{
				// Let it go into free fall.
				pThis->FallRate = 0;
				pThis->IsFallingDown = true;

				const auto pCell = MapClass::Instance.TryGetCellAt(pThis->Location);

				if (pCell && !pCell->IsClearToMove(pCurrentType->SpeedType, true, true,
					-1, pCurrentType->MovementZone, pCell->GetLevel(), pCell->ContainsBridge()))
				{
					// If it's landing position cannot be moved, then it is granted a crash death.
					pThis->IsABomb = true;
				}
				else
				{
					// If it's gonna land on the bridge, then it needs this.
					pThis->OnBridge = pCell ? pCell->ContainsBridge() : false;
				}

				if (abs == AbstractType::Infantry)
				{
					// Infantry changed to parachute status (not required).
					static_cast<InfantryClass*>(pThis)->PlayAnim(Sequence::Paradrop, true, false);
				}
			}
		}

		if (abs == AbstractType::Unit)
		{
			// Yes, synchronize its turret facing or it will turn strangely.
			if (pOldType->Turret != pCurrentType->Turret)
			{
				const auto primaryFacing = pThis->PrimaryFacing.Current();
				auto& secondaryFacing = pThis->SecondaryFacing;

				secondaryFacing.SetCurrent(primaryFacing);
				secondaryFacing.SetDesired(primaryFacing);
			}
		}
	}

	this->PreviousType = nullptr;
}

void TechnoExt::ExtData::UpdateLaserTrails()
{
	if (this->LaserTrails.size() <= 0)
		return;

	auto const pThis = this->OwnerObject();
	auto const pOwner = pThis->Owner;
	auto const cloakState = pThis->CloakState;

	// LaserTrails update routine is in TechnoClass::AI hook because LaserDrawClass-es are updated in LogicClass::AI
	for (const auto& pTrail : this->LaserTrails)
	{
		auto const pType = pTrail->Type;

		if (pType->DroppodOnly && (pThis->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
		{
			auto const pFoot = static_cast<FootClass*>(pThis);

			// @Kerbiter if you want to limit it to certain locos you do it here
			// // with vtable check you can avoid the tedious process of Query IPersit/IUnknown Interface, GetClassID, compare with loco GUID, which is omnipresent in vanilla code
			if (VTable::Get(pFoot->Locomotor.GetInterfacePtr()) != 0x7E8278)
				continue;
		}

		pTrail->Cloaked = false;

		if (cloakState == CloakState::Cloaked)
		{
			if (pType->CloakVisible && pType->CloakVisible_DetectedOnly && !HouseClass::IsCurrentPlayerObserver() && !pOwner->IsAlliedWith(HouseClass::CurrentPlayer))
				pTrail->Cloaked = !pThis->GetCell()->Sensors_InclHouse(HouseClass::CurrentPlayer->ArrayIndex);
			else if (!pType->CloakVisible)
				pTrail->Cloaked = true;
		}

		if (!this->IsInTunnel)
			pTrail->Visible = true;

		auto const trailLoc = TechnoExt::GetFLHAbsoluteCoords(pThis, pTrail->FLH, pTrail->IsOnTurret);

		if (cloakState == CloakState::Uncloaking && !pType->CloakVisible)
			pTrail->LastLocation = trailLoc;
		else
			pTrail->Update(trailLoc);
	}
}

void TechnoExt::ExtData::UpdateMindControlAnim()
{
	auto const pThis = this->OwnerObject();
	if (pThis->IsMindControlled())
	{
		if (pThis->MindControlRingAnim && !this->MindControlRingAnimType)
		{
			this->MindControlRingAnimType = pThis->MindControlRingAnim->Type;
		}
		else if (!pThis->MindControlRingAnim && this->MindControlRingAnimType
			&& pThis->CloakState == CloakState::Uncloaked && !pThis->InLimbo && pThis->IsAlive)
		{
			auto coords = pThis->GetCoords();
			int offset = 0;

			if (const auto pBuilding = specific_cast<BuildingClass*, true>(pThis))
				offset = Unsorted::LevelHeight * pBuilding->Type->Height;
			else
				offset = pThis->GetTechnoType()->MindControlRingOffset;

			coords.Z += offset;
			pThis->MindControlRingAnim = GameCreate<AnimClass>(this->MindControlRingAnimType, coords, 0, 1);
			pThis->MindControlRingAnim->SetOwnerObject(pThis);

			if (pThis->WhatAmI() == AbstractType::Building)
				pThis->MindControlRingAnim->ZAdjust = -1024;
		}
	}
	else if (this->MindControlRingAnimType)
	{
		this->MindControlRingAnimType = nullptr;
	}
}

void TechnoExt::ExtData::UpdateRecountBurst()
{
	const auto pThis = this->OwnerObject();

	if (pThis->CurrentBurstIndex && !pThis->Target && this->TypeExtData->RecountBurst.Get(RulesExt::Global()->RecountBurst))
	{
		const auto pWeapon = this->LastWeaponType;

		if (pWeapon && pWeapon->Burst && pThis->LastFireBulletFrame + std::max(pWeapon->ROF, 30) <= Unsorted::CurrentFrame)
		{
			const auto ratio = static_cast<double>(pThis->CurrentBurstIndex) / pWeapon->Burst;
			const auto rof = static_cast<int>(ratio * pWeapon->ROF * this->AE.ROFMultiplier) - std::max(pWeapon->ROF, 30);

			if (rof > 0)
			{
				pThis->ChargeTurretDelay = rof;
				pThis->RearmTimer.Start(rof);
			}

			pThis->CurrentBurstIndex = 0;
		}
	}
}

void TechnoExt::ExtData::UpdateGattlingRateDownReset()
{
	const auto pTypeExt = this->TypeExtData;

	if (pTypeExt->OwnerObject()->IsGattling)
	{
		const auto pThis = this->OwnerObject();

		if (pTypeExt->RateDown_Reset && (!pThis->Target || this->LastTargetID != pThis->Target->UniqueID))
		{
			int oldStage = pThis->CurrentGattlingStage;
			this->LastTargetID = pThis->Target ? pThis->Target->UniqueID : 0xFFFFFFFF;
			pThis->GattlingValue = 0;
			pThis->CurrentGattlingStage = 0;
			this->AccumulatedGattlingValue = 0;
			this->ShouldUpdateGattlingValue = false;

			if (oldStage != 0)
			{
				pThis->GattlingRateDown(0);
			}
		}
	}
}

void TechnoExt::ApplyGainedSelfHeal(TechnoClass* pThis)
{
	if (!RulesExt::Global()->GainSelfHealAllowMultiplayPassive && pThis->Owner->Type->MultiplayPassive)
		return;

	auto const pType = pThis->GetTechnoType();
	const int healthDeficit = pType->Strength - pThis->Health;

	if (pThis->Health && healthDeficit > 0)
	{
		auto defaultSelfHealType = SelfHealGainType::NoHeal;
		auto const whatAmI = pThis->WhatAmI();

		if (whatAmI == AbstractType::Infantry)
			defaultSelfHealType = SelfHealGainType::Infantry;
		else if (whatAmI == AbstractType::Unit)
			defaultSelfHealType = (pType->Organic ? SelfHealGainType::Infantry : SelfHealGainType::Units);

		auto const selfHealType = TechnoTypeExt::ExtMap.Find(pType)->SelfHealGainType.Get(defaultSelfHealType);

		if (selfHealType == SelfHealGainType::NoHeal)
			return;

		if ((selfHealType == SelfHealGainType::Infantry)
			? (Unsorted::CurrentFrame % RulesClass::Instance->SelfHealInfantryFrames)
			: (Unsorted::CurrentFrame % RulesClass::Instance->SelfHealUnitFrames))
		{
			return;
		}

		int amount = 0;

		auto countSelfHealing = [pThis](const bool infantryHeal)
			{
				auto const pOwner = pThis->Owner;
				const bool hasCap = infantryHeal ? RulesExt::Global()->InfantryGainSelfHealCap.isset() : RulesExt::Global()->UnitsGainSelfHealCap.isset();
				const int cap = std::max(infantryHeal ? RulesExt::Global()->InfantryGainSelfHealCap.Get() : RulesExt::Global()->UnitsGainSelfHealCap.Get(), 1);

				auto healCount = [infantryHeal](HouseClass* pHouse)
					{
						return (infantryHeal ? pHouse->InfantrySelfHeal : pHouse->UnitsSelfHeal);
					};
				int count = healCount(pOwner);

				if (hasCap && count >= cap)
					return cap;

				const bool isCampaign = SessionClass::IsCampaign();
				const bool fromPlayer = RulesExt::Global()->GainSelfHealFromPlayerControl && isCampaign && (pOwner->IsHumanPlayer || pOwner->IsInPlayerControl);
				const bool fromAllies = RulesExt::Global()->GainSelfHealFromAllies;

				if (fromPlayer || fromAllies)
				{
					auto checkHouse = [fromPlayer, fromAllies, isCampaign, pOwner](HouseClass* pHouse)
						{
							if (pHouse == pOwner)
								return false;

							return (fromPlayer && (pHouse->IsHumanPlayer || pHouse->IsInPlayerControl)) // pHouse->IsControlledByCurrentPlayer()
								|| (fromAllies && (!isCampaign || (!pHouse->IsHumanPlayer && !pHouse->IsInPlayerControl)) && pHouse->IsAlliedWith(pOwner));
						};

					for (auto const pHouse : HouseClass::Array)
					{
						if (checkHouse(pHouse))
						{
							count += healCount(pHouse);

							if (hasCap && count >= cap)
								return cap;
						}
					}
				}

				return count;
			};

		if (selfHealType == SelfHealGainType::Infantry)
			amount = RulesClass::Instance->SelfHealInfantryAmount * countSelfHealing(true);
		else
			amount = RulesClass::Instance->SelfHealUnitAmount * countSelfHealing(false);

		if (amount)
		{
			if (amount >= healthDeficit)
				amount = healthDeficit;

			const bool wasDamaged = pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;

			pThis->Health += amount;

			if (wasDamaged && (pThis->GetHealthPercentage() > RulesClass::Instance->ConditionYellow
				|| pThis->GetHeight() < -10))
			{
				if (auto const pBuilding = abstract_cast<BuildingClass*, true>(pThis))
				{
					pBuilding->Mark(MarkType::Change);
					pBuilding->ToggleDamagedAnims(false);
				}

				auto const dmgParticle = pThis->DamageParticleSystem;

				if (dmgParticle)
					dmgParticle->UnInit();
			}
		}
	}

	return;
}

void TechnoExt::ExtData::ApplyMindControlRangeLimit()
{
	auto const pThis = this->OwnerObject();

	if (auto const pCapturer = pThis->MindControlledBy)
	{
		auto const pCapturerExt = TechnoExt::ExtMap.Find(pCapturer)->TypeExtData;

		if (pCapturerExt->MindControlRangeLimit.Get() > 0
			&& pCapturer->DistanceFrom(pThis) > pCapturerExt->MindControlRangeLimit.Get())
		{
			pCapturer->CaptureManager->FreeUnit(pThis);
		}
	}
}

void TechnoExt::KillSelf(TechnoClass* pThis, AutoDeathBehavior deathOption, const std::vector<AnimTypeClass*>& pVanishAnimation, bool isInLimbo)
{
	if (isInLimbo)
	{
		// Remove parasite units first before deleting them.
		if (auto const pFoot = abstract_cast<FootClass*, true>(pThis))
		{
			if (pFoot->ParasiteImUsing && pFoot->ParasiteImUsing->Victim)
				pFoot->ParasiteImUsing->ExitUnit();
		}

		// Remove limbo buildings' tracking here because their are not truely InLimbo
		if (auto const pBuilding = abstract_cast<BuildingClass*, true>(pThis))
		{
			auto const pBldType = pBuilding->Type;

			if (!pBuilding->InLimbo && !pBldType->Insignificant && !pBldType->DontScore)
				HouseExt::ExtMap.Find(pBuilding->Owner)->RemoveFromLimboTracking(pBldType);
		}

		auto const pTransport = pThis->Transporter;

		// Handle extra power
		if (pTransport && pThis->Absorbed)
			pTransport->Owner->RecheckPower = true;

		pThis->RegisterKill(pThis->Owner);
		pThis->UnInit();

		// Handle gunner change.
		if (auto const pTransportFoot = abstract_cast<FootClass*>(pTransport))
		{
			if (pTransportFoot->GetTechnoType()->Gunner)
			{
				pTransportFoot->RemoveGunner(nullptr);

				if (auto pGunner = pTransportFoot->Passengers.GetFirstPassenger())
				{
					for (auto pNext = abstract_cast<FootClass*>(pGunner->NextObject); pNext; pNext = abstract_cast<FootClass*>(pNext->NextObject))
						pGunner = pNext;

					pTransportFoot->ReceiveGunner(pGunner);
				}
			}
		}

		return;
	}

	switch (deathOption)
	{

	case AutoDeathBehavior::Vanish:
	{
		AnimExt::CreateRandomAnim(pVanishAnimation, pThis->GetCoords(), pThis, nullptr, true);

		pThis->KillPassengers(pThis);
		pThis->Stun();
		pThis->Limbo();
		pThis->RegisterKill(pThis->Owner);
		pThis->UnInit();

		return;
	}

	case AutoDeathBehavior::Sell:
	{
		if (auto const pBld = abstract_cast<BuildingClass*, true>(pThis))
		{
			if (pBld->HasBuildUp)
			{
				// Sorry FirestormWall
				if (pBld->GetCurrentMission() != Mission::Selling)
				{
					pBld->QueueMission(Mission::Selling, false);
					pBld->NextMission();
				}
				return;
			}
		}
		if (Phobos::Config::DevelopmentCommands)
			Debug::Log("[Developer warning] AutoDeath: [%s] can't be sold, killing it instead\n", pThis->get_ID());
	}

	default: //must be AutoDeathBehavior::Kill
		if (AresFunctions::SpawnSurvivors)
		{
			switch (pThis->WhatAmI())
			{
			case AbstractType::Unit:
			case AbstractType::Aircraft:
				AresFunctions::SpawnSurvivors(static_cast<FootClass*>(pThis), nullptr, false, false);
			default:;
			}
		}
		pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
		return;
	}
}

void TechnoExt::UpdateSharedAmmo(TechnoClass* pThis)
{
	const auto pType = pThis->GetTechnoType();

	if (pType->OpenTopped)
	{
		const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

		if (pExt->Ammo_Shared && pType->Ammo > 0)
		{
			for (auto pPassenger = pThis->Passengers.GetFirstPassenger(); pPassenger; pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject))
			{
				const auto pPassengerType = pPassenger->GetTechnoType();
				const auto pPassengerExt = TechnoTypeExt::ExtMap.Find(pPassengerType);

				if (pPassengerExt->Ammo_Shared)
				{
					if (pExt->Ammo_Shared_Group < 0 || pExt->Ammo_Shared_Group == pPassengerExt->Ammo_Shared_Group)
					{
						if (pThis->Ammo > 0 && (pPassenger->Ammo < pPassengerType->Ammo))
						{
							pThis->Ammo--;
							pPassenger->Ammo++;
						}
					}
				}
			}
		}
	}
}

void TechnoExt::ExtData::UpdateTemporal()
{
	if (const auto pShieldData = this->Shield.get())
	{
		if (pShieldData->IsAvailable())
			pShieldData->AI_Temporal();
	}

	for (auto const& ae : this->AttachedEffects)
		ae->AI_Temporal();

	this->UpdateRearmInTemporal();
}

void TechnoExt::ExtData::UpdateRearmInEMPState()
{
	const auto pThis = this->OwnerObject();

	if (!pThis->IsUnderEMP() && !pThis->Deactivated)
		return;

	const auto pTypeExt = this->TypeExtData;

	if (pThis->RearmTimer.InProgress() && pTypeExt->NoRearm_UnderEMP.Get(RulesExt::Global()->NoRearm_UnderEMP))
		pThis->RearmTimer.StartTime++;

	if (pThis->ReloadTimer.InProgress() && pTypeExt->NoReload_UnderEMP.Get(RulesExt::Global()->NoReload_UnderEMP))
		pThis->ReloadTimer.StartTime++;
}

void TechnoExt::ExtData::UpdateRearmInTemporal()
{
	const auto pThis = this->OwnerObject();
	const auto pTypeExt = this->TypeExtData;

	if (pThis->RearmTimer.InProgress() && pTypeExt->NoRearm_Temporal.Get(RulesExt::Global()->NoRearm_Temporal))
		pThis->RearmTimer.StartTime++;

	if (pThis->ReloadTimer.InProgress() && pTypeExt->NoReload_Temporal.Get(RulesExt::Global()->NoReload_Temporal))
		pThis->ReloadTimer.StartTime++;
}

// Resets target if KeepTargetOnMove unit moves beyond weapon range.
void TechnoExt::ExtData::UpdateKeepTargetOnMove()
{
	auto const pThis = this->OwnerObject();

	if (!this->KeepTargetOnMove)
		return;

	if (!pThis->Target)
	{
		this->KeepTargetOnMove = false;
		return;
	}

	const auto pTypeExt = this->TypeExtData;

	if (!pTypeExt->KeepTargetOnMove)
	{
		pThis->SetTarget(nullptr);
		this->KeepTargetOnMove = false;
		return;
	}

	if (pThis->CurrentMission == Mission::Guard)
	{
		if (!pTypeExt->KeepTargetOnMove_NoMorePursuit)
		{
			pThis->QueueMission(Mission::Attack, false);
			this->KeepTargetOnMove = false;
			return;
		}
	}
	else if (pThis->CurrentMission != Mission::Move)
	{
		return;
	}

	const int weaponIndex = pThis->SelectWeapon(pThis->Target);

	if (auto const pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType)
	{
		const int extraDistance = static_cast<int>(pTypeExt->KeepTargetOnMove_ExtraDistance.Get());
		const int range = pWeapon->Range;
		pWeapon->Range += extraDistance; // Temporarily adjust weapon range based on the extra distance.

		if (!pThis->IsCloseEnough(pThis->Target, weaponIndex))
		{
			pThis->SetTarget(nullptr);
			this->KeepTargetOnMove = false;
		}

		pWeapon->Range = range;
	}
}

void TechnoExt::ExtData::UpdateWarpInDelay()
{
	if (this->HasRemainingWarpInDelay)
	{
		if (this->LastWarpInDelay)
		{
			this->LastWarpInDelay--;
		}
		else
		{
			this->HasRemainingWarpInDelay = false;
			this->IsBeingChronoSphered = false;
			this->OwnerObject()->WarpingOut = false;
		}
	}
}

// Updates state of all AttachEffects on techno.
void TechnoExt::ExtData::UpdateAttachEffects()
{
	if (!this->AttachedEffects.size())
		return;

	auto const pThis = this->OwnerObject();
	const bool inTunnel = this->IsInTunnel || this->IsBurrowed;
	bool markForRedraw = false;
	bool altered = false;
	std::vector<std::unique_ptr<AttachEffectClass>>::iterator it;
	std::vector<std::pair<WeaponTypeClass*, TechnoClass*>> expireWeapons;

	for (it = this->AttachedEffects.begin(); it != this->AttachedEffects.end(); )
	{
		auto const attachEffect = it->get();

		if (!inTunnel)
			attachEffect->SetAnimationTunnelState(true);

		attachEffect->AI();

		if (attachEffect->NeedsRecalculateStat)
		{
			altered = true;
			attachEffect->NeedsRecalculateStat = false;
		}

		const bool hasExpired = attachEffect->HasExpired();
		const bool shouldDiscard = attachEffect->IsActiveIgnorePowered() && attachEffect->ShouldBeDiscardedNow();

		if (hasExpired || shouldDiscard)
		{
			auto const pType = attachEffect->GetType();
			attachEffect->ShouldBeDiscarded = false;

			if (pType->HasTint())
				markForRedraw = true;

			if (pType->Cumulative && pType->CumulativeAnimations.size() > 0)
				this->UpdateCumulativeAttachEffects(attachEffect->GetType(), attachEffect);

			if (pType->ExpireWeapon && ((hasExpired && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Expire) != ExpireWeaponCondition::None)
				|| (shouldDiscard && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Discard) != ExpireWeaponCondition::None)))
			{
				if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || this->GetAttachedEffectCumulativeCount(pType) < 1)
				{
					if (pType->ExpireWeapon_UseInvokerAsOwner)
					{
						if (auto const pInvoker = attachEffect->GetInvoker())
							expireWeapons.push_back(std::make_pair(pType->ExpireWeapon, pInvoker));
					}
					else
					{
						expireWeapons.push_back(std::make_pair(pType->ExpireWeapon, pThis));
					}
				}
			}

			if (shouldDiscard && attachEffect->ResetIfRecreatable())
			{
				++it;
				continue;
			}

			it = this->AttachedEffects.erase(it);
			altered = true;
		}
		else
		{
			++it;
		}
	}

	if (altered)
		this->RecalculateStatMultipliers();

	if (markForRedraw)
		pThis->MarkForRedraw();

	auto const coords = pThis->GetCoords();

	for (auto const& pair : expireWeapons)
	{
		auto const pInvoker = pair.second;
		WeaponTypeExt::DetonateAt(pair.first, coords, pInvoker, pInvoker->Owner, pThis);
	}
}

// Updates self-owned (defined on TechnoType) AttachEffects, called on type conversion.
void TechnoExt::ExtData::UpdateSelfOwnedAttachEffects()
{
	auto const pThis = this->OwnerObject();
	auto const pTypeExt = this->TypeExtData;
	std::vector<std::unique_ptr<AttachEffectClass>>::iterator it;
	std::vector<std::pair<WeaponTypeClass*, TechnoClass*>> expireWeapons;
	bool markForRedraw = false;
	bool altered = false;

	// Delete ones on old type and not on current.
	for (it = this->AttachedEffects.begin(); it != this->AttachedEffects.end(); )
	{
		auto const attachEffect = it->get();
		auto const pType = attachEffect->GetType();
		bool selfOwned = attachEffect->IsSelfOwned();
		bool remove = selfOwned && !pTypeExt->AttachEffects.AttachTypes.Contains(pType);

		if (remove)
		{
			if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Expire) != ExpireWeaponCondition::None)
			{
				if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || this->GetAttachedEffectCumulativeCount(pType) < 1)
				{
					if (pType->ExpireWeapon_UseInvokerAsOwner)
					{
						if (auto const pInvoker = attachEffect->GetInvoker())
							expireWeapons.push_back(std::make_pair(pType->ExpireWeapon, pInvoker));
					}
					else
					{
						expireWeapons.push_back(std::make_pair(pType->ExpireWeapon, pThis));
					}
				}
			}

			markForRedraw |= pType->HasTint();
			it = this->AttachedEffects.erase(it);
			altered = true;
		}
		else
		{
			it++;
		}
	}

	auto const coords = pThis->GetCoords();

	for (auto const& pair : expireWeapons)
	{
		auto const pInvoker = pair.second;
		WeaponTypeExt::DetonateAt(pair.first, coords, pInvoker, pInvoker->Owner, pThis);
	}

	// Add new ones.
	const int count = AttachEffectClass::Attach(pThis, pThis->Owner, pThis, pThis, pTypeExt->AttachEffects);

	if (altered && !count)
		this->RecalculateStatMultipliers();

	if (markForRedraw)
		pThis->MarkForRedraw();
}

// Updates CumulativeAnimations AE's on techno.
void TechnoExt::ExtData::UpdateCumulativeAttachEffects(AttachEffectTypeClass* pAttachEffectType, AttachEffectClass* pRemoved)
{
	AttachEffectClass* pAELargestDuration = nullptr;
	AttachEffectClass* pAEWithAnim = nullptr;
	int duration = 0;

	for (auto const& attachEffect : this->AttachedEffects)
	{
		if (attachEffect->GetType() != pAttachEffectType)
			continue;

		if (attachEffect->HasCumulativeAnim)
		{
			pAEWithAnim = attachEffect.get();
		}
		else if (attachEffect->CanShowAnim())
		{
			const int currentDuration = attachEffect->GetRemainingDuration();

			if (currentDuration < 0 || currentDuration > duration)
			{
				pAELargestDuration = attachEffect.get();
				duration = currentDuration;
			}
		}
	}

	if (pAEWithAnim)
	{
		pAEWithAnim->UpdateCumulativeAnim();

		if (pRemoved == pAEWithAnim)
		{
			pAEWithAnim->HasCumulativeAnim = false;

			if (pAELargestDuration)
				pAELargestDuration->TransferCumulativeAnim(pAEWithAnim);
		}
	}
}

// Recalculates AttachEffect stat multipliers and other bonuses.
void TechnoExt::ExtData::RecalculateStatMultipliers()
{
	auto const pThis = this->OwnerObject();
	auto& pAE = this->AE;
	const bool wasTint = pAE.HasTint;

	double firepower = 1.0;
	double armor = 1.0;
	double speed = 1.0;
	double ROF = 1.0;
	bool cloak = false;
	bool forceDecloak = false;
	bool disableWeapons = false;
	bool unkillable = false;
	bool hasRangeModifier = false;
	bool hasTint = false;
	bool reflectsDamage = false;
	bool hasOnFireDiscardables = false;
	bool hasRestrictedArmorMultipliers = false;

	for (const auto& attachEffect : this->AttachedEffects)
	{
		if (!attachEffect->IsActive())
			continue;

		auto const type = attachEffect->GetType();
		firepower *= type->FirepowerMultiplier;
		speed *= type->SpeedMultiplier;

		if (type->ArmorMultiplier != 1.0 && (type->ArmorMultiplier_AllowWarheads.size() > 0 || type->ArmorMultiplier_DisallowWarheads.size() > 0))
			hasRestrictedArmorMultipliers = true;
		else
			armor *= type->ArmorMultiplier;

		ROF *= type->ROFMultiplier;
		cloak |= type->Cloakable;
		forceDecloak |= type->ForceDecloak;
		disableWeapons |= type->DisableWeapons;
		unkillable |= type->Unkillable;
		hasRangeModifier |= (type->WeaponRange_ExtraRange != 0.0 || type->WeaponRange_Multiplier != 0.0);
		hasTint |= type->HasTint();
		reflectsDamage |= type->ReflectDamage;
		hasOnFireDiscardables |= (type->DiscardOn & DiscardCondition::Firing) != DiscardCondition::None;
	}

	pAE.FirepowerMultiplier = firepower;
	pAE.ArmorMultiplier = armor;
	pAE.SpeedMultiplier = speed;
	pAE.ROFMultiplier = ROF;
	pAE.Cloakable = cloak;
	pAE.ForceDecloak = forceDecloak;
	pAE.DisableWeapons = disableWeapons;
	pAE.Unkillable = unkillable;
	pAE.HasRangeModifier = hasRangeModifier;
	pAE.HasTint = hasTint;
	pAE.ReflectDamage = reflectsDamage;
	pAE.HasOnFireDiscardables = hasOnFireDiscardables;
	pAE.HasRestrictedArmorMultipliers = hasRestrictedArmorMultipliers;

	if (forceDecloak && pThis->CloakState == CloakState::Cloaked)
		pThis->Uncloak(true);

	if (wasTint || hasTint)
		this->UpdateTintValues();
}

// Recalculates tint values.
void TechnoExt::ExtData::UpdateTintValues()
{
	// reset values
	this->TintColorOwner = 0;
	this->TintColorAllies = 0;
	this->TintColorEnemies = 0;
	this->TintIntensityOwner = 0;
	this->TintIntensityAllies = 0;
	this->TintIntensityEnemies = 0;

	auto const pTypeExt = this->TypeExtData;
	const bool hasTechnoTint = pTypeExt->Tint_Color.isset() || pTypeExt->Tint_Intensity;
	const bool hasShieldTint = this->Shield && this->Shield->IsActive() && this->Shield->GetType()->HasTint();

	// bail out early if no custom tint is applied.
	if (!hasTechnoTint && !this->AE.HasTint && !hasShieldTint)
		return;

	auto calculateTint = [this](const int color, const int intensity, const AffectedHouse affectedHouse)
		{
			if ((affectedHouse & AffectedHouse::Owner) != AffectedHouse::None)
			{
				this->TintColorOwner |= color;
				this->TintIntensityOwner += intensity;
			}

			if ((affectedHouse & AffectedHouse::Allies) != AffectedHouse::None)
			{
				this->TintColorAllies |= color;
				this->TintIntensityAllies += intensity;
			}

			if ((affectedHouse & AffectedHouse::Enemies) != AffectedHouse::None)
			{
				this->TintColorEnemies |= color;
				this->TintIntensityEnemies += intensity;
			}
		};

	if (hasTechnoTint)
		calculateTint(Drawing::RGB_To_Int(pTypeExt->Tint_Color), static_cast<int>(pTypeExt->Tint_Intensity * 1000), pTypeExt->Tint_VisibleToHouses);

	if (this->AE.HasTint)
	{
		for (auto const& attachEffect : this->AttachedEffects)
		{
			auto const type = attachEffect->GetType();

			if (!attachEffect->IsActive() || !type->HasTint())
				continue;

			calculateTint(Drawing::RGB_To_Int(type->Tint_Color), static_cast<int>(type->Tint_Intensity * 1000), type->Tint_VisibleToHouses);
		}
	}

	if (hasShieldTint)
	{
		auto const pShieldType = this->Shield->GetType();
		calculateTint(Drawing::RGB_To_Int(pShieldType->Tint_Color), static_cast<int>(pShieldType->Tint_Intensity * 1000), pShieldType->Tint_VisibleToHouses);
	}
}
