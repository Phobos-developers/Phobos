#pragma once
#include <BulletClass.h>

#include <Ext/BulletType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <New/Entity/LaserTrailClass.h>
#include <Ext/Object/Body.h>

#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>

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

struct RadialFireStruct
{
	int Segments = 0;
	int Index = 0;
	DirStruct Direction {};
};

class BulletExt final : public ObjectExt
{
public:
	using base_type = BulletClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = BulletExt;

	static constexpr DWORD Canary = 0x2A2A2A2A;

public:
	// typed owner accessor
	BulletClass* OwnerObject() const
	{
		return static_cast<BulletClass*>(this->GetAttachedObject());
	}

	BulletTypeExt* TypeExtData;
	HouseClass* FirerHouse;
	int CurrentStrength;
	TechnoTypeExt* InterceptorTechnoType;
	InterceptedStatus InterceptedStatus;
	bool DetonateOnInterception;
	std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
	bool SnappedToTarget; // Used for custom trajectory projectile target snap checks
	int DamageNumberOffset;
	int ParabombFallRate;
	bool IsInstantDetonation;
	double FirepowerMult;
	bool IsSplitFromAirburst;

	TrajectoryPointer Trajectory;
	bool DispersedTrajectory;
	CDTimerClass LifeDurationTimer;
	CDTimerClass NoTargetLifeTimer;
	CDTimerClass RetargetTimer;
	int AttenuationRange;
	bool TargetIsInAir;
	bool TargetIsTechno;
	bool NotMainWeapon;
	TrajectoryStatus Status;
	CoordStruct FLHCoord;
	std::shared_ptr<PhobosMap<BulletTypeClass*, BulletGroupData>> TrajectoryGroup;
	int GroupIndex;
	int PassDetonateDamage;
	CDTimerClass PassDetonateTimer;
	int ProximityImpact;
	int ProximityDamage;
	TechnoClass* ExtraCheck;
	std::map<DWORD, int> Casualty;
	int DisperseIndex;
	int DisperseCount;
	int DisperseCycle;
	CDTimerClass DisperseTimer;

	BulletExt(BulletClass* OwnerObject) : ObjectExt(OwnerObject)
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
		, IsInstantDetonation { false }
		, FirepowerMult { 1.0 }
		, IsSplitFromAirburst { false }

		, Trajectory { nullptr }
		, DispersedTrajectory { false }
		, LifeDurationTimer {}
		, NoTargetLifeTimer {}
		, RetargetTimer {}
		, AttenuationRange { 0 }
		, TargetIsInAir { false }
		, TargetIsTechno { false }
		, NotMainWeapon { false }
		, Status { TrajectoryStatus::None }
		, FLHCoord { CoordStruct::Empty }
		, TrajectoryGroup {}
		, GroupIndex { -1 }
		, PassDetonateDamage { 0 }
		, PassDetonateTimer {}
		, ProximityImpact { 0 }
		, ProximityDamage { 0 }
		, ExtraCheck { nullptr }
		, Casualty {}
		, DisperseIndex { 0 }
		, DisperseCount { 0 }
		, DisperseCycle { 0 }
		, DisperseTimer {}
	{}

	virtual ~BulletExt() override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	// the extension state that goes with BulletClass::Init
	void InitializeState();

	// the bullet was created while a savegame was loading, so BulletClass::Init found
	// no extension to initialize; catch up now that there is one
	virtual void OnDeferredAllocation() override { this->InitializeState(); }

	void InterceptBullet(TechnoClass* pSource, BulletClass* pInterceptor);
	void ApplyRadiationToCell(CellStruct cell, int spread, int radLevel);
	void InitializeLaserTrails();

	void InitializeOnUnlimbo();
	bool CheckOnEarlyUpdate();
	void CheckOnPreDetonate();
	bool FireAdditionals();
	void DetonateOnObstacle();
	bool CheckSynchronize();
	bool CheckNoTargetLifeTime();
	void UpdateGroupIndex();

	std::vector<CellClass*> GetCellsInProximityRadius();
	bool CheckThroughAndSubjectInCell(CellClass* pCell, HouseClass* pOwner);
	void CalculateNewDamage();
	void PassWithDetonateAt();
	template<bool allies, bool sphere>
	std::vector<TechnoClass*> GetTargetsInProximityRadius(HouseClass* pOwner);
	void PrepareForDetonateAt();
	void ProximityDetonateAt(HouseClass* pOwner, TechnoClass* pTarget);
	int GetTrueDamage(int damage, bool self);
	double GetExtraDamageMultiplier();

