// methods used in TechnoClass_AI hooks or anything similar
#include "Body.h"

#include <SpawnManagerClass.h>
#include <ParticleSystemClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/AresFunctions.h>
#include <unordered_set>

// TechnoClass_AI_0x6F9E50
// It's not recommended to do anything more here it could have a better place for performance consideration
void TechnoExt::ExtData::OnEarlyUpdate()
{
	auto pType = this->OwnerObject()->GetTechnoType();

	// Set only if unset or type is changed
	// Notice that Ares may handle type conversion in the same hook here, which is executed right before this one thankfully
	if (!this->TypeExtData || this->TypeExtData->OwnerObject() != pType)
		this->UpdateTypeData(pType);

	// Update tunnel state on exit, TechnoClass::AI is only called when not in tunnel.
	if (this->IsInTunnel)
	{
		this->IsInTunnel = false;

		if (const auto pShieldData = this->Shield.get())
			pShieldData->SetAnimationVisibility(true);
	}

	if (this->CheckDeathConditions())
		return;

	this->ApplyInterceptor();
	this->EatPassengers();
	this->UpdateShield();
	this->ApplySpawnLimitRange();
	this->UpdateLaserTrails();
	this->DepletedAmmoActions();
	this->UpdateAttachEffects();
}

void TechnoExt::ExtData::ApplyInterceptor()
{
	if (this->IsBurrowed)
		return;

	auto const pThis = this->OwnerObject();

	if (pThis->Target)
		return;

	auto const pTypeExt = this->TypeExtData;

	if (pTypeExt->InterceptorType)
	{
		BulletClass* pTargetBullet = nullptr;
		const auto pInterceptorType = pTypeExt->InterceptorType.get();
		const auto& guardRange = pInterceptorType->GuardRange.Get(pThis);
		const auto& minguardRange = pInterceptorType->MinimumGuardRange.Get(pThis);
		// Interceptor weapon is always fixed
		const auto pWeapon = pThis->GetWeapon(pInterceptorType->Weapon)->WeaponType;

		// DO NOT iterate BulletExt::ExtMap here, the order of items is not deterministic
		// so it can differ across players throwing target management out of sync.
		for (auto const& pBullet : *BulletClass::Array())
		{
			auto distance = pBullet->Location.DistanceFrom(pThis->Location);

			if (distance > guardRange || distance < minguardRange)
				continue;

			auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
			auto const pBulletTypeExt = pBulletExt->TypeExtData;

			if (!pBulletTypeExt->Interceptable)
				continue;

			if (pBulletTypeExt->Armor.isset())
			{
				double versus = GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pBulletTypeExt->Armor.Get());

				if (versus == 0.0)
					continue;
			}

			auto bulletOwner = pBullet->Owner ? pBullet->Owner->Owner : pBulletExt->FirerHouse;

			if (EnumFunctions::CanTargetHouse(pInterceptorType->CanTargetHouses, pThis->Owner, bulletOwner))
			{
				if (pBulletExt->InterceptedStatus == InterceptedStatus::Targeted)
					continue;

				pTargetBullet = pBullet;
				break;
			}
		}

		if (pTargetBullet)
			pThis->SetTarget(pTargetBullet);
	}
}

