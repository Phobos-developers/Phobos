#pragma once
#include <BulletTypeClass.h>

#include <Ext/ObjectType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/LaserTrailTypeClass.h>

#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>

class BulletTypeExt final : public ObjectTypeExt
{
public:
	using base_type = BulletTypeClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = BulletTypeExt;

	static constexpr DWORD Canary = 0xF00DF00D;

public:
	// typed owner accessor
	BulletTypeClass* OwnerObject() const
	{
		return static_cast<BulletTypeClass*>(this->GetAttachedObject());
	}

	// Valueable<int> Strength; //Use OwnerObject()->ObjectTypeClass::Strength
	Nullable<ArmorType> Armor;
	Nullable<bool> Interceptable;
	Valueable<bool> Interceptable_DeleteOnIntercept;
	Valueable<WeaponTypeClass*> Interceptable_WeaponOverride;
	ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
	Nullable<double> Gravity;
	Nullable<bool> Vertical_AircraftFix;
	Nullable<bool> VerticalInitialFacing;

	TrajectoryTypePointer TrajectoryType;

	Nullable<bool> Shrapnel_AffectsGround;
	Nullable<bool> Shrapnel_AffectsBuildings;
	Nullable<bool> Shrapnel_UseWeaponTargeting;
	Nullable<bool> Shrapnel_IgnoreHitBuildings;
	Nullable<bool> Shrapnel_ObeyWarheadTriggerConditions;
	Nullable<bool> SubjectToLand;
	Valueable<bool> SubjectToLand_Detonate;
	Nullable<bool> SubjectToWater;
	Valueable<bool> SubjectToWater_Detonate;

	Valueable<Leptons> ClusterScatter_Min;
	Valueable<Leptons> ClusterScatter_Max;

	Valueable<bool> AAOnly;
	Nullable<bool> Arcing_AllowElevationInaccuracy;
	Valueable<WeaponTypeClass*> ReturnWeapon;
	Nullable<bool> ReturnWeapon_ApplyFirepowerMult;

	Valueable<bool> SubjectToGround;

	Valueable<bool> Splits;
	Valueable<double> AirburstSpread;
	Valueable<double> RetargetAccuracy;
	Valueable<bool> RetargetSelf;
	Valueable<double> RetargetSelf_Probability;
	Nullable<bool> AroundTarget;
	Nullable<bool> Airburst_UseCluster;
	Valueable<bool> Airburst_RandomClusters;
	Valueable<bool> Airburst_TargetAsSource;
	Nullable<bool> Airburst_TargetAsSource_SkipHeight;
	Valueable<Leptons> Splits_TargetingDistance;
	Nullable<bool> Splits_TargetingDistance_Cylindrical;
	Nullable<bool> Splits_AllowRepeatTargets;
	Valueable<int> Splits_TargetCellRange;
	Nullable<bool> Splits_UseWeaponTargeting;
	Nullable<bool> AirburstWeapon_ApplyFirepowerMult;
	Valueable<Leptons> AirburstWeapon_SourceScatterMin;
	Valueable<Leptons> AirburstWeapon_SourceScatterMax;
	Nullable<bool> AirburstWeapon_UseFiringEffects;
	Nullable<bool> AirburstWeapon_HeadToTarget;
	Valueable<int> AirburstWeapon_RadialFireSegments;

	Valueable<bool> Parachuted;
	Valueable<int> Parachuted_FallRate;
	Nullable<int> Parachuted_MaxFallRate;
	Nullable<AnimTypeClass*> BombParachute;

	Valueable<bool> AU;

	Valueable<int> ZAdjust;

	// Ares 0.7
	Nullable<Leptons> BallisticScatter_Min;
	Nullable<Leptons> BallisticScatter_Max;

	BulletTypeExt(BulletTypeClass* OwnerObject) : ObjectTypeExt(OwnerObject)
		, Armor {}
		, Interceptable {}
		, Interceptable_DeleteOnIntercept { false }
		, Interceptable_WeaponOverride {}
		, LaserTrail_Types {}
		, Gravity {}
		, Vertical_AircraftFix {}
		, VerticalInitialFacing {}
		, TrajectoryType { }
		, Shrapnel_AffectsGround {}
		, Shrapnel_AffectsBuildings {}
		, Shrapnel_UseWeaponTargeting {}
		, Shrapnel_IgnoreHitBuildings {}
		, Shrapnel_ObeyWarheadTriggerConditions {}
		, ClusterScatter_Min { Leptons(256) }
		, ClusterScatter_Max { Leptons(512) }
		, BallisticScatter_Min {}
		, BallisticScatter_Max {}
		, SubjectToLand {}
		, SubjectToLand_Detonate { true }
		, SubjectToWater {}
		, SubjectToWater_Detonate { true }
		, AAOnly { false }
		, Arcing_AllowElevationInaccuracy {}
		, ReturnWeapon {}
		, ReturnWeapon_ApplyFirepowerMult {}
		, SubjectToGround { false }
		, Splits { false }
		, AirburstSpread { 1.5 }
		, RetargetAccuracy { 0.0 }
		, RetargetSelf { true }
		, RetargetSelf_Probability { 0.5 }
		, AroundTarget {}
		, Airburst_UseCluster {}
		, Airburst_RandomClusters { false }
		, Airburst_TargetAsSource { false }
		, Airburst_TargetAsSource_SkipHeight {}
		, Splits_TargetingDistance{ Leptons(1280) }
		, Splits_TargetingDistance_Cylindrical {}
		, Splits_AllowRepeatTargets {}
		, Splits_TargetCellRange { 3 }
		, Splits_UseWeaponTargeting {}
		, AirburstWeapon_ApplyFirepowerMult {}
		, AirburstWeapon_SourceScatterMin { Leptons(0) }
		, AirburstWeapon_SourceScatterMax { Leptons(0) }
		, AirburstWeapon_UseFiringEffects {}
		, AirburstWeapon_HeadToTarget {}
		, AirburstWeapon_RadialFireSegments { 0 }
		, Parachuted { false }
		, Parachuted_FallRate { 1 }
		, Parachuted_MaxFallRate {}
		, BombParachute {}
		, AU { false }
		, ZAdjust { 0 }
	{ }

	virtual ~BulletTypeExt() = default;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;
	// virtual void Initialize() override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

	void TrajectoryValidation() const;

public:
	class ExtContainer final : public Container<BulletTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static BulletTypeExt* Fetch(const BulletTypeClass* pThis)
	{
		return AbstractExt::Fetch<BulletTypeExt>(pThis);
	}

	static BulletTypeExt* TryFetch(const BulletTypeClass* pThis)
	{
		return AbstractExt::TryFetch<BulletTypeExt>(pThis);
	}

	static double GetAdjustedGravity(BulletTypeClass* pType);
	static BulletTypeClass* GetDefaultBulletType();
};

