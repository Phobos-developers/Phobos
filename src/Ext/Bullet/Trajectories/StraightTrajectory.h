#pragma once

#include "PhobosActualTrajectory.h"

class StraightTrajectoryType final : public ActualTrajectoryType
{
public:
	StraightTrajectoryType() : ActualTrajectoryType()
		, PassThrough { false }
		, ConfineAtHeight { 0 }
	{ }

	Valueable<bool> PassThrough;
	Valueable<int> ConfineAtHeight;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual std::unique_ptr<PhobosTrajectory> CreateInstance(BulletClass* pBullet) const override;
	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Straight; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class StraightTrajectory final : public ActualTrajectory
{
public:
	StraightTrajectory(noinit_t) { }
	StraightTrajectory(StraightTrajectoryType const* trajType, BulletClass* pBullet)
		: ActualTrajectory(trajType, pBullet)
		, Type { trajType }
		, DetonationDistance { trajType->DetonationDistance }
	{ }

	const StraightTrajectoryType* Type;
	Leptons DetonationDistance;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Straight; }
	virtual void OnUnlimbo() override;
	virtual bool OnVelocityCheck() override;
	virtual TrajectoryCheckReturnType OnDetonateUpdate(const CoordStruct& position) override;
	virtual void OnPreDetonate() override;
	virtual const PhobosTrajectoryType* GetType() const override { return this->Type; }
	virtual void OpenFire() override;
	virtual void FireTrajectory() override;
	virtual bool GetCanHitGround() const override { return this->Type->SubjectToGround; }

private:
	CoordStruct CalculateBulletLeadTime();
	int GetVelocityZ();
	bool PassAndConfineAtHeight();

	template <typename T>
	void Serialize(T& Stm);
};
