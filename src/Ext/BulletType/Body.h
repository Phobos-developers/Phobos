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

		TrajectoryTypePointer TrajectoryType;

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

		Valueable<bool> Splits;
		Valueable<double> AirburstSpread;
		Valueable<double> RetargetAccuracy;
		Valueable<bool> RetargetSelf;
		Valueable<double> RetargetSelf_Probability;
		Nullable<bool> AroundTarget;
		Valueable<bool> Airburst_UseCluster;
		Valueable<bool> Airburst_RandomClusters;
		Valueable<Leptons> Splits_TargetingDistance;
		Valueable<int> Splits_TargetCellRange;
		Valueable<bool> Splits_UseWeaponTargeting;
		Valueable<bool> AirburstWeapon_ApplyFirepowerMult;

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
			, TrajectoryType { }
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
			, Splits { false }
			, AirburstSpread { 1.5 }
			, RetargetAccuracy { 0.0 }
			, RetargetSelf { true }
			, RetargetSelf_Probability { 0.5 }
			, AroundTarget {}
			, Airburst_UseCluster { false }
			, Airburst_RandomClusters { false }
			, Splits_TargetingDistance{ Leptons(1280) }
			, Splits_TargetCellRange { 3 }
			, Splits_UseWeaponTargeting { false }
			, AirburstWeapon_ApplyFirepowerMult { false }
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
