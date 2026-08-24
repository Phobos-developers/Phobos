#pragma once
#include <WeaponTypeClass.h>
#include <Ext/AbstractType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/AttachEffectTypeClass.h>

class WeaponTypeExt final : public AbstractTypeExt
{
public:
	using base_type = WeaponTypeClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = WeaponTypeExt;

	static constexpr DWORD Canary = 0x22222222;

public:
	// typed owner accessor
	WeaponTypeClass* OwnerObject() const
	{
		return static_cast<WeaponTypeClass*>(this->GetAttachedObject());
	}


	Valueable<double> DiskLaser_Radius;
	Valueable<Leptons> ProjectileRange;
	Nullable<bool> ProjectileRange_ApplyModifiers;
	Valueable<RadTypeClass*> RadType;
	Nullable<ColorStruct> Bolt_Color[3];
	Valueable<bool> Bolt_Disable[3];
	Nullable<ParticleSystemTypeClass*> Bolt_ParticleSystem;
	Valueable<int> Bolt_Arcs;
	Valueable<int> Bolt_Duration;
	Nullable<bool> Bolt_FollowFLH;
	Nullable<AffectedHouse> IvanBomb_Visibility;
	Nullable<bool> Strafing;
	Nullable<int> Strafing_Shots;
	Nullable<bool> Strafing_SimulateBurst;
	Nullable<bool> Strafing_UseAmmoPerShot;
	Nullable<bool> Strafing_TargetCell;
	Nullable<int> Strafing_EndDelay;
	Valueable<AffectedTarget> CanTarget;
	Valueable<AffectedHouse> CanTargetHouses;
	Valueable<double> CanTarget_MaxHealth;
	Valueable<double> CanTarget_MinHealth;
	Valueable<AffectedVeterancy> CanTargetVeterancy;
	Nullable<bool> CanTarget_IronCurtained;
	Nullable<bool> AutoTarget_IronCurtained;
	ValueableVector<int> Burst_Delays;
	Valueable<bool> Burst_FireWithinSequence;
	Valueable<bool> Burst_NoDelay;
	Valueable<AreaFireTarget> AreaFire_Target;
	Valueable<WeaponTypeClass*> FeedbackWeapon;
	Valueable<bool> Laser_IsSingleColor;
	Valueable<PositionFollow> LaserPositionUpdate;
	Nullable<bool> LaserPositionUpdate_StopOnFirerConvert;
	Nullable<int> LaserZAdjust;
	Nullable<int> EBoltZAdjust;
	Nullable<bool> EBoltZAdjust_ClampInitialDepthForBuilding;
	Valueable<bool> VisualScatter;
	Nullable<PartialVector2D<int>> ROF_RandomDelay;
	ValueableVector<int> ChargeTurret_Delays;
	Nullable<bool> OmniFire_TurnToTarget;
	Valueable<bool> FireOnce_ResetSequence;
	Valueable<bool> TurretRecoil_Suppress;
	ValueableVector<WarheadTypeClass*> ExtraWarheads;
	ValueableVector<int> ExtraWarheads_DamageOverrides;
	ValueableVector<double> ExtraWarheads_DetonationChances;
	ValueableVector<float> ExtraWarheads_RollChances;
	std::vector<ValueableVector<int>> ExtraWarheads_WeightsData;
	ValueableVector<bool> ExtraWarheads_FullDetonation;
	Nullable<WarheadTypeClass*> AmbientDamage_Warhead;
	Nullable<bool> AmbientDamage_IgnoreTarget;
	ValueableVector<AttachEffectTypeClass*> AttachEffect_RequiredTypes;
	ValueableVector<AttachEffectTypeClass*> AttachEffect_DisallowedTypes;
	std::vector<std::string> AttachEffect_RequiredGroups;
	std::vector<std::string> AttachEffect_DisallowedGroups;
	ValueableVector<int> AttachEffect_RequiredMinCounts;
	ValueableVector<int> AttachEffect_RequiredMaxCounts;
	ValueableVector<int> AttachEffect_DisallowedMinCounts;
	ValueableVector<int> AttachEffect_DisallowedMaxCounts;
	Valueable<bool> AttachEffect_CheckOnFirer;
	Valueable<bool> AttachEffect_IgnoreFromSameSource;
	Valueable<Leptons> KeepRange;
	Nullable<bool> KeepRange_AllowAI;
	Nullable<bool> KeepRange_AllowPlayer;
	Nullable<int> KeepRange_EarlyStopFrame;
	Nullable<bool> KickOutPassengers;
	Nullable<ColorStruct> Beam_Color;
	Valueable<int> Beam_Duration;
	Valueable<double> Beam_Amplitude;
	Valueable<bool> Beam_IsHouseColor;
	Valueable<int> LaserThickness;
	Nullable<PartialVector2D<int>> DelayedFire_Duration;
	Valueable<bool> DelayedFire_SkipInTransport;
	Valueable<AnimTypeClass*> DelayedFire_Animation;
	Nullable<AnimTypeClass*> DelayedFire_OpenToppedAnimation;
	Valueable<bool> DelayedFire_AnimIsAttached;
	Valueable<bool> DelayedFire_CenterAnimOnFirer;
	Valueable<bool> DelayedFire_RemoveAnimOnNoDelay;
	Valueable<bool> DelayedFire_PauseFiringSequence;
	Valueable<bool> DelayedFire_OnlyOnInitialBurst;
	Nullable<CoordStruct> DelayedFire_AnimOffset;
	Valueable<bool> DelayedFire_AnimOnTurret;
	Nullable<Leptons> ExtraRange_TargetMoving;
	Nullable<Leptons> ExtraRange_FirerMoving;
	Nullable<Leptons> ExtraRange_Prefiring;
	Nullable<bool> ExtraRange_Prefiring_IncludeBurst;
	Nullable<bool> AttackFriendlies;
	Nullable<bool> AttackCursorOnFriendlies;
	Nullable<bool> AttackNoThreatBuildings;

