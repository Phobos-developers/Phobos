#pragma once

#include "PhobosTrajectory.h"

class BombardTrajectoryType final : public PhobosTrajectoryType
{
public:
	BombardTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Bombard)
		, Height { 0.0 }
		, HeightShift { 0.0 }
		, FallPercent { 1.0 }
		, FallPercentShift { 0.0 }
		, FallScatter_Max { Leptons(0) }
		, FallScatter_Min { Leptons(0) }
		, FallSpeed { 0.0 }
		, TargetSnapDistance { Leptons(128) }
		, FreeFallOnTarget { true }
		, NoLaunch { false }
		, TurningPointAnim {}
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual PhobosTrajectory* CreateInstance() const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	Valueable<double> Height;
	Valueable<double> HeightShift;
	Valueable<double> FallPercent;
	Valueable<double> FallPercentShift;
	Valueable<Leptons> FallScatter_Max;
	Valueable<Leptons> FallScatter_Min;
	Valueable<double> FallSpeed;
	Valueable<Leptons> TargetSnapDistance;
	Valueable<bool> FreeFallOnTarget;
	Valueable<bool> NoLaunch;
	Valueable<AnimTypeClass*> TurningPointAnim;
};

class BombardTrajectory final : public PhobosTrajectory
{
public:
	BombardTrajectory() : PhobosTrajectory(TrajectoryFlag::Bombard)
		, IsFalling { false }
		, Height { 0.0 }
		, RemainingDistance { 1 }
		, FallPercent { 1.0 }
		, FallSpeed { 0.0 }
		, TargetSnapDistance { Leptons(128) }
		, FreeFallOnTarget { true }
	{}

	BombardTrajectory(PhobosTrajectoryType const* pType) : PhobosTrajectory(TrajectoryFlag::Bombard)
		, IsFalling { false }
		, Height { 0.0 }
		, RemainingDistance { 1 }
		, FallPercent { 1.0 }
		, FallSpeed { 0.0 }
		, TargetSnapDistance { Leptons(128) }
		, FreeFallOnTarget { true }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	bool IsFalling;
	double Height;
	int RemainingDistance;
	double FallPercent;
	double FallSpeed;
	Leptons TargetSnapDistance;
	bool FreeFallOnTarget;

private:
	bool BulletDetonatePreCheck(BulletClass* pBullet, HouseClass* pOwner, double StraightSpeed);
};