void TechnoExt::ExtData::DepletedAmmoActions()
{
	auto const pThis = specific_cast<UnitClass*>(this->OwnerObject());

	if (!pThis || (pThis->Type->Ammo <= 0) || !pThis->Type->IsSimpleDeployer)
		return;

	auto const pTypeExt = this->TypeExtData;

	const bool skipMinimum = pTypeExt->Ammo_AutoDeployMinimumAmount < 0;
	const bool skipMaximum = pTypeExt->Ammo_AutoDeployMaximumAmount < 0;

	if (skipMinimum && skipMaximum)
		return;

	const bool moreThanMinimum = pThis->Ammo >= pTypeExt->Ammo_AutoDeployMinimumAmount;
	const bool lessThanMaximum = pThis->Ammo <= pTypeExt->Ammo_AutoDeployMaximumAmount;

	if ((skipMinimum || moreThanMinimum) && (skipMaximum || lessThanMaximum))
		pThis->QueueMission(Mission::Unload, true);
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
	const auto pVanishAnim = pTypeExt->AutoDeath_VanishAnimation;

	// Death if no ammo
	if (pType->Ammo > 0 && pThis->Ammo <= 0 && pTypeExt->AutoDeath_OnAmmoDepletion)
	{
		TechnoExt::KillSelf(pThis, howToDie, pVanishAnim, isInLimbo);
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
			TechnoExt::KillSelf(pThis, howToDie, pVanishAnim, isInLimbo);
			return true;
		}
	}

	auto existTechnoTypes = [pThis](const ValueableVector<TechnoTypeClass*>& vTypes, std::unordered_set<HouseClass*> validHouses, bool any, bool allowLimbo)
		{
			auto existSingleType = [pThis, validHouses, allowLimbo](TechnoTypeClass* pType)
				{
					for (HouseClass* pHouse : validHouses)
					{
						if (allowLimbo ? HouseExt::ExtMap.Find(pHouse)->CountOwnedPresentAndLimboed(pType) > 0 : pHouse->CountOwnedAndPresent(pType) > 0)
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
		// pre-calculate valid houses
		std::unordered_set<HouseClass*> validHouses;

		for (auto pHouse : *HouseClass::Array)
		{
			if (EnumFunctions::CanTargetHouse(pTypeExt->AutoDeath_TechnosDontExist_Houses, pThis->Owner, pHouse))
				validHouses.insert(pHouse);
		}

		if (!existTechnoTypes(pTypeExt->AutoDeath_TechnosDontExist, validHouses, !pTypeExt->AutoDeath_TechnosDontExist_Any, pTypeExt->AutoDeath_TechnosDontExist_AllowLimboed))
		{
			TechnoExt::KillSelf(pThis, howToDie, pVanishAnim, isInLimbo);

			return true;
		}
	}

	// death if listed technos exist
	if (!pTypeExt->AutoDeath_TechnosExist.empty())
	{
		// pre-calculate valid houses
		std::unordered_set<HouseClass*> validHouses;

		for (auto pHouse : *HouseClass::Array)
		{
			if (EnumFunctions::CanTargetHouse(pTypeExt->AutoDeath_TechnosExist_Houses, pThis->Owner, pHouse))
				validHouses.insert(pHouse);
		}

		if (existTechnoTypes(pTypeExt->AutoDeath_TechnosExist, validHouses, pTypeExt->AutoDeath_TechnosExist_Any, pTypeExt->AutoDeath_TechnosExist_AllowLimboed))
		{
			TechnoExt::KillSelf(pThis, howToDie, pVanishAnim, isInLimbo);

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

	auto pDelType = pTypeExt->PassengerDeletionType.get();

	if (!pDelType->UnderEMP && (pThis->Deactivated || pThis->IsUnderEMP()))
		return;

	if (pTypeExt && (pDelType->Rate > 0 || pDelType->UseCostAsRate))
	{
		if (pThis->Passengers.NumPassengers > 0)
		{
			// Passengers / CargoClass is essentially a stack, last in, first out (LIFO) kind of data structure
			FootClass* pPassenger = nullptr;          // Passenger to potentially delete
			FootClass* pPreviousPassenger = nullptr;  // Passenger immediately prior to the deleted one in the stack
			ObjectClass* pLastPassenger = nullptr;    // Passenger that is last in the stack
			auto pCurrentPassenger = pThis->Passengers.GetFirstPassenger();

			// Find the first entered passenger that is eligible for deletion.
			while (pCurrentPassenger)
			{
				if (EnumFunctions::CanTargetHouse(pDelType->AllowedHouses, pThis->Owner, pCurrentPassenger->Owner))
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

			auto const pPassengerType = pPassenger->GetTechnoType();

			if (!this->PassengerDeletionTimer.IsTicking()) // Execute only if timer has been stopped or not started
			{
				int timerLength = 0;

				if (pDelType->UseCostAsRate)
				{
					// Use passenger cost as countdown.
					timerLength = static_cast<int>(pPassengerType->Cost * pDelType->CostMultiplier);

					if (pDelType->CostRateCap.isset())
						timerLength = std::min(timerLength, pDelType->CostRateCap.Get());
				}
				else
				{
					// Use explicit rate optionally multiplied by unit size as countdown.
					timerLength = pDelType->Rate;

					if (pDelType->Rate_SizeMultiply && pPassengerType->Size > 1.0)
						timerLength *= static_cast<int>(pPassengerType->Size + 0.5);
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

				if (const auto pAnimType = pDelType->Anim.Get())
				{
					auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location);
					pAnim->SetOwnerObject(pThis);
					pAnim->Owner = pThis->Owner;
				}

				// Check if there is money refund
				if (pDelType->Soylent && EnumFunctions::CanTargetHouse(pDelType->SoylentAllowedHouses, pThis->Owner, pPassenger->Owner))
				{
					int nMoneyToGive = static_cast<int>(pPassengerType->GetRefund(pPassenger->Owner, true) * pDelType->SoylentMultiplier);

					if (nMoneyToGive > 0)
					{
						pThis->Owner->GiveMoney(nMoneyToGive);

						if (pDelType->DisplaySoylent)
						{
							FlyingStrings::AddMoneyString(nMoneyToGive, pThis->Owner,
								pDelType->DisplaySoylentToHouses, pThis->Location, pDelType->DisplaySoylentOffset);
						}
					}
				}

				// Handle gunner change.
				if (pThis->GetTechnoType()->Gunner)
				{
					if (auto const pFoot = abstract_cast<FootClass*>(pThis))
					{
						pFoot->RemoveGunner(pPassenger);

						if (pThis->Passengers.NumPassengers > 0)
							pFoot->ReceiveGunner(pLastPassenger ? abstract_cast<FootClass*>(pLastPassenger) : pPreviousPassenger);
					}
				}

				auto pSource = pDelType->DontScore ? nullptr : pThis;
				pPassenger->KillPassengers(pSource);
				pPassenger->RegisterDestruction(pSource);
				pPassenger->UnInit();
			}

			this->PassengerDeletionTimer.Stop();
		}
		else
		{
			this->PassengerDeletionTimer.Stop();
		}
	}
}

void TechnoExt::ExtData::UpdateShield()
{
	auto const pTypeExt = this->TypeExtData;

	// Set current shield type if it is not set.
	if (!this->CurrentShieldType->Strength && pTypeExt->ShieldType->Strength)
		this->CurrentShieldType = pTypeExt->ShieldType;

	// Create shield class instance if it does not exist.
	if (this->CurrentShieldType && this->CurrentShieldType->Strength && !this->Shield)
		this->Shield = std::make_unique<ShieldClass>(this->OwnerObject());

	if (const auto pShieldData = this->Shield.get())
		pShieldData->AI();
}

void TechnoExt::ExtData::UpdateOnTunnelEnter()
{
	if (!this->IsInTunnel)
	{
		if (const auto pShieldData = this->Shield.get())
			pShieldData->SetAnimationVisibility(false);

		for (auto& trail : this->LaserTrails)
		{
			trail.Visible = false;
			trail.LastLocation = { };
		}

		this->IsInTunnel = true;
	}
}

void TechnoExt::ExtData::ApplySpawnLimitRange()
{
	auto const pTypeExt = this->TypeExtData;

	if (pTypeExt->Spawner_LimitRange)
	{
		auto const pThis = this->OwnerObject();

		if (auto const pManager = pThis->SpawnManager)
		{
			auto pTechnoType = pThis->GetTechnoType();
			int weaponRange = 0;
			int weaponRangeExtra = pTypeExt->Spawner_ExtraLimitRange * Unsorted::LeptonsPerCell;

			auto setWeaponRange = [&weaponRange](WeaponTypeClass* pWeaponType)
				{
					if (pWeaponType && pWeaponType->Spawner && pWeaponType->Range > weaponRange)
						weaponRange = pWeaponType->Range;
				};

			setWeaponRange(pTechnoType->Weapon[0].WeaponType);
			setWeaponRange(pTechnoType->Weapon[1].WeaponType);
			setWeaponRange(pTechnoType->EliteWeapon[0].WeaponType);
			setWeaponRange(pTechnoType->EliteWeapon[1].WeaponType);

			weaponRange += weaponRangeExtra;

			if (pManager->Target && (pThis->DistanceFrom(pManager->Target) > weaponRange))
				pManager->ResetTarget();
		}
	}
}

void TechnoExt::ExtData::UpdateTypeData(TechnoTypeClass* pCurrentType)
{
	auto const pThis = this->OwnerObject();
	auto const pOldTypeExt = this->TypeExtData;
	auto const pOldType = pOldTypeExt->OwnerObject();
	auto const pNewTypeExt = TechnoTypeExt::ExtMap.Find(pCurrentType);

	if (this->LaserTrails.size())
		this->LaserTrails.clear();

	this->TypeExtData = pNewTypeExt;
	this->UpdateSelfOwnedAttachEffects();
	this->LaserTrails.reserve(pNewTypeExt->LaserTrailData.size());

	// Recreate Laser Trails
	for (auto const& entry : pNewTypeExt->LaserTrailData)
	{
		this->LaserTrails.emplace_back(entry.GetType(), pThis->Owner, entry.FLH, entry.IsOnTurret);
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
	if (pOldTypeExt->Harvester_Counted && !!pNewTypeExt->Harvester_Counted)
	{
		auto& vec = HouseExt::ExtMap.Find(pThis->Owner)->OwnedCountedHarvesters;
		vec.erase(std::remove(vec.begin(), vec.end(), pThis), vec.end());
	}

	// Remove from limbo reloaders if no longer applicable
	if (pOldType->Ammo > 0 && pOldTypeExt->ReloadInTransport && !pNewTypeExt->ReloadInTransport)
	{
		auto& vec = ScenarioExt::Global()->TransportReloaders;
		vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
	}

	if (pThis->Passengers.NumPassengers > 0)
	{
		// Update open topped state of potential passengers if transport's OpenTopped value changes.
		bool toOpenTopped = pCurrentType->OpenTopped && !pOldType->OpenTopped;

		if (toOpenTopped || (!pCurrentType->OpenTopped && pOldType->OpenTopped))
		{
			const auto processPassenger = [&](FootClass* pPassenger)
				{
					if (toOpenTopped)
					{
						pThis->EnteredOpenTopped(pPassenger);
					}
					else
					{
						pThis->ExitedOpenTopped(pPassenger);

						// Lose target & destination
						pPassenger->Guard();

						// OpenTopped adds passengers to logic layer when enabled. Under normal conditions this does not need to be removed since
						// OpenTopped state does not change while passengers are still in transport but in case of type conversion that can happen.
						LogicClass::Instance->RemoveObject(pPassenger);
					}
				};

			for (auto pPassenger = pThis->Passengers.FirstPassenger; pPassenger; )
			{
				auto pNextPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
				processPassenger(pPassenger);
				pPassenger = pNextPassenger;
			}
		}
	}

	// Update movement sound if still moving while type changed.
	if (auto const pFoot = abstract_cast<FootClass*>(pThis))
	{
		if (pFoot->Locomotor->Is_Moving_Now() && pFoot->IsMoveSoundPlaying)
		{
			if (pCurrentType->MoveSound != pOldType->MoveSound)
			{
				// End the old sound.
				pFoot->MoveSoundAudioController.End();

				if (auto const count = pCurrentType->MoveSound.Count)
				{
					// Play a new sound.
					int soundIndex = pCurrentType->MoveSound[Randomizer::Global->Random() % count];
					VocClass::PlayAt(soundIndex, pFoot->Location, &pFoot->MoveSoundAudioController);
					pFoot->IsMoveSoundPlaying = true;
				}
				else
				{
					pFoot->IsMoveSoundPlaying = false;
				}

				pFoot->MoveSoundDelay = 0;
			}
		}

		// Update Gunner
		if (pThis->Passengers.NumPassengers > 0)
		{
			if (pCurrentType->Gunner && !pOldType->Gunner)
			{
				FootClass* pGunner = nullptr;

				for (auto pNext = pThis->Passengers.FirstPassenger; pNext; pNext = abstract_cast<FootClass*>(pNext->NextObject))
					pGunner = pNext;

				pFoot->ReceiveGunner(pGunner);
			}
			else if (!pCurrentType->Gunner && pOldType->Gunner)
			{
				FootClass* pGunner = nullptr;

				for (auto pNext = pThis->Passengers.FirstPassenger; pNext; pNext = abstract_cast<FootClass*>(pNext->NextObject))
					pGunner = pNext;

				pFoot->RemoveGunner(pGunner);
			}
		}

		if (auto pInf = specific_cast<InfantryClass*>(pFoot))
		{
			// It's still not recommended to have such idea, please avoid using this
			if (static_cast<InfantryTypeClass*>(pOldType)->Deployer && !static_cast<InfantryTypeClass*>(pCurrentType)->Deployer)
			{
				static const std::unordered_map<Sequence, Sequence> sequenceMap = {
					{Sequence::Deploy, Sequence::Ready},
					{Sequence::Deployed, Sequence::Ready},
					{Sequence::DeployedIdle, Sequence::Ready},
					{Sequence::DeployedFire, Sequence::FireUp}
				};

				if (auto it = sequenceMap.find(pInf->SequenceAnim); it != sequenceMap.end())
					pInf->PlayAnim(it->second, true);
			}
		}

		if (pOldType->Locomotor == LocomotionClass::CLSIDs::Teleport && pCurrentType->Locomotor != LocomotionClass::CLSIDs::Teleport && pThis->WarpingOut)
			this->HasRemainingWarpInDelay = true;
	}
}

void TechnoExt::ExtData::UpdateLaserTrails()
{
	auto const pThis = generic_cast<FootClass*>(this->OwnerObject());

	if (!pThis || !this->LaserTrails.size())
		return;

	const auto cloakState = pThis->CloakState;
	const bool isDroppodLoco = VTable::Get(pThis->Locomotor.GetInterfacePtr()) != 0x7E8278;

	// LaserTrails update routine is in TechnoClass::AI hook because LaserDrawClass-es are updated in LogicClass::AI
	for (auto& trail : this->LaserTrails)
	{
		// @Kerbiter if you want to limit it to certain locos you do it here
		// with vtable check you can avoid the tedious process of Query IPersit/IUnknown Interface, GetClassID, compare with loco GUID, which is omnipresent in vanilla code
		if (trail.Type->DroppodOnly && isDroppodLoco)
			continue;

		trail.Cloaked = false;

		if (cloakState == CloakState::Cloaked)
		{
			if (trail.Type->CloakVisible && trail.Type->CloakVisible_DetectedOnly && !HouseClass::IsCurrentPlayerObserver() && !pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer))
				trail.Cloaked = !pThis->GetCell()->Sensors_InclHouse(HouseClass::CurrentPlayer->ArrayIndex);
			else if (!trail.Type->CloakVisible)
				trail.Cloaked = true;
		}

		if (!this->IsInTunnel)
			trail.Visible = true;

		CoordStruct trailLoc = TechnoExt::GetFLHAbsoluteCoords(pThis, trail.FLH, trail.IsOnTurret);

		if (cloakState == CloakState::Uncloaking && !trail.Type->CloakVisible)
			trail.LastLocation = trailLoc;
		else
			trail.Update(trailLoc);
	}
}

void TechnoExt::ExtData::UpdateMindControlAnim()
{
	auto const pThis = this->OwnerObject();

	if (pThis->IsMindControlled())
	{
		auto& pAnim = pThis->MindControlRingAnim;
		auto& pAnimType = this->MindControlRingAnimType;

		if (pAnim && !pAnimType)
		{
			pAnimType = pAnim->Type;
		}
		else if (!pAnim && pAnimType && pThis->CloakState == CloakState::Uncloaked && pThis->IsAlive && pThis->Health > 0 && !pThis->InLimbo)
		{
			auto coords = pThis->GetCoords();
			int offset = 0;

			if (const auto pBuilding = specific_cast<BuildingClass*>(pThis))
				offset = Unsorted::LevelHeight * pBuilding->Type->Height;
			else
				offset = pThis->GetTechnoType()->MindControlRingOffset;

			coords.Z += offset;
			auto anim = GameCreate<AnimClass>(pAnimType, coords, 0, 1);
			pAnim = anim;
			pAnim->SetOwnerObject(pThis);

			if (pThis->WhatAmI() == AbstractType::Building)
				pAnim->ZAdjust = -1024;
		}
	}
	else if (this->MindControlRingAnimType)
	{
		this->MindControlRingAnimType = nullptr;
	}
}

void TechnoExt::ApplyGainedSelfHeal(TechnoClass* pThis)
{
	auto const pRulesExt = RulesExt::Global();

	if (!pRulesExt->GainSelfHealAllowMultiplayPassive && pThis->Owner->Type->MultiplayPassive)
		return;

	auto const pType = pThis->GetTechnoType();
	int& health = pThis->Health;
	int healthDeficit = pType->Strength - health;

	if (health && healthDeficit > 0)
	{
		auto defaultSelfHealType = SelfHealGainType::NoHeal;

		if (pThis->WhatAmI() == AbstractType::Infantry || (pThis->WhatAmI() == AbstractType::Unit && pType->Organic))
			defaultSelfHealType = SelfHealGainType::Infantry;
		else if (pThis->WhatAmI() == AbstractType::Unit)
			defaultSelfHealType = SelfHealGainType::Units;

		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		auto selfHealType = pTypeExt->SelfHealGainType.Get(defaultSelfHealType);

		if (selfHealType == SelfHealGainType::NoHeal)
			return;

		bool applyHeal = false;
		int amount = 0;
		auto const pOwner = pThis->Owner;

		if (selfHealType == SelfHealGainType::Infantry)
		{
			int count = pRulesExt->InfantryGainSelfHealCap.isset() ?
				std::clamp(pOwner->InfantrySelfHeal, 1, pRulesExt->InfantryGainSelfHealCap.Get()) : pOwner->InfantrySelfHeal;

			amount = RulesClass::Instance->SelfHealInfantryAmount * count;

			if (amount && !(Unsorted::CurrentFrame % RulesClass::Instance->SelfHealInfantryFrames))
				applyHeal = true;
		}
		else
		{
			int count = pRulesExt->UnitsGainSelfHealCap.isset() ?
				std::clamp(pOwner->UnitsSelfHeal, 1, pRulesExt->UnitsGainSelfHealCap.Get()) : pOwner->UnitsSelfHeal;

			amount = RulesClass::Instance->SelfHealUnitAmount * count;

			if (amount && !(Unsorted::CurrentFrame % RulesClass::Instance->SelfHealUnitFrames))
				applyHeal = true;
		}

		if (applyHeal)
		{
			if (amount >= healthDeficit)
				amount = healthDeficit;

			health += amount;

			if (pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow && pThis->GetHeight() < -10)
			{
				if (auto const pBuilding = abstract_cast<BuildingClass*>(pThis))
				{
					pBuilding->Mark(MarkType::Change);
					pBuilding->ToggleDamagedAnims(false);
				}

				if (auto dmgParticle = pThis->DamageParticleSystem)
					dmgParticle->UnInit();
			}
		}
	}

	return;
}

void TechnoExt::ApplyMindControlRangeLimit(TechnoClass* pThis)
{
	if (auto pCapturer = pThis->MindControlledBy)
	{
		auto pCapturerExt = TechnoTypeExt::ExtMap.Find(pCapturer->GetTechnoType());

		if (pCapturerExt && pCapturerExt->MindControlRangeLimit.Get() > 0 &&
			pThis->DistanceFrom(pCapturer) > pCapturerExt->MindControlRangeLimit.Get())
		{
			pCapturer->CaptureManager->FreeUnit(pThis);
		}
	}
}

void TechnoExt::KillSelf(TechnoClass* pThis, AutoDeathBehavior deathOption, AnimTypeClass* pVanishAnimation, bool isInLimbo)
{
	auto const pOwner = pThis->Owner;
	auto const pBuilding = specific_cast<BuildingClass*>(pThis);

	if (isInLimbo)
	{
		if (pBuilding) // Remove limbo buildings' tracking here because their are not truely InLimbo
		{
			if (!pBuilding->InLimbo && !pBuilding->Type->Insignificant && !pBuilding->Type->DontScore)
				HouseExt::ExtMap.Find(pBuilding->Owner)->RemoveFromLimboTracking(pBuilding->Type);
		}
		else if (auto const pFoot = abstract_cast<FootClass*>(pThis)) // Remove parasite units first before deleting them.
		{
			if (pFoot->ParasiteImUsing && pFoot->ParasiteImUsing->Victim)
				pFoot->ParasiteImUsing->ExitUnit();
		}

		pThis->RegisterKill(pOwner);
		pThis->UnInit();
		return;
	}

	switch (deathOption)
	{

	case AutoDeathBehavior::Vanish:
	{
		if (pVanishAnimation)
		{
			auto const pAnim = GameCreate<AnimClass>(pVanishAnimation, pThis->GetCoords());
			pAnim->Owner = pOwner;
			AnimExt::ExtMap.Find(pAnim)->SetInvoker(pThis);
		}

		pThis->KillPassengers(pThis);
		pThis->Stun();
		pThis->Limbo();
		pThis->RegisterKill(pOwner);
		pThis->UnInit();

		return;
	}

	case AutoDeathBehavior::Sell:
	{
		if (pBuilding)
		{
			if (pBuilding->HasBuildUp)
			{
				// Sorry FirestormWall
				if (pBuilding->GetCurrentMission() != Mission::Selling)
				{
					pBuilding->QueueMission(Mission::Selling, false);
					pBuilding->NextMission();
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
			auto const absType = pThis->WhatAmI();

			if (absType == AbstractType::Unit || absType == AbstractType::Aircraft)
				AresFunctions::SpawnSurvivors(static_cast<FootClass*>(pThis), nullptr, false, false);
		}

		pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pOwner);
		return;
	}
}

void TechnoExt::UpdateSharedAmmo(TechnoClass* pThis)
{
	if (const auto pType = pThis->GetTechnoType())
	{
		if (pType->OpenTopped && pThis->Passengers.NumPassengers > 0 && pType->Ammo > 0)
		{
			if (const auto pExt = TechnoTypeExt::ExtMap.Find(pType))
			{
				if (pExt->Ammo_Shared)
				{
					FootClass* passenger = static_cast<FootClass*>(pThis->Passengers.FirstPassenger);
					TechnoTypeClass* passengerType;
					int& hostAmmo = pThis->Ammo;

					while (passenger)
					{
						passengerType = passenger->GetTechnoType();
						auto pPassengerExt = TechnoTypeExt::ExtMap.Find(passengerType);

						if (pPassengerExt && pPassengerExt->Ammo_Shared)
						{
							if (pExt->Ammo_Shared_Group < 0 || pExt->Ammo_Shared_Group == pPassengerExt->Ammo_Shared_Group)
							{
								int& passengerAmmo = passenger->Ammo;

								if (hostAmmo > 0 && (passengerAmmo < passengerType->Ammo))
								{
									hostAmmo--;
									passengerAmmo++;

									if (!hostAmmo)
										break;
								}
							}
						}

						passenger = static_cast<FootClass*>(passenger->NextObject);
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
}

// Updates state of all AttachEffects on techno.
void TechnoExt::ExtData::UpdateAttachEffects()
{
	auto& attachEffects = this->AttachedEffects;

	if (!attachEffects.size())
	{
		this->RecalculateStatMultipliers();
		return;
	}

	bool inTunnel = this->IsInTunnel || this->IsBurrowed;
	bool markForRedraw = false;
	std::vector<std::unique_ptr<AttachEffectClass>>::iterator it;
	std::vector<WeaponTypeClass*> expireWeapons;
	expireWeapons.reserve(attachEffects.size());

	for (it = attachEffects.begin(); it != attachEffects.end(); )
	{
		auto const attachEffect = it->get();

		if (!inTunnel)
			attachEffect->SetAnimationTunnelState(true);

		attachEffect->AI();
		bool hasExpired = attachEffect->HasExpired();
		bool shouldDiscard = attachEffect->IsActive() && attachEffect->ShouldBeDiscardedNow();

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
					expireWeapons.push_back(pType->ExpireWeapon);
			}

			if (shouldDiscard && attachEffect->ResetIfRecreatable())
			{
				++it;
				continue;
			}

			it = attachEffects.erase(it);
		}
		else
		{
			++it;
		}
	}

	this->RecalculateStatMultipliers();

	if (markForRedraw)
		this->OwnerObject()->MarkForRedraw();

	if (expireWeapons.size())
	{
		auto const pThis = this->OwnerObject();
		auto const coords = pThis->GetCoords();
		auto const pOwner = pThis->Owner;

		for (auto const& pWeapon : expireWeapons)
		{
			WeaponTypeExt::DetonateAt(pWeapon, coords, pThis, pOwner, pThis);
		}
	}
}

// Updates self-owned (defined on TechnoType) AttachEffects, called on type conversion.
void TechnoExt::ExtData::UpdateSelfOwnedAttachEffects()
{
	auto& attachEffects = this->AttachedEffects;
	auto const pThis = this->OwnerObject();
	auto const pTypeExt = this->TypeExtData;

	if (!attachEffects.size())
	{
		int count = AttachEffectClass::Attach(pThis, pThis->Owner, pThis, pThis, pTypeExt->AttachEffects);

		if (!count)
			this->RecalculateStatMultipliers();

		return;
	}

	// Delete ones on old type and not on current.
	std::vector<std::unique_ptr<AttachEffectClass>>::iterator it;
	std::vector<WeaponTypeClass*> expireWeapons;
	expireWeapons.reserve(attachEffects.size());
	bool markForRedraw = false;
	auto& typeAttachEffects = pTypeExt->AttachEffects;

	// Delete ones on old type and not on current.
	for (it = attachEffects.begin(); it != attachEffects.end(); )
	{
		auto const attachEffect = it->get();
		auto const pType = attachEffect->GetType();
		bool selfOwned = attachEffect->IsSelfOwned();
		bool remove = selfOwned && !typeAttachEffects.AttachTypes.Contains(pType);

		if (remove)
		{
			if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Expire) != ExpireWeaponCondition::None)
			{
				if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || this->GetAttachedEffectCumulativeCount(pType) < 1)
					expireWeapons.push_back(pType->ExpireWeapon);
			}

			markForRedraw |= pType->HasTint();
			it = attachEffects.erase(it);
		}
		else
		{
			it++;
		}
	}

	if (expireWeapons.size())
	{
		auto const coords = pThis->GetCoords();
		auto const pOwner = pThis->Owner;

		for (auto const& pWeapon : expireWeapons)
		{
			WeaponTypeExt::DetonateAt(pWeapon, coords, pThis, pOwner, pThis);
		}
	}

	// Add new ones.
	int count = AttachEffectClass::Attach(pThis, pThis->Owner, pThis, pThis, typeAttachEffects);

	if (!count)
		this->RecalculateStatMultipliers();

	if (markForRedraw)
		pThis->MarkForRedraw();
}

// Updates CumulativeAnimations AE's on techno.
void TechnoExt::ExtData::UpdateCumulativeAttachEffects(AttachEffectTypeClass* pAttachEffectType, AttachEffectClass* pRemoved)
{
	auto const& attachEffects = this->AttachedEffects;

	if (!attachEffects.size())
		return;

	AttachEffectClass* pAELargestDuration = nullptr;
	AttachEffectClass* pAEWithAnim = nullptr;
	int duration = 0;

	for (auto const& attachEffect : attachEffects)
	{
		if (attachEffect->GetType() != pAttachEffectType)
			continue;

		if (attachEffect->HasCumulativeAnim)
		{
			pAEWithAnim = attachEffect.get();
		}
		else if (attachEffect->CanShowAnim())
		{
			int currentDuration = attachEffect->GetRemainingDuration();

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

	double firepower = 1.0;
	double armor = 1.0;
	double speed = 1.0;
	double ROF = 1.0;
	bool cloak = false;
	bool forceDecloak = false;
	bool disableWeapons = false;
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
		armor *= type->ArmorMultiplier;
		ROF *= type->ROFMultiplier;
		cloak |= type->Cloakable;
		forceDecloak |= type->ForceDecloak;
		disableWeapons |= type->DisableWeapons;
		hasRangeModifier |= (type->WeaponRange_ExtraRange != 0.0 || type->WeaponRange_Multiplier != 0.0);
		hasTint |= type->HasTint();
		reflectsDamage |= type->ReflectDamage;
		hasOnFireDiscardables |= (type->DiscardOn & DiscardCondition::Firing) != DiscardCondition::None;
		hasRestrictedArmorMultipliers |= (type->ArmorMultiplier != 1.0 && (type->ArmorMultiplier_AllowWarheads.size() > 0 || type->ArmorMultiplier_DisallowWarheads.size() > 0));
	}

	this->AE.FirepowerMultiplier = firepower;
	this->AE.ArmorMultiplier = armor;
	this->AE.SpeedMultiplier = speed;
	this->AE.ROFMultiplier = ROF;
	this->AE.Cloakable = cloak;
	this->AE.ForceDecloak = forceDecloak;
	this->AE.DisableWeapons = disableWeapons;
	this->AE.HasRangeModifier = hasRangeModifier;
	this->AE.HasTint = hasTint;
	this->AE.ReflectDamage = reflectsDamage;
	this->AE.HasOnFireDiscardables = hasOnFireDiscardables;
	this->AE.HasRestrictedArmorMultipliers = hasRestrictedArmorMultipliers;

	if (forceDecloak && pThis->CloakState == CloakState::Cloaked)
		pThis->Uncloak(true);
}
