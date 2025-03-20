#pragma once

#include "PhobosActualTrajectory.h"

#include <Ext/WeaponType/Body.h>

class MissileTrajectoryType final : public ActualTrajectoryType
{
public:
	MissileTrajectoryType() : ActualTrajectoryType()
		, UniqueCurve { false }
		, FacingCoord { false }
		, ReduceCoord { true }
		, PreAimCoord { { 0, 0, 0 } }
		, LaunchSpeed { 0 }
		, Acceleration { 10.0 }
		, TurningSpeed { 10.0 }
		, LockDirection { false }
		, CruiseEnable { false }
		, CruiseUnableRange { Leptons(1280) }
		, CruiseAltitude { 800 }
		, CruiseAlongLevel { false }
		, SuicideAboveRange { -3.0 }
		, SuicideShortOfROT { false }
	{ }

	Valueable<bool> UniqueCurve;
	Valueable<bool> FacingCoord;
	Valueable<bool> ReduceCoord;
	Valueable<CoordStruct> PreAimCoord;
	Valueable<double> LaunchSpeed;
	Valueable<double> Acceleration;
	Valueable<double> TurningSpeed;
	Valueable<bool> LockDirection;
	Valueable<bool> CruiseEnable;
	Valueable<Leptons> CruiseUnableRange;
	Valueable<int> CruiseAltitude;
	Valueable<bool> CruiseAlongLevel;
	Valueable<double> SuicideAboveRange;
	Valueable<bool> SuicideShortOfROT;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual std::unique_ptr<PhobosTrajectory> CreateInstance(BulletClass* pBullet) const override;
	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Missile; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class MissileTrajectory final : public ActualTrajectory
{
public:
	MissileTrajectory(noinit_t) { }
	MissileTrajectory(MissileTrajectoryType const* trajType, BulletClass* pBullet)
		: ActualTrajectory(trajType, pBullet)
		, Type { trajType }
		, CruiseEnable { trajType->CruiseEnable }
		, InStraight { false }
		, Accelerate { true }
		, OriginalDistance { 0 }
		, OffsetCoord { CoordStruct::Empty }
		, PreAimDistance { 0 }
		, LastDotProduct { 0 }
	{ }

	const MissileTrajectoryType* Type;
	bool CruiseEnable;
	bool InStraight;
	bool Accelerate;
	int OriginalDistance;
	CoordStruct OffsetCoord;
	double PreAimDistance;
	double LastDotProduct;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Missile; }
	virtual void OnUnlimbo() override;
	virtual bool OnEarlyUpdate() override;
	virtual bool OnVelocityCheck() override;
	virtual TrajectoryCheckReturnType OnDetonateUpdate(const CoordStruct& position) override;
	virtual const PhobosTrajectoryType* GetType() const override { return this->Type; }
	virtual void OpenFire() override;
	virtual CoordStruct GetRetargetCenter() const override;
	virtual void SetBulletNewTarget(AbstractClass* const pTarget) override;
	virtual bool CalculateBulletVelocity(const double speed) override;

private:
	void InitializeBulletNotCurve();
	CoordStruct GetPreAimCoordsWithBurst();
	bool CalculateReducedVelocity(double rotateRadian);
	bool CurveVelocityChange();
	bool NotCurveVelocityChange();
	bool StandardVelocityChange();
	bool ChangeBulletVelocity(const CoordStruct& targetLocation);

	template <typename T>
	void Serialize(T& Stm);
};
