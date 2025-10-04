#pragma once

#include "../PhobosVirtualTrajectory.h"

class EngraveTrajectoryType final : public VirtualTrajectoryType
{
public:
	EngraveTrajectoryType() : VirtualTrajectoryType()
		, AttachToTarget { false }
		, UpdateDirection { false }
	{ }

	Valueable<bool> AttachToTarget;
	Valueable<bool> UpdateDirection;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual std::unique_ptr<PhobosTrajectory> CreateInstance(BulletClass* pBullet) const override;
	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Engrave; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class EngraveTrajectory final : public VirtualTrajectory
{
public:
	EngraveTrajectory(noinit_t) { }
	EngraveTrajectory(EngraveTrajectoryType const* pTrajType, BulletClass* pBullet)
		: VirtualTrajectory(pTrajType, pBullet)
		, Type { pTrajType }
		, RotateRadian { 0 }
	{ }

	const EngraveTrajectoryType* Type;
	double RotateRadian;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Engrave; }
	virtual bool OnVelocityCheck() override;
	virtual const PhobosTrajectoryType* GetType() const override { return this->Type; }
	virtual void OpenFire() override;
	virtual bool GetCanHitGround() const override { return false; }
	virtual bool CalculateBulletVelocity(const double speed) override;

private:
	int GetFloorCoordHeight(const CoordStruct& coord);
	void ChangeVelocity();
	bool PlaceOnCorrectHeight();

	template <typename T>
	void Serialize(T& Stm);
};
