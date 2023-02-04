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

	class ExtData final : public Extension<BulletTypeClass>
	{
	public:
		// Valueable<int> Strength; //Use OwnerObject()->ObjectTypeClass::Strength
		Nullable<ArmorType> Armor;
		Valueable<bool> Interceptable;
		Valueable<bool> Interceptable_DeleteOnIntercept;
		Nullable<WeaponTypeClass*> Interceptable_WeaponOverride;
		ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
		Nullable<double> Gravity;

		PhobosTrajectoryType* TrajectoryType;
		Valueable<double> Trajectory_Speed;

		Valueable<bool> Shrapnel_AffectsGround;
		Valueable<bool> Shrapnel_AffectsBuildings;

		ExtData(BulletTypeClass* OwnerObject) : Extension<BulletTypeClass>(OwnerObject)
			, Armor {}
			, Interceptable { false }
			, Interceptable_DeleteOnIntercept { false }
			, Interceptable_WeaponOverride {}
			, LaserTrail_Types {}
			, Gravity {}
			, TrajectoryType { nullptr }
			, Trajectory_Speed { 100.0 }
			, Shrapnel_AffectsGround { false }
			, Shrapnel_AffectsBuildings { false }
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
	};

	class ExtContainer final : public Container<BulletTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static double GetAdjustedGravity(BulletTypeClass* pType);
	static BulletTypeClass* GetDefaultBulletType();
};
