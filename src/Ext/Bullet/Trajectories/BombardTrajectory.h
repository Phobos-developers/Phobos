#pragma once

#include "PhobosTrajectory.h"

class BombardTrajectoryType final : public PhobosTrajectoryType
{
public:
	BombardTrajectoryType() : PhobosTrajectoryType()
		, Height { 0.0 }
		, FallPercent { 1.0 }
		, FallPercentShift { 0.0 }
		, FallScatter_Max { Leptons(0) }
		, FallScatter_Min { Leptons(0) }
		, FallScatter_Linear { false }
		, FallSpeed { 0.0 }
		, DetonationDistance { Leptons(102) }
		, DetonationHeight { -1 }
		, EarlyDetonation { false }
		, TargetSnapDistance { Leptons(128) }
		, FreeFallOnTarget { true }
		, LeadTimeCalculate { false }
		, NoLaunch { false }
		, TurningPointAnims {}
		, OffsetCoord { { 0, 0, 0 } }
		, RotateCoord { 0 }
		, MirrorCoord { true }
		, UseDisperseBurst { false }
		, AxisOfRotation { { 0, 0, 1 } }
		, SubjectToGround { false }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual std::unique_ptr<PhobosTrajectory> CreateInstance() const override;
	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Bombard; }

	Valueable<double> Height;
	Valueable<double> FallPercent;
	Valueable<double> FallPercentShift;
	Valueable<Leptons> FallScatter_Max;
	Valueable<Leptons> FallScatter_Min;
	Valueable<bool> FallScatter_Linear;
	Valueable<double> FallSpeed;
	Valueable<Leptons> DetonationDistance;
	Valueable<int> DetonationHeight;
	Valueable<bool> EarlyDetonation;
	Valueable<Leptons> TargetSnapDistance;
	Valueable<bool> FreeFallOnTarget;
	Valueable<bool> LeadTimeCalculate;
	Valueable<bool> NoLaunch;
	ValueableVector<AnimTypeClass*> TurningPointAnims;
	Valueable<CoordStruct> OffsetCoord;
	Valueable<double> RotateCoord;
	Valueable<bool> MirrorCoord;
	Valueable<bool> UseDisperseBurst;
	Valueable<CoordStruct> AxisOfRotation;
	Valueable<bool> SubjectToGround;

private:
	template <typename T>
	void Serialize(T& Stm);
};

class BombardTrajectory final : public PhobosTrajectory
{
public:
	BombardTrajectory(noinit_t) { }

	BombardTrajectory(BombardTrajectoryType const* trajType) : Type { trajType }
		, Height { trajType->Height }
		, FallPercent { trajType->FallPercent - trajType->FallPercentShift }
		, OffsetCoord { trajType->OffsetCoord.Get() }
		, UseDisperseBurst { trajType->UseDisperseBurst }
		, IsFalling { false }
		, ToFalling { false }
		, RemainingDistance { 1 }
		, LastTargetCoord {}
		, InitialTargetCoord {}
		, CountOfBurst { 0 }
		, CurrentBurst { 0 }
		, RotateAngle { 0 }
		, WaitOneFrame { 0 }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Bombard; }
	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	const BombardTrajectoryType* Type;
	double Height;
	double FallPercent;
	CoordStruct OffsetCoord;
	bool UseDisperseBurst;
	bool IsFalling;
	bool ToFalling;
	int RemainingDistance;
	CoordStruct LastTargetCoord;
	CoordStruct InitialTargetCoord;
	int CountOfBurst;
	int CurrentBurst;
	double RotateAngle;
	int WaitOneFrame;

private:
	template <typename T>
	void Serialize(T& Stm);

	void PrepareForOpenFire(BulletClass* pBullet);
	CoordStruct CalculateMiddleCoords(BulletClass* pBullet);
	void CalculateTargetCoords(BulletClass* pBullet);
	CoordStruct CalculateBulletLeadTime(BulletClass* pBullet);
	void CalculateDisperseBurst(BulletClass* pBullet);
	bool BulletPrepareCheck(BulletClass* pBullet);
	bool BulletDetonatePreCheck(BulletClass* pBullet);
	bool BulletDetonateRemainCheck(BulletClass* pBullet, HouseClass* pOwner);
	void BulletVelocityChange(BulletClass* pBullet);
	void RefreshBulletLineTrail(BulletClass* pBullet);
};
