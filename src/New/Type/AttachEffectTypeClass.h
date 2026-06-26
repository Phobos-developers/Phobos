#pragma once

#include <set>
#include <unordered_map>

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include "LaserTrailTypeClass.h"

// AE discard condition
enum class DiscardCondition : unsigned char
{
	None = 0x0,
	Entry = 0x1,
	Move = 0x2,
	Stationary = 0x4,
	Drain = 0x8,
	InRange = 0x10,
	OutOfRange = 0x20,
	Firing = 0x40
};

MAKE_ENUM_FLAGS(DiscardCondition);

// AE expire weapon condition
enum class ExpireWeaponCondition : unsigned char
{
	None = 0x0,
	Expire = 0x1,
	Remove = 0x2,
	Death = 0x4,
	Discard = 0x8,

	All = 0xFF,
};

MAKE_ENUM_FLAGS(ExpireWeaponCondition);

enum class VectorOrigin : int
{
	Self = 0,
	Launcher = 1,
	Target = 2,
	Source = 3
};

class AttachEffectTypeClass final : public Enumerable<AttachEffectTypeClass>
{
	static std::unordered_map<std::string, std::set<AttachEffectTypeClass*>> GroupsMap;

public:
	Valueable<int> Duration;
	Valueable<bool> Duration_ApplyFirepowerMult;
	Valueable<bool> Duration_ApplyArmorMultOnTarget;
	Valueable<bool> Cumulative;
	Valueable<int> Cumulative_MaxCount;
	Valueable<bool> Powered;
	Valueable<DiscardCondition> DiscardOn;
	Nullable<Leptons> DiscardOn_RangeOverride;
	Nullable<bool> DiscardOn_MoveBasedOnDestination;
	Valueable<bool> PenetratesIronCurtain;
	Nullable<bool> PenetratesForceShield;
	ValueableVector<TechnoTypeClass*> AffectTypes;
	ValueableVector<TechnoTypeClass*> IgnoreTypes;
	Valueable<AffectedTarget> AffectsTarget;
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
	Valueable<bool> ExpireWeapon_UseInvokerAsOwner;
	ValueableVector<AttachEffectTypeClass*> Next;
	Nullable<ColorStruct> Tint_Color;
	Valueable<double> Tint_Intensity;
	Valueable<AffectedHouse> Tint_VisibleToHouses;
	Valueable<double> FirepowerMultiplier;
	Valueable<double> ArmorMultiplier;
	ValueableVector<WarheadTypeClass*> ArmorMultiplier_AllowWarheads;
	ValueableVector<WarheadTypeClass*> ArmorMultiplier_DisallowWarheads;
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
	Valueable<AffectedHouse> RevengeWeapon_AffectsHouse;
	Valueable<bool> RevengeWeapon_UseInvokerAsOwner;
	Valueable<bool> ReflectDamage;
	Nullable<WarheadTypeClass*> ReflectDamage_Warhead;
	Valueable<bool> ReflectDamage_Warhead_Detonate;
	Valueable<double> ReflectDamage_Multiplier;
	Valueable<AffectedHouse> ReflectDamage_AffectsHouse;
	Valueable<double> ReflectDamage_Chance;
	Nullable<int> ReflectDamage_Override;
	Valueable<bool> ReflectDamage_UseInvokerAsOwner;
	Valueable<bool> DisableWeapons;
	Valueable<bool> Unkillable;
	ValueableIdx<LaserTrailTypeClass> LaserTrail_Type;

