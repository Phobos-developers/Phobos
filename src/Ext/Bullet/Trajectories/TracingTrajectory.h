#pragma once

#include "PhobosVirtualTrajectory.h"

enum class TraceTargetMode : int
{
	Connection = 0,
	Global = 1,
	Body = 2,
	Turret = 3,
	RotateCW = 4,
	RotateCCW = 5,
};

class TracingTrajectoryType final : public VirtualTrajectoryType
{
public:
	TracingTrajectoryType() : VirtualTrajectoryType()
		, TraceMode { TraceTargetMode::Connection }
		, TraceTheTarget { true }
		, CreateAtTarget { false }
		, ChasableDistance { Leptons(0) }
	{ }

	Valueable<TraceTargetMode> TraceMode;
	Valueable<bool> TraceTheTarget;
	Valueable<bool> CreateAtTarget;
	Valueable<Leptons> ChasableDistance;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual std::unique_ptr<PhobosTrajectory> CreateInstance(BulletClass* pBullet) const override;
	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Tracing; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class TracingTrajectory final : public VirtualTrajectory
{
public:
	TracingTrajectory(noinit_t) { }
	TracingTrajectory(TracingTrajectoryType const* trajType, BulletClass* pBullet)
		: VirtualTrajectory(trajType, pBullet)
		, Type { trajType }
	{ }

	const TracingTrajectoryType* Type;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Tracing; }
	virtual void OnUnlimbo() override;
	virtual bool OnEarlyUpdate() override;
	virtual bool OnVelocityCheck() override;
	virtual const PhobosTrajectoryType* GetType() const override { return this->Type; }
	virtual void OpenFire() override;
	virtual bool GetCanHitGround() const override { return false; }
	virtual CoordStruct GetRetargetCenter() const override { return this->Bullet->Location; }

private:
	bool ChangeVelocity();

	template <typename T>
	void Serialize(T& Stm);
};
