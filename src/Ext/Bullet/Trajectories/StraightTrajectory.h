#pragma once

#include "PhobosTrajectory.h"

class StraightTrajectoryType final : public PhobosTrajectoryType
{
public:
	StraightTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Straight)
		, DetonationDistance { Leptons(102) }
		, TargetSnapDistance { Leptons(0) }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	Valueable<Leptons> DetonationDistance;
	Valueable<Leptons> TargetSnapDistance;
};

class StraightTrajectory final : public PhobosTrajectory
{
public:
	StraightTrajectory() : PhobosTrajectory(TrajectoryFlag::Straight)
		, DetonationDistance { Leptons(102) }
		, TargetSnapDistance { Leptons(0) }
	{}

	StraightTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory(TrajectoryFlag::Straight)
		, DetonationDistance { Leptons(102) }
		, TargetSnapDistance { Leptons(0) }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	Leptons DetonationDistance;
	Leptons TargetSnapDistance;
};