	Valueable<int> Vector_TimeStep;
	Valueable<int> Vector_DisabledFrames;
	Valueable<bool> Vector_SyncFacing;
	Valueable<bool> Vector_OriginIsOnWorld;
	Valueable<bool> Vector_OriginIsOnBody;
	Valueable<VectorOrigin> Vector_Origin;
	Nullable<CoordStruct> Vector_OriginFLH;
	Valueable<bool> Vector_OriginNoUpdate;
	Valueable<bool> Vector_Force;
	Valueable<bool> Vector_Freeze;
	Valueable<bool> Vector_AllowedTilt;
	Nullable<CoordStruct> Vector_NormalVector;
	Valueable<double> Vector_NormalRandomF;
	Valueable<double> Vector_NormalRandomL;
	Valueable<double> Vector_NormalRandomH;
	Valueable<double> Vector_NormalFAnglePerStep;
	Valueable<double> Vector_NormalLAnglePerStep;
	Valueable<double> Vector_NormalHAnglePerStep;
	Valueable<CoordStruct> Vector_MoveTo;
	Nullable<CoordStruct> Vector_GrowRate;
	Valueable<double> Vector_AnglePerStep;
	Valueable<int> Vector_CircleRadius;
	Valueable<int> Vector_CircleSpeed;
	Valueable<int> Vector_CircleSpeedAcceleration;
	Valueable<int> Vector_CircleMaxSpeed;
	Valueable<int> Vector_CircleMinSpeed;
	Valueable<double> Vector_CircleAnglePerStep;
	Valueable<double> Vector_CircleAngleAcceleration;
	Nullable<CoordStruct> Vector_CircleOrigin;
	Valueable<bool> Vector_AllowOriginTilt;
	Valueable<int> Vector_CircleRadiusGrow;
	Valueable<int> Vector_CircleMaxRadius;
	Valueable<int> Vector_CircleMinRadius;
	Valueable<bool> Vector_CircleEndOnMaxRadius;
	Valueable<bool> Vector_CircleEndOnMinRadius;
	Nullable<CoordStruct> Vector_TargetFLH;
	Valueable<bool> Vector_ReachTarget;
	Valueable<int> Vector_ReachTargetEarlyEnd;
	Valueable<int> Vector_ArcHeight;
	Valueable<double> Vector_ArcPeakPercent;
	Valueable<double> Vector_ArcPeakRandomPercent;
	Valueable<double> Vector_ArcPeakRandomPercentMin;
	Valueable<double> Vector_ArcPeakRandomPercentMax;
	Valueable<double> Vector_ArcRotation;
	Valueable<int> Vector_InitialSpeed;
	Valueable<int> Vector_MaxSpeed;
	Valueable<int> Vector_MinSpeed;
	Valueable<int> Vector_Acceleration;
	Valueable<bool> Vector_AllowFallingDestroy;
	Valueable<int> Vector_FallingDestroyHeight;
	Valueable<bool> Vector_AffectTechno;
	Valueable<bool> Vector_AffectBullets;
	Valueable<int> Vector_CircleRandomRadiusMin;
	Valueable<int> Vector_CircleRandomRadiusMax;
	Valueable<double> Vector_CircleRandomAngleMin;
	Valueable<double> Vector_CircleRandomAngleMax;
	Valueable<double> Vector_CircleMaxAngle;
	Valueable<double> Vector_CircleMinAngle;
	Valueable<int> Vector_TargetOffsetFMin;
	Valueable<int> Vector_TargetOffsetFMax;
	Valueable<int> Vector_TargetOffsetLMin;
	Valueable<int> Vector_TargetOffsetLMax;
	Valueable<int> Vector_TargetOffsetHMin;
	Valueable<int> Vector_TargetOffsetHMax;
	Valueable<int> Vector_ArcRandomHeightMin;
	Valueable<int> Vector_ArcRandomHeightMax;
	Valueable<double> Vector_ArcRandomRotationMin;
	Valueable<double> Vector_ArcRandomRotationMax;
	Valueable<int> Vector_RandomSpeedMin;
	Valueable<int> Vector_RandomSpeedMax;
	Nullable<CoordStruct> Vector_OriginMoveTo;
	Nullable<CoordStruct> Vector_OriginGrowRate;
	Nullable<CoordStruct> Vector_OriginTargetFLH;
	Valueable<int> Vector_OriginInitialSpeed;
	Valueable<bool> Vector_OriginReachTarget;
	Valueable<int> Vector_OriginArcHeight;
	Valueable<int> Vector_OriginCircleRadius;
	Valueable<int> Vector_OriginCircleSpeed;
	Valueable<double> Vector_OriginCircleAnglePerStep;
	Valueable<int> Vector_OriginCircleRadiusGrow;
	Valueable<int> Vector_OriginCircleMaxRadius;
	Valueable<int> Vector_OriginCircleMinRadius;
	Valueable<bool> Vector_OriginCircleEndOnMaxRadius;
	Valueable<bool> Vector_OriginCircleEndOnMinRadius;
	Nullable<CoordStruct> Vector_OriginNormalVector;
	Valueable<double> Vector_OriginNormalFAnglePerStep;
	Valueable<double> Vector_OriginNormalLAnglePerStep;
	Valueable<double> Vector_OriginNormalHAnglePerStep;
	Valueable<bool> Vector_OriginAllowedTilt;
	Nullable<CoordStruct> Vector_OriginCircleOffset;
	Valueable<VectorOrigin> Vector_OriginOrigin;
	Nullable<CoordStruct> Vector_OriginOriginFLH;
	Valueable<double> Vector_NormalFAngleRMin;
	Valueable<double> Vector_NormalFAngleRMax;
	Valueable<double> Vector_NormalFAngleRMin2;
	Valueable<double> Vector_NormalFAngleRMax2;
	Valueable<double> Vector_NormalLAngleRMin;
	Valueable<double> Vector_NormalLAngleRMax;
	Valueable<double> Vector_NormalLAngleRMin2;
	Valueable<double> Vector_NormalLAngleRMax2;
	Valueable<double> Vector_NormalHAngleRMin;
	Valueable<double> Vector_NormalHAngleRMax;
	Valueable<double> Vector_NormalHAngleRMin2;
	Valueable<double> Vector_NormalHAngleRMax2;
	Valueable<double> Vector_OriginNormalFAngleRMin;
	Valueable<double> Vector_OriginNormalFAngleRMax;
	Valueable<double> Vector_OriginNormalFAngleRMin2;
	Valueable<double> Vector_OriginNormalFAngleRMax2;
	Valueable<double> Vector_OriginNormalLAngleRMin;
	Valueable<double> Vector_OriginNormalLAngleRMax;
	Valueable<double> Vector_OriginNormalLAngleRMin2;
	Valueable<double> Vector_OriginNormalLAngleRMax2;
	Valueable<double> Vector_OriginNormalHAngleRMin;
	Valueable<double> Vector_OriginNormalHAngleRMax;
	Valueable<double> Vector_OriginNormalHAngleRMin2;
	Valueable<double> Vector_OriginNormalHAngleRMax2;

