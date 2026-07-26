// methods used in TechnoClass_AI hooks or anything similar
#include "Body.h"

#include <Ext/Rules/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Foot/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Misc/FlyingStrings.h>
#include <Utilities/AresFunctions.h>
#include <New/Type/Affiliated/TypeConvertGroup.h>


// TechnoClass_AI_0x6F9E50
// It's not recommended to do anything more here it could have a better place for performance consideration
void TechnoExt::OnEarlyUpdate()
{
	this->UpdateShield();
	this->UpdateAttachEffects();
	this->EatPassengers();
	this->ApplySpawnLimitRange();
	this->ApplyMindControlRangeLimit();
	this->UpdateRecountBurst();
	this->UpdateRearmInEMPState();
	this->UpdateLastTargetCrd();

	if (this->CheckDeathConditions())
		return;

	this->ApplyInterceptor();
}

void TechnoExt::ApplyInterceptor()
{
	const auto pTypeExt = this->TypeExtData;
	const auto pInterceptorType = pTypeExt->InterceptorType.get();

	if (!pInterceptorType || Unsorted::CurrentFrame % pInterceptorType->TargetingDelay != 0)
		return;

	const auto pThis = this->OwnerObject();

	if (!BulletClass::Array.Count || this->IsBurrowedState() || !pThis->IsArmed())
		return;

	const auto pTarget = pThis->Target;

	if (pTarget)
	{
		if (pTarget->WhatAmI() != AbstractType::Bullet)
			return;

		const auto pTargetExt = BulletExt::Fetch(static_cast<BulletClass*>(pTarget));

		if ((pTargetExt->InterceptedStatus & InterceptedStatus::Locked) == InterceptedStatus::None)
			return;
	}

	const bool isBuilding = pThis->WhatAmI() == AbstractType::Building;

	if (isBuilding && (pThis->CurrentMission == Mission::Selling || pThis->CurrentMission == Mission::Construction))
		return;

	BulletClass* pOptionalTarget = nullptr;
	BulletClass* pTargetBullet = nullptr;
	const double guardRange = pInterceptorType->GuardRange.Get(pThis);
	const double guardRangeSq = guardRange * guardRange;
	const double minGuardRange = pInterceptorType->MinimumGuardRange.Get(pThis);
	const double minGuardRangeSq = minGuardRange * minGuardRange;
	const auto location = pThis->Location;
	const auto pWeapon = pThis->GetWeapon(pInterceptorType->Weapon)->WeaponType; // Interceptor weapon is always fixed
	const auto pWH = pWeapon->Warhead;
	const auto pOwner = pThis->Owner;

	for (auto const pBullet : BulletClass::Array)
	{
		const auto pBulletExt = BulletExt::Fetch(pBullet);
		const auto pBulletTypeExt = pBulletExt->TypeExtData;

		if (!pBulletTypeExt->Interceptable || pBullet->SpawnNextAnim)
			continue;

		const bool isTargetedOrLocked = static_cast<bool>(pBulletExt->InterceptedStatus & (InterceptedStatus::Targeted | InterceptedStatus::Locked));

		// If we already have an optional target skip ones that are already being targeted etc.
		if (pOptionalTarget && isTargetedOrLocked)
			continue;

		auto bulletLoc = pBullet->Location;

		if (pInterceptorType->GuardRange_IsCylindrical)
			bulletLoc.Z = location.Z;

		const auto distanceSq = bulletLoc.DistanceFromSquared(location);

		if (distanceSq > guardRangeSq || distanceSq < minGuardRangeSq)
			continue;

		if (pBulletTypeExt->Armor.isset())
		{
			if (!GeneralUtils::GetWarheadVersusArmor(pWH, pBulletTypeExt->Armor.Get()))
				continue;
		}

		const auto bulletOwner = pBullet->Owner ? pBullet->Owner->Owner : pBulletExt->FirerHouse;

		if (!EnumFunctions::CanTargetHouse(pInterceptorType->CanTargetHouses, pOwner, bulletOwner))
			continue;

		if (!pOptionalTarget && isTargetedOrLocked)
		{
			pOptionalTarget = pBullet;  // Set as optional target
			continue;
		}

		// Establish target
		pTargetBullet = pBullet;
		break;
	}

	// There is no more suitable target, establish optional target
	if (!pTargetBullet && pOptionalTarget)
		pTargetBullet = pOptionalTarget;

	if (pTargetBullet)
	{
		pThis->SetTarget(pTargetBullet);

		// Skip normal transition from idle to attack for building interceptors.
		if (isBuilding)
		{
			pThis->QueueMission(Mission::Attack, false);
			pThis->NextMission();
		}
	}
}

