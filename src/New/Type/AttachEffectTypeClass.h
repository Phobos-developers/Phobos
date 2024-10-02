#pragma once

#include <set>
#include <unordered_map>

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>

class AttachEffectTypeClass final : public Enumerable<AttachEffectTypeClass>
{
	static std::unordered_map<std::string, std::set<AttachEffectTypeClass*>> GroupsMap;

public:
	Valueable<int> Duration;
	Valueable<bool> Cumulative;
	Valueable<int> Cumulative_MaxCount;
	Valueable<bool> Powered;
	Valueable<DiscardCondition> DiscardOn;
	Nullable<Leptons> DiscardOn_RangeOverride;
	Valueable<bool> PenetratesIronCurtain;
	Nullable<bool> PenetratesForceShield;
	Valueable<AnimTypeClass*> Animation;
	ValueableVector<AnimTypeClass*> CumulativeAnimations;
	Valueable<bool> CumulativeAnimations_RestartOnChange;
	Valueable<bool> Animation_ResetOnReapply;
	Valueable<AttachedAnimFlag> Animation_OfflineAction;
	Valueable<AttachedAnimFlag> Animation_TemporalAction;
	Valueable<bool> Animation_UseInvokerAsOwner;
	ValueableVector<AttachEffectTypeClass*> Animation_HideIfAttachedWith;
	Valueable<WeaponTypeClass*> ExpireWeapon;
	Valueable<ExpireWeaponCondition> ExpireWeapon_TriggerOn;
	Valueable<bool> ExpireWeapon_CumulativeOnlyOnce;
	Nullable<ColorStruct> Tint_Color;
	Valueable<double> Tint_Intensity;
	Valueable<AffectedHouse> Tint_VisibleToHouses;
	Valueable<double> FirepowerMultiplier;
	Valueable<double> ArmorMultiplier;
	Valueable<double> SpeedMultiplier;
	Valueable<double> ROFMultiplier;
	Valueable<bool> ROFMultiplier_ApplyOnCurrentTimer;
	Valueable<bool> Cloakable;
	Valueable<bool> ForceDecloak;
	Valueable<double> WeaponRange_Multiplier;
	Valueable<double> WeaponRange_ExtraRange;
	ValueableVector<WeaponTypeClass*> WeaponRange_AllowWeapons;
	ValueableVector<WeaponTypeClass*> WeaponRange_DisallowWeapons;
	Valueable<double> Crit_Multiplier;
	Valueable<double> Crit_ExtraChance;
	ValueableVector<WarheadTypeClass*> Crit_AllowWarheads;
	ValueableVector<WarheadTypeClass*> Crit_DisallowWarheads;
	Valueable<WeaponTypeClass*> RevengeWeapon;
	Valueable<AffectedHouse> RevengeWeapon_AffectsHouses;
	Valueable<bool> ReflectDamage;
	Nullable<WarheadTypeClass*> ReflectDamage_Warhead;
	Valueable<bool> ReflectDamage_Warhead_Detonate;
	Valueable<double> ReflectDamage_Multiplier;
	Valueable<AffectedHouse> ReflectDamage_AffectsHouses;
	Valueable<bool> DisableWeapons;

	std::vector<std::string> Groups;

	AttachEffectTypeClass(const char* const pTitle) : Enumerable<AttachEffectTypeClass>(pTitle)
		, Duration { 0 }
		, Cumulative { false }
		, Cumulative_MaxCount { -1 }
		, Powered { false }
		, DiscardOn { DiscardCondition::None }
		, DiscardOn_RangeOverride {}
		, PenetratesIronCurtain { false }
		, PenetratesForceShield {}
		, Animation {}
		, CumulativeAnimations {}
		, CumulativeAnimations_RestartOnChange { true }
		, Animation_ResetOnReapply { false }
		, Animation_OfflineAction { AttachedAnimFlag::Hides }
		, Animation_TemporalAction { AttachedAnimFlag::None }
		, Animation_UseInvokerAsOwner { false }
		, Animation_HideIfAttachedWith {}
		, ExpireWeapon {}
		, ExpireWeapon_TriggerOn { ExpireWeaponCondition::Expire }
		, ExpireWeapon_CumulativeOnlyOnce { false }
		, Tint_Color {}
		, Tint_Intensity { 0.0 }
		, Tint_VisibleToHouses { AffectedHouse::All }
		, FirepowerMultiplier { 1.0 }
		, ArmorMultiplier { 1.0 }
		, SpeedMultiplier { 1.0 }
		, ROFMultiplier { 1.0 }
		, ROFMultiplier_ApplyOnCurrentTimer { true }
		, Cloakable { false }
		, ForceDecloak { false }
		, WeaponRange_Multiplier { 1.0 }
		, WeaponRange_ExtraRange { 0.0 }
		, WeaponRange_AllowWeapons {}
		, WeaponRange_DisallowWeapons {}
		, Crit_Multiplier { 1.0 }
		, Crit_ExtraChance { 0.0 }
		, Crit_AllowWarheads {}
		, Crit_DisallowWarheads {}
		, RevengeWeapon {}
		, RevengeWeapon_AffectsHouses { AffectedHouse::All }
		, ReflectDamage { false }
		, ReflectDamage_Warhead {}
		, ReflectDamage_Warhead_Detonate { false }
		, ReflectDamage_Multiplier { 1.0 }
		, ReflectDamage_AffectsHouses { AffectedHouse::All }
		, DisableWeapons { false }
		, Groups {}
	{};

	bool HasTint() const
	{
		return this->Tint_Color.isset() || this->Tint_Intensity != 0.0;
	}

	bool HasGroup(const std::string& groupID) const;
	bool HasGroups(const std::vector<std::string>& groupIDs, bool requireAll) const;
	AnimTypeClass* GetCumulativeAnimation(int cumulativeCount) const;

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	static void Clear()
	{
		AttachEffectTypeClass::GroupsMap.clear();
	}

	static std::vector<AttachEffectTypeClass*> GetTypesFromGroups(const std::vector<std::string>& groupIDs);

private:
	template <typename T>
	void Serialize(T& Stm);
	void AddToGroupsMap();
};
