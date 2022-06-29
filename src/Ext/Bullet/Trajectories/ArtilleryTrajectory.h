#pragma once

#include "PhobosTrajectory.h"

class ArtilleryTrajectoryType final : public PhobosTrajectoryType
{
public:
	ArtilleryTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Artillery)
		, MaxHeight{ 0.0 }
	{ }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
	double MaxHeight;
};

class ArtilleryTrajectory final : public PhobosTrajectory
{
public:
	ArtilleryTrajectory() : PhobosTrajectory(TrajectoryFlag::Artillery)
		, InitialTargetLocation{ CoordStruct::Empty }
		, InitialSourceLocation{ CoordStruct::Empty }
		, MaxHeight{ 0.0 }
	{}

	ArtilleryTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory(TrajectoryFlag::Artillery)
		, InitialTargetLocation{ CoordStruct::Empty }
		, InitialSourceLocation{ CoordStruct::Empty }
		, MaxHeight{ 0.0 }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual void OnAI(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	CoordStruct InitialTargetLocation;
	CoordStruct InitialSourceLocation;
	double MaxHeight;
};
