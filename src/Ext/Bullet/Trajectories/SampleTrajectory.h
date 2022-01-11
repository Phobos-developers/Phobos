#pragma once

#include "PhobosTrajectory.h"

/*
* This is a sample class telling you how to make a new type of Trajectory
* Author: secsome
*/

// Used in BulletTypeExt
class SampleTrajectoryType final : public PhobosTrajectoryType
{
public:
	SampleTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Sample)
		, ExtraHeight { 0.0 }
	{ }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	// Your type properties
	double ExtraHeight;
};

// Used in BulletExt
class SampleTrajectory final : public PhobosTrajectory
{
public:
	// This constructor is for Save & Load
	SampleTrajectory() : PhobosTrajectory(TrajectoryFlag::Sample)
		, IsFalling { false }
	{}

	SampleTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory(TrajectoryFlag::Sample)
		, IsFalling { false }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual void OnAI(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;

	// Your properties
	bool IsFalling;
};