	std::vector<std::string> Groups;

	AttachEffectTypeClass(const char* const pTitle) : Enumerable<AttachEffectTypeClass>(pTitle)
		, Duration { 0 }
		, Duration_ApplyFirepowerMult { false }
		, Duration_ApplyArmorMultOnTarget { false }
		, Cumulative { false }
		, Cumulative_MaxCount { -1 }
		, Powered { false }
		, DiscardOn { DiscardCondition::None }
		, DiscardOn_RangeOverride {}
		, DiscardOn_MoveBasedOnDestination {}
		, PenetratesIronCurtain { false }
		, PenetratesForceShield {}
		, AffectTypes {}
		, IgnoreTypes {}
		, AffectsTarget { AffectedTarget::All }
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
		, ExpireWeapon_UseInvokerAsOwner { false }
		, Next {}
		, Tint_Color {}
		, Tint_Intensity { 0.0 }
		, Tint_VisibleToHouses { AffectedHouse::All }
		, FirepowerMultiplier { 1.0 }
		, ArmorMultiplier { 1.0 }
		, ArmorMultiplier_AllowWarheads {}
		, ArmorMultiplier_DisallowWarheads {}
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
		, RevengeWeapon_AffectsHouse { AffectedHouse::All }
		, ReflectDamage { false }
		, RevengeWeapon_UseInvokerAsOwner { false }
		, ReflectDamage_Warhead {}
		, ReflectDamage_Warhead_Detonate { false }
		, ReflectDamage_Multiplier { 1.0 }
		, ReflectDamage_AffectsHouse { AffectedHouse::All }
		, ReflectDamage_Chance { 1.0 }
		, ReflectDamage_Override {}
		, ReflectDamage_UseInvokerAsOwner { false }
		, DisableWeapons { false }
		, Unkillable { false }
		, LaserTrail_Type { -1 }
		, Vector_TimeStep { 1 }
		, Vector_DisabledFrames { 0 }
		, Vector_SyncFacing { true }
		, Vector_OriginIsOnWorld { false }
		, Vector_OriginIsOnBody { false }
		, Vector_Origin { VectorOrigin::Self }
		, Vector_OriginFLH {}
		, Vector_OriginNoUpdate { false }
		, Vector_Force { true }
		, Vector_Freeze { false }
		, Vector_AllowedTilt { false }
		, Vector_NormalVector {}
		, Vector_NormalRandomF { 0.0 }
		, Vector_NormalRandomL { 0.0 }
		, Vector_NormalRandomH { 0.0 }
		, Vector_NormalFAnglePerStep { 0.0 }
		, Vector_NormalLAnglePerStep { 0.0 }
		, Vector_NormalHAnglePerStep { 0.0 }
		, Vector_MoveTo { CoordStruct::Empty }
		, Vector_GrowRate {}
		, Vector_AnglePerStep { 0.0 }
		, Vector_CircleRadius { -1 }
		, Vector_CircleSpeed { 0 }
		, Vector_CircleSpeedAcceleration { 0 }
		, Vector_CircleMaxSpeed { 0 }
		, Vector_CircleMinSpeed { 0 }
		, Vector_CircleAnglePerStep { 0.0 }
		, Vector_CircleAngleAcceleration { 0.0 }
		, Vector_CircleOrigin {}
		, Vector_AllowOriginTilt { false }
		, Vector_CircleRadiusGrow { 0 }
		, Vector_CircleMaxRadius { 0 }
		, Vector_CircleMinRadius { 0 }
		, Vector_CircleEndOnMaxRadius { false }
		, Vector_CircleEndOnMinRadius { false }
		, Vector_TargetFLH {}
		, Vector_ReachTarget { false }
		, Vector_ReachTargetEarlyEnd { 0 }
		, Vector_ArcHeight { 0 }
		, Vector_ArcPeakPercent { 0.0 }
		, Vector_ArcPeakRandomPercent { 0.0 }
		, Vector_ArcPeakRandomPercentMin { 0.0 }
		, Vector_ArcPeakRandomPercentMax { 0.0 }
		, Vector_ArcRotation { 0.0 }
		, Vector_InitialSpeed { -1 }
		, Vector_MaxSpeed { -1 }
		, Vector_MinSpeed { -1 }
		, Vector_Acceleration { 0 }
		, Vector_AllowFallingDestroy { false }
		, Vector_FallingDestroyHeight { 0 }
		, Vector_AffectTechno { true }
		, Vector_AffectBullets { false }
		, Vector_CircleRandomRadiusMin { -1 }
		, Vector_CircleRandomRadiusMax { -1 }
		, Vector_CircleRandomAngleMin { -1 }
		, Vector_CircleRandomAngleMax { -1 }
		, Vector_CircleMaxAngle { 0.0 }
		, Vector_CircleMinAngle { 0.0 }
		, Vector_TargetOffsetFMin { 0 }
		, Vector_TargetOffsetFMax { 0 }
		, Vector_TargetOffsetLMin { 0 }
		, Vector_TargetOffsetLMax { 0 }
		, Vector_TargetOffsetHMin { 0 }
		, Vector_TargetOffsetHMax { 0 }
		, Vector_ArcRandomHeightMin { 0 }
		, Vector_ArcRandomHeightMax { 0 }
		, Vector_ArcRandomRotationMin { 0.0 }
		, Vector_ArcRandomRotationMax { 0.0 }
		, Vector_RandomSpeedMin { -1 }
		, Vector_RandomSpeedMax { -1 }
		, Vector_OriginMoveTo {}
		, Vector_OriginGrowRate {}
		, Vector_OriginTargetFLH {}
		, Vector_OriginInitialSpeed { -1 }
		, Vector_OriginReachTarget { false }
		, Vector_OriginArcHeight { 0 }
		, Vector_OriginCircleRadius { -1 }
		, Vector_OriginCircleSpeed { 0 }
		, Vector_OriginCircleAnglePerStep { 0.0 }
		, Vector_OriginCircleRadiusGrow { 0 }
		, Vector_OriginCircleMaxRadius { 0 }
		, Vector_OriginCircleMinRadius { 0 }
		, Vector_OriginCircleEndOnMaxRadius { false }
		, Vector_OriginCircleEndOnMinRadius { false }
		, Vector_OriginNormalVector {}
		, Vector_OriginNormalFAnglePerStep { 0.0 }
		, Vector_OriginNormalLAnglePerStep { 0.0 }
		, Vector_OriginNormalHAnglePerStep { 0.0 }
		, Vector_OriginAllowedTilt { false }
		, Vector_OriginCircleOffset {}
		, Vector_OriginOrigin { VectorOrigin::Self }
		, Vector_OriginOriginFLH {}
		, Vector_NormalFAngleRMin { 0.0 }
		, Vector_NormalFAngleRMax { 0.0 }
		, Vector_NormalFAngleRMin2 { 0.0 }
		, Vector_NormalFAngleRMax2 { 0.0 }
		, Vector_NormalLAngleRMin { 0.0 }
		, Vector_NormalLAngleRMax { 0.0 }
		, Vector_NormalLAngleRMin2 { 0.0 }
		, Vector_NormalLAngleRMax2 { 0.0 }
		, Vector_NormalHAngleRMin { 0.0 }
		, Vector_NormalHAngleRMax { 0.0 }
		, Vector_NormalHAngleRMin2 { 0.0 }
		, Vector_NormalHAngleRMax2 { 0.0 }
		, Vector_OriginNormalFAngleRMin { 0.0 }
		, Vector_OriginNormalFAngleRMax { 0.0 }
		, Vector_OriginNormalFAngleRMin2 { 0.0 }
		, Vector_OriginNormalFAngleRMax2 { 0.0 }
		, Vector_OriginNormalLAngleRMin { 0.0 }
		, Vector_OriginNormalLAngleRMax { 0.0 }
		, Vector_OriginNormalLAngleRMin2 { 0.0 }
		, Vector_OriginNormalLAngleRMax2 { 0.0 }
		, Vector_OriginNormalHAngleRMin { 0.0 }
		, Vector_OriginNormalHAngleRMax { 0.0 }
		, Vector_OriginNormalHAngleRMin2 { 0.0 }
		, Vector_OriginNormalHAngleRMax2 { 0.0 }
		, Groups {}
	{};

