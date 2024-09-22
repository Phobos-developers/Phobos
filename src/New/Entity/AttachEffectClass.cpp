#include "AttachEffectClass.h"
#include "Memory.h"

#include <AnimClass.h>
#include <BuildingClass.h>

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
	, NeedsDurationRefresh { false }
	, HasCumulativeAnim { false }
{
	this->HasInitialized = false;

	if (this->InitialDelay <= 0)
		this->HasInitialized = true;

	Duration = this->DurationOverride != 0 ? this->DurationOverride : this->Type->Duration;

	AttachEffectClass::Array.emplace_back(this);
}

AttachEffectClass::~AttachEffectClass()
{
	auto it = std::find(AttachEffectClass::Array.begin(), AttachEffectClass::Array.end(), this);

	if (it != AttachEffectClass::Array.end())
		AttachEffectClass::Array.erase(it);

	this->KillAnim();
}

void AttachEffectClass::PointerGotInvalid(void* ptr, bool removed)
{
	auto const abs = static_cast<AbstractClass*>(ptr);
	auto const absType = abs->WhatAmI();

	if (absType == AbstractType::Anim)
	{
		for (auto pEffect : AttachEffectClass::Array)
		{
			if (ptr == pEffect->Animation)
				pEffect->Animation = nullptr;
		}
	}
	else if ((abs->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None)
	{
		for (auto pEffect : AttachEffectClass::Array)
			AnnounceInvalidPointer(pEffect->Invoker, ptr);
	}
}

// =============================
// actual logic

void AttachEffectClass::AI()
{
	if (!this->Techno || this->Techno->InLimbo || this->Techno->IsImmobilized || this->Techno->Transporter)
		return;

	if (this->InitialDelay > 0)
	{
		this->InitialDelay--;
		return;
	}

	if (!this->HasInitialized && this->InitialDelay == 0)
	{
		this->HasInitialized = true;

		if (this->Type->ROFMultiplier > 0.0 && this->Type->ROFMultiplier_ApplyOnCurrentTimer)
		{
			double ROFModifier = this->Type->ROFMultiplier;
			auto const pTechno = this->Techno;
			pTechno->RearmTimer.Start(static_cast<int>(pTechno->RearmTimer.GetTimeLeft() * ROFModifier));
			pTechno->ChargeTurretDelay = static_cast<int>(pTechno->ChargeTurretDelay * ROFModifier);
		}

		if (this->Type->HasTint())
			this->Techno->MarkForRedraw();
	}

	if (this->CurrentDelay > 0)
	{
		this->CurrentDelay--;

		if (this->CurrentDelay == 0)
			this->NeedsDurationRefresh = true;

		return;
	}

	if (this->NeedsDurationRefresh)
	{
		if (AllowedToBeActive())
		{
			this->RefreshDuration();
			this->NeedsDurationRefresh = false;
		}

		return;
	}

	if (this->Duration > 0)
		this->Duration--;

	if (this->Duration == 0)
	{
		if (!this->IsSelfOwned() || this->Delay < 0)
			return;

		this->CurrentDelay = this->Delay;

		if (this->Delay > 0)
			this->KillAnim();
		else if (AllowedToBeActive())
			this->RefreshDuration();
		else
			this->NeedsDurationRefresh = true;

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

	auto pTechno = this->Techno;
	bool isActive = !(pTechno->Deactivated || pTechno->IsUnderEMP());

	if (isActive && this->Techno->WhatAmI() == AbstractType::Building)
	{
		auto const pBuilding = static_cast<BuildingClass const*>(this->Techno);
		isActive = pBuilding->IsPowerOnline();
	}

	this->IsOnline = isActive;

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

	if (cloakState == CloakState::Cloaked || cloakState == CloakState::Cloaking)
	{
		this->IsCloaked = true;
		this->KillAnim();
	}
	else
	{
		this->IsCloaked = false;
	}
}

void AttachEffectClass::CreateAnim()
{
	if (!this->Type)
		return;

	AnimTypeClass* pAnimType = nullptr;

	if (this->Type->Cumulative && this->Type->CumulativeAnimations.size() > 0)
	{
		if (!this->HasCumulativeAnim)
			return;

		int count = TechnoExt::ExtMap.Find(this->Techno)->GetAttachedEffectCumulativeCount(this->Type);
		pAnimType = this->Type->GetCumulativeAnimation(count);
	}
	else
	{
		pAnimType = this->Type->Animation;
	}

	if (!this->Animation && pAnimType)
	{
		if (auto const pAnim = GameCreate<AnimClass>(pAnimType, this->Techno->Location))
		{
			pAnim->SetOwnerObject(this->Techno);
			pAnim->Owner = this->Type->Animation_UseInvokerAsOwner ? InvokerHouse : this->Techno->Owner;
			pAnim->RemainingIterations = 0xFFu;
			this->Animation = pAnim;

			if (this->Type->Animation_UseInvokerAsOwner)
			{
				auto const pAnimExt = AnimExt::ExtMap.Find(pAnim);
				pAnimExt->SetInvoker(Invoker);
			}
		}
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
	if (!this->HasCumulativeAnim || !this->Animation)
		return;

	int count = TechnoExt::ExtMap.Find(this->Techno)->GetAttachedEffectCumulativeCount(this->Type);

	if (count < 1)
	{
		this->KillAnim();
		return;
	}

	auto const pAnimType = this->Type->GetCumulativeAnimation(count);

	if (this->Animation->Type != pAnimType)
		AnimExt::ChangeAnimType(this->Animation, pAnimType, false, this->Type->CumulativeAnimations_RestartOnChange);
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
		&& !this->IsCloaked && !this->IsInTunnel && !this->IsAnimHidden;
}

void AttachEffectClass::SetAnimationTunnelState(bool visible)
{
	if (!this->IsInTunnel && !visible)
		this->KillAnim();

	this->IsInTunnel = !visible;
}

void AttachEffectClass::RefreshDuration(int durationOverride)
{
	if (durationOverride)
		this->Duration = durationOverride;
	else
		this->Duration = this->DurationOverride ? this->DurationOverride : this->Type->Duration;

	if (this->Type->Animation_ResetOnReapply)
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

bool AttachEffectClass::AllowedToBeActive() const
{
	auto const pTechno = this->Techno;

	if (auto const pFoot = abstract_cast<FootClass*>(pTechno))
	{
		bool isMoving = pFoot->Locomotor->Is_Moving();

		if (isMoving && (this->Type->DiscardOn & DiscardCondition::Move) != DiscardCondition::None)
			return false;

		if (!isMoving && (this->Type->DiscardOn & DiscardCondition::Stationary) != DiscardCondition::None)
			return false;
	}

	if (pTechno->DrainingMe && (this->Type->DiscardOn & DiscardCondition::Drain) != DiscardCondition::None)
		return false;

	if (pTechno->Target)
	{
		bool inRange = (this->Type->DiscardOn & DiscardCondition::InRange) != DiscardCondition::None;
		bool outOfRange = (this->Type->DiscardOn & DiscardCondition::OutOfRange) != DiscardCondition::None;

		if (inRange || outOfRange)
		{
			int distance = -1;

			if (this->Type->DiscardOn_RangeOverride.isset())
			{
				distance = this->Type->DiscardOn_RangeOverride.Get();
			}
			else
			{
				int weaponIndex = pTechno->SelectWeapon(pTechno->Target);
				auto const pWeapon = pTechno->GetWeapon(weaponIndex)->WeaponType;

				if (pWeapon)
					distance = pWeapon->Range;
			}

			int distanceFromTgt = pTechno->DistanceFrom(pTechno->Target);

			if ((inRange && distanceFromTgt <= distance) || (outOfRange && distanceFromTgt >= distance))
				return false;
		}
	}

	return true;
}

bool AttachEffectClass::IsActive() const
{
	if (this->IsSelfOwned())
		return this->InitialDelay <= 0 && this->CurrentDelay == 0 && this->HasInitialized && this->IsOnline && !this->NeedsDurationRefresh;
	else
		return this->Duration && this->IsOnline;
}

bool AttachEffectClass::IsFromSource(TechnoClass* pInvoker, AbstractClass* pSource) const
{
	return pInvoker == this->Invoker && pSource == this->Source;
}

#pragma region StaticFunctions_AttachDetachTransfer

/// <summary>
/// Creates and attaches AttachEffect of a given type to a techno.
/// </summary>
/// <param name="pType">AttachEffect type.</param>
/// <param name="pTarget">Target techno.</param>
/// <param name="pInvokerHouse">House that invoked the attachment.</param>
/// <param name="pInvoker">Techno that invoked the attachment.</param>
/// <param name="pSource">Source object for the attachment e.g a Warhead or Techno.</param>
/// <param name="durationOverride">Override for AttachEffect duration.</param>
/// <param name="delay">Override for AttachEffect delay.</param>
/// <param name="initialDelay">Override for AttachEffect initial delay.</param>
/// <param name="recreationDelay">Override for AttachEffect recreation delay.</param>
/// <returns>True if successful, false if not.</returns>
bool AttachEffectClass::Attach(AttachEffectTypeClass* pType, TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
	AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay)
{
	if (!pType || !pTarget)
		return false;

	auto const pTargetExt = TechnoExt::ExtMap.Find(pTarget);

	if (auto const pAE = AttachEffectClass::CreateAndAttach(pType, pTarget, pTargetExt->AttachedEffects, pInvokerHouse, pInvoker, pSource, durationOverride, delay, initialDelay, recreationDelay))
	{
		if (initialDelay <= 0)
		{
			if (pType->ROFMultiplier > 0.0 && pType->ROFMultiplier_ApplyOnCurrentTimer)
			{
				pTarget->RearmTimer.Start(static_cast<int>(pTarget->RearmTimer.GetTimeLeft() * pType->ROFMultiplier));
				pTarget->ChargeTurretDelay = static_cast<int>(pTarget->ChargeTurretDelay * pType->ROFMultiplier);
			}

			pTargetExt->RecalculateStatMultipliers();

			if (pType->HasTint())
				pTarget->MarkForRedraw();
		}

		return true;
	}

	return false;
}

/// <summary>
/// Creates and attaches AttachEffects of given types to a techno.
/// </summary>
/// <param name="types">List of AttachEffect types.</param>
/// <param name="pTarget">Target techno.</param>
/// <param name="pInvokerHouse">House that invoked the attachment.</param>
/// <param name="pInvoker">Techno that invoked the attachment.</param>
/// <param name="pSource">Source object for the attachment e.g a Warhead or Techno.</param>
/// <param name="durationOverrides">Overrides for AttachEffect duration.</param>
/// <param name="delays">Overrides for AttachEffect delay.</param>
/// <param name="initialDelays">Overrides for AttachEffect initial delay.</param>
/// <param name="recreationDelays">Overrides for AttachEffect recreation delay.</param>
/// <returns>Number of AttachEffect instances created and attached.</returns>
int AttachEffectClass::Attach(std::vector<AttachEffectTypeClass*> const& types, TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
	AbstractClass* pSource, std::vector<int>& durationOverrides, std::vector<int> const& delays, std::vector<int> const& initialDelays, std::vector<int> const& recreationDelays)
{
	if (types.size() < 1 || !pTarget)
		return false;

	auto const pTargetExt = TechnoExt::ExtMap.Find(pTarget);

	int attachedCount = 0;
	bool markForRedraw = false;
	double ROFModifier = 1.0;

	for (size_t i = 0; i < types.size(); i++)
	{
		auto const pType = types[i];
		int durationOverride = 0;
		int delay = 0;
		int initialDelay = 0;
		int recreationDelay = -1;

		AttachEffectClass::SetValuesHelper(i, durationOverrides, delays, initialDelays, recreationDelays, durationOverride, delay, initialDelay, recreationDelay);

		if (auto const pAE = AttachEffectClass::CreateAndAttach(pType, pTarget, pTargetExt->AttachedEffects, pInvokerHouse, pInvoker, pSource, durationOverride, delay, initialDelay, recreationDelay))
		{
			attachedCount++;

			if (initialDelay <= 0)
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
/// <param name="durationOverride">Override for AttachEffect duration.</param>
/// <param name="delay">Override for AttachEffect delay.</param>
/// <param name="initialDelay">Override for AttachEffect initial delay.</param>
/// <param name="recreationDelay">Override for AttachEffect recreation delay.</param>
/// <returns>The created and attached AttachEffect if successful, nullptr if not.</returns>
AttachEffectClass* AttachEffectClass::CreateAndAttach(AttachEffectTypeClass* pType, TechnoClass* pTarget, std::vector<std::unique_ptr<AttachEffectClass>>& targetAEs,
	HouseClass* pInvokerHouse, TechnoClass* pInvoker, AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay)
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
	AttachEffectClass* sourceMatch = nullptr;

	for (auto const& aePtr : targetAEs)
	{
		auto const attachEffect = aePtr.get();

		if (attachEffect->GetType() == pType)
		{
			currentTypeCount++;
			match = attachEffect;

			if (attachEffect->Source == pSource && attachEffect->Invoker == pInvoker)
				sourceMatch = attachEffect;
		}
	}

	if (pType->Cumulative && pType->Cumulative_MaxCount >= 0 && currentTypeCount >= pType->Cumulative_MaxCount)
	{
		if (sourceMatch)
			sourceMatch->RefreshDuration(durationOverride);

		return nullptr;
	}

	if (!pType->Cumulative && currentTypeCount > 0 && match)
		match->RefreshDuration(durationOverride);
	else
	{
		targetAEs.push_back(std::make_unique<AttachEffectClass>(pType, pTarget, pInvokerHouse, pInvoker, pSource, durationOverride, delay, initialDelay, recreationDelay));
		auto const pAE = targetAEs.back().get();

		if (!currentTypeCount && pType->Cumulative && pType->CumulativeAnimations.size() > 0)
			pAE->HasCumulativeAnim = true;

		return pAE;
	}

	return nullptr;
}

/// <summary>
/// Remove AttachEffects of given type from techno.
/// </summary>
/// <param name="types">AttachEffect type.</param>
/// <param name="pTarget">Target techno.</param>
/// <param name="minCount">Minimum instance count needed for cumulative type to be removed.</param>
/// <param name="maxCount">Maximum instance count of cumulative type to be removed.</param>
/// <returns>Number of AttachEffect instances removed.</returns>
int AttachEffectClass::Detach(AttachEffectTypeClass* pType, TechnoClass* pTarget, int minCount, int maxCount)
{
	if (!pType || !pTarget)
		return 0;

	auto const pTargetExt = TechnoExt::ExtMap.Find(pTarget);
	int detachedCount = AttachEffectClass::RemoveAllOfType(pType, pTarget, minCount, maxCount);

	if (detachedCount > 0)
	{
		pTargetExt->RecalculateStatMultipliers();

		if (pType->HasTint())
			pTarget->MarkForRedraw();
	}

	return detachedCount;
}

/// <summary>
/// Remove all AttachEffects matching given types from techno.
/// </summary>
/// <param name="types">List of AttachEffect types.</param>
/// <param name="pTarget">Target techno.</param>
/// <param name="minCounts">Minimum instance counts needed for cumulative types to be removed.</param>
/// <param name="maxCounts">Maximum instance counts of cumulative types to be removed.</param>
/// <returns>Number of AttachEffect instances removed.</returns>
int AttachEffectClass::Detach(std::vector<AttachEffectTypeClass*> const& types, TechnoClass* pTarget, std::vector<int> const& minCounts, std::vector<int> const& maxCounts)
{
	if (types.size() < 1 || !pTarget)
		return 0;

	auto const pTargetExt = TechnoExt::ExtMap.Find(pTarget);
	int detachedCount = 0;
	bool markForRedraw = false;
	size_t index = 0, minSize = minCounts.size(), maxSize = maxCounts.size();

	for (auto const pType : types)
	{
		int minCount = minSize > 0 ? (index < minSize ? minCounts.at(index) : minCounts.at(minSize-1)) : -1;
		int maxCount = maxSize > 0 ? (index < maxSize ? maxCounts.at(index) : maxCounts.at(maxSize - 1)) : -1;

		int count = AttachEffectClass::RemoveAllOfType(pType, pTarget, minCount, maxCount);

		if (count && pType->HasTint())
			markForRedraw = true;

		detachedCount += count;
		index++;
	}

	if (detachedCount > 0)
		pTargetExt->RecalculateStatMultipliers();

	if (markForRedraw)
		pTarget->MarkForRedraw();

	return detachedCount;
}

/// <summary>
/// Remove all AttachEffects matching given groups from techno.
/// </summary>
/// <param name="groups">List of group ID's.</param>
/// <param name="pTarget">Target techno.</param>
/// <param name="minCounts">Minimum instance counts needed for cumulative types to be removed.</param>
/// <param name="maxCounts">Maximum instance counts of cumulative types to be removed.</param>
/// <returns>Number of AttachEffect instances removed.</returns>
int AttachEffectClass::DetachByGroups(std::vector<std::string> const& groups, TechnoClass* pTarget, std::vector<int> const& minCounts, std::vector<int> const& maxCounts)
{
	if (groups.size() < 1 || !pTarget)
		return 0;

	auto const pTargetExt = TechnoExt::ExtMap.Find(pTarget);
	std::vector<AttachEffectTypeClass*> types;

	for (auto const& attachEffect : pTargetExt->AttachedEffects)
	{
		auto const pType = attachEffect->Type;

		if (pType->HasGroups(groups, false))
			types.push_back(pType);
	}

	return Detach(types, pTarget, minCounts, maxCounts);
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
	std::vector<WeaponTypeClass*> expireWeapons;

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
				if (!pType->Cumulative || !pType->ExpireWeapon_CumulativeOnlyOnce || pTargetExt->GetAttachedEffectCumulativeCount(pType) < 2)
					expireWeapons.push_back(pType->ExpireWeapon);
			}


			if (pType->Cumulative && pType->CumulativeAnimations.size() > 0)
				pTargetExt->UpdateCumulativeAttachEffects(pType, attachEffect);

			if (attachEffect->ResetIfRecreatable())
			{
				++it;
				continue;
			}

			it = targetAEs->erase(it);
		}
		else
		{
			++it;
		}
	}


	auto const coords = pTarget->GetCoords();
	auto const pOwner = pTarget->Owner;

	for (auto const& pWeapon : expireWeapons)
	{
		WeaponTypeExt::DetonateAt(pWeapon, coords, pTarget, pOwner, pTarget);
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
			if (auto const pAE = AttachEffectClass::CreateAndAttach(type, pTarget, pTargetExt->AttachedEffects, attachEffect->InvokerHouse, attachEffect->Invoker, attachEffect->Source, attachEffect->DurationOverride))
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

