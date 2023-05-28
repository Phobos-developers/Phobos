#include "AttachEffectClass.h"

#include <AnimClass.h>
#include <BuildingClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>

std::vector<AttachEffectClass*> AttachEffectClass::Array;

AttachEffectClass::AttachEffectClass() : Type { nullptr }, Techno { nullptr }, InvokerHouse { nullptr }, Invoker { nullptr }, Source { nullptr }, DurationOverride { 0 }, Delay { 0 }, InitialDelay { 0 }, RecreationDelay { -1 }
, Duration { 0 }
, CurrentDelay { 0 }
{
	this->HasInitialized = false;
	AttachEffectClass::Array.emplace_back(this);
}

AttachEffectClass::AttachEffectClass(AttachEffectTypeClass* pType, TechnoClass* pTechno,
	HouseClass* pInvokerHouse, TechnoClass* pInvoker, AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay) :
	Type { pType }, Techno { pTechno }, InvokerHouse { pInvokerHouse }, Invoker { pInvoker }, Source { pSource },
	DurationOverride { durationOverride }, Delay { delay }, InitialDelay { initialDelay }, RecreationDelay { recreationDelay }
	, Duration { 0 }
	, CurrentDelay { 0 }
	, Animation { nullptr }
	, IsAnimHidden { false }
	, IsUnderTemporal { false }
	, IsOnline { true }
	, IsCloaked { false }
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
	else if (absType == AbstractType::House)
	{
		for (auto pEffect : AttachEffectClass::Array)
			AnnounceInvalidPointer(pEffect->InvokerHouse, ptr);
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
			pTechno->DiskLaserTimer.Start(static_cast<int>(pTechno->DiskLaserTimer.GetTimeLeft() * ROFModifier));
			pTechno->ROF = static_cast<int>(pTechno->ROF * ROFModifier);
		}

		if (this->Type->HasTint())
			this->Techno->MarkForRedraw();
	}

	if (this->CurrentDelay > 0)
	{
		this->CurrentDelay--;

		if (this->CurrentDelay == 0)
			this->RefreshDuration();

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
			KillAnim();
		else
			this->RefreshDuration();

		return;
	}

	if (this->IsUnderTemporal)
		this->IsUnderTemporal = false;

	this->CloakCheck();
	this->OnlineCheck();

	if (!this->Animation && !this->IsUnderTemporal && this->IsOnline && !this->IsCloaked && !this->IsAnimHidden)
		CreateAnim();
}