void TechnoExt::AmmoAutoConvertActions()
{
	const auto pTypeExt = this->TypeExtData;

	if (!pTypeExt->Ammo_AutoConvertType.isset())
		return;

	const int min = pTypeExt->Ammo_AutoConvertMinimumAmount;
	const int max = pTypeExt->Ammo_AutoConvertMaximumAmount;

	if (min < 0 && max < 0)
		return;

	if (pTypeExt->OwnerObject()->Ammo <= 0)
		return;

	const auto pThis = this->OwnerObject();
	const int ammo = pThis->Ammo;

	if ((min < 0 || ammo >= min) && (max < 0 || ammo <= max))
	{
		const auto pFoot = abstract_cast<FootClass*, true>(pThis);
		TechnoExt::ConvertToType(pFoot, pTypeExt->Ammo_AutoConvertType);
	}
}

// TODO : Merge into new AttachEffects
bool TechnoExt::CheckDeathConditions(bool isInLimbo)
{
	auto const pTypeExt = this->TypeExtData;

	if (!pTypeExt->AutoDeath_Behavior.isset())
		return false;

	auto const pThis = this->OwnerObject();

	if (pThis->InLimbo && !pTypeExt->AutoDeath_AllowLimboed.Get(RulesExt::Global()->AutoDeath_AllowLimboed))
		return false;

	// Self-destruction must be enabled
	const auto howToDie = pTypeExt->AutoDeath_Behavior.Get();

	// Death by conditions out of this function
	if (this->ShouldBeDead)
	{
		TechnoExt::KillSelf(pThis, howToDie, pTypeExt->AutoDeath_VanishAnimation, isInLimbo);
		return true;
	}

	// Death if no ammo
	if (pTypeExt->OwnerObject()->Ammo > 0 && pThis->Ammo <= 0 && pTypeExt->AutoDeath_OnAmmoDepletion)
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
						return allowLimbo ? HouseExt::Fetch(pOwner)->CountOwnedPresentAndLimboed(pType) > 0 : pOwner->CountOwnedAndPresent(pType) > 0;

					for (auto const pHouse : HouseClass::Array)
					{
						if (EnumFunctions::CanTargetHouse(affectedHouse, pOwner, pHouse)
							&& (allowLimbo ? HouseExt::Fetch(pHouse)->CountOwnedPresentAndLimboed(pType) > 0 : pHouse->CountOwnedAndPresent(pType) > 0))
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

void TechnoExt::EatPassengers()
{
	auto const pTypeExt = this->TypeExtData;
	auto const pDelType = pTypeExt->PassengerDeletionType.get();

	if (!pDelType)
		return;

	auto const pThis = this->OwnerObject();

	if (!TechnoExt::IsActiveIgnoreEMP(pThis))
		return;

	if (!pDelType->UnderEMP && (pThis->Deactivated || pThis->IsUnderEMP()))
	{
		if (this->PassengerDeletionTimer.InProgress())
			this->PassengerDeletionTimer.StartTime++;

		return;
	}

	if (pDelType->Rate > 0 || pDelType->UseCostAsRate)
	{
		if (pThis->Passengers.NumPassengers > 0)
		{
			// Passengers / CargoClass is essentially a stack, last in, first out (LIFO) kind of data structure
			FootClass* pPoorGuy = nullptr;          // Passenger to potentially delete
			FootClass* pPreviousPassenger = nullptr;  // Passenger immediately prior to the deleted one in the stack
			ObjectClass* pLastPassenger = nullptr;    // Passenger that is last in the stack
			auto pCurrentPassenger = pThis->Passengers.GetFirstPassenger();
			const auto allowedHouses = pDelType->AllowedHouses;
			const auto pOwner = pThis->Owner;
			const bool displayCash = pDelType->DisplaySoylent && pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer);

			// Find the first entered passenger that is eligible for deletion.
			while (pCurrentPassenger)
			{
				if (EnumFunctions::CanTargetHouse(allowedHouses, pOwner, pCurrentPassenger->Owner))
				{
					pPreviousPassenger = abstract_cast<FootClass*>(pLastPassenger);
					pPoorGuy = pCurrentPassenger;
				}

				pLastPassenger = pCurrentPassenger;
				pCurrentPassenger = abstract_cast<FootClass*>(pCurrentPassenger->NextObject);
			}

			if (!pPoorGuy)
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
					timerLength = (int)(pPoorGuy->GetTechnoType()->Cost * pDelType->CostMultiplier);

					if (pDelType->CostRateCap.isset())
						timerLength = std::min(timerLength, pDelType->CostRateCap.Get());
				}
				else
				{
					// Use explicit rate optionally multiplied by unit size as countdown.
					timerLength = pDelType->Rate;
					const double size = (double)pPoorGuy->GetTechnoType()->Size;

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
					pPreviousPassenger->NextObject = pPoorGuy->NextObject;

				if (pThis->Passengers.NumPassengers <= 0)
					pThis->Passengers.FirstPassenger = nullptr;

				if (pDelType->ReportSound >= 0)
					VocClass::PlayAt(pDelType->ReportSound.Get(), pThis->GetCoords(), nullptr);

				AnimExt::CreateRandomAnim(pDelType->Anim, pThis->Location, pThis, nullptr, true, true);

				// Check if there is money refund
				if (pDelType->Soylent
					&& EnumFunctions::CanTargetHouse(pDelType->SoylentAllowedHouses, pOwner, pPoorGuy->Owner))
				{
					const double multiplier = pDelType->SoylentMultiplier;
					int moneyToGive = static_cast<int>(pPoorGuy->GetTechnoType()->GetRefund(pPoorGuy->Owner, true) * multiplier);

					for (auto pPassenger = pPoorGuy->Passengers.GetFirstPassenger(); pPassenger; pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject))
						moneyToGive += static_cast<int>(pPassenger->GetTechnoType()->GetRefund(pPassenger->Owner, true) * multiplier);

					if (const auto pParasite = pPoorGuy->ParasiteEatingMe)
					{
						moneyToGive += static_cast<int>(pParasite->GetTechnoType()->GetRefund(pParasite->Owner, true) * multiplier);
						pParasite->ParasiteImUsing->SuppressionTimer.Start(50);
						pParasite->ParasiteImUsing->ExitUnit();
					}

					const int hijack = pPoorGuy->HijackerInfantryType;

					if (hijack != -1)
					{
						const auto pHijackerType = InfantryTypeClass::Array[hijack];
						moneyToGive += static_cast<int>(pHijackerType->GetRefund(pPoorGuy->Owner, true) * multiplier);
					}

					if (moneyToGive > 0)
					{
						pOwner->GiveMoney(moneyToGive);

						if (displayCash)
						{
							FlyingStrings::AddMoneyString(moneyToGive, pThis, pOwner,
								pDelType->DisplaySoylentToHouses, pThis->Location, pDelType->DisplaySoylentOffset);
						}
					}
				}
				else
				{
					if (const auto pParasite = pPoorGuy->ParasiteEatingMe)
					{
						pParasite->ParasiteImUsing->SuppressionTimer.Start(50);
						pParasite->ParasiteImUsing->ExitUnit();
					}
				}

				// Handle gunner change.
				auto const pTransportType = pTypeExt->OwnerObject();

				if (pTransportType->Gunner)
				{
					if (auto const pFoot = abstract_cast<FootClass*, true>(pThis))
					{
						pFoot->RemoveGunner(pPoorGuy);

						if (auto pGunner = pFoot->Passengers.GetFirstPassenger())
						{
							for (auto pNext = abstract_cast<FootClass*>(pGunner->NextObject); pNext; pNext = abstract_cast<FootClass*>(pNext->NextObject))
								pGunner = pNext;

							pFoot->ReceiveGunner(pGunner);
						}
					}
				}

				auto const pSource = pDelType->DontScore ? nullptr : pThis;
				pPoorGuy->KillPassengers(pSource);
				pPoorGuy->RegisterDestruction(pSource);
				pPoorGuy->UnInit();

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

void TechnoExt::UpdateShield()
{
	if (const auto pShieldData = this->Shield.get())
		pShieldData->AI();
}

void TechnoExt::ApplySpawnLimitRange()
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

void TechnoExt::UpdateLaserTrails()
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

		if (!this->IsInTunnelState())
			pTrail->Visible = true;

		auto const trailLoc = TechnoExt::GetFLHAbsoluteCoords(pThis, pTrail->FLH, pTrail->IsOnTurret);

		if (cloakState == CloakState::Uncloaking && !pType->CloakVisible)
			pTrail->LastLocation = trailLoc;
		else
			pTrail->Update(trailLoc);
	}
}

void TechnoExt::UpdateMindControlAnim()
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

void TechnoExt::UpdateRecountBurst()
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

void TechnoExt::UpdateGattlingRateDownReset()
{
	const auto pTypeExt = this->TypeExtData;

	if (pTypeExt->OwnerObject()->IsGattling)
	{
		const auto pThis = this->OwnerObject();

		if (pTypeExt->RateDown_Reset && (!pThis->Target || this->LastTargetID != pThis->Target->UniqueID))
		{
			const int oldStage = pThis->CurrentGattlingStage;
			this->LastTargetID = pThis->Target ? pThis->Target->UniqueID : 0xFFFFFFFF;
			pThis->GattlingValue = 0;
			pThis->CurrentGattlingStage = 0;
			this->AccumulatedGattlingValue = 0;
			this->ShouldUpdateGattlingValue = false;

			if (oldStage != 0)
				pThis->GattlingRateDown(0);
		}
	}
}

void TechnoExt::ApplyGainedSelfHeal(TechnoClass* pThis)
{
	if (!RulesExt::Global()->GainSelfHealAllowMultiplayPassive && pThis->Owner->Type->MultiplayPassive)
		return;

	auto const pTypeExt = TechnoExt::Fetch(pThis)->TypeExtData;
	auto const pType = pTypeExt->OwnerObject();
	const int healthDeficit = pType->Strength - pThis->Health;

	if (pThis->Health && healthDeficit > 0)
	{
		auto defaultSelfHealType = SelfHealGainType::NoHeal;
		auto const whatAmI = pThis->WhatAmI();

		if (whatAmI == AbstractType::Infantry)
			defaultSelfHealType = SelfHealGainType::Infantry;
		else if (whatAmI == AbstractType::Unit)
			defaultSelfHealType = (pType->Organic ? SelfHealGainType::Infantry : SelfHealGainType::Units);

		auto const selfHealType = pTypeExt->SelfHealGainType.Get(defaultSelfHealType);

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

void TechnoExt::ApplyMindControlRangeLimit()
{
	auto const pThis = this->OwnerObject();

	if (auto const pCapturer = pThis->MindControlledBy)
	{
		auto const pCapturerExt = TechnoExt::Fetch(pCapturer)->TypeExtData;

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
				HouseExt::Fetch(pBuilding->Owner)->RemoveFromLimboTracking(pBldType);
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

		if (const auto pBuilding = abstract_cast<BuildingClass*, true>(pThis))
		{
			if (pThis->BunkerLinkedItem)
				pBuilding->UnloadBunker();
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
	const auto pExt = TechnoExt::Fetch(pThis)->TypeExtData;

	if (pExt->Ammo_Shared)
	{
		const auto pType = pExt->OwnerObject();

		if (pType->OpenTopped && pType->Ammo > 0)
		{
			for (auto pPassenger = pThis->Passengers.GetFirstPassenger(); pPassenger; pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject))
			{
				const auto pPassengerExt = TechnoExt::Fetch(pPassenger)->TypeExtData;

				if (pPassengerExt->Ammo_Shared)
				{
					if (pExt->Ammo_Shared_Group < 0 || pExt->Ammo_Shared_Group == pPassengerExt->Ammo_Shared_Group)
					{
						if (pThis->Ammo > 0 && (pPassenger->Ammo < pPassengerExt->OwnerObject()->Ammo))
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

void TechnoExt::UpdateTemporal()
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

void TechnoExt::UpdateRearmInEMPState()
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

void TechnoExt::UpdateRearmInTemporal()
{
	const auto pThis = this->OwnerObject();
	const auto pTypeExt = this->TypeExtData;

	if (pThis->RearmTimer.InProgress() && pTypeExt->NoRearm_Temporal.Get(RulesExt::Global()->NoRearm_Temporal))
		pThis->RearmTimer.StartTime++;

	if (pThis->ReloadTimer.InProgress() && pTypeExt->NoReload_Temporal.Get(RulesExt::Global()->NoReload_Temporal))
		pThis->ReloadTimer.StartTime++;
}


// Updates state of all AttachEffects on techno.
void TechnoExt::UpdateAttachEffects()
{
	if (!this->AttachedEffects.size())
		return;

	auto const pThis = this->OwnerObject();
	const bool inTunnel = this->IsInTunnelState() || this->IsBurrowedState();
	bool markForRedraw = false;
	bool requiresRecalc = false;
	std::vector<std::unique_ptr<AttachEffectClass>>::iterator it;
	std::vector<std::pair<WeaponTypeClass*, TechnoClass*>> expireWeapons;

	for (it = this->AttachedEffects.begin(); it != this->AttachedEffects.end(); )
	{
		auto const attachEffect = it->get();

		if (!inTunnel)
			attachEffect->SetAnimationTunnelState(true);

		attachEffect->AI();

		if (attachEffect->ShouldRecalculateStats)
		{
			requiresRecalc = true;
			attachEffect->ShouldRecalculateStats = false;
		}

		const bool hasExpired = attachEffect->HasExpired();
		const bool shouldDiscard = attachEffect->IsActiveIgnorePowered() && attachEffect->ShouldBeDiscardedNow();

		if (hasExpired || shouldDiscard)
		{
			auto const pType = attachEffect->GetType();
			attachEffect->ShouldBeDiscarded = false;

			if (pType->RequiresRecalculation)
				requiresRecalc = true;

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
		}
		else
		{
			++it;
		}
	}

	if (requiresRecalc)
		this->RecalculateStatMultipliers();

	if (markForRedraw)
	{
		pThis->MarkForRedraw();
		this->UpdateTintValues();
	}

	auto const coords = pThis->GetCoords();

	for (auto const& pair : expireWeapons)
	{
		auto const pInvoker = pair.second;
		WeaponTypeExt::DetonateAt(pair.first, coords, pInvoker, pInvoker->Owner, pThis);
	}
}

// Updates self-owned (defined on TechnoType) AttachEffects, called on type conversion.
void TechnoExt::UpdateSelfOwnedAttachEffects()
{
	auto const pThis = this->OwnerObject();
	auto const pTypeExt = this->TypeExtData;
	auto const pTechnoType = pTypeExt->OwnerObject();
	std::vector<std::unique_ptr<AttachEffectClass>>::iterator it;
	std::vector<std::pair<WeaponTypeClass*, TechnoClass*>> expireWeapons;
	bool requiresRecalc = false;

	// Delete ones on old type and not on current.
	for (it = this->AttachedEffects.begin(); it != this->AttachedEffects.end(); )
	{
		auto const attachEffect = it->get();
		auto const pType = attachEffect->GetType();
		const bool isValid = EnumFunctions::IsTechnoEligible(pThis, pType->AffectsTarget, true)
			&& (pType->AffectTypes.empty() || pType->AffectTypes.Contains(pTechnoType)) && !pType->IgnoreTypes.Contains(pTechnoType);
		const bool remove = !isValid || (attachEffect->IsSelfOwned() && !pTypeExt->AttachEffects.AttachTypes.Contains(pType));

		if (remove)
		{
			if (pType->RequiresRecalculation)
				requiresRecalc = true;

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

			it = this->AttachedEffects.erase(it);
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

	if (requiresRecalc && !count)
		this->RecalculateStatMultipliers();
}

// Updates CumulativeAnimations AE's on techno.
void TechnoExt::UpdateCumulativeAttachEffects(AttachEffectTypeClass* pAttachEffectType, AttachEffectClass* pRemoved)
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
		else if (attachEffect->CanShowAnim(true))
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
bool TechnoExt::RecalculateStatMultipliers(AttachEffectClass* pAttachEffect)
{
	auto const pThis = this->OwnerObject();
	auto& pAE = this->AE;

	if (pAttachEffect)
	{
		auto const type = pAttachEffect->GetType();
		pAE.FirepowerMultiplier *= type->FirepowerMultiplier;
		pAE.SpeedMultiplier *= type->SpeedMultiplier;
		pAE.ROFMultiplier *= type->ROFMultiplier;
		pAE.Cloakable |= type->Cloakable;
		pAE.ForceDecloak |= type->ForceDecloak;
		pAE.DisableWeapons |= type->DisableWeapons;
		pAE.Unkillable |= type->Unkillable;
		pAE.HasRangeModifier |= (type->WeaponRange_ExtraRange != 0.0 || type->WeaponRange_Multiplier != 0.0);
		pAE.HasTint |= type->HasTint();
		pAE.ReflectDamage |= type->ReflectDamage;
		pAE.HasOnFireDiscardables |= (type->DiscardOn & DiscardCondition::Firing) != DiscardCondition::None;
		pAE.HasCritModifiers |= (type->Crit_Multiplier != 1.0 || type->Crit_ExtraChance != 0.0);

		if (type->ArmorMultiplier != 1.0 && (type->ArmorMultiplier_AllowWarheads.size() > 0 || type->ArmorMultiplier_DisallowWarheads.size() > 0))
			pAE.HasRestrictedArmorMultipliers = true;
		else
			pAE.ArmorMultiplier *= type->ArmorMultiplier;

		return pAE.ForceDecloak;
	}

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
	bool hasCritModifiers = false;

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
		hasCritModifiers |= (type->Crit_Multiplier != 1.0 || type->Crit_ExtraChance != 0.0);
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
	pAE.HasCritModifiers = hasCritModifiers;

	if (forceDecloak && pThis->CloakState == CloakState::Cloaked)
		pThis->Uncloak(true);

	return false;
}

// Recalculates tint values.
void TechnoExt::UpdateTintValues()
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

void TechnoExt::UpdateLastTargetCrd()
{
	if (!this->TypeExtData->ExtraThreat_Enabled)
		return;

	auto const pThis = this->OwnerObject();
	auto pTimer = &this->LastTargetCrdClearTimer;

	if (pThis->Target)
	{
		this->LastTargetCrd = pThis->Target->GetCoords();
		pTimer->Stop();
	}
	else
	{
		if (!pTimer->IsTicking())
			pTimer->Start(45);

		if (pTimer->Completed())
		{
			this->LastTargetCrd = CoordStruct::Empty;
			pTimer->Stop();
		}
	}
}
