#pragma once

#include "PhobosTrajectory.h"

class FullLineTrajectoryType final : public PhobosTrajectoryType
{
public:
	FullLineTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::FullLine)
	{ }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
};

class FullLineTrajectory final : public PhobosTrajectory
{
public:
	FullLineTrajectory() : PhobosTrajectory(TrajectoryFlag::FullLine)
	{ }

	FullLineTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory(TrajectoryFlag::FullLine)
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual void OnAI(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;
};
