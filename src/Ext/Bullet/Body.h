#pragma once
#include <BulletClass.h>

#include <Ext/BulletType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <New/Entity/LaserTrailClass.h>
#include <Ext/Object/Body.h>
#include <New/Entity/VectorRevibedState.h>
#include <New/Type/VectorTypeClass.h>

// VectorRevibed 弹体侧运行时：一个 Vector 实例（类型 + 状态）
struct VectorBulletRuntime
{
	VectorTypeClass* Type = nullptr;
	VectorRevibedState State;
	bool NeedInit = true; // 首次 / 链切换后置真，Step 前 Init

	void Reset()
	{
		Type = nullptr;
		State = VectorRevibedState();
		NeedInit = true;
	}

	template <typename T>
	bool Process(T& stream)
	{
		return stream
			.Process(this->Type)
			.Process(this->NeedInit)
			.Process(this->State)
			.Success();
	}
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
	int DistanceTraveled;

	TrajectoryPointer Trajectory;

	// VectorRevibed：多实例并存 + 每帧合并结果（HasActiveVector 判据 + UpdateEnd 应用）
	std::vector<VectorBulletRuntime> VectorList;
	VectorRevibedResult LastResult;
	CoordStruct VectorStartPos; // VectorAI 每帧开头快照（Kratos _vectorStartPos 语义）
	bool VectorStarted = false; // 挂载一次性标记：true=已挂载过（含已结束），禁止 Duration 结束后重挂

	bool HasActiveVector() const
	{
		// 官方 YRpp 的 Vector3D 无 IsEmpty，用 == CoordStruct::Empty
		return LastResult.Force || !(LastResult.MoveDisp == CoordStruct::Empty) || LastResult.Freeze;
	}

	void VectorAI();

	BulletExt(BulletClass* OwnerObject) : ObjectExt(OwnerObject)
		, TypeExtData { nullptr }
		, FirerHouse { nullptr }
		, CurrentStrength { 0 }
		, InterceptorTechnoType { nullptr }
		, InterceptedStatus { InterceptedStatus::None }
		, DetonateOnInterception { true }
		, LaserTrails {}
		, Trajectory { nullptr }
		, VectorList {}
		, LastResult {}
		, VectorStartPos {}
		, SnappedToTarget { false }
		, DamageNumberOffset { INT32_MIN }
		, ParabombFallRate { 0 }
		, IsInstantDetonation { false }
		, FirepowerMult { 1.0 }
		, IsSplitFromAirburst { false }
		, DistanceTraveled { 0 }
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

