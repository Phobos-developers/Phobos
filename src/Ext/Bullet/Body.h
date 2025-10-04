#pragma once
#include <BulletClass.h>

#include <Ext/BulletType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <New/Entity/LaserTrailClass.h>
#include "Trajectories/PhobosTrajectory.h"

struct BulletGroupData
{
	std::vector<DWORD> Bullets {}; // <UniqueID>, Capacity
	double Angle { 0.0 }; // Tracing.StableRotation use this value to update the angle
	bool ShouldUpdate { true }; // Remind members to update themselves

	BulletGroupData() = default;

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};

class BulletExt
{
public:
	using base_type = BulletClass;

	static constexpr DWORD Canary = 0x2A2A2A2A;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<BulletClass>
	{
	public:
		BulletTypeExt::ExtData* TypeExtData;
		HouseClass* FirerHouse;
		int CurrentStrength;
		TechnoTypeExt::ExtData* InterceptorTechnoType;
		InterceptedStatus InterceptedStatus;
		bool DetonateOnInterception;
		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
		bool SnappedToTarget; // Used for custom trajectory projectile target snap checks
		int DamageNumberOffset;
		int ParabombFallRate;

		TrajectoryPointer Trajectory;
		double FirepowerMult;
		int AttenuationRange;
		bool TargetIsInAir;
		bool TargetIsTechno;
		bool NotMainWeapon;
		TrajectoryStatus Status;
		int PassDetonateDamage;
		CDTimerClass PassDetonateTimer;
		int ProximityImpact;
		int ProximityDamage;
		TechnoClass* ExtraCheck;
		std::map<DWORD, int> Casualty;

		ExtData(BulletClass* OwnerObject) : Extension<BulletClass>(OwnerObject)
			, TypeExtData { nullptr }
			, FirerHouse { nullptr }
			, CurrentStrength { 0 }
			, InterceptorTechnoType { nullptr }
			, InterceptedStatus { InterceptedStatus::None }
			, DetonateOnInterception { true }
			, LaserTrails {}
			, SnappedToTarget { false }
			, DamageNumberOffset { INT32_MIN }
			, ParabombFallRate { 0 }

			, Trajectory { nullptr }
			, FirepowerMult { 1.0 }
			, AttenuationRange { 0 }
			, TargetIsInAir { false }
			, TargetIsTechno { false }
			, NotMainWeapon { false }
			, Status { TrajectoryStatus::None }
			, PassDetonateDamage { 0 }
			, PassDetonateTimer {}
			, ProximityImpact { 0 }
			, ProximityDamage { 0 }
			, ExtraCheck { nullptr }
			, Casualty {}
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void InterceptBullet(TechnoClass* pSource, BulletClass* pInterceptor);
		void ApplyRadiationToCell(CellStruct cell, int spread, int radLevel);
		void InitializeLaserTrails();

		void InitializeOnUnlimbo();
		bool CheckOnEarlyUpdate();
		void CheckOnPreDetonate();
		bool FireAdditionals();
		void DetonateOnObstacle();

		std::vector<CellClass*> GetCellsInProximityRadius();
		bool CheckThroughAndSubjectInCell(CellClass* pCell, HouseClass* pOwner);
		void CalculateNewDamage();
		void PassWithDetonateAt();
		void PrepareForDetonateAt();
		void ProximityDetonateAt(HouseClass* pOwner, TechnoClass* pTarget);
		int GetTrueDamage(int damage, bool self);
		double GetExtraDamageMultiplier();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BulletExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static constexpr double Epsilon = 1e-10;

	static void Detonate(const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse, AbstractClass* pTarget, bool isBright, WeaponTypeClass* pWeapon, WarheadTypeClass* pWarhead);
	static void ApplyArcingFix(BulletClass* pThis, const CoordStruct& sourceCoords, const CoordStruct& targetCoords, BulletVelocity& velocity);

	static void SimulatedFiringUnlimbo(BulletClass* pBullet, HouseClass* pHouse, WeaponTypeClass* pWeapon, const CoordStruct& sourceCoords, bool randomVelocity);
	static void SimulatedFiringEffects(BulletClass* pBullet, HouseClass* pHouse, ObjectClass* pAttach, bool firingEffect, bool visualEffect);
	static inline void SimulatedFiringAnim(BulletClass* pBullet, HouseClass* pHouse, ObjectClass* pAttach);
	static inline void SimulatedFiringReport(BulletClass* pBullet);
	static inline void SimulatedFiringLaser(BulletClass* pBullet, HouseClass* pHouse);
	static inline void SimulatedFiringElectricBolt(BulletClass* pBullet);
	static inline void SimulatedFiringRadBeam(BulletClass* pBullet, HouseClass* pHouse);
	static inline void SimulatedFiringParticleSystem(BulletClass* pBullet, HouseClass* pHouse);

	static inline double Get2DDistance(const CoordStruct& coords)
	{
		return Point2D { coords.X, coords.Y }.Magnitude();
	}
	static inline double Get2DDistance(const CoordStruct& source, const CoordStruct& target)
	{
		return Point2D { source.X, source.Y }.DistanceFrom(Point2D { target.X, target.Y });
	}
	static inline double Get2DVelocity(const BulletVelocity& velocity)
	{
		return Vector2D<double>{ velocity.X, velocity.Y }.Magnitude();
	}
	static inline double Get2DOpRadian(const CoordStruct& source, const CoordStruct& target)
	{
		return Math::atan2(target.Y - source.Y , target.X - source.X);
	}
	static inline BulletVelocity Coord2Vector(const CoordStruct& coords)
	{
		return BulletVelocity { static_cast<double>(coords.X), static_cast<double>(coords.Y), static_cast<double>(coords.Z) };
	}
	static inline CoordStruct Vector2Coord(const BulletVelocity& velocity)
	{
		return CoordStruct { static_cast<int>(velocity.X), static_cast<int>(velocity.Y), static_cast<int>(velocity.Z) };
	}
	static inline BulletVelocity HorizontalRotate(const CoordStruct& coords, const double radian)
	{
		return BulletVelocity { coords.X * Math::cos(radian) + coords.Y * Math::sin(radian), coords.X * Math::sin(radian) - coords.Y * Math::cos(radian), static_cast<double>(coords.Z) };
	}
	static inline bool CheckTechnoIsInvalid(const TechnoClass* const pTechno)
	{
		return (!pTechno->IsAlive || !pTechno->IsOnMap || pTechno->InLimbo || pTechno->IsSinking || pTechno->Health <= 0);
	}
	static inline void SetNewDamage(int& damage, const double ratio)
	{
		if (damage)
		{
			if (const auto newDamage = static_cast<int>(damage * ratio))
				damage = newDamage;
			else
				damage = Math::sgn(damage);
		}
	}
	static std::vector<CellStruct> GetCellsInRectangle(const CellStruct bottomStaCell, const CellStruct leftMidCell, const CellStruct rightMidCell, const CellStruct topEndCell);
};
