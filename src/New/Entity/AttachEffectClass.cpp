#include "AttachEffectClass.h"
#include "Memory.h"

#include <AnimClass.h>
#include <BuildingClass.h>

#include <Ext/TEvent/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

std::vector<AttachEffectClass*> AttachEffectClass::Array;

AttachEffectClass::AttachEffectClass()
	: Type { nullptr }, Techno { nullptr }, InvokerHouse { nullptr }, Invoker { nullptr },
	Source { nullptr }, DurationOverride { 0 }, Delay { 0 }, InitialDelay { 0 }, RecreationDelay { -1 }
	, Duration { 0 }
	, CurrentDelay { 0 }
	, NeedsDurationRefresh { false }
	, HasCumulativeAnim { false }
	, ShouldBeDiscarded { false }
	, NeedsRecalculateStat { false }
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
	, NeedsDurationRefresh { false }
	, HasCumulativeAnim { false }
	, ShouldBeDiscarded { false }
	, NeedsRecalculateStat { false }
	, LastDiscardCheckFrame { -1 }
	, LastDiscardCheckValue { false }
{
	this->HasInitialized = false;

	if (this->InitialDelay <= 0)
	{
		this->HasInitialized = true;
		pType->HandleEvent(pTechno);
	}

	auto& duration = this->Duration;

	duration = this->DurationOverride != 0 ? this->DurationOverride : pType->Duration;

	if (pType->Duration_ApplyFirepowerMult && duration > 0 && pInvoker)
		duration = Math::max(static_cast<int>(duration * TechnoExt::GetCurrentFirepowerMultiplier(pInvoker)), 0);

	const auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

	if (pType->Duration_ApplyArmorMultOnTarget && duration > 0) // count its own ArmorMultiplier as well
		duration = Math::max(static_cast<int>(duration / pTechno->ArmorMultiplier / pTechnoExt->AE.ArmorMultiplier / pType->ArmorMultiplier), 0);

	const int laserTrailIdx = pType->LaserTrail_Type;

	if (laserTrailIdx != -1)
	{
		this->LaserTrail = pTechnoExt->LaserTrails.emplace_back(std::make_unique<LaserTrailClass>(LaserTrailTypeClass::Array[laserTrailIdx].get(), pTechno->Owner)).get();
		this->LaserTrail->Intrinsic = false;
	}

	if (pInvoker)
		TechnoExt::ExtMap.Find(pInvoker)->AttachedEffectInvokerCount++;

	AttachEffectClass::Array.emplace_back(this);
}

AttachEffectClass::~AttachEffectClass()
{
	if (const auto& pTrail = this->LaserTrail)
	{
		const auto pTechnoExt = TechnoExt::ExtMap.Find(this->Techno);
		const auto it = std::find_if(pTechnoExt->LaserTrails.cbegin(), pTechnoExt->LaserTrails.cend(), [pTrail](std::unique_ptr<LaserTrailClass> const& item) { return item.get() == pTrail; });

		if (it != pTechnoExt->LaserTrails.cend())
			pTechnoExt->LaserTrails.erase(it);

		this->LaserTrail = nullptr;
	}

	auto it = std::find(AttachEffectClass::Array.begin(), AttachEffectClass::Array.end(), this);

	if (it != AttachEffectClass::Array.end())
		AttachEffectClass::Array.erase(it);

	this->KillAnim();

	if (this->Invoker)
		TechnoExt::ExtMap.Find(this->Invoker)->AttachedEffectInvokerCount--;
}

