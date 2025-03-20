#pragma once

#include "PhobosVirtualTrajectory.h"

class EngraveTrajectoryType final : public VirtualTrajectoryType
{
public:
	EngraveTrajectoryType() : VirtualTrajectoryType()
		, IsLaser { true }
		, IsIntense { false }
		, IsHouseColor { false }
		, IsSingleColor { false }
		, LaserInnerColor { { 0, 0, 0 } }
		, LaserOuterColor { { 0, 0, 0 } }
		, LaserOuterSpread { { 0, 0, 0 } }
		, LaserThickness { 3 }
		, LaserDuration { 1 }
		, LaserDelay { 1 }
		, AttachToTarget { false }
		, UpdateDirection { false }
	{ }

	Valueable<bool> IsLaser;
	Valueable<bool> IsIntense;
	Valueable<bool> IsHouseColor;
	Valueable<bool> IsSingleColor;
	Valueable<ColorStruct> LaserInnerColor;
	Valueable<ColorStruct> LaserOuterColor;
	Valueable<ColorStruct> LaserOuterSpread;
	Valueable<int> LaserThickness;
	Valueable<int> LaserDuration;
	Valueable<int> LaserDelay;
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
	EngraveTrajectory(EngraveTrajectoryType const* trajType, BulletClass* pBullet)
		: VirtualTrajectory(trajType, pBullet)
		, Type { trajType }
		, LaserTimer {}
		, RotateRadian { 0 }
	{ }

	const EngraveTrajectoryType* Type;
	CDTimerClass LaserTimer;
	double RotateRadian;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Engrave; }
	virtual void OnUnlimbo() override;
	virtual bool OnEarlyUpdate() override;
	virtual bool OnVelocityCheck() override;
	virtual const PhobosTrajectoryType* GetType() const override { return this->Type; }
	virtual void OpenFire() override;
	virtual bool GetCanHitGround() const override { return false; }
	virtual bool CalculateBulletVelocity(const double speed) override;

private:
	int GetFloorCoordHeight(const CoordStruct& coord);
	void ChangeVelocity();
	bool PlaceOnCorrectHeight();
	void DrawEngraveLaser();

	template <typename T>
	void Serialize(T& Stm);
};
