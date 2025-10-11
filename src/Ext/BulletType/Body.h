#pragma once
#include <BulletTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/LaserTrailTypeClass.h>

#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>

class BulletTypeExt
{
public:
	using base_type = BulletTypeClass;

	static constexpr DWORD Canary = 0xF00DF00D;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<BulletTypeClass>
	{
	public:
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
		Valueable<int> Splits_TargetCellRange;
		Valueable<bool> Splits_UseWeaponTargeting;
		Valueable<bool> AirburstWeapon_ApplyFirepowerMult;
		Valueable<Leptons> AirburstWeapon_SourceScatterMin;
		Valueable<Leptons> AirburstWeapon_SourceScatterMax;

		Valueable<bool> Parachuted;
		Valueable<int> Parachuted_FallRate;
		Nullable<int> Parachuted_MaxFallRate;
		Nullable<AnimTypeClass*> BombParachute;

		Valueable<bool> AU;

		// Ares 0.1
		Valueable<bool> SubjectToSolid;

		// Ares 0.7
		Nullable<Leptons> BallisticScatter_Min;
		Nullable<Leptons> BallisticScatter_Max;

		ExtData(BulletTypeClass* OwnerObject) : Extension<BulletTypeClass>(OwnerObject)
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
			, Splits_TargetingDistance{ Leptons(1280) }
			, Splits_TargetCellRange { 3 }
			, Splits_UseWeaponTargeting { false }
			, AirburstWeapon_ApplyFirepowerMult { false }
			, AirburstWeapon_SourceScatterMin { Leptons(0) }
			, AirburstWeapon_SourceScatterMax { Leptons(0) }
			, Parachuted { false }
			, Parachuted_FallRate { 1 }
			, Parachuted_MaxFallRate {}
			, BombParachute {}
			, AU { false }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		// virtual void Initialize() override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);

		void TrajectoryValidation() const;
	};

	class ExtContainer final : public Container<BulletTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static double GetAdjustedGravity(BulletTypeClass* pType);
	static BulletTypeClass* GetDefaultBulletType();
};
