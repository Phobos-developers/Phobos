#pragma once

#include "PhobosTrajectory.h"

class SampleTrajectoryType final : public PhobosTrajectoryType
{
public:
	SampleTrajectoryType() : PhobosTrajectoryType()
		, TargetSnapDistance { Leptons(128) }
	{ }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual std::unique_ptr<PhobosTrajectory> CreateInstance() const override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Invalid; } // TrajectoryFlag
	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	Valueable<Leptons> TargetSnapDistance;

private:
	template <typename T>
	void Serialize(T& Stm);
};

class SampleTrajectory final : public PhobosTrajectory
{
public:
	SampleTrajectory(noinit_t) { }

	SampleTrajectory(SampleTrajectoryType const* trajType) : Type { trajType }
		, TargetSnapDistance { trajType->TargetSnapDistance }
	{ }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Invalid; } // TrajectoryFlag
	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	SampleTrajectoryType const* Type;
	Leptons TargetSnapDistance;

private:
	template <typename T>
	void Serialize(T& Stm);
};