	Nullable<bool> Anim_Update;

	bool SkipWeaponPicking;

	Nullable<bool> CylinderRangefinding;
	

	WeaponTypeExt(WeaponTypeClass* OwnerObject) : AbstractTypeExt(OwnerObject)
		, DiskLaser_Radius { DiskLaserClass::Radius }
		, ProjectileRange { Leptons(100000) }
		, ProjectileRange_ApplyModifiers {}
		, RadType {}
		, Bolt_Color {}
		, Bolt_Disable { Valueable<bool>(false) }
		, Bolt_ParticleSystem {}
		, Bolt_Arcs { 8 }
		, Bolt_Duration { 17 }
		, Bolt_FollowFLH {}
		, IvanBomb_Visibility {}
		, Strafing { }
		, Strafing_Shots {}
		, Strafing_SimulateBurst {}
		, Strafing_UseAmmoPerShot {}
		, Strafing_TargetCell {}
		, Strafing_EndDelay {}
		, CanTarget { AffectedTarget::All }
		, CanTargetHouses { AffectedHouse::All }
		, CanTarget_MaxHealth { 1.0 }
		, CanTarget_MinHealth { 0.0 }
		, CanTargetVeterancy { AffectedVeterancy::All }
		, CanTarget_IronCurtained {}
		, AutoTarget_IronCurtained {}
		, Burst_Delays {}
		, Burst_FireWithinSequence { false }
		, Burst_NoDelay { false }
		, AreaFire_Target { AreaFireTarget::Base }
		, FeedbackWeapon {}
		, Laser_IsSingleColor { false }
		, LaserPositionUpdate { PositionFollow::None }
		, LaserPositionUpdate_StopOnFirerConvert {}
		, LaserZAdjust {}
		, EBoltZAdjust {}
		, EBoltZAdjust_ClampInitialDepthForBuilding {}
		, VisualScatter { false }
		, ROF_RandomDelay {}
		, ChargeTurret_Delays {}
		, OmniFire_TurnToTarget {}
		, FireOnce_ResetSequence { true }
		, TurretRecoil_Suppress { false }
		, ExtraWarheads {}
		, ExtraWarheads_DamageOverrides {}
		, ExtraWarheads_DetonationChances {}
		, ExtraWarheads_RollChances {}
		, ExtraWarheads_WeightsData {}
		, ExtraWarheads_FullDetonation {}
		, AmbientDamage_Warhead {}
		, AmbientDamage_IgnoreTarget {}
		, AttachEffect_RequiredTypes {}
		, AttachEffect_DisallowedTypes {}
		, AttachEffect_RequiredGroups {}
		, AttachEffect_DisallowedGroups {}
		, AttachEffect_RequiredMinCounts {}
		, AttachEffect_RequiredMaxCounts {}
		, AttachEffect_DisallowedMinCounts {}
		, AttachEffect_DisallowedMaxCounts {}
		, AttachEffect_CheckOnFirer { false }
		, AttachEffect_IgnoreFromSameSource { false }
		, KeepRange { Leptons(0) }
		, KeepRange_AllowAI {}
		, KeepRange_AllowPlayer {}
		, KeepRange_EarlyStopFrame {}
		, KickOutPassengers {}
		, Beam_Color {}
		, Beam_Duration { 15 }
		, Beam_Amplitude { 40.0 }
		, Beam_IsHouseColor { false }
		, LaserThickness { 3 }
		, SkipWeaponPicking { true }
		, DelayedFire_Duration {}
		, DelayedFire_SkipInTransport { false }
		, DelayedFire_Animation {}
		, DelayedFire_OpenToppedAnimation {}
		, DelayedFire_AnimIsAttached { true }
		, DelayedFire_CenterAnimOnFirer { false }
		, DelayedFire_RemoveAnimOnNoDelay { false }
		, DelayedFire_PauseFiringSequence { false }
		, DelayedFire_OnlyOnInitialBurst { false }
		, DelayedFire_AnimOffset {}
		, DelayedFire_AnimOnTurret { true }
		, ExtraRange_TargetMoving {}
		, ExtraRange_FirerMoving {}
		, ExtraRange_Prefiring {}
		, ExtraRange_Prefiring_IncludeBurst {}
		, AttackFriendlies {}
		, AttackCursorOnFriendlies {}
		, AttackNoThreatBuildings {}
		, CylinderRangefinding {}
		, Anim_Update {}
	{ }

