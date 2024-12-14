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
	auto const pThis = this->OwnerObject();
	auto const pTypeExt = this->TypeExtData;

	if (pTypeExt && pTypeExt->InterceptorType && !pThis->Target && !this->IsBurrowed)
	{
		BulletClass* pTargetBullet = nullptr;

		// DO NOT iterate BulletExt::ExtMap here, the order of items is not deterministic
		// so it can differ across players throwing target management out of sync.
		for (auto const& pBullet : *BulletClass::Array())
		{
			const auto pInterceptorType = pTypeExt->InterceptorType.get();
			const auto& guardRange = pInterceptorType->GuardRange.Get(pThis);
			const auto& minguardRange = pInterceptorType->MinimumGuardRange.Get(pThis);

			auto distance = pBullet->Location.DistanceFrom(pThis->Location);

			if (distance > guardRange || distance < minguardRange)
				continue;

			auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
			auto const pBulletTypeExt = pBulletExt->TypeExtData;

			if (!pBulletTypeExt || !pBulletTypeExt->Interceptable)
				continue;

			if (pBulletTypeExt->Armor.isset())
			{
				int weaponIndex = pThis->SelectWeapon(pBullet);
				auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
				double versus = GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pBulletTypeExt->Armor.Get());

				if (versus == 0.0)
					continue;
			}

			auto bulletOwner = pBullet->Owner ? pBullet->Owner->Owner : pBulletExt->FirerHouse;

			if (EnumFunctions::CanTargetHouse(pInterceptorType->CanTargetHouses, pThis->Owner, bulletOwner))
			{
				pTargetBullet = pBullet;

				if (pBulletExt->InterceptedStatus == InterceptedStatus::Targeted)
					continue;

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

	auto existTechnoTypes = [pThis](const ValueableVector<TechnoTypeClass*>& vTypes, AffectedHouse affectedHouse, bool any, bool allowLimbo)
		{
			auto existSingleType = [pThis, affectedHouse, allowLimbo](TechnoTypeClass* pType)
				{
					for (HouseClass* pHouse : *HouseClass::Array)
					{
						if (EnumFunctions::CanTargetHouse(affectedHouse, pThis->Owner, pHouse)
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
			TechnoExt::KillSelf(pThis, howToDie, pVanishAnim, isInLimbo);

			return true;
		}
	}

	// death if listed technos exist
	if (!pTypeExt->AutoDeath_TechnosExist.empty())
	{
		if (existTechnoTypes(pTypeExt->AutoDeath_TechnosExist, pTypeExt->AutoDeath_TechnosExist_Houses, pTypeExt->AutoDeath_TechnosExist_Any, pTypeExt->AutoDeath_TechnosExist_AllowLimboed))
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

	if (!pTypeExt->PassengerDeletionType || !TechnoExt::IsActive(pThis))
		return;

	auto pDelType = pTypeExt->PassengerDeletionType.get();

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
					pPreviousPassenger = abstract_cast<FootClass*>(pLastPassenger);;
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

					if (pDelType->Rate_SizeMultiply && pPassenger->GetTechnoType()->Size > 1.0)
						timerLength *= (int)(pPassenger->GetTechnoType()->Size + 0.5);
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

				if (auto const pPassengerType = pPassenger->GetTechnoType())
				{
					if (pDelType->ReportSound >= 0)
						VocClass::PlayAt(pDelType->ReportSound.Get(), pThis->GetCoords(), nullptr);

					if (const auto pAnimType = pDelType->Anim.Get())
					{
						auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location);
						pAnim->SetOwnerObject(pThis);
						pAnim->Owner = pThis->Owner;
					}

					// Check if there is money refund
					if (pDelType->Soylent &&
						EnumFunctions::CanTargetHouse(pDelType->SoylentAllowedHouses, pThis->Owner, pPassenger->Owner))
					{
						int nMoneyToGive = (int)(pPassenger->GetTechnoType()->GetRefund(pPassenger->Owner, true) * pDelType->SoylentMultiplier);

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
							{
								FootClass* pGunner = nullptr;

								for (auto pNext = pThis->Passengers.FirstPassenger; pNext; pNext = abstract_cast<FootClass*>(pNext->NextObject))
									pGunner = pNext;

								pFoot->ReceiveGunner(pGunner);
							}
						}
					}

					auto pSource = pDelType->DontScore ? nullptr : pThis;
					pPassenger->KillPassengers(pSource);
					pPassenger->RegisterDestruction(pSource);
					pPassenger->UnInit();
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
	auto const pThis = this->OwnerObject();
	auto const pTypeExt = this->TypeExtData;

	if (pTypeExt->Spawner_LimitRange)
	{
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
	auto const pOldType = this->TypeExtData->OwnerObject();

	if (this->LaserTrails.size())
		this->LaserTrails.clear();

	this->TypeExtData = TechnoTypeExt::ExtMap.Find(pCurrentType);

	this->UpdateSelfOwnedAttachEffects();

	// Recreate Laser Trails
	for (auto const& entry : this->TypeExtData->LaserTrailData)
	{
		this->LaserTrails.emplace_back(entry.GetType(), pThis->Owner, entry.FLH, entry.IsOnTurret);
	}

	// Reset AutoDeath Timer
	if (this->AutoDeathTimer.HasStarted())
		this->AutoDeathTimer.Stop();

	// Reset PassengerDeletion Timer
	if (this->PassengerDeletionTimer.IsTicking() && this->TypeExtData->PassengerDeletionType && this->TypeExtData->PassengerDeletionType->Rate <= 0)
		this->PassengerDeletionTimer.Stop();

	// Remove from tracked AutoDeath objects if no longer has AutoDeath
	if (pOldTypeExt->AutoDeath_Behavior.isset() && !this->TypeExtData->AutoDeath_Behavior.isset())
	{
		auto& vec = ScenarioExt::Global()->AutoDeathObjects;
		vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
	}

	// Remove from limbo reloaders if no longer applicable
	if (pOldType->Ammo > 0 && pOldTypeExt->ReloadInTransport && !this->TypeExtData->ReloadInTransport)
	{
		auto& vec = ScenarioExt::Global()->TransportReloaders;
		vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
	}

	// Update open topped state of potential passengers if transport's OpenTopped value changes.
	bool toOpenTopped = pCurrentType->OpenTopped && !pOldType->OpenTopped;

	if ((toOpenTopped || (!pCurrentType->OpenTopped && pOldType->OpenTopped)) && pThis->Passengers.NumPassengers > 0)
	{
		auto pPassenger = pThis->Passengers.FirstPassenger;

		while (pPassenger)
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

			pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
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

		if (pOldType->Locomotor == LocomotionClass::CLSIDs::Teleport && pCurrentType->Locomotor != LocomotionClass::CLSIDs::Teleport && pThis->WarpingOut)
			this->HasRemainingWarpInDelay = true;
	}
}

void TechnoExt::ExtData::UpdateLaserTrails()
{
	auto const pThis = generic_cast<FootClass*>(this->OwnerObject());

	if (!pThis)
		return;

	// LaserTrails update routine is in TechnoClass::AI hook because LaserDrawClass-es are updated in LogicClass::AI
	for (auto& trail : this->LaserTrails)
	{
		// @Kerbiter if you want to limit it to certain locos you do it here
		// with vtable check you can avoid the tedious process of Query IPersit/IUnknown Interface, GetClassID, compare with loco GUID, which is omnipresent in vanilla code
		if (VTable::Get(pThis->Locomotor.GetInterfacePtr()) != 0x7E8278 && trail.Type->DroppodOnly)
			continue;

		trail.Cloaked = false;

		if (pThis->CloakState == CloakState::Cloaked)
		{
			if (trail.Type->CloakVisible && trail.Type->CloakVisible_DetectedOnly && !HouseClass::IsCurrentPlayerObserver() && !pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer))
			{
				auto const pCell = pThis->GetCell();
				trail.Cloaked = !pCell || !pCell->Sensors_InclHouse(HouseClass::CurrentPlayer->ArrayIndex);
			}
			else if (!trail.Type->CloakVisible)
			{
				trail.Cloaked = true;
			}
		}

		if (!this->IsInTunnel)
			trail.Visible = true;

		CoordStruct trailLoc = TechnoExt::GetFLHAbsoluteCoords(pThis, trail.FLH, trail.IsOnTurret);

		if (pThis->CloakState == CloakState::Uncloaking && !trail.Type->CloakVisible)
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
		if (pThis->MindControlRingAnim && !this->MindControlRingAnimType)
		{
			this->MindControlRingAnimType = pThis->MindControlRingAnim->Type;
		}
		else if (!pThis->MindControlRingAnim && this->MindControlRingAnimType &&
			pThis->CloakState == CloakState::Uncloaked && !pThis->InLimbo && pThis->IsAlive)
		{
			auto coords = pThis->GetCoords();
			int offset = 0;

			if (const auto pBuilding = specific_cast<BuildingClass*>(pThis))
				offset = Unsorted::LevelHeight * pBuilding->Type->Height;
			else
				offset = pThis->GetTechnoType()->MindControlRingOffset;

			coords.Z += offset;
			auto anim = GameCreate<AnimClass>(this->MindControlRingAnimType, coords, 0, 1);

			if (anim)
			{
				pThis->MindControlRingAnim = anim;
				pThis->MindControlRingAnim->SetOwnerObject(pThis);

				if (pThis->WhatAmI() == AbstractType::Building)
					pThis->MindControlRingAnim->ZAdjust = -1024;
			}
		}
	}
	else if (this->MindControlRingAnimType)
	{
		this->MindControlRingAnimType = nullptr;
	}
}

void TechnoExt::ApplyGainedSelfHeal(TechnoClass* pThis)
{
	if (!RulesExt::Global()->GainSelfHealAllowMultiplayPassive && pThis->Owner->Type->MultiplayPassive)
		return;

	int healthDeficit = pThis->GetTechnoType()->Strength - pThis->Health;

	if (pThis->Health && healthDeficit > 0)
	{
		auto defaultSelfHealType = SelfHealGainType::NoHeal;

		if (pThis->WhatAmI() == AbstractType::Infantry || (pThis->WhatAmI() == AbstractType::Unit && pThis->GetTechnoType()->Organic))
			defaultSelfHealType = SelfHealGainType::Infantry;
		else if (pThis->WhatAmI() == AbstractType::Unit)
			defaultSelfHealType = SelfHealGainType::Units;

		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
		auto selfHealType = pTypeExt->SelfHealGainType.Get(defaultSelfHealType);

		if (selfHealType == SelfHealGainType::NoHeal)
			return;

		bool applyHeal = false;
		int amount = 0;

		if (selfHealType == SelfHealGainType::Infantry)
		{
			int count = RulesExt::Global()->InfantryGainSelfHealCap.isset() ?
				std::min(std::max(RulesExt::Global()->InfantryGainSelfHealCap.Get(), 1), pThis->Owner->InfantrySelfHeal) :
				pThis->Owner->InfantrySelfHeal;

			amount = RulesClass::Instance->SelfHealInfantryAmount * count;

			if (!(Unsorted::CurrentFrame % RulesClass::Instance->SelfHealInfantryFrames) && amount)
				applyHeal = true;
		}
		else
		{
			int count = RulesExt::Global()->UnitsGainSelfHealCap.isset() ?
				std::min(std::max(RulesExt::Global()->UnitsGainSelfHealCap.Get(), 1), pThis->Owner->UnitsSelfHeal) :
				pThis->Owner->UnitsSelfHeal;

			amount = RulesClass::Instance->SelfHealUnitAmount * count;

			if (!(Unsorted::CurrentFrame % RulesClass::Instance->SelfHealUnitFrames) && amount)
				applyHeal = true;
		}

		if (applyHeal && amount)
		{
			if (amount >= healthDeficit)
				amount = healthDeficit;

			bool wasDamaged = pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;

			pThis->Health += amount;

			if (wasDamaged && (pThis->GetHealthPercentage() > RulesClass::Instance->ConditionYellow
				|| pThis->GetHeight() < -10))
			{
				if (auto const pBuilding = abstract_cast<BuildingClass*>(pThis))
				{
					pBuilding->Mark(MarkType::Change);
					pBuilding->ToggleDamagedAnims(false);
				}

				auto dmgParticle = pThis->DamageParticleSystem;

				if (dmgParticle)
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
	if (isInLimbo)
	{
		// Remove parasite units first before deleting them.
		if (auto const pFoot = abstract_cast<FootClass*>(pThis))
		{
			if (pFoot->ParasiteImUsing && pFoot->ParasiteImUsing->Victim)
				pFoot->ParasiteImUsing->ExitUnit();
		}

		// Remove limbo buildings' tracking here because their are not truely InLimbo
		if (auto const pBuilding = abstract_cast<BuildingClass*>(pThis))
		{
			if (!pBuilding->InLimbo && !pBuilding->Type->Insignificant && !pBuilding->Type->DontScore)
				HouseExt::ExtMap.Find(pBuilding->Owner)->RemoveFromLimboTracking(pBuilding->Type);
		}

		pThis->RegisterKill(pThis->Owner);
		pThis->UnInit();
		return;
	}

	switch (deathOption)
	{

	case AutoDeathBehavior::Vanish:
	{
		if (pVanishAnimation)
		{
			if (auto const pAnim = GameCreate<AnimClass>(pVanishAnimation, pThis->GetCoords()))
			{
				auto const pAnimExt = AnimExt::ExtMap.Find(pAnim);
				pAnim->Owner = pThis->Owner;
				pAnimExt->SetInvoker(pThis);
			}
		}

		pThis->KillPassengers(pThis);
		pThis->Stun();
		pThis->Limbo();
		pThis->RegisterKill(pThis->Owner);
		pThis->UnInit();

		return;
	}

	case AutoDeathBehavior::Sell:
	{
		if (auto pBld = abstract_cast<BuildingClass*>(pThis))
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
		pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pThis->Owner);
		return;
	}
}

void TechnoExt::UpdateSharedAmmo(TechnoClass* pThis)
{
	if (!pThis)
		return;

	if (const auto pType = pThis->GetTechnoType())
	{
		if (pType->OpenTopped && pThis->Passengers.NumPassengers > 0)
		{
			if (const auto pExt = TechnoTypeExt::ExtMap.Find(pType))
			{
				if (pExt->Ammo_Shared && pType->Ammo > 0)
				{
					auto passenger = pThis->Passengers.FirstPassenger;
					TechnoTypeClass* passengerType;

					do
					{
						passengerType = passenger->GetTechnoType();
						auto pPassengerExt = TechnoTypeExt::ExtMap.Find(passengerType);

						if (pPassengerExt && pPassengerExt->Ammo_Shared)
						{
							if (pExt->Ammo_Shared_Group < 0 || pExt->Ammo_Shared_Group == pPassengerExt->Ammo_Shared_Group)
							{
								if (pThis->Ammo > 0 && (passenger->Ammo < passengerType->Ammo))
								{
									pThis->Ammo--;
									passenger->Ammo++;
								}
							}
						}

						passenger = static_cast<FootClass*>(passenger->NextObject);
					}
					while (passenger);
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
	auto const pThis = this->OwnerObject();
	bool inTunnel = this->IsInTunnel || this->IsBurrowed;
	bool markForRedraw = false;
	std::vector<std::unique_ptr<AttachEffectClass>>::iterator it;
	std::vector<WeaponTypeClass*> expireWeapons;

	for (it = this->AttachedEffects.begin(); it != this->AttachedEffects.end(); )
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

			it = this->AttachedEffects.erase(it);
		}
		else
		{
			++it;
		}
	}

	this->RecalculateStatMultipliers();

	if (markForRedraw)
		pThis->MarkForRedraw();

	auto const coords = pThis->GetCoords();
	auto const pOwner = pThis->Owner;

	for (auto const& pWeapon : expireWeapons)
	{
		WeaponTypeExt::DetonateAt(pWeapon, coords, pThis, pOwner, pThis);
	}
}

// Updates self-owned (defined on TechnoType) AttachEffects, called on type conversion.
void TechnoExt::ExtData::UpdateSelfOwnedAttachEffects()
{
	auto const pThis = this->OwnerObject();
	auto const pTypeExt = this->TypeExtData;
	std::vector<std::unique_ptr<AttachEffectClass>>::iterator it;
	std::vector<WeaponTypeClass*> expireWeapons;
	bool markForRedraw = false;

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
					expireWeapons.push_back(pType->ExpireWeapon);
			}

			markForRedraw |= pType->HasTint();
			it = this->AttachedEffects.erase(it);
		}
		else
		{
			it++;
		}
	}

	auto const coords = pThis->GetCoords();
	auto const pOwner = pThis->Owner;

	for (auto const& pWeapon : expireWeapons)
	{
		WeaponTypeExt::DetonateAt(pWeapon, coords, pThis, pOwner, pThis);
	}

	// Add new ones.
	int count = AttachEffectClass::Attach(pThis, pThis->Owner, pThis, pThis, pTypeExt->AttachEffects);

	if (!count)
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