	bool HasTint() const
	{
		return this->Tint_Color.isset() || this->Tint_Intensity != 0.0;
	}

	bool HasVector() const
	{
		return static_cast<const CoordStruct&>(this->Vector_MoveTo) != CoordStruct::Empty
			|| this->Vector_TargetFLH.isset()
			|| this->Vector_Freeze
			|| this->Vector_ReachTarget
			|| this->Vector_CircleRadius > 0
			|| this->Vector_CircleSpeed != 0
			|| this->Vector_CircleAnglePerStep > 0.0
			|| (this->Vector_CircleRandomRadiusMax > this->Vector_CircleRandomRadiusMin)
			|| (this->Vector_CircleRandomAngleMax > this->Vector_CircleRandomAngleMin)
			|| this->Vector_OriginMoveTo.isset();
	}

	bool HasGroup(const std::string& groupID) const;
	bool HasGroups(const std::vector<std::string>& groupIDs, bool requireAll) const;

	AnimTypeClass* GetCumulativeAnimation(int cumulativeCount) const
	{
		if (cumulativeCount < 0 || this->CumulativeAnimations.size() < 1)
			return nullptr;

		const int index = static_cast<size_t>(cumulativeCount) >= this->CumulativeAnimations.size() ? this->CumulativeAnimations.size() - 1 : cumulativeCount - 1;

		return this->CumulativeAnimations.at(index);
	}

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	static void Clear()
	{
		AttachEffectTypeClass::GroupsMap.clear();
	}

