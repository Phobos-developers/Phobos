#pragma once
#include <BulletClass.h>

#include <Ext/BulletType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <New/Entity/LaserTrailClass.h>
#include <Ext/Object/Body.h>

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

	BulletExt(BulletClass* OwnerObject) : ObjectExt(OwnerObject)
		, TypeExtData { nullptr }
		, FirerHouse { nullptr }
		, CurrentStrength { 0 }
		, InterceptorTechnoType { nullptr }
		, InterceptedStatus { InterceptedStatus::None }
		, DetonateOnInterception { true }
		, LaserTrails {}
		, Trajectory { nullptr }
		, SnappedToTarget { false }
		, DamageNumberOffset { INT32_MIN }
		, ParabombFallRate { 0 }
		, IsInstantDetonation { false }
		, FirepowerMult { 1.0 }
		, IsSplitFromAirburst { false }
	{ }

	virtual ~BulletExt() = default;

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
};

