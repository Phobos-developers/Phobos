#include "AttachEffectClass.h"

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

std::vector<AttachEffectClass*> AttachEffectClass::Array;

AttachEffectClass::AttachEffectClass()
	: Type { nullptr }, Techno { nullptr }, InvokerHouse { nullptr }, Invoker { nullptr },
	Source { nullptr }, DurationOverride { 0 }, Delay { 0 }, InitialDelay { 0 }, RecreationDelay { -1 }
	, Duration { 0 }
	, CurrentDelay { 0 }
	, ShouldRefreshDuration { false }
	, HasCumulativeAnim { false }
	, ShouldBeDiscarded { false }
	, ShouldRecalculateStats { false }
	, LastDiscardCheckFrame { -1 }
	, LastDiscardCheckValue { false }
{
	this->HasInitialized = false;
	AttachEffectClass::Array.emplace_back(this);
}

AttachEffectClass::AttachEffectClass(AttachEffectTypeClass* pType, TechnoClass* pTechno, HouseClass* pInvokerHouse,
	TechnoClass* pInvoker, AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay)
	: Type { pType }, Techno { pTechno }, InvokerHouse { pInvokerHouse }, Invoker { pInvoker }, Source { pSource },
	DurationOverride { durationOverride }, Delay { delay }, InitialDelay { initialDelay }, RecreationDelay { recreationDelay }
	, Duration { 0 }
	, CurrentDelay { 0 }
	, Animation { nullptr }
	, IsAnimHidden { false }
	, IsInTunnel { false }
	, IsUnderTemporal { false }
	, IsOnline { true }
	, IsCloaked { false }
	, LastActiveStat { true }
	, LaserTrail { nullptr }
	, ShouldRefreshDuration { false }
	, HasCumulativeAnim { false }
	, ShouldBeDiscarded { false }
	, ShouldRecalculateStats { false }
	, LastDiscardCheckFrame { -1 }
	, LastDiscardCheckValue { false }
{
	this->HasInitialized = false;

	if (this->InitialDelay <= 0)
	{
		this->HasInitialized = true;
		AttachEffectTypeClass::HandleEvent(pTechno);
	}

	int& duration = this->Duration;

	duration = this->DurationOverride != 0 ? this->DurationOverride : pType->Duration;

	if (pType->Duration_ApplyFirepowerMult && duration > 0 && pInvoker)
		duration = Math::max(static_cast<int>(duration * TechnoExt::GetCurrentFirepowerMultiplier(pInvoker)), 0);

	const auto pTechnoExt = TechnoExt::Fetch(pTechno);

	if (pType->Duration_ApplyArmorMultOnTarget && duration > 0) // count its own ArmorMultiplier as well
	{
		double armorMultiplier = TechnoExt::GetCurrentArmorMultiplier(pTechno, pTechnoExt->TypeExtData->OwnerObject(), pInvokerHouse);

		if (!pType->RestrictedArmorMultiplier || (pType->ArmorMultiplier_Chance >= ScenarioClass::Instance->Random.RandomDouble()
			&& (!pInvokerHouse || EnumFunctions::CanTargetHouse(pType->ArmorMultiplier_AffectsHouse, pTechno->Owner, pInvokerHouse))))
		{
			armorMultiplier *= pType->ArmorMultiplier;
		}

		duration = Math::max(static_cast<int>(duration / armorMultiplier), 0);
	}

	const int laserTrailIdx = pType->LaserTrail_Type;

	if (laserTrailIdx != -1)
	{
		this->LaserTrail = pTechnoExt->LaserTrails.emplace_back(std::make_unique<LaserTrailClass>(LaserTrailTypeClass::Array[laserTrailIdx].get(), pTechno->Owner)).get();
		this->LaserTrail->Intrinsic = false;
	}

	if (pInvoker)
		TechnoExt::Fetch(pInvoker)->AttachedEffectInvokerCount++;

	AttachEffectClass::Array.emplace_back(this);
}

AttachEffectClass::~AttachEffectClass()
{
	if (const auto& pTrail = this->LaserTrail)
	{
		const auto pTechnoExt = TechnoExt::Fetch(this->Techno);
		const auto it = std::find_if(pTechnoExt->LaserTrails.cbegin(), pTechnoExt->LaserTrails.cend(), [pTrail](std::unique_ptr<LaserTrailClass> const& item) { return item.get() == pTrail; });

		if (it != pTechnoExt->LaserTrails.cend())
			pTechnoExt->LaserTrails.erase(it);

		this->LaserTrail = nullptr;
	}

	auto it = std::find(AttachEffectClass::Array.begin(), AttachEffectClass::Array.end(), this);

	if (it != AttachEffectClass::Array.end())
		AttachEffectClass::Array.erase(it);

	this->KillAnim();

	// the invoker may be mid-destruction with its extension already removed
	if (this->Invoker)
	{
		if (auto const pInvokerExt = TechnoExt::TryFetch(this->Invoker))
			pInvokerExt->AttachedEffectInvokerCount--;
	}
}