	static std::vector<AttachEffectTypeClass*> GetTypesFromGroups(const std::vector<std::string>& groupIDs);
	static void HandleEvent(TechnoClass* pTarget);

private:
	template <typename T>
	void Serialize(T& Stm);
	void AddToGroupsMap();
};

// Container for AttachEffect attachment for an individual effect passed to AE attach function.
struct AEAttachParams
{
	int DurationOverride;
	int Delay;
	int InitialDelay;
	int RecreationDelay;
	int CumulativeSourceMaxCount;
	bool CumulativeRefreshAll;
	bool CumulativeRefreshAll_OnAttach;
	bool CumulativeRefreshSameSourceOnly;

	AEAttachParams() :
		DurationOverride { 0 }
		, Delay { 0 }
		, InitialDelay { 0 }
		, RecreationDelay { -1 }
		, CumulativeSourceMaxCount { -1 }
		, CumulativeRefreshAll { false }
		, CumulativeRefreshAll_OnAttach { false }
		, CumulativeRefreshSameSourceOnly { true }
	{
	}
};

// Container for AttachEffect attachment info parsed from INI.
class AEAttachInfoTypeClass
{
public:
	ValueableVector<AttachEffectTypeClass*> AttachTypes;
	Valueable<int> CumulativeSourceMaxCount;
	Valueable<bool> CumulativeRefreshAll;
	Valueable<bool> CumulativeRefreshAll_OnAttach;
	Valueable<bool> CumulativeRefreshSameSourceOnly;
	ValueableVector<AttachEffectTypeClass*> RemoveTypes;
	std::vector<std::string> RemoveGroups;
	ValueableVector<int> CumulativeRemoveMinCounts;
	ValueableVector<int> CumulativeRemoveMaxCounts;
	ValueableVector<int> DurationOverrides;
	ValueableVector<int> Delays;
	ValueableVector<int> InitialDelays;
	ValueableVector<int> RecreationDelays;

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	AEAttachParams GetAttachParams(unsigned int index, bool selfOwned) const;

	AEAttachInfoTypeClass() :
		AttachTypes {}
		, CumulativeSourceMaxCount { -1 }
		, CumulativeRefreshAll { false }
		, CumulativeRefreshAll_OnAttach { false }
		, CumulativeRefreshSameSourceOnly { true }
		, RemoveTypes {}
		, RemoveGroups {}
		, CumulativeRemoveMinCounts {}
		, CumulativeRemoveMaxCounts {}
		, DurationOverrides {}
		, Delays {}
		, InitialDelays {}
		, RecreationDelays {}
	{
	}

private:
	template <typename T>
	bool Serialize(T& stm);
};
