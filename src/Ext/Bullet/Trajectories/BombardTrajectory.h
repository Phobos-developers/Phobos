#pragma once

#include "PhobosActualTrajectory.h"

class BombardTrajectoryType final : public ActualTrajectoryType
{
public:
	BombardTrajectoryType() : ActualTrajectoryType()
		, Height { 0.0 }
		, FallPercent { 1.0 }
		, FallPercentShift { 0.0 }
		, FallScatter_Max { Leptons(0) }
		, FallScatter_Min { Leptons(0) }
		, FallScatter_Linear { false }
		, FallSpeed {}
		, FreeFallOnTarget { true }
		, NoLaunch { false }
		, TurningPointAnims {}
	{}

	Valueable<double> Height;
	Valueable<double> FallPercent;
	Valueable<double> FallPercentShift;
	Valueable<Leptons> FallScatter_Max;
	Valueable<Leptons> FallScatter_Min;
	Valueable<bool> FallScatter_Linear;
	Nullable<double> FallSpeed;
	Valueable<bool> FreeFallOnTarget;
	Valueable<bool> NoLaunch;
	ValueableVector<AnimTypeClass*> TurningPointAnims;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual std::unique_ptr<PhobosTrajectory> CreateInstance(BulletClass* pBullet) const override;
	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Bombard; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class BombardTrajectory final : public ActualTrajectory
{
public:
	BombardTrajectory(noinit_t) { }
	BombardTrajectory(BombardTrajectoryType const* trajType, BulletClass* pBullet)
		: ActualTrajectory(trajType, pBullet)
		, Type { trajType }
		, Height { trajType->Height }
		, FallPercent { trajType->FallPercent - trajType->FallPercentShift }
		, IsFalling { false }
		, ToFalling { false }
		, InitialTargetCoord {}
		, RotateRadian { 0 }
	{}

	const BombardTrajectoryType* Type;
	double Height;
	double FallPercent;
	bool IsFalling;
	bool ToFalling;
	CoordStruct InitialTargetCoord;
	double RotateRadian;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Bombard; }
	virtual void OnUnlimbo() override;
	virtual bool OnVelocityCheck() override;
	virtual TrajectoryCheckReturnType OnDetonateUpdate(const CoordStruct& position) override;
	virtual const PhobosTrajectoryType* GetType() const override { return this->Type; }
	virtual void OpenFire() override;
	virtual void FireTrajectory() override;
	virtual bool GetCanHitGround() const override { return this->Type->SubjectToGround || this->IsFalling; }
	virtual void SetBulletNewTarget(AbstractClass* const pTarget) override;
	virtual void MultiplyBulletVelocity(const double ratio, const bool shouldDetonate) override;

private:
	CoordStruct CalculateMiddleCoords();
	void CalculateTargetCoords();
	CoordStruct CalculateBulletLeadTime();
	bool BulletVelocityChange();
	void RefreshBulletLineTrail();

	template <typename T>
	void Serialize(T& Stm);
};