void AttachEffectClass::PointerGotInvalid(void* ptr, bool removed)
{
	if (!removed)
		return;

	auto const abs = static_cast<AbstractClass*>(ptr);

	if (auto const pAnim = abstract_cast<AnimClass*, true>(abs))
	{
		auto const pAnimExt = AnimExt::TryFetch(pAnim);

		// the flag is only a fast-path gate: during scenario teardown the anim's
		// extension is already gone, and the references must still be dropped
		if (!pAnimExt || pAnimExt->IsAttachedEffectAnim)
		{
			for (auto const pEffect : AttachEffectClass::Array)
			{
				if (pAnim == pEffect->Animation)
				{
					pEffect->Animation = nullptr;
					break; // one anim must be used by less than one AE
				}
			}
		}
	}
	else if ((abs->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
	{
		auto const pTechno = abstract_cast<TechnoClass*, true>(abs);
		auto const pTechnoExt = TechnoExt::TryFetch(pTechno);

		// the counter is only a fast-path gate: during scenario teardown the techno's
		// extension is already gone, and the invoker references must still be dropped
		int count = pTechnoExt ? pTechnoExt->AttachedEffectInvokerCount : -1;

		if (count != 0)
		{
			for (auto const pEffect : AttachEffectClass::Array)
			{
				if (pTechno == pEffect->Invoker)
				{
					AnnounceInvalidPointer(pEffect->Invoker, ptr);

					if ((pEffect->Type->DiscardOn & DiscardCondition::InvokerDie) != DiscardCondition::None)
						pEffect->ShouldBeDiscarded = true;

					if (--count == 0)
						break;
				}
			}
		}
	}
}

// =============================
// actual logic

void AttachEffectClass::AI()
{
	auto const pTechno = this->Techno;

	if (!pTechno || pTechno->InLimbo || pTechno->IsImmobilized || pTechno->Transporter)
		return;

	if (this->InitialDelay > 0)
	{
		this->InitialDelay--;
		return;
	}

	auto const pType = this->Type;
	auto const pExt = TechnoExt::Fetch(pTechno);

	if (!this->HasInitialized && this->InitialDelay == 0)
	{
		this->HasInitialized = true;

		if (pType->ROFMultiplier > 0.0 && pType->ROFMultiplier_ApplyOnCurrentTimer)
		{
			const double ROFModifier = pType->ROFMultiplier;
			pTechno->RearmTimer.Start(static_cast<int>(pTechno->RearmTimer.GetTimeLeft() * ROFModifier));

			if (!pExt->ChargeTurretTimer.HasStarted() && pExt->LastRearmWasFullDelay)
				pTechno->ChargeTurretDelay = static_cast<int>(pTechno->ChargeTurretDelay * ROFModifier);
		}

		if (pExt->RecalculateStatMultipliers(this) && pTechno->CloakState == CloakState::Cloaked)
			pTechno->Uncloak(true);

		if (pType->HasTint())
		{
			pTechno->MarkForRedraw();
			pExt->UpdateTintValues();
		}

		AttachEffectTypeClass::HandleEvent(pTechno);
	}

	if (this->CurrentDelay > 0)
	{
		if (!this->ShouldBeDiscardedNow())
		{
			this->CurrentDelay--;

			if (this->CurrentDelay == 0)
				this->ShouldRefreshDuration = true;
		}

		return;
	}

	if (this->ShouldRefreshDuration)
	{
		if (!this->ShouldBeDiscardedNow())
		{
			this->RefreshDuration();

			if (pExt->RecalculateStatMultipliers(this) && pTechno->CloakState == CloakState::Cloaked)
				pTechno->Uncloak(true);

			if (pType->HasTint())
			{
				pTechno->MarkForRedraw();
				pExt->UpdateTintValues();
			}

			this->ShouldRefreshDuration = false;
			AttachEffectTypeClass::HandleEvent(pTechno);
		}

		return;
	}

	if (this->Duration > 0)
		this->Duration--;

	if (this->Duration == 0)
	{
		const int delay = this->Delay;

		if (!this->IsSelfOwned() || delay < 0)
			return;

		this->CurrentDelay = delay;

		if (delay > 0)
		{
			this->KillAnim();

			if (pType->RequiresRecalculation)
				this->ShouldRecalculateStats = true;
		}
		else if (!this->ShouldBeDiscardedNow())
		{
			this->RefreshDuration();
		}
		else
		{
			this->ShouldRefreshDuration = true;
		}

		return;
	}

	if (this->IsUnderTemporal)
		this->IsUnderTemporal = false;

	this->CloakCheck();
	this->OnlineCheck();
	this->AnimCheck();
}

void AttachEffectClass::AI_Temporal()
{
	if (!this->IsUnderTemporal)
	{
		this->IsUnderTemporal = true;

		this->CloakCheck();
		this->AnimCheck();

		if (this->Animation)
		{
			switch (this->Type->Animation_TemporalAction)
			{
			case AttachedAnimFlag::Hides:
				this->KillAnim();
				break;
			case AttachedAnimFlag::Temporal:
				this->Animation->UnderTemporal = true;
				break;

			case AttachedAnimFlag::Paused:
				this->Animation->Pause();
				break;

			case AttachedAnimFlag::PausedTemporal:
				this->Animation->Pause();
				this->Animation->UnderTemporal = true;
				break;
			}
		}
	}
}

void AttachEffectClass::AnimCheck()
{
	if (this->Type->Animation_HideIfAttachedWith.size() > 0)
	{
		auto const pTechnoExt = TechnoExt::Fetch(this->Techno);

		if (pTechnoExt->HasAttachedEffects(this->Type->Animation_HideIfAttachedWith, false, false, nullptr, nullptr, nullptr, nullptr))
		{
			this->KillAnim();
			this->IsAnimHidden = true;
			return;
		}
	}

	this->IsAnimHidden = false;

	if (!this->Animation && this->CanShowAnim())
		this->CreateAnim();
}

void AttachEffectClass::OnlineCheck()
{
	if (!this->Type->Powered)
		return;

	auto const pTechno = this->Techno;
	bool isActive = !(pTechno->Deactivated || pTechno->IsUnderEMP());

	if (isActive && pTechno->WhatAmI() == AbstractType::Building)
	{
		auto const pBuilding = static_cast<BuildingClass const*>(pTechno);
		isActive = pBuilding->IsPowerOnline();
	}

	this->IsOnline = isActive;

	if (isActive != this->LastActiveStat)
	{
		auto const pExt = TechnoExt::Fetch(pTechno);

		if (pExt->RecalculateStatMultipliers(this) && pTechno->CloakState == CloakState::Cloaked)
			pTechno->Uncloak(true);

		if (this->Type->HasTint())
		{
			pTechno->MarkForRedraw();
			pExt->UpdateTintValues();
		}

		this->LastActiveStat = isActive;
	}

	if (!this->Animation)
		return;

	if (!isActive)
	{
		switch (this->Type->Animation_OfflineAction)
		{
		case AttachedAnimFlag::Hides:
			this->KillAnim();
			break;

		case AttachedAnimFlag::Temporal:
			this->Animation->UnderTemporal = true;
			break;

		case AttachedAnimFlag::Paused:
			this->Animation->Pause();
			break;

		case AttachedAnimFlag::PausedTemporal:
			this->Animation->Pause();
			this->Animation->UnderTemporal = true;
			break;
		}
	}
	else
	{
		this->Animation->UnderTemporal = false;
		this->Animation->Unpause();
	}
}

void AttachEffectClass::CloakCheck()
{
	const auto cloakState = this->Techno->CloakState;
	this->IsCloaked = cloakState == CloakState::Cloaked || cloakState == CloakState::Cloaking;

	if (this->IsCloaked && this->Animation && AnimTypeExt::Fetch(this->Animation->Type)->DetachOnCloak)
		this->KillAnim();
}

void AttachEffectClass::CreateAnim()
{
	auto const pType = this->Type;
	auto const pTechno = this->Techno;
	AnimTypeClass* pAnimType = nullptr;

	if (pType->Cumulative && pType->CumulativeAnimations.size() > 0)
	{
		if (!this->HasCumulativeAnim)
			return;

		const int count = TechnoExt::Fetch(pTechno)->GetAttachedEffectCumulativeCount(pType);
		pAnimType = pType->GetCumulativeAnimation(count);
	}
	else
	{
		pAnimType = pType->Animation;
	}

	if (pAnimType)
	{
		if (this->IsCloaked && AnimTypeExt::Fetch(pAnimType)->DetachOnCloak)
			return;

		auto const pAnim = GameCreate<AnimClass>(pAnimType, pTechno->Location);

		pAnim->SetOwnerObject(pTechno);
		pAnim->Owner = pType->Animation_UseInvokerAsOwner ? this->InvokerHouse : pTechno->Owner;

		auto const pAnimExt = AnimExt::Fetch(pAnim);
		pAnimExt->IsAttachedEffectAnim = true;

		if (pType->Animation_UseInvokerAsOwner)
			pAnimExt->SetInvoker(this->Invoker, this->InvokerHouse);
		else
			pAnimExt->SetInvoker(pTechno);

		pAnim->RemainingIterations = 0xFFu;
		this->Animation = pAnim;
	}
}

void AttachEffectClass::KillAnim()
{
	if (this->Animation)
	{
		this->Animation->UnInit();
		this->Animation = nullptr;
	}
}

void AttachEffectClass::UpdateCumulativeAnim(int count)
{
	const auto pAnim = this->Animation;

	if (!pAnim)
		return;

	if (count < 1)
	{
		this->KillAnim();
		return;
	}

	const auto pType = this->Type;
	auto const pAnimType = pType->GetCumulativeAnimation(count);

	if (pAnim->Type != pAnimType)
		AnimExt::ChangeAnimType(pAnim, pAnimType, false, pType->CumulativeAnimations_RestartOnChange);
}

void AttachEffectClass::SetAnimationTunnelState(bool visible)
{
	if (!this->IsInTunnel && !visible)
		this->KillAnim();

	this->IsInTunnel = !visible;
}

void AttachEffectClass::RefreshDuration(int durationOverride)
{
	int& duration = this->Duration;
	auto const pType = this->Type;

	if (durationOverride)
		duration = durationOverride;
	else
		duration = this->DurationOverride ? this->DurationOverride : pType->Duration;

	if (pType->Duration_ApplyFirepowerMult && duration > 0 && this->Invoker)
		duration = Math::max(static_cast<int>(duration * TechnoExt::GetCurrentFirepowerMultiplier(this->Invoker)), 0);

	if (pType->Duration_ApplyArmorMultOnTarget && duration > 0) // no need to count its own effect again
	{
		const auto pTechnoExt = TechnoExt::Fetch(this->Techno);
		const double armorMultiplier = TechnoExt::GetCurrentArmorMultiplier(this->Techno, pTechnoExt->TypeExtData->OwnerObject(), this->InvokerHouse);
		duration = Math::max(static_cast<int>(duration / armorMultiplier), 0);
	}

	if (pType->Animation_ResetOnReapply)
	{
		this->KillAnim();

		if (this->CanShowAnim())
			this->CreateAnim();
	}
}

bool AttachEffectClass::ResetIfRecreatable()
{
	if (!this->IsSelfOwned() || this->RecreationDelay < 0)
		return false;

	this->KillAnim();
	this->Duration = 0;
	this->CurrentDelay = this->RecreationDelay;
	this->ShouldRefreshDuration = true;

	return true;
}

bool AttachEffectClass::ShouldBeDiscardedNow()
{
	if (this->LastDiscardCheckFrame == Unsorted::CurrentFrame)
		return this->LastDiscardCheckValue;

	this->LastDiscardCheckFrame = Unsorted::CurrentFrame;

	if (this->ShouldBeDiscarded)
	{
		this->LastDiscardCheckValue = true;
		return true;
	}

	auto const pType = this->Type;
	auto const discardOn = pType->DiscardOn;

	if (discardOn == DiscardCondition::None)
	{
		this->LastDiscardCheckValue = false;
		return false;
	}

	auto const pTechno = this->Techno;

	if (auto const pFoot = abstract_cast<FootClass*, true>(pTechno))
	{
		const bool isMoving = this->Type->DiscardOn_MoveBasedOnDestination.Get(RulesExt::Global()->DiscardOn_MoveBasedOnDestination)
			? pFoot->Locomotor->Is_Moving()
			: pFoot->Locomotor->Is_Really_Moving_Now();

		if (isMoving)
		{
			if ((discardOn & DiscardCondition::Move) != DiscardCondition::None)
			{
				this->LastDiscardCheckValue = true;
				return true;
			}
		}
		else if (pType->DiscardOn_ConsiderHarvestingAsStationary.Get(RulesExt::Global()->DiscardOn_ConsiderHarvestingAsStationary))
		{
			if ((discardOn & DiscardCondition::Stationary) != DiscardCondition::None)
			{
				this->LastDiscardCheckValue = true;
				return true;
			}
		}
		else
		{
			bool isHarvestingNow = false;
			if (auto const pUnit = abstract_cast<UnitClass*, true>(pFoot))
				isHarvestingNow = pUnit->IsHarvesting;
			else if (auto const pInf = abstract_cast<InfantryClass*, true>(pFoot))
				isHarvestingNow = (pInf->SequenceAnim == Sequence::Shovel);

			if (isHarvestingNow)
			{
				if ((discardOn & DiscardCondition::Harvesting) != DiscardCondition::None)
				{
					this->LastDiscardCheckValue = true;
					return true;
				}
			}
			else if (pFoot->CurrentMission == Mission::Harvest && pFoot->GetCell()->LandType == LandType::Tiberium)
			{
				// Handle the intermediate state that is about to start harvesting but does not satisfy the above judgment.
				this->LastDiscardCheckValue = false;
				return false;
			}
			else if ((discardOn & DiscardCondition::Stationary) != DiscardCondition::None)
			{
				this->LastDiscardCheckValue = true;
				return true;
			}
		}
	}

	if (auto const pBuilding = abstract_cast<BuildingClass*, true>(pTechno))
	{
		if (pBuilding->CurrentMission == Mission::Selling)
		{
			if (pBuilding->ArchiveTarget)
			{
				if ((discardOn & DiscardCondition::Undeploying) != DiscardCondition::None)
				{
					this->LastDiscardCheckValue = true;
					return true;
				}
			}
			else if ((discardOn & DiscardCondition::Selling) != DiscardCondition::None)
			{
				this->LastDiscardCheckValue = true;
				return true;
			}
		}
	}

	if (pTechno->DrainingMe && (discardOn & DiscardCondition::Drain) != DiscardCondition::None)
	{
		this->LastDiscardCheckValue = true;
		return true;
	}

	if ((discardOn & DiscardCondition::Ammo) != DiscardCondition::None)
	{
		bool trigger = false;
		if (pType->DiscardOn_Ammo_Min.isset() || pType->DiscardOn_Ammo_Max.isset())
		{
			const int min = pType->DiscardOn_Ammo_Min.Get(-1);
			const int max = pType->DiscardOn_Ammo_Max.Get(-1);
			const int ammo = pTechno->Ammo;

			trigger = (min < 0 || ammo >= min) && (max < 0 || ammo <= max);
		}

		if (trigger)
		{
			this->LastDiscardCheckValue = true;
			return true;
		}
	}

	if ((discardOn & DiscardCondition::Health) != DiscardCondition::None)
	{
		if (auto const pTypeData = pTechno->GetTechnoType())
		{
			const double hp = pTechno->GetHealthPercentage();

			if (pType->DiscardOn_Health_Min.isset() || pType->DiscardOn_Health_Max.isset())
			{
				const double min = pType->DiscardOn_Health_Min.Get(0.0);
				const double max = pType->DiscardOn_Health_Max.Get(1.0);

				if ((hp > 0.0 ? hp > min : hp >= min) && hp <= max)
				{
					this->LastDiscardCheckValue = true;
					return true;
				}
			}
		}
	}

	if ((discardOn & DiscardCondition::LandType) != DiscardCondition::None)
	{
		if (pType->DiscardOn_LandTypes != LandTypeFlags::None)
		{
			if (auto const pCell = pTechno->GetCell())
			{
				LandTypeFlags landFlags = pType->DiscardOn_LandTypes;
				if (IsLandTypeInFlags(landFlags, pCell->LandType))
				{
					this->LastDiscardCheckValue = true;
					return true;
				}
			}
		}
	}
	
	if ((discardOn & DiscardCondition::Mission) != DiscardCondition::None)
	{
		auto const& missions = pTechno->Owner->IsControlledByHuman()
			? pType->DiscardOn_Missions
			: (pType->DiscardOn_AIMissions.HasValue()
				? static_cast<ValueableVector<Mission>&>(pType->DiscardOn_AIMissions)
				: pType->DiscardOn_Missions);

		if (missions.size() > 0 && missions.Contains(pTechno->CurrentMission))
		{
			this->LastDiscardCheckValue = true;
			return true;
		}
	}

	if ((discardOn & DiscardCondition::Sequence) != DiscardCondition::None)
	{
		if (auto const pInf = abstract_cast<InfantryClass*, true>(pTechno))
		{
			if (pType->DiscardOn_Sequences.size() > 0 && pType->DiscardOn_Sequences.Contains(pInf->SequenceAnim))
			{
				this->LastDiscardCheckValue = true;
				return true;
			}
		}
	}

	if (pTechno->Target)
	{
		const bool inRange = (discardOn & DiscardCondition::InRange) != DiscardCondition::None;
		const bool outOfRange = (discardOn & DiscardCondition::OutOfRange) != DiscardCondition::None;

		if (inRange || outOfRange)
		{
			int distance = -1;

			if (pType->DiscardOn_RangeOverride.isset())
			{
				distance = pType->DiscardOn_RangeOverride.Get();
			}
			else
			{
				const int weaponIndex = pTechno->SelectWeapon(pTechno->Target);
				auto const pWeapon = pTechno->GetWeapon(weaponIndex)->WeaponType;

				if (pWeapon)
					distance = WeaponTypeExt::GetRangeWithModifiers(pWeapon, pTechno);
			}

			const int distanceFromTgt = pTechno->DistanceFrom(pTechno->Target);

			if ((inRange && distanceFromTgt <= distance) || (outOfRange && distanceFromTgt >= distance))
			{
				this->LastDiscardCheckValue = true;
				return true;
			}
		}
	}

	this->LastDiscardCheckValue = false;
	return false;
}

#pragma region StaticFunctions_AttachDetachTransfer

/// <summary>
/// Creates and attaches AttachEffects of given types to a techno.
/// </summary>
/// <param name="pTarget">Target techno.</param>
/// <param name="pInvokerHouse">House that invoked the attachment.</param>
/// <param name="pInvoker">Techno that invoked the attachment.</param>
/// <param name="pSource">Source object for the attachment e.g a Warhead or Techno.</param>
/// <param name="attachEffectInfo">AttachEffect attach info.</param>
/// <returns>Number of AttachEffect instances created and attached.</returns>
int AttachEffectClass::Attach(TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker, AbstractClass* pSource, AEAttachInfoTypeClass const& attachEffectInfo)
{
	auto const& types = attachEffectInfo.AttachTypes;

	if (types.size() < 1 || !pTarget)
		return false;

	auto const pTargetExt = TechnoExt::Fetch(pTarget);
	auto const pTargetType = pTargetExt->TypeExtData->OwnerObject();
	int attachedCount = 0;
	bool markForRedraw = false;
	bool decloak = false;
	double ROFModifier = 1.0;
	const bool selfOwned = pTarget == pSource;
	std::set<AttachEffectTypeClass*> cumulativeAnimTypes;

	for (size_t i = 0; i < types.size(); i++)
	{
		auto const pType = types[i];
		auto const params = attachEffectInfo.GetAttachParams(i, selfOwned);

		if (auto const pAE = AttachEffectClass::CreateAndAttach(pType, pTarget, pTargetType, pTargetExt->AttachedEffects, pInvokerHouse, pInvoker, pSource, params))
		{
			attachedCount++;

			if (params.InitialDelay <= 0)
			{
				if (pTargetExt->RecalculateStatMultipliers(pAE))
					decloak = true;

				if (pType->ROFMultiplier > 0.0 && pType->ROFMultiplier_ApplyOnCurrentTimer)
					ROFModifier *= pType->ROFMultiplier;

				if (pType->HasTint())
					markForRedraw = true;

				if (pType->Cumulative && pType->CumulativeAnimations.size() > 0)
					cumulativeAnimTypes.insert(pType);
			}
		}
	}

	if (ROFModifier != 1.0)
	{
		pTarget->RearmTimer.Start(static_cast<int>(pTarget->RearmTimer.GetTimeLeft() * ROFModifier));

		if (!pTargetExt->ChargeTurretTimer.HasStarted() && pTargetExt->LastRearmWasFullDelay)
			pTarget->ChargeTurretDelay = static_cast<int>(pTarget->ChargeTurretDelay * ROFModifier);
	}

	if (attachedCount > 0)
	{
		if (markForRedraw)
		{
			pTarget->MarkForRedraw();
			pTargetExt->UpdateTintValues();
		}

		if (decloak && pTarget->CloakState == CloakState::Cloaked)
			pTarget->Uncloak(true);
	}

	for (auto const pType : cumulativeAnimTypes)
	{
		pTargetExt->UpdateCumulativeAttachEffects(pType);
	}
	          
	return attachedCount;
}

/// <summary>
/// Creates and attaches a single AttachEffect instance of specified type on techno.
/// </summary>
/// <param name="pType">AttachEffect type.</param>
/// <param name="pTarget">Target techno.</param>
/// <param name="targetAEs">Target's AttachEffect vector</param>
/// <param name="pInvokerHouse">House that invoked the attachment.</param>
/// <param name="pInvoker">Techno that invoked the attachment.</param>
/// <param name="pSource">Source object for the attachment e.g a Warhead or Techno.</param>
/// <param name="attachParams">Attachment parameters.</param>
/// <param name="checkCumulative">Whether cumulative AE needs to be processed.</param>
/// <returns>The created and attached AttachEffect if successful, nullptr if not.</returns>
AttachEffectClass* AttachEffectClass::CreateAndAttach(AttachEffectTypeClass* pType, TechnoClass* pTarget, TechnoTypeClass* pTargetType, std::vector<std::unique_ptr<AttachEffectClass>>& targetAEs,
	HouseClass* pInvokerHouse, TechnoClass* pInvoker, AbstractClass* pSource, AEAttachParams const& attachParams, bool checkCumulative)
{
	if (!pType)
		return nullptr;

	if (pTarget->IsIronCurtained())
	{
		const bool penetrates = pTarget->ForceShielded ? pType->PenetratesForceShield.Get(pType->PenetratesIronCurtain) : pType->PenetratesIronCurtain;

		if (!penetrates)
			return nullptr;
	}

	if (!EnumFunctions::IsTechnoEligible(pTarget, pType->AffectsTarget, true))
		return nullptr;

	if ((!pType->AffectTypes.empty() && !pType->AffectTypes.Contains(pTargetType)) || pType->IgnoreTypes.Contains(pTargetType))
		return nullptr;

	int currentTypeCount = 0;
	int currentSourceCount = 0;
	const bool cumulative = pType->Cumulative && checkCumulative;
	AttachEffectClass* match = nullptr;
	std::vector<AttachEffectClass*> cumulativeMatches;
	cumulativeMatches.reserve(targetAEs.size());

	for (auto const& aePtr : targetAEs)
	{
		auto const attachEffect = aePtr.get();

		if (attachEffect->GetType() == pType)
		{
			currentTypeCount++;

			if (!cumulative)
			{
				attachEffect->RefreshDuration(attachParams.DurationOverride);
				AttachEffectTypeClass::HandleEvent(pTarget);
				return nullptr;
			}
			else
			{
				if (attachEffect->IsFromSource(pInvoker, pSource))
					currentSourceCount++;

				if (!attachParams.CumulativeRefreshSameSourceOnly || attachEffect->IsFromSource(pInvoker, pSource))
				{
					cumulativeMatches.push_back(attachEffect);

					if (!match || attachEffect->Duration < match->Duration)
						match = attachEffect;
				}
			}
		}
	}

	if (cumulative)
	{
		if ((pType->Cumulative_MaxCount >= 0 && currentTypeCount >= pType->Cumulative_MaxCount)
			|| (attachParams.CumulativeSourceMaxCount >= 0 && currentSourceCount >= attachParams.CumulativeSourceMaxCount))
		{
			if (attachParams.CumulativeRefreshAll)
			{
				for (auto const& ae : cumulativeMatches)
				{
					ae->RefreshDuration(attachParams.DurationOverride);
				}
			}
			else if (match)
			{
				match->RefreshDuration(attachParams.DurationOverride);
			}

			AttachEffectTypeClass::HandleEvent(pTarget);
			return nullptr;
		}
		else if (attachParams.CumulativeRefreshAll && attachParams.CumulativeRefreshAll_OnAttach)
		{
			for (auto const& ae : cumulativeMatches)
			{
				ae->RefreshDuration(attachParams.DurationOverride);
			}
		}
	}

	targetAEs.emplace_back(std::make_unique<AttachEffectClass>(pType, pTarget, pInvokerHouse, pInvoker, pSource, attachParams.DurationOverride, attachParams.Delay, attachParams.InitialDelay, attachParams.RecreationDelay));
	auto const pAE = targetAEs.back().get();

	if (!currentTypeCount && cumulative && pType->CumulativeAnimations.size() > 0)
		pAE->HasCumulativeAnim = true;

	return pAE;
}

/// <summary>
/// Remove all AttachEffects matching given types from techno.
/// </summary>
/// <param name="pTarget">Target techno.</param>
/// <param name="attachEffectInfo">AttachEffect attach info.</param>
/// <returns>Number of AttachEffect instances removed.</returns>
int AttachEffectClass::Detach(TechnoClass* pTarget, AEAttachInfoTypeClass const& attachEffectInfo)
{
	if (attachEffectInfo.RemoveTypes.size() < 1 || !pTarget)
		return 0;

	return DetachTypes(pTarget, attachEffectInfo, attachEffectInfo.RemoveTypes);
}

/// <summary>
/// Remove all AttachEffects matching given groups from techno.
/// </summary>
/// <param name="pTarget">Target techno.</param>
/// <param name="attachEffectInfo">AttachEffect attach info.</param>
/// <returns>Number of AttachEffect instances removed.</returns>
int AttachEffectClass::DetachByGroups(TechnoClass* pTarget, AEAttachInfoTypeClass const& attachEffectInfo)
{
	auto const& groups = attachEffectInfo.RemoveGroups;

	if (groups.size() < 1 || !pTarget)
		return 0;

	auto const pTargetExt = TechnoExt::Fetch(pTarget);
	std::vector<AttachEffectTypeClass*> types;
	types.reserve(pTargetExt->AttachedEffects.size());

	for (auto const& attachEffect : pTargetExt->AttachedEffects)
	{
		auto const pType = attachEffect->Type;

		if (pType->HasGroups(groups, false))
			types.push_back(pType);
	}

	return DetachTypes(pTarget, attachEffectInfo, types);
}

/// <summary>
/// Remove all AttachEffects matching given types from techno.
/// </summary>
/// <param name="pTarget">Target techno.</param>
/// <param name="attachEffectInfo">AttachEffect attach info.</param>
/// <param name="types">AttachEffect types.</param>
/// <returns>Number of AttachEffect instances removed.</returns>
int AttachEffectClass::DetachTypes(TechnoClass* pTarget, AEAttachInfoTypeClass const& attachEffectInfo, std::vector<AttachEffectTypeClass*> const& types)
{
	int detachedCount = 0;
	bool markForRedraw = false;
	bool requiresRecalc = false;
	auto const& minCounts = attachEffectInfo.CumulativeRemoveMinCounts;
	auto const& maxCounts = attachEffectInfo.CumulativeRemoveMaxCounts;
	size_t index = 0;
	const size_t minSize = minCounts.size();
	const size_t maxSize = maxCounts.size();

	for (auto const pType : types)
	{
		const int minCount = minSize > 0 ? (index < minSize ? minCounts.at(index) : minCounts.at(minSize - 1)) : -1;
		const int maxCount = maxSize > 0 ? (index < maxSize ? maxCounts.at(index) : maxCounts.at(maxSize - 1)) : -1;

		const int count = AttachEffectClass::RemoveAllOfType(pType, pTarget, minCount, maxCount);

		if (count)
		{
			if (pType->RequiresRecalculation)
				requiresRecalc = true;

			if (pType->HasTint())
				markForRedraw = true;
		}

		detachedCount += count;
		index++;
	}

	if (detachedCount > 0)
	{
		const auto pExt = TechnoExt::Fetch(pTarget);

		if (requiresRecalc)
			pExt->RecalculateStatMultipliers();

		if (markForRedraw)
		{
			pTarget->MarkForRedraw();
			pExt->UpdateTintValues();
		}
	}

	return detachedCount;
}

/// <summary>
/// Remove all AttachEffects of given type from a techno.
/// </summary>
/// <param name="pType">Type of AttachEffect to remove.</param>
/// <param name="targetAEs">Target techno.</param>
/// <param name="minCount">Minimum instance count needed for cumulative type to be removed.</param>
/// <param name="maxCount">Maximum instance count of cumulative type to be removed.</param>
/// <returns>Number of AttachEffect instances removed.</returns>
int AttachEffectClass::RemoveAllOfType(AttachEffectTypeClass* pType, TechnoClass* pTarget, int minCount, int maxCount)
{
	if (!pType || !pTarget)
		return 0;

	auto const pTargetExt = TechnoExt::Fetch(pTarget);
	int detachedCount = 0;
	int stackCount = -1;

	if (pType->Cumulative)
		stackCount = pTargetExt->GetAttachedEffectCumulativeCount(pType);

	if (minCount > 0 && stackCount > -1 && pType->Cumulative && minCount > stackCount)
		return 0;

	auto const targetAEs = &pTargetExt->AttachedEffects;
	std::vector<std::unique_ptr<AttachEffectClass>>::iterator it;
	std::vector<AEWeaponParams> expireWeapons;
	std::set<AttachEffectTypeClass*> cumulativeAnimTypes;

	for (it = targetAEs->begin(); it != targetAEs->end(); )
	{
		if (maxCount > 0 && detachedCount >= maxCount)
			break;

		auto const attachEffect = it->get();

		if (pType == attachEffect->Type)
		{
			detachedCount++;

			if (pType->ExpireWeapon && (pType->ExpireWeapon_TriggerOn & ExpireWeaponCondition::Remove) != ExpireWeaponCondition::None)
			{
				// can't be GetAttachedEffectCumulativeCount(pType) < 2, or inactive AE might make it stack more than once
				if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || stackCount == 1)
				{
					if (pType->ExpireWeapon_UseInvokerAsOwner)
					{
						if (auto const pInvoker = attachEffect->Invoker)
							expireWeapons.push_back(AEWeaponParams { pType->ExpireWeapon, pInvoker, pInvoker->Owner });
						else
							expireWeapons.push_back(AEWeaponParams { pType->ExpireWeapon, nullptr, attachEffect->GetInvokerHouse() });
					}
					else
					{
						expireWeapons.push_back(AEWeaponParams { pType->ExpireWeapon, pTarget, pTarget->Owner });
					}
				}
			}

			if (pType->Cumulative && pType->CumulativeAnimations.size() > 0)
				cumulativeAnimTypes.insert(pType);

			if (attachEffect->ResetIfRecreatable())
			{
				++it;
				continue;
			}

			it = targetAEs->erase(it);

			if (!pType->Cumulative)
				break;

			stackCount--;
		}
		else
		{
			++it;
		}
	}

	for (auto const type : cumulativeAnimTypes)
	{
		pTargetExt->UpdateCumulativeAttachEffects(type, true);
	}

	auto const coords = pTarget->GetCoords();

	for (auto const& info : expireWeapons)
	{
		WeaponTypeExt::DetonateAt(info.Weapon, coords, info.Invoker, info.InvokerHouse, pTarget);
	}

	return detachedCount;
}

/// <summary>
/// Transfer AttachEffects from one techno to another.
/// Note that this currently assumes the source techno is deleted afterwards.
/// </summary>
/// <param name="pSource">Source techno.</param>
/// <param name="pTarget">Target techno.</param>
void AttachEffectClass::TransferAttachedEffects(TechnoClass* pSource, TechnoClass* pTarget)
{
	bool markForRedraw = false;
	bool requiresRecalc = false;
	int transferCount = 0;
	const auto pSourceExt = TechnoExt::Fetch(pSource);
	const auto pTargetExt = TechnoExt::Fetch(pTarget);
	const auto pTargetType = pTarget->GetTechnoType();
	std::vector<std::unique_ptr<AttachEffectClass>>::iterator it;

	for (it = pSourceExt->AttachedEffects.begin(); it != pSourceExt->AttachedEffects.end(); )
	{
		auto const attachEffect = it->get();

		if (attachEffect->IsSelfOwned())
		{
			++it;
			continue;
		}

		auto const type = attachEffect->GetType();
		const bool isValid = EnumFunctions::IsTechnoEligible(pTarget, type->AffectsTarget, true)
			&& (type->AffectTypes.empty() || type->AffectTypes.Contains(pTargetType)) && !type->IgnoreTypes.Contains(pTargetType);

		if (!isValid)
		{
			it = pSourceExt->AttachedEffects.erase(it);
			continue;
		}

		int currentTypeCount = 0;
		const bool cumulative = type->Cumulative;
		AttachEffectClass* match = nullptr;

		for (auto const& aePtr : pTargetExt->AttachedEffects)
		{
			auto const targetAttachEffect = aePtr.get();

			if (targetAttachEffect->GetType() == type)
			{
				currentTypeCount++;	

				if (!cumulative)
				{
					match = targetAttachEffect;
					break;
				}
				else if (targetAttachEffect->IsFromSource(attachEffect->Invoker, attachEffect->Source))
				{
					if (!match || targetAttachEffect->Duration < match->Duration)
						match = targetAttachEffect;
				}
			}
		}

		if (match)
		{
			if (!cumulative || (type->Cumulative_MaxCount >= 0 && currentTypeCount >= type->Cumulative_MaxCount))
				match->Duration = Math::max(match->Duration, attachEffect->Duration);
		}
		else
		{
			AEAttachParams info {};
			info.DurationOverride = attachEffect->DurationOverride;

			if (auto const pAE = AttachEffectClass::CreateAndAttach(type, pTarget, pTargetType, pTargetExt->AttachedEffects, attachEffect->InvokerHouse, attachEffect->Invoker, attachEffect->Source, info, false))
				pAE->Duration = attachEffect->Duration;
		}

		if (type->RequiresRecalculation)
			requiresRecalc = true;

		if (type->HasTint())
			markForRedraw = true;

		transferCount++;
		it = pSourceExt->AttachedEffects.erase(it);
	} 

	if (transferCount > 0)
	{
		if (requiresRecalc)
			pTargetExt->RecalculateStatMultipliers();

		if (markForRedraw)
		{
			pTarget->MarkForRedraw();
			pTargetExt->UpdateTintValues();
		}
	}
}

#pragma endregion

// =============================
// load / save

template <typename T>
bool AttachEffectClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->Duration)
		.Process(this->DurationOverride)
		.Process(this->Delay)
		.Process(this->CurrentDelay)
		.Process(this->InitialDelay)
		.Process(this->RecreationDelay)
		.Process(this->Type)
		.Process(this->Techno)
		.Process(this->InvokerHouse)
		.Process(this->Invoker)
		.Process(this->Source)
		.Process(this->Animation)
		.Process(this->IsAnimHidden)
		.Process(this->IsInTunnel)
		.Process(this->IsUnderTemporal)
		.Process(this->IsOnline)
		.Process(this->IsCloaked)
		.Process(this->HasInitialized)
		.Process(this->ShouldRefreshDuration)
		.Process(this->LastDiscardCheckFrame)
		.Process(this->LastDiscardCheckValue)
		.Process(this->HasCumulativeAnim)
		.Process(this->ShouldBeDiscarded)
		.Process(this->LastActiveStat)
		.Process(this->LaserTrail)
		.Process(this->ShouldRecalculateStats)
		.Success();
}

bool AttachEffectClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool AttachEffectClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<AttachEffectClass*>(this)->Serialize(Stm);
}
