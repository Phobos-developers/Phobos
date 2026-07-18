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
	Valueable<bool> Interceptable;
	Valueable<bool> Interceptable_DeleteOnIntercept;
	Valueable<WeaponTypeClass*> Interceptable_WeaponOverride;
	ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
	Nullable<double> Gravity;
	Valueable<bool> Vertical_AircraftFix;
	Nullable<bool> VerticalInitialFacing;

	TrajectoryTypePointer TrajectoryType;

	Valueable<int> LifeDuration;
	Valueable<int> NoTargetLifeTime;
	Valueable<int> CreateCapacity;
	Valueable<int> RetargetInterval;
	Valueable<double> RetargetRadius;
	Valueable<AffectedHouse> RetargetHouses;
	Valueable<bool> Synchronize;
	Nullable<bool> PeacefulVanish;
	Valueable<bool> ApplyRangeModifiers;
	Valueable<bool> UseDisperseCoord;

	Valueable<bool> PassDetonate;
	Valueable<bool> PassDetonateLocal;
	Valueable<WarheadTypeClass*> PassDetonateWarhead;
	Nullable<int> PassDetonateDamage;
	Valueable<int> PassDetonateDelay;
	Valueable<int> PassDetonateInitialDelay;
	Valueable<int> ProximityImpact;
	Valueable<WarheadTypeClass*> ProximityWarhead;
	Nullable<int> ProximityDamage;
	Valueable<Leptons> ProximityRadius;
	Valueable<bool> ProximityDirect;
	Valueable<bool> ProximityMedial;
	Valueable<bool> ProximityAllies;
	Valueable<bool> ProximityFlight;
	Valueable<bool> ProximitySphere;
	Valueable<bool> ThroughVehicles;
	Valueable<bool> ThroughBuilding;
	Valueable<double> DamageEdgeAttenuation;
	Valueable<double> DamageCountAttenuation;

	ValueableVector<WeaponTypeClass*> DisperseWeapons;
	ValueableVector<int> DisperseBursts;
	ValueableVector<int> DisperseCounts;
	ValueableVector<int> DisperseDelays;
	Valueable<int> DisperseCycle;
	Valueable<int> DisperseInitialDelay;
	Valueable<Leptons> DisperseEffectiveRange;
	Valueable<bool> DisperseSeparate;
	Valueable<bool> DisperseRetarget;
	Valueable<bool> DisperseLocation;
	Valueable<bool> DisperseTendency;
	Valueable<bool> DisperseHolistic;
	Valueable<bool> DisperseMarginal;
	Valueable<bool> DisperseDoRepeat;
	Valueable<bool> DisperseSuicide;
	Nullable<bool> DisperseFromFirer;
	Valueable<bool> DisperseFaceCheck;
	Valueable<bool> DisperseForceFire;
	Valueable<CoordStruct> DisperseCoord;

	Valueable<bool> Shrapnel_AffectsGround;
	Valueable<bool> Shrapnel_AffectsBuildings;
	Valueable<bool> Shrapnel_UseWeaponTargeting;
	Nullable<bool> Shrapnel_IgnoreHitBuildings;
	Nullable<bool> Shrapnel_ObeyWarheadTriggerConditions;
	Nullable<bool> SubjectToLand;
	Valueable<bool> SubjectToLand_Detonate;
	Nullable<bool> SubjectToWater;
	Valueable<bool> SubjectToWater_Detonate;

	Valueable<Leptons> ClusterScatter_Min;
	Valueable<Leptons> ClusterScatter_Max;

	Valueable<bool> AAOnly;
	Valueable<bool> Arcing_AllowElevationInaccuracy;
	Valueable<WeaponTypeClass*> ReturnWeapon;
	Valueable<bool> ReturnWeapon_ApplyFirepowerMult;

	Valueable<bool> SubjectToGround;
	Valueable<bool> SubjectToSolid;

	Valueable<bool> BallisticScatter_IncreaseByRange;
	Nullable<Leptons> BallisticScatter_MinRange;
	Nullable<Leptons> BallisticScatter_MaxRange;
	Nullable<Leptons> BallisticScatter_Min_InMinRange;
	Nullable<Leptons> BallisticScatter_Min_InMaxRange;
	Nullable<Leptons> BallisticScatter_Max_InMinRange;
	Nullable<Leptons> BallisticScatter_Max_InMaxRange;
	Valueable<double> BallisticScatter_Chance;

	Valueable<bool> Splits;
	Valueable<double> AirburstSpread;
	Valueable<double> RetargetAccuracy;
	Valueable<bool> RetargetSelf;
	Valueable<double> RetargetSelf_Probability;
	Nullable<bool> AroundTarget;
	Valueable<bool> Airburst_UseCluster;
	Valueable<bool> Airburst_RandomClusters;
	Valueable<bool> Airburst_TargetAsSource;
	Valueable<bool> Airburst_TargetAsSource_SkipHeight;
	Valueable<Leptons> Splits_TargetingDistance;
	Valueable<bool> Splits_TargetingDistance_Cylindrical;
	Valueable<bool> Splits_AllowRepeatTargets;
	Valueable<int> Splits_TargetCellRange;
	Valueable<bool> Splits_UseWeaponTargeting;
	Valueable<bool> AirburstWeapon_ApplyFirepowerMult;
	Valueable<Leptons> AirburstWeapon_SourceScatterMin;
	Valueable<Leptons> AirburstWeapon_SourceScatterMax;
	Valueable<bool> AirburstWeapon_UseFiringEffects;
	Valueable<bool> AirburstWeapon_HeadToTarget;
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
		, Interceptable { false }
		, Interceptable_DeleteOnIntercept { false }
		, Interceptable_WeaponOverride {}
		, LaserTrail_Types {}
		, Gravity {}
		, Vertical_AircraftFix { true }
		, VerticalInitialFacing {}
		, TrajectoryType { }
		, LifeDuration { 0 }
		, NoTargetLifeTime { -1 }
		, CreateCapacity { -1 }
		, RetargetInterval { 1 }
		, RetargetRadius { 0 }
		, RetargetHouses { AffectedHouse::Enemies }
		, Synchronize { false }
		, PeacefulVanish {}
		, ApplyRangeModifiers { false }
		, UseDisperseCoord { false }
		, PassDetonate { false }
		, PassDetonateLocal { false }
		, PassDetonateWarhead {}
		, PassDetonateDamage {}
		, PassDetonateDelay { 1 }
		, PassDetonateInitialDelay { 0 }
		, ProximityImpact { 0 }
		, ProximityWarhead {}
		, ProximityDamage {}
		, ProximityRadius { Leptons(179) }
		, ProximityDirect { false }
		, ProximityMedial { false }
		, ProximityAllies { false }
		, ProximityFlight { false }
		, ProximitySphere { true }
		, ThroughVehicles { true }
		, ThroughBuilding { true }
		, DamageEdgeAttenuation { 1.0 }
		, DamageCountAttenuation { 1.0 }
		, DisperseWeapons {}
		, DisperseBursts {}
		, DisperseCounts {}
		, DisperseDelays {}
		, DisperseCycle { 0 }
		, DisperseInitialDelay { 0 }
		, DisperseEffectiveRange { Leptons(0) }
		, DisperseSeparate { false }
		, DisperseRetarget { false }
		, DisperseLocation { false }
		, DisperseTendency { false }
		, DisperseHolistic { false }
		, DisperseMarginal { false }
		, DisperseDoRepeat { false }
		, DisperseSuicide { true }
		, DisperseFromFirer {}
		, DisperseFaceCheck { false }
		, DisperseForceFire { true }
		, DisperseCoord { CoordStruct::Empty }
		, Shrapnel_AffectsGround { false }
		, Shrapnel_AffectsBuildings { false }
		, Shrapnel_UseWeaponTargeting { false }
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
		, Arcing_AllowElevationInaccuracy { true }
		, ReturnWeapon {}
		, ReturnWeapon_ApplyFirepowerMult { false }
		, SubjectToSolid { false }
		, SubjectToGround { false }
		, BallisticScatter_IncreaseByRange { false }
		, BallisticScatter_MinRange {}
		, BallisticScatter_MaxRange {}
		, BallisticScatter_Min_InMinRange {}
		, BallisticScatter_Min_InMaxRange {}
		, BallisticScatter_Max_InMinRange {}
		, BallisticScatter_Max_InMaxRange {}
		, BallisticScatter_Chance { 1.0 }
		, Splits { false }
		, AirburstSpread { 1.5 }
		, RetargetAccuracy { 0.0 }
		, RetargetSelf { true }
		, RetargetSelf_Probability { 0.5 }
		, AroundTarget {}
		, Airburst_UseCluster { false }
		, Airburst_RandomClusters { false }
		, Airburst_TargetAsSource { false }
		, Airburst_TargetAsSource_SkipHeight { false }
		, Splits_TargetingDistance { Leptons(1280) }
		, Splits_TargetingDistance_Cylindrical { false }
		, Splits_AllowRepeatTargets { false }
		, Splits_TargetCellRange { 3 }
		, Splits_UseWeaponTargeting { false }
		, AirburstWeapon_ApplyFirepowerMult { false }
		, AirburstWeapon_SourceScatterMin { Leptons(0) }
		, AirburstWeapon_SourceScatterMax { Leptons(0) }
		, AirburstWeapon_UseFiringEffects { false }
		, AirburstWeapon_HeadToTarget { false }
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

