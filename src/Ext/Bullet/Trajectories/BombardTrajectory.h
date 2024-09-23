#pragma once

#include "PhobosTrajectory.h"

class BombardTrajectoryType final : public PhobosTrajectoryType
{
public:
	BombardTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Bombard)
		, Height { 0.0 }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual PhobosTrajectory* CreateInstance() const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	double Height;

private:
	template <typename T>
	void Serialize(T& Stm);
};

class BombardTrajectory final : public PhobosTrajectory
{
public:
	BombardTrajectory(noinit_t) :PhobosTrajectory { noinit_t{} } { }

	BombardTrajectory(PhobosTrajectoryType const* pType) : PhobosTrajectory(TrajectoryFlag::Bombard)
		, IsFalling { false }
		, Height { 0.0 }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override {};
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	bool IsFalling;
	double Height;

private:
	template <typename T>
	void Serialize(T& Stm);
};