void AttachEffectClass::PointerGotInvalid(void* ptr, bool removed)
{
	auto const abs = static_cast<AbstractClass*>(ptr);

	if (auto const pAnim = abstract_cast<AnimClass*, true>(abs))
	{
		if (auto const pAnimExt = AnimExt::ExtMap.Find(pAnim))
		{
			if (pAnimExt->IsAttachedEffectAnim)
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
	}
	else if ((abs->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
	{
		auto const pTechno = abstract_cast<TechnoClass*, true>(abs);

		if (int count = TechnoExt::ExtMap.Find(pTechno)->AttachedEffectInvokerCount)
		{
			for (auto const pEffect : AttachEffectClass::Array)
			{
				if (pTechno == pEffect->Invoker)
				{
					AnnounceInvalidPointer(pEffect->Invoker, ptr);
					count--;

					if (count <= 0)
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

	if (!this->HasInitialized && this->InitialDelay == 0)
	{
		this->HasInitialized = true;

		if (pType->ROFMultiplier > 0.0 && pType->ROFMultiplier_ApplyOnCurrentTimer)
		{
			const double ROFModifier = pType->ROFMultiplier;
			auto const pExt = TechnoExt::ExtMap.Find(pTechno);
			pTechno->RearmTimer.Start(static_cast<int>(pTechno->RearmTimer.GetTimeLeft() * ROFModifier));

			if (!pExt->ChargeTurretTimer.HasStarted() && pExt->LastRearmWasFullDelay)
				pTechno->ChargeTurretDelay = static_cast<int>(pTechno->ChargeTurretDelay * ROFModifier);
		}

		if (pType->HasTint())
			pTechno->MarkForRedraw();

		this->NeedsRecalculateStat = true;
		pType->HandleEvent(pTechno);
	}

	if (this->CurrentDelay > 0)
	{
		if (!this->ShouldBeDiscardedNow())
		{
			this->CurrentDelay--;

			if (this->CurrentDelay == 0)
				this->NeedsDurationRefresh = true;
		}

		return;
	}

	if (this->NeedsDurationRefresh)
	{
		if (!this->ShouldBeDiscardedNow())
		{
			this->RefreshDuration();
			this->NeedsRecalculateStat = true;
			this->NeedsDurationRefresh = false;
			pType->HandleEvent(pTechno);
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
			this->NeedsRecalculateStat = true;
		}
		else if (!this->ShouldBeDiscardedNow())
		{
			this->RefreshDuration();
		}
		else
		{
			this->NeedsDurationRefresh = true;
		}

		return;
	}

	if (this->IsUnderTemporal)
		this->IsUnderTemporal = false;

	this->CloakCheck();
	this->OnlineCheck();

	if (!this->Animation && this->CanShowAnim())
		this->CreateAnim();

	this->AnimCheck();
}

void AttachEffectClass::AI_Temporal()
{
	if (!this->IsUnderTemporal)
	{
		this->IsUnderTemporal = true;

		this->CloakCheck();

		if (!this->Animation && this->CanShowAnim())
			this->CreateAnim();

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

		this->AnimCheck();
	}
}

void AttachEffectClass::AnimCheck()
{
	if (this->Type->Animation_HideIfAttachedWith.size() > 0)
	{
		auto const pTechnoExt = TechnoExt::ExtMap.Find(this->Techno);

		if (pTechnoExt->HasAttachedEffects(this->Type->Animation_HideIfAttachedWith, false, false, nullptr, nullptr, nullptr, nullptr))
		{
			this->KillAnim();
			this->IsAnimHidden = true;
		}
		else
		{
			this->IsAnimHidden = false;

			if (!this->Animation && this->CanShowAnim())
				this->CreateAnim();
		}
	}
}

void AttachEffectClass::OnlineCheck()
{
	if (!this->Type->Powered)
		return;

	auto const pTechno = this->Techno;
	bool isActive = !(pTechno->Deactivated || pTechno->IsUnderEMP());

	if (isActive && this->Techno->WhatAmI() == AbstractType::Building)
	{
		auto const pBuilding = static_cast<BuildingClass const*>(this->Techno);
		isActive = pBuilding->IsPowerOnline();
	}

	this->IsOnline = isActive;

	if (isActive != this->LastActiveStat)
	{
		this->NeedsRecalculateStat = true;
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

	if (this->IsCloaked && this->Animation && AnimTypeExt::ExtMap.Find(this->Animation->Type)->DetachOnCloak)
		this->KillAnim();
}

void AttachEffectClass::CreateAnim()
{
	auto const pType = this->Type;

	if (!pType)
		return;

	AnimTypeClass* pAnimType = nullptr;
	auto const pTechno = this->Techno;

	if (pType->Cumulative && pType->CumulativeAnimations.size() > 0)
	{
		if (!this->HasCumulativeAnim)
			return;

		const int count = TechnoExt::ExtMap.Find(pTechno)->GetAttachedEffectCumulativeCount(pType);
		pAnimType = pType->GetCumulativeAnimation(count);
	}
	else
	{
		pAnimType = pType->Animation;
	}

	if (this->IsCloaked && (!pAnimType || AnimTypeExt::ExtMap.Find(pAnimType)->DetachOnCloak))
		return;

	if (!this->Animation && pAnimType)
	{
		auto const pAnim = GameCreate<AnimClass>(pAnimType, pTechno->Location);

		pAnim->SetOwnerObject(pTechno);
		pAnim->Owner = pType->Animation_UseInvokerAsOwner ? this->InvokerHouse : pTechno->Owner;

		auto const pAnimExt = AnimExt::ExtMap.Find(pAnim);
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

void AttachEffectClass::UpdateCumulativeAnim()
{
	if (!this->HasCumulativeAnim)
		return;

	const auto pAnim = this->Animation;

	if (!pAnim)
		return;

	const auto pType = this->Type;
	const int count = TechnoExt::ExtMap.Find(this->Techno)->GetAttachedEffectCumulativeCount(pType);

	if (count < 1)
	{
		this->KillAnim();
		return;
	}

	auto const pAnimType = pType->GetCumulativeAnimation(count);

	if (pAnim->Type != pAnimType)
		AnimExt::ChangeAnimType(pAnim, pAnimType, false, pType->CumulativeAnimations_RestartOnChange);
}

void AttachEffectClass::TransferCumulativeAnim(AttachEffectClass* pSource)
{
	if (!pSource || !pSource->Animation)
		return;

	this->KillAnim();
	this->Animation = pSource->Animation;
	this->HasCumulativeAnim = true;
	pSource->Animation = nullptr;
	pSource->HasCumulativeAnim = false;
}

bool AttachEffectClass::CanShowAnim() const
{
	return (!this->IsUnderTemporal || this->Type->Animation_TemporalAction != AttachedAnimFlag::Hides)
		&& (this->IsOnline || this->Type->Animation_OfflineAction != AttachedAnimFlag::Hides)
		&& !this->IsInTunnel && !this->IsAnimHidden;
}

void AttachEffectClass::SetAnimationTunnelState(bool visible)
{
	if (!this->IsInTunnel && !visible)
		this->KillAnim();

	this->IsInTunnel = !visible;
}

void AttachEffectClass::RefreshDuration(int durationOverride)
{
	auto& duration = this->Duration;
	auto const pType = this->Type;

	if (durationOverride)
		duration = durationOverride;
	else
		duration = this->DurationOverride ? this->DurationOverride : pType->Duration;

	if (pType->Duration_ApplyFirepowerMult && duration > 0 && this->Invoker)
		duration = Math::max(static_cast<int>(duration * TechnoExt::GetCurrentFirepowerMultiplier(this->Invoker)), 0);

	if (pType->Duration_ApplyArmorMultOnTarget && duration > 0) // no need to count its own effect again
		duration = Math::max(static_cast<int>(duration / this->Techno->ArmorMultiplier / TechnoExt::ExtMap.Find(this->Techno)->AE.ArmorMultiplier), 0);

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

	return true;
}

bool AttachEffectClass::HasExpired() const
{
	return this->IsSelfOwned() && this->Delay >= 0 ? false : !this->Duration;
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
		if (pFoot->Locomotor->Is_Really_Moving_Now())
		{
			if ((discardOn & DiscardCondition::Move) != DiscardCondition::None)
			{
				this->LastDiscardCheckValue = true;
				return true;
			}
		}
		else if ((discardOn & DiscardCondition::Stationary) != DiscardCondition::None)
		{
			this->LastDiscardCheckValue = true;
			return true;
		}
	}

	if (pTechno->DrainingMe && (discardOn & DiscardCondition::Drain) != DiscardCondition::None)
	{
		this->LastDiscardCheckValue = true;
		return true;
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

bool AttachEffectClass::IsActiveIgnorePowered() const
{
	if (this->IsSelfOwned())
		return this->InitialDelay <= 0 && this->CurrentDelay == 0 && this->HasInitialized && !this->NeedsDurationRefresh;
	else
		return this->Duration;
}

bool AttachEffectClass::IsActive() const
{
	return this->IsOnline && this->IsActiveIgnorePowered();
}

bool AttachEffectClass::IsFromSource(TechnoClass* pInvoker, AbstractClass* pSource) const
{
	return pInvoker == this->Invoker && pSource == this->Source;
}

TechnoClass* AttachEffectClass::GetInvoker() const
{
	return this->Invoker;
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

	auto const pTargetExt = TechnoExt::ExtMap.Find(pTarget);
	int attachedCount = 0;
	bool markForRedraw = false;
	double ROFModifier = 1.0;
	const bool selfOwned = pTarget == pSource;

	for (size_t i = 0; i < types.size(); i++)
	{
		auto const pType = types[i];
		auto const params = attachEffectInfo.GetAttachParams(i, selfOwned);

		if (auto const pAE = AttachEffectClass::CreateAndAttach(pType, pTarget, pTargetExt->AttachedEffects, pInvokerHouse, pInvoker, pSource, params))
		{
			attachedCount++;

			if (params.InitialDelay <= 0)
			{
				if (pType->ROFMultiplier > 0.0 && pType->ROFMultiplier_ApplyOnCurrentTimer)
					ROFModifier *= pType->ROFMultiplier;

				if (pType->HasTint())
					markForRedraw = true;

				if (pType->Cumulative && pType->CumulativeAnimations.size() > 0)
					pTargetExt->UpdateCumulativeAttachEffects(pType);
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
		pTargetExt->RecalculateStatMultipliers();

	if (markForRedraw)
		pTarget->MarkForRedraw();

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
/// <returns>The created and attached AttachEffect if successful, nullptr if not.</returns>
AttachEffectClass* AttachEffectClass::CreateAndAttach(AttachEffectTypeClass* pType, TechnoClass* pTarget, std::vector<std::unique_ptr<AttachEffectClass>>& targetAEs,
	HouseClass* pInvokerHouse, TechnoClass* pInvoker, AbstractClass* pSource, AEAttachParams const& attachParams)
{
	if (!pType || !pTarget)
		return nullptr;

	if (pTarget->IsIronCurtained())
	{
		bool penetrates = pTarget->ForceShielded ? pType->PenetratesForceShield.Get(pType->PenetratesIronCurtain) : pType->PenetratesIronCurtain;

		if (!penetrates)
			return nullptr;
	}

	int currentTypeCount = 0;
	AttachEffectClass* match = nullptr;
	std::vector<AttachEffectClass*> cumulativeMatches;
	cumulativeMatches.reserve(targetAEs.size());

	for (auto const& aePtr : targetAEs)
	{
		auto const attachEffect = aePtr.get();

		if (attachEffect->GetType() == pType)
		{
			currentTypeCount++;
			match = attachEffect;

			if (!pType->Cumulative)
				break;
			else if (!attachParams.CumulativeRefreshSameSourceOnly || (attachEffect->Source == pSource && attachEffect->Invoker == pInvoker))
				cumulativeMatches.push_back(attachEffect);
		}
	}

	if (cumulativeMatches.size() > 0)
	{
		if (pType->Cumulative_MaxCount >= 0 && currentTypeCount >= pType->Cumulative_MaxCount)
		{
			if (attachParams.CumulativeRefreshAll)
			{
				for (auto const& ae : cumulativeMatches)
				{
					ae->RefreshDuration(attachParams.DurationOverride);
				}
			}
			else
			{
				AttachEffectClass* best = nullptr;

				for (auto const& ae : cumulativeMatches)
				{
					if (!best || ae->Duration < best->Duration)
						best = ae;
				}

				best->RefreshDuration(attachParams.DurationOverride);
			}

			pType->HandleEvent(pTarget);
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

	if (!pType->Cumulative && currentTypeCount > 0 && match)
	{
		match->RefreshDuration(attachParams.DurationOverride);
		pType->HandleEvent(pTarget);
	}
	else
	{
		targetAEs.emplace_back(std::make_unique<AttachEffectClass>(pType, pTarget, pInvokerHouse, pInvoker, pSource, attachParams.DurationOverride, attachParams.Delay, attachParams.InitialDelay, attachParams.RecreationDelay));
		auto const pAE = targetAEs.back().get();

		if (!currentTypeCount && pType->Cumulative && pType->CumulativeAnimations.size() > 0)
			pAE->HasCumulativeAnim = true;

		return pAE;
	}

	return nullptr;
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

	auto const pTargetExt = TechnoExt::ExtMap.Find(pTarget);
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

		if (count && pType->HasTint())
			markForRedraw = true;

		detachedCount += count;
		index++;
	}

	if (detachedCount > 0)
		TechnoExt::ExtMap.Find(pTarget)->RecalculateStatMultipliers();

	if (markForRedraw)
		pTarget->MarkForRedraw();

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

	auto const pTargetExt = TechnoExt::ExtMap.Find(pTarget);
	int detachedCount = 0;
	int stackCount = -1;

	if (pType->Cumulative)
		stackCount = pTargetExt->GetAttachedEffectCumulativeCount(pType);

	if (minCount > 0 && stackCount > -1 && pType->Cumulative && minCount > stackCount)
		return 0;

	auto const targetAEs = &pTargetExt->AttachedEffects;
	std::vector<std::unique_ptr<AttachEffectClass>>::iterator it;
	std::vector<std::pair<WeaponTypeClass*, TechnoClass*>> expireWeapons;

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
							expireWeapons.push_back(std::make_pair(pType->ExpireWeapon, pInvoker));
					}
					else
					{
						expireWeapons.push_back(std::make_pair(pType->ExpireWeapon, pTarget));
					}
				}
			}

			if (pType->Cumulative && pType->CumulativeAnimations.size() > 0)
				pTargetExt->UpdateCumulativeAttachEffects(pType, attachEffect);

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

	auto const coords = pTarget->GetCoords();

	for (auto const& pair : expireWeapons)
	{
		auto const pInvoker = pair.second;
		WeaponTypeExt::DetonateAt(pair.first, coords, pInvoker, pInvoker->Owner, pTarget);
	}

	return detachedCount;
}

/// <summary>
/// Transfer AttachEffects from one techno to another.
/// </summary>
/// <param name="pSource">Source techno.</param>
/// <param name="pTarget">Target techno.</param>
void AttachEffectClass::TransferAttachedEffects(TechnoClass* pSource, TechnoClass* pTarget)
{
	const auto pSourceExt = TechnoExt::ExtMap.Find(pSource);
	const auto pTargetExt = TechnoExt::ExtMap.Find(pTarget);
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
		int currentTypeCount = 0;
		AttachEffectClass* match = nullptr;
		AttachEffectClass* sourceMatch = nullptr;

		for (auto const& aePtr : pTargetExt->AttachedEffects)
		{
			auto const targetAttachEffect = aePtr.get();

			if (targetAttachEffect->GetType() == type)
			{
				currentTypeCount++;
				match = targetAttachEffect;

				if (targetAttachEffect->Source == attachEffect->Source && targetAttachEffect->Invoker == attachEffect->Invoker)
					sourceMatch = targetAttachEffect;
			}
		}

		if (type->Cumulative && type->Cumulative_MaxCount >= 0 && currentTypeCount >= type->Cumulative_MaxCount && sourceMatch)
		{
			sourceMatch->Duration = Math::max(sourceMatch->Duration, attachEffect->Duration);
		}
		else if (!type->Cumulative && currentTypeCount > 0 && match)
		{
			match->Duration = Math::max(match->Duration, attachEffect->Duration);
		}
		else
		{
			AEAttachParams info {};
			info.DurationOverride = attachEffect->DurationOverride;

			if (auto const pAE = AttachEffectClass::CreateAndAttach(type, pTarget, pTargetExt->AttachedEffects, attachEffect->InvokerHouse, attachEffect->Invoker, attachEffect->Source, info))
				pAE->Duration = attachEffect->Duration;
		}

		it = pSourceExt->AttachedEffects.erase(it);
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
		.Process(this->NeedsDurationRefresh)
		.Process(this->HasCumulativeAnim)
		.Process(this->ShouldBeDiscarded)
		.Process(this->LastActiveStat)
		.Process(this->LaserTrail)
		.Process(this->NeedsRecalculateStat)
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