void AttachEffectClass::AI_Temporal()
{
	if (!this->IsUnderTemporal)
	{
		this->IsUnderTemporal = true;

		this->CloakCheck();

		if (!this->Animation && this->Type->Animation_TemporalAction != AttachedAnimFlag::Hides && this->IsOnline && !this->IsCloaked && !this->IsAnimHidden)
			CreateAnim();

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
		KillAnim();
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

	auto pAnimType = this->Type->Animation.Get(nullptr);

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

void AttachEffectClass::SetAnimationVisibility(bool visible)
{
	if (!this->IsAnimHidden && !visible)
		this->KillAnim();

	this->IsAnimHidden = !visible;
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
		this->CreateAnim();
	}
}

bool AttachEffectClass::IsSelfOwned() const
{
	return this->Source == this->Techno;
}

bool AttachEffectClass::HasExpired() const
{
	return IsSelfOwned() && this->Delay >= 0 ? false : !this->Duration;
}

bool AttachEffectClass::IsActive() const
{
	if (this->IsSelfOwned())
		return this->InitialDelay <= 0 && this->CurrentDelay == 0 && this->HasInitialized && this->IsOnline;
	else
		return this->Duration && this->IsOnline;
}

bool AttachEffectClass::IsFromSource(TechnoClass* pInvoker, AbstractClass* pSource) const
{
	return pInvoker == this->Invoker && pSource == this->Source;
}

AttachEffectTypeClass* AttachEffectClass::GetType() const
{
	return Type;
}

void AttachEffectClass::Attach(std::vector<AttachEffectTypeClass*> const& types, TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
	AbstractClass* pSource, std::vector<int>& durationOverrides, std::vector<int> const& delays, std::vector<int> const& initialDelays, std::vector<int> const& recreationDelays)
{
	if (types.size() < 1 || !pTarget)
		return;

	auto const pTargetExt = TechnoExt::ExtMap.Find(pTarget);

	int modifiedCount = 0;
	bool markForRedraw = false;
	double ROFModifier = 1.0;

	for (size_t i = 0; i < types.size(); i++)
	{
		auto const type = types[i];

		if (!type->PenetratesIronCurtain && pTarget->IsIronCurtained())
			continue;

		int currentTypeCount = 0;
		AttachEffectClass* match = nullptr;
		AttachEffectClass* sourceMatch = nullptr;

		for (auto const& aePtr : pTargetExt->AttachedEffects)
		{
			auto const attachEffect = aePtr.get();

			if (attachEffect->GetType() == type)
			{
				currentTypeCount++;
				match = attachEffect;

				if (attachEffect->Source == pSource && attachEffect->Invoker == pInvoker)
					sourceMatch = attachEffect;
			}
		}

		int durationOverride = 0;

		if (durationOverrides.size() > 0)
			durationOverride = durationOverrides[durationOverrides.size() > i ? i : durationOverrides.size() - 1];

		if (type->Cumulative && type->Cumulative_MaxCount >= 0 && currentTypeCount >= type->Cumulative_MaxCount)
		{
			if (sourceMatch)
				sourceMatch->RefreshDuration(durationOverride);
			else
				continue;
		}

		if (!type->Cumulative && currentTypeCount > 0 && match)
		{
			match->RefreshDuration(durationOverride);
		}
		else
		{
			int delay = 0;
			int initialDelay = 0;
			int recreationDelay = -1;

			if (delays.size() > 0)
				delay = delays[delays.size() > i ? i : delays.size() - 1];

			if (initialDelays.size() > 0)
				initialDelay = initialDelays[initialDelays.size() > i ? i : initialDelays.size() - 1];

			if (recreationDelays.size() > 0)
				recreationDelay = recreationDelays[recreationDelays.size() > i ? i : recreationDelays.size() - 1];

			pTargetExt->AttachedEffects.push_back(std::make_unique<AttachEffectClass>(type, pTarget, pInvokerHouse, pInvoker, pSource, durationOverride, delay, initialDelay, recreationDelay));
			modifiedCount++;

			if (initialDelay <= 0)
			{
				if (type->ROFMultiplier > 0.0 && type->ROFMultiplier_ApplyOnCurrentTimer)
					ROFModifier *= type->ROFMultiplier;

				if (type->HasTint())
					markForRedraw = true;
			}
		}
	}

	if (ROFModifier != 1.0)
	{
		pTarget->DiskLaserTimer.Start(static_cast<int>(pTarget->DiskLaserTimer.GetTimeLeft() * ROFModifier));
		pTarget->ROF = static_cast<int>(pTarget->ROF * ROFModifier);
	}

	if (modifiedCount > 0)
		pTargetExt->RecalculateStatMultipliers();

	if (markForRedraw)
		pTarget->MarkForRedraw();
}

void AttachEffectClass::Detach(std::vector<AttachEffectTypeClass*> const& types, TechnoClass* pTarget)
{
	if (types.size() < 1 || !pTarget)
		return;

	auto const pTargetExt = TechnoExt::ExtMap.Find(pTarget);
	int modifiedCount = 0;
	bool markForRedraw = false;
	std::vector<std::unique_ptr<AttachEffectClass>>::iterator it;

	for (it = pTargetExt->AttachedEffects.begin(); it != pTargetExt->AttachedEffects.end(); )
	{
		auto const attachEffect = it->get();
		bool contains = std::find(types.begin(), types.end(), attachEffect->GetType()) != types.end();

		if (contains)
		{
			if (attachEffect->IsSelfOwned() && attachEffect->RecreationDelay >= 0)
			{
				attachEffect->KillAnim();
				attachEffect->CurrentDelay = attachEffect->RecreationDelay;
				++it;
				continue;
			}

			if (attachEffect->GetType()->HasTint())
				markForRedraw = true;

			modifiedCount++;
			it = pTargetExt->AttachedEffects.erase(it);
		}
		else
		{
			++it;
		}
	}

	if (modifiedCount > 0)
		pTargetExt->RecalculateStatMultipliers();

	if (markForRedraw)
		pTarget->MarkForRedraw();
}

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
			match->Duration = Math::max(sourceMatch->Duration, attachEffect->Duration);
		}
		else
		{
			attachEffect->Techno = pTarget;
			pTargetExt->AttachedEffects.push_back(std::unique_ptr<AttachEffectClass>(attachEffect));
		}

		it = pSourceExt->AttachedEffects.erase(it);
	}
}

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
		.Process(this->IsUnderTemporal)
		.Process(this->IsOnline)
		.Process(this->IsCloaked)
		.Process(this->HasInitialized)
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

