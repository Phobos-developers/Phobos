#pragma once

#include "PhobosTrajectory.h"

class SampleTrajectoryType final : public PhobosTrajectoryType
{
public:
	SampleTrajectoryType() : PhobosTrajectoryType()
		, TargetSnapDistance { Leptons(128) }
	{ }

	Valueable<Leptons> TargetSnapDistance;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual std::unique_ptr<PhobosTrajectory> CreateInstance(BulletClass* pBullet) const override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Invalid; } // TrajectoryFlag
	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};

class SampleTrajectory final : public PhobosTrajectory
{
public:
	SampleTrajectory(noinit_t) { }
	SampleTrajectory(SampleTrajectoryType const* trajType, BulletClass* pBullet)
		: PhobosTrajectory(trajType, pBullet)
		, Type { trajType }
	{ }

	SampleTrajectoryType const* Type;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Invalid; } // TrajectoryFlag
	virtual void OnUnlimbo() override;
	virtual bool OnEarlyUpdate() override;
	virtual bool OnVelocityCheck() override;
	virtual void OnVelocityUpdate(BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnDetonateUpdate(const CoordStruct& position) override;
	virtual void OnPreDetonate() override;
	virtual const PhobosTrajectoryType* GetType() const override { return this->Type; }
	virtual void OpenFire() override;
	virtual bool GetCanHitGround() const override;
	virtual CoordStruct GetRetargetCenter() const override;
	virtual void SetBulletNewTarget(AbstractClass* const pTarget) override;
	virtual bool CalculateBulletVelocity(const double speed) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};