	bool BulletRetargetTechno();
	void GetTechnoFLHCoord();
	CoordStruct GetDisperseWeaponFireCoord(TechnoClass* pTechno);
	bool PrepareDisperseWeapon();
	bool FireDisperseWeapon(TechnoClass* pFirer, const CoordStruct& sourceCoord, HouseClass* pOwner);
	void CreateDisperseBullets(TechnoClass* pTechno, const CoordStruct& sourceCoord, WeaponTypeClass* pWeapon, AbstractClass* pTarget, HouseClass* pOwner, int curBurst, int maxBurst);

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<BulletExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static constexpr double Epsilon = 1e-10;
	static constexpr double EpsilonSquared = 1e-20;

	static BulletExt* Fetch(const BulletClass* pThis)
	{
		return AbstractExt::Fetch<BulletExt>(pThis);
	}

	static BulletExt* TryFetch(const BulletClass* pThis)
	{
		return AbstractExt::TryFetch<BulletExt>(pThis);
	}

	static void Detonate(const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse, AbstractClass* pTarget, bool isBright, WeaponTypeClass* pWeapon, WarheadTypeClass* pWarhead);
	static void ApplyArcingFix(BulletClass* pThis, const CoordStruct& sourceCoords, const CoordStruct& targetCoords, BulletVelocity& velocity);
	static CoordStruct GetTargetCoordsForFiring(BulletClass* pBullet);

	static void SimulatedFiringUnlimbo(BulletClass* pBullet, HouseClass* pHouse, WeaponTypeClass* pWeapon, const CoordStruct& sourceCoords, bool headToTarget, const RadialFireStruct& radialFire = {});
	static void SimulatedFiringEffects(BulletClass* pBullet, HouseClass* pHouse, ObjectClass* pAttach, bool firingEffect, bool visualEffect);
	static inline void SimulatedFiringAnim(BulletClass* pBullet, HouseClass* pHouse, ObjectClass* pAttach);
	static inline void SimulatedFiringReport(BulletClass* pBullet);
	static inline void SimulatedFiringLaser(BulletClass* pBullet, HouseClass* pHouse);
	static inline void SimulatedFiringElectricBolt(BulletClass* pBullet);
	static inline void SimulatedFiringRadBeam(BulletClass* pBullet, HouseClass* pHouse);
	static inline void SimulatedFiringParticleSystem(BulletClass* pBullet, HouseClass* pHouse);
	static inline BulletVelocity ApplyRadialFireVelocityWarp(BulletVelocity velocity, const RadialFireStruct& radialFire);

