#pragma once

#include "PhobosTrajectory.h"

class StraightTrajectoryType final : public PhobosTrajectoryType
{
public:
	StraightTrajectoryType() : PhobosTrajectoryType()
		, DetonationDistance { Leptons(102) }
		, ApplyRangeModifiers { false }
		, TargetSnapDistance { Leptons(128) }
		, PassThrough { false }
	{ }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual std::unique_ptr<PhobosTrajectory> CreateInstance() const override;
	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Straight; }
	Valueable<Leptons> DetonationDistance;
	Valueable<bool> ApplyRangeModifiers;
	Valueable<Leptons> TargetSnapDistance;
	Valueable<bool> PassThrough;

private:
	template <typename T>
	void Serialize(T& Stm);
};

class StraightTrajectory final : public PhobosTrajectory
{
public:
	StraightTrajectory(noinit_t) { }

	StraightTrajectory(StraightTrajectoryType const* trajType) : Type { trajType }
		, DetonationDistance { trajType->DetonationDistance }
		, TargetSnapDistance { trajType->TargetSnapDistance }
		, FirerZPosition { 0 }
		, TargetZPosition { 0 }
	{ }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Straight; }
	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	Leptons DetonationDistance;
	Leptons TargetSnapDistance;
	int FirerZPosition;
	int TargetZPosition;
	StraightTrajectoryType const* Type;
private:
	int GetVelocityZ(BulletClass* pBullet);
	int GetFirerZPosition(BulletClass* pBullet);
	int GetTargetZPosition(BulletClass* pBullet);
	bool ElevationDetonationCheck(BulletClass* pBullet);

	template <typename T>
	void Serialize(T& Stm);
};
