#pragma once

#include <set>
#include <unordered_map>

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/TemplateDef.h>

class AttachEffectTypeClass final : public Enumerable<AttachEffectTypeClass>
{
	static std::unordered_map<const char*, std::set<AttachEffectTypeClass*>> GroupsMap;

public:
	Valueable<int> Duration;
	Valueable<bool> Cumulative;
	Valueable<int> Cumulative_MaxCount;
	Valueable<bool> Powered;
	Valueable<DiscardCondition> DiscardOn;
	Valueable<bool> PenetratesIronCurtain;
	Nullable<AnimTypeClass*> Animation;
	NullableVector<AnimTypeClass*> CumulativeAnimations;
	Valueable<bool> Animation_ResetOnReapply;
	Valueable<AttachedAnimFlag> Animation_OfflineAction;
	Valueable<AttachedAnimFlag> Animation_TemporalAction;
	Valueable<bool> Animation_UseInvokerAsOwner;
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
	Nullable<WeaponTypeClass*> RevengeWeapon;
	Valueable<AffectedHouse> RevengeWeapon_AffectsHouses;
	Valueable<bool> DisableWeapons;

	std::vector<const char*> Groups;

	AttachEffectTypeClass(const char* const pTitle) : Enumerable<AttachEffectTypeClass>(pTitle)
		, Duration { 0 }
		, Cumulative { false }
		, Cumulative_MaxCount { -1 }
		, Powered { false }
		, DiscardOn { DiscardCondition::None }
		, PenetratesIronCurtain { false }
		, Animation {}
		, CumulativeAnimations {}
		, Animation_ResetOnReapply { false }
		, Animation_OfflineAction { AttachedAnimFlag::Hides }
		, Animation_TemporalAction { AttachedAnimFlag::None }
		, Animation_UseInvokerAsOwner { false }
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
		, RevengeWeapon_AffectsHouses{ AffectedHouse::All }
		, DisableWeapons { false }
		, Groups {}
	{};

	bool HasTint();
	bool HasGroup(const char* pGroupID);
	bool HasGroups(std::vector<const char*> groupIDs, bool requireAll);
	AnimTypeClass* GetCumulativeAnimation(int cumulativeCount);

	virtual ~AttachEffectTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	static std::vector<AttachEffectTypeClass*> GetTypesFromGroups(std::vector<const char*> groupIDs);

private:
	template <typename T>
	void Serialize(T& Stm);
};

