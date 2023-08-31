#include "Body.h"

#include <SpawnManagerClass.h>
#include <ParticleSystemClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/House/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/AresFunctions.h>

// methods used in TechnoClass_AI hooks or anything similar

void TechnoExt::ExtData::ApplyInterceptor()
{
	auto const pThis = this->OwnerObject();
	auto const pTypeExt = this->TypeExtData;

	if (pTypeExt && pTypeExt->InterceptorType && !pThis->Target &&
		!(pThis->WhatAmI() == AbstractType::Aircraft && pThis->GetHeight() <= 0))
	{
		BulletClass* pTargetBullet = nullptr;

		for (auto const& [pBullet, pBulletExt] : BulletExt::ExtMap)
		{
			const auto pInterceptorType = pTypeExt->InterceptorType.get();
			const auto& guardRange = pInterceptorType->GuardRange.Get(pThis);
			const auto& minguardRange = pInterceptorType->MinimumGuardRange.Get(pThis);

			auto distance = pBullet->Location.DistanceFrom(pThis->Location);

			if (distance > guardRange || distance < minguardRange)
				continue;

			auto pBulletTypeExt = pBulletExt->TypeExtData;

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
	auto const pThis = this->OwnerObject();
	auto const pType = pThis->GetTechnoType();
	if ((pThis->WhatAmI() != AbstractType::Unit) || (pType->Ammo <= 0))
		return;

	auto const pTypeExt = this->TypeExtData;
	if (!(pTypeExt->Ammo_AutoDeployAmount < 0))
		return;

	auto const pUnit = abstract_cast<UnitClass*>(pThis);

	if (!pUnit->Type->IsSimpleDeployer)
		return;
	if (pThis->Ammo == pTypeExt->Ammo_AutoDeployAmount)
		pThis->QueueMission(Mission::Unload, true);
	return;
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
	const auto pVanishAnim = pTypeExt->AutoDeath_VanishAnimation.Get();

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
		if (existTechnoTypes(pTypeExt->AutoDeath_TechnosExist, pTypeExt->AutoDeath_TechnosExist_Houses, pTypeExt->AutoDeath_TechnosExist_Any, pTypeExt->AutoDeath_TechnosDontExist_AllowLimboed))
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
					if (pDelType->ReportSound.isset())
						VocClass::PlayAt(pDelType->ReportSound.Get(), pThis->GetCoords(), nullptr);

					if (pDelType->Anim.isset())
					{
						const auto pAnimType = pDelType->Anim.Get();
						if (auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location))
						{
							pAnim->SetOwnerObject(pThis);
							pAnim->Owner = pThis->Owner;
						}
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

void TechnoExt::ExtData::UpdateTypeData(TechnoTypeClass* currentType)
{
	auto const pThis = this->OwnerObject();

	if (this->LaserTrails.size())
		this->LaserTrails.clear();

	this->TypeExtData = TechnoTypeExt::ExtMap.Find(currentType);

	// Recreate Laser Trails
	for (auto const& entry : this->TypeExtData->LaserTrailData)
	{
		if (auto const pLaserType = LaserTrailTypeClass::Array[entry.idxType].get())
		{
			this->LaserTrails.push_back(LaserTrailClass {
				pLaserType, pThis->Owner, entry.FLH, entry.IsOnTurret });
		}
	}

	// Reset Shield
	// This part should have been done by UpdateShield
	// But that doesn't work correctly either, FIX THAT

	// Reset AutoDeath Timer
	if (this->AutoDeathTimer.HasStarted())
		this->AutoDeathTimer.Stop();

	// Reset PassengerDeletion Timer - TODO : unchecked
	if (this->PassengerDeletionTimer.IsTicking() && this->TypeExtData->PassengerDeletionType && this->TypeExtData->PassengerDeletionType->Rate <= 0)
		this->PassengerDeletionTimer.Stop();
}

void TechnoExt::ExtData::UpdateLaserTrails()
{
	auto const pThis = this->OwnerObject();

	// LaserTrails update routine is in TechnoClass::AI hook because TechnoClass::Draw
	// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
	for (auto& trail : this->LaserTrails)
	{
		if (pThis->CloakState == CloakState::Cloaked && !trail.Type->CloakVisible)
			continue;

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
			auto coords = CoordStruct::Empty;
			coords = *pThis->GetCoords(&coords);
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
	int healthDeficit = pThis->GetTechnoType()->Strength - pThis->Health;

	if (pThis->Health && healthDeficit > 0)
	{
		if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			bool isBuilding = pThis->WhatAmI() == AbstractType::Building;
			bool isOrganic = pThis->WhatAmI() == AbstractType::Infantry || pThis->WhatAmI() == AbstractType::Unit && pThis->GetTechnoType()->Organic;
			auto defaultSelfHealType = isBuilding ? SelfHealGainType::None : isOrganic ? SelfHealGainType::Infantry : SelfHealGainType::Units;
			auto selfHealType = pExt->SelfHealGainType.Get(defaultSelfHealType);

			if (selfHealType == SelfHealGainType::None)
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

					if (pThis->WhatAmI() == AbstractType::Unit || pThis->WhatAmI() == AbstractType::Building)
					{
						auto dmgParticle = pThis->DamageParticleSystem;

						if (dmgParticle)
							dmgParticle->UnInit();
					}
				}
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
		if (IS_ARES_FUN_AVAILABLE(SpawnSurvivors))
		{
			switch (pThis->WhatAmI())
			{
			case AbstractType::Unit:
			case AbstractType::Aircraft:
				AresFunctions::SpawnSurvivors(static_cast<FootClass*>(pThis), nullptr, false, false);
			default:break;
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
