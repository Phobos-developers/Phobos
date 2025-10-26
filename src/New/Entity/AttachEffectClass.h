// SPDX-License-Identifier: GPL-3.0-or-later
// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
#pragma once

#include <New/Type/AttachEffectTypeClass.h>

class LaserTrailClass;

class AttachEffectClass
{
public:
	static std::vector<AttachEffectClass*> Array;

	AttachEffectClass();

	AttachEffectClass(AttachEffectTypeClass* pType, TechnoClass* pTechno, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
		AbstractClass* pSource, int durationOverride, int delay, int initialDelay, int recreationDelay);

	~AttachEffectClass();

	void AI();
	void AI_Temporal();
	void KillAnim();
	void CreateAnim();
	void UpdateCumulativeAnim();
	void TransferCumulativeAnim(AttachEffectClass* pSource);
	bool CanShowAnim() const;
	void SetAnimationTunnelState(bool visible);
	AttachEffectTypeClass* GetType() const { return this->Type; }
	int GetRemainingDuration() const { return this->Duration; }
	void RefreshDuration(int durationOverride = 0);
	bool ResetIfRecreatable();
	bool IsSelfOwned() const { return this->Source == this->Techno; }
	bool HasExpired() const;
	bool ShouldBeDiscardedNow();
	bool IsActive() const;
	bool IsActiveIgnorePowered() const;
	bool IsFromSource(TechnoClass* pInvoker, AbstractClass* pSource) const;
	TechnoClass* GetInvoker() const;

	static void PointerGotInvalid(void* ptr, bool removed);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	static int Attach(TechnoClass* pTarget, HouseClass* pInvokerHouse, TechnoClass* pInvoker, AbstractClass* pSource, AEAttachInfoTypeClass const& attachEffectInfo);
	static int Detach(TechnoClass* pTarget, AEAttachInfoTypeClass const& attachEffectInfo);
	static int DetachByGroups(TechnoClass* pTarget, AEAttachInfoTypeClass const& attachEffectInfo);
	static void TransferAttachedEffects(TechnoClass* pSource, TechnoClass* pTarget);

private:
	void OnlineCheck();
	void CloakCheck();
	void AnimCheck();

	static AttachEffectClass* CreateAndAttach(AttachEffectTypeClass* pType, TechnoClass* pTarget, std::vector<std::unique_ptr<AttachEffectClass>>& targetAEs, HouseClass* pInvokerHouse, TechnoClass* pInvoker,
		AbstractClass* pSource, AEAttachParams const& attachInfo);

	static int DetachTypes(TechnoClass* pTarget, AEAttachInfoTypeClass const& attachEffectInfo, std::vector<AttachEffectTypeClass*> const& types);
	static int RemoveAllOfType(AttachEffectTypeClass* pType, TechnoClass* pTarget, int minCount, int maxCount);

	template <typename T>
	bool Serialize(T& Stm);

	int Duration;
	int DurationOverride;
	int Delay;
	int CurrentDelay;
	int InitialDelay;
	int RecreationDelay;
	AttachEffectTypeClass* Type;
	TechnoClass* Techno;
	HouseClass* InvokerHouse;
	TechnoClass* Invoker;
	AbstractClass* Source;
	AnimClass* Animation;
	bool IsAnimHidden;
	bool IsInTunnel;
	bool IsUnderTemporal;
	bool IsOnline;
	bool IsCloaked;
	bool HasInitialized;
	bool NeedsDurationRefresh;
	int LastDiscardCheckFrame;
	bool LastDiscardCheckValue;
	bool LastActiveStat;
	LaserTrailClass* LaserTrail;

public:
	bool HasCumulativeAnim;
	bool ShouldBeDiscarded;
	bool NeedsRecalculateStat;
};

// Container for TechnoClass-specific AttachEffect fields.
struct AttachEffectTechnoProperties
{
	double FirepowerMultiplier;
	double ArmorMultiplier;
	double SpeedMultiplier;
	double ROFMultiplier;
	bool Cloakable;
	bool ForceDecloak;
	bool DisableWeapons;
	bool Unkillable;
	bool HasRangeModifier;
	bool HasTint;
	bool ReflectDamage;
	bool HasOnFireDiscardables;
	bool HasRestrictedArmorMultipliers;

	AttachEffectTechnoProperties() :
		FirepowerMultiplier { 1.0 }
		, ArmorMultiplier { 1.0 }
		, SpeedMultiplier { 1.0 }
		, ROFMultiplier { 1.0 }
		, Cloakable { false }
		, ForceDecloak { false }
		, DisableWeapons { false }
		, Unkillable { false }
		, HasRangeModifier { false }
		, HasTint { false }
		, ReflectDamage { false }
		, HasOnFireDiscardables { false }
		, HasRestrictedArmorMultipliers { false }
	{ }
};