	static inline double Get2DDistance(const CoordStruct& coords)
	{
		return Point2D { coords.X, coords.Y }.Magnitude();
	}
	static inline double Get2DDistanceSquared(const CoordStruct& coords)
	{
		return Point2D { coords.X, coords.Y }.MagnitudeSquared();
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
		return Math::atan2(target.Y - source.Y, target.X - source.X);
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
	static inline Point2D Coord2Point(const CoordStruct& coords)
	{
		return Point2D { coords.X, coords.Y };
	}
	static inline CoordStruct Point2Coord(const Point2D& point, const int z = 0)
	{
		return CoordStruct { point.X, point.Y, z };
	}
	static inline Point2D PointRotate(const Point2D& point, const double radian)
	{
		return Point2D { static_cast<int>(point.X * Math::cos(radian) + point.Y * Math::sin(radian)), static_cast<int>(point.X * Math::sin(radian) - point.Y * Math::cos(radian)) };
	}
	static inline double GetDistanceFrom(const CoordStruct& source, const TechnoClass* const pTarget)
	{
		auto distance = source.DistanceFrom(pTarget->GetCoords());

		if (const auto pBuilding = abstract_cast<const BuildingClass*, true>(pTarget))
		{
			const auto pType = pBuilding->Type;
			distance = Math::max(0, distance - 64 * (pType->GetFoundationHeight(false) + pType->GetFoundationWidth()));
		}

		return distance;
	}
	static inline bool CheckTechnoIsInvalid(const TechnoClass* const pTechno)
	{
		return (!pTechno->IsAlive || !pTechno->IsOnMap || pTechno->InLimbo || pTechno->IsSinking || pTechno->Health <= 0);
	}
	static inline bool CheckWeaponCanTarget(const WeaponTypeExt* const pWeaponExt, TechnoClass* const pFirer, TechnoClass* const pTarget)
	{
		return !pWeaponExt || (EnumFunctions::IsTechnoEligible(pTarget, pWeaponExt->CanTarget) && pWeaponExt->IsHealthInThreshold(pTarget) && pWeaponExt->HasRequiredAttachedEffects(pTarget, pFirer));
	}
	static inline bool CheckWeaponValidness(HouseClass* const pHouse, const TechnoClass* const pTechno, const CellClass* const pCell, const AffectedHouse flags)
	{
		if (pHouse == pTechno->Owner)
			return (flags & AffectedHouse::Owner) != AffectedHouse::None;
		else if (pHouse->IsAlliedWith(pTechno->Owner) || pTechno->IsDisguisedAs(pHouse))
			return (flags & AffectedHouse::Allies) != AffectedHouse::None;
		else if ((flags & AffectedHouse::Enemies) == AffectedHouse::None)
			return false;

		return pTechno->CloakState != CloakState::Cloaked || pCell->Sensors_InclHouse(pHouse->ArrayIndex);
	}
	static inline bool CheckCanRetarget(TechnoClass* const pTechno, HouseClass* const pOwner, const AffectedHouse retargetHouses, const CoordStruct& center, const double retargetRange, const int range,
		const BulletClass* const pBullet, const WeaponTypeClass* const pWeapon, const WeaponTypeExt* const pWeaponExt, TechnoClass* const pFirer)
	{
		const auto pTechnoType = pTechno->GetTechnoType();

		return pTechnoType->LegalTarget
			&& !pTechno->IsBeingWarpedOut()
			&& BulletExt::CheckWeaponValidness(pOwner, pTechno, pTechno->GetCell(), retargetHouses)
			&& BulletExt::GetDistanceFrom(center, pTechno) <= retargetRange
			&& MapClass::GetTotalDamage(100, pBullet->WH, pTechnoType->Armor, 0) != 0
			&& (!pWeapon || BulletExt::GetDistanceFrom(pFirer ? pFirer->GetCoords() : pBullet->SourceCoords, pTechno) <= range)
			&& BulletExt::CheckWeaponCanTarget(pWeaponExt, pFirer, pTechno);
	}
	static inline bool CheckCanDisperse(TechnoClass* const pTechno, HouseClass* const pOwner, const BulletTypeExt* const pType, const CoordStruct& center, const CellClass* const pCell, const int range,
		const AbstractClass* const pTarget, const WeaponTypeClass* const pWeapon, const WeaponTypeExt* const pWeaponExt, TechnoClass* const pFirer)
	{
		const auto pTechnoType = pTechno->GetTechnoType();

		return pTechnoType->LegalTarget
			&& (!pType->DisperseTendency || pType->DisperseDoRepeat || pTechno != pTarget)
			&& !pTechno->IsBeingWarpedOut()
			&& BulletExt::CheckWeaponValidness(pOwner, pTechno, pCell, pWeaponExt->CanTargetHouses)
			&& BulletExt::GetDistanceFrom(center, pTechno) <= range
			&& MapClass::GetTotalDamage(100, pWeapon->Warhead, pTechnoType->Armor, 0) != 0
			&& BulletExt::CheckWeaponCanTarget(pWeaponExt, pFirer, pTechno);
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
	static inline TechnoClass* GetSurfaceFirer(TechnoClass* pFirer)
	{
		for (auto pTrans = pFirer; pTrans; pTrans = pTrans->Transporter)
			pFirer = pTrans;

		return pFirer;
	}
	static inline std::pair<int, int> GetScatterOffsets(BulletClass* pBullet, const CoordStruct& sourceCoord, const CoordStruct& targetCoord)
	{
		const auto pWeapon = pBullet->WeaponType;
		const auto pType = pBullet->Type;
		const auto pTypeExt = BulletTypeExt::Fetch(pType);

		if (!pType->FlakScatter)
			return std::make_pair(static_cast<int>(pTypeExt->BallisticScatter_Min.Get(Leptons(RulesClass::Instance->BallisticScatter / 2))), static_cast<int>(pTypeExt->BallisticScatter_Max.Get(Leptons(RulesClass::Instance->BallisticScatter))));

		const double offsetMult = sourceCoord.DistanceFrom(targetCoord) / (pWeapon ? pWeapon->Range : (10.0 * Unsorted::LeptonsPerCell));
		return std::make_pair(static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Min.Get(Leptons(0))), static_cast<int>(offsetMult * pTypeExt->BallisticScatter_Max.Get(Leptons(RulesClass::Instance->BallisticScatter))));
	}
	static bool CheckExceededCapacity(TechnoClass* pTechno, BulletTypeClass* pBulletType, BulletExt* pBulletExt = nullptr);
	static std::vector<CellStruct> GetCellsInRectangle(const CellStruct bottomStaCell, const CellStruct leftMidCell, const CellStruct rightMidCell, const CellStruct topEndCell);
};