	int GetBurstDelay(int burstIndex) const;
	bool HasRequiredAttachedEffects(TechnoClass* pTechno, TechnoClass* pFirer) const;
	bool IsHealthInThreshold(TechnoClass* pTarget) const;
	bool IsVeterancyInThreshold(TechnoClass* pTarget) const;

	virtual ~WeaponTypeExt() = default;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;
	virtual void Initialize() override;
	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<WeaponTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static WeaponTypeExt* Fetch(const WeaponTypeClass* pThis)
	{
		return AbstractExt::Fetch<WeaponTypeExt>(pThis);
	}

	static WeaponTypeExt* TryFetch(const WeaponTypeClass* pThis)
	{
		return AbstractExt::TryFetch<WeaponTypeExt>(pThis);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static double OldRadius;
	static PhobosMap<BombClass*, WeaponTypeExt*> BombExtMap;

	static WeaponTypeExt* GetBombExtData(BombClass* pBomb);

	static void DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, HouseClass* pFiringHouse = nullptr);
	static void DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse = nullptr);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, HouseClass* pFiringHouse = nullptr, AbstractClass* pTarget = nullptr);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse = nullptr, AbstractClass* pTarget = nullptr);
	static int GetRangeWithModifiers(WeaponTypeClass* pThis, TechnoClass* pFirer);
	static int GetRangeWithModifiers(WeaponTypeClass* pThis, TechnoClass* pFirer, int range);
	static int GetTechnoKeepRange(WeaponTypeClass* pThis, TechnoClass* pFirer, bool isMinimum);

	// Misc/Hooks.LaserDraw.cpp
	static void OnObjectRemoved(ObjectClass* pObject);
};

