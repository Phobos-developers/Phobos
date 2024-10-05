#pragma once

#include "PhobosTrajectory.h"

class BombardTrajectoryType final : public PhobosTrajectoryType
{
public:
	BombardTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Bombard)
		, Height { 0.0 }
		, FallPercent { 1.0 }
		, FallPercentShift { 0.0 }
		, FallScatter_Max { Leptons(0) }
		, FallScatter_Min { Leptons(0) }
		, FallSpeed { 0.0 }
		, DetonationDistance { Leptons(102) }
		, ApplyRangeModifiers { false }
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
	virtual PhobosTrajectory* CreateInstance() const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	Valueable<double> Height;
	Valueable<double> FallPercent;
	Valueable<double> FallPercentShift;
	Valueable<Leptons> FallScatter_Max;
	Valueable<Leptons> FallScatter_Min;
	Valueable<double> FallSpeed;
	Valueable<Leptons> DetonationDistance;
	Valueable<bool> ApplyRangeModifiers;
	Valueable<int> DetonationHeight;
	Valueable<bool> EarlyDetonation;
	Valueable<Leptons> TargetSnapDistance;
	Valueable<bool> FreeFallOnTarget;
	Valueable<bool> LeadTimeCalculate;
	Valueable<bool> NoLaunch;
	ValueableVector<AnimTypeClass*> TurningPointAnims;
	Valueable<CoordStruct> OffsetCoord;
	Valueable<int> RotateCoord;
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
	BombardTrajectory(noinit_t) :PhobosTrajectory { noinit_t{} } { }

	BombardTrajectory(BombardTrajectoryType const* trajType) : PhobosTrajectory(TrajectoryFlag::Bombard)
		, IsFalling { false }
		, Height { trajType->Height }
		, RemainingDistance { 1 }
		, FallPercent { trajType->FallPercent }
		, FallSpeed { trajType->FallSpeed }
		, DetonationDistance { trajType->DetonationDistance }
		, DetonationHeight { trajType->DetonationHeight }
		, EarlyDetonation { trajType->EarlyDetonation }
		, TargetSnapDistance { trajType->TargetSnapDistance }
		, FreeFallOnTarget { trajType->FreeFallOnTarget }
		, LeadTimeCalculate { trajType->LeadTimeCalculate }
		, OffsetCoord { static_cast<CoordStruct>(trajType->OffsetCoord) }
		, RotateCoord { trajType->RotateCoord }
		, MirrorCoord { trajType->MirrorCoord }
		, UseDisperseBurst { trajType->UseDisperseBurst }
		, AxisOfRotation { static_cast<CoordStruct>(trajType->AxisOfRotation) }
		, SubjectToGround { trajType->SubjectToGround }
		, LastTargetCoord {}
		, CountOfBurst { 0 }
		, CurrentBurst { 0 }
		, RotateAngle { 0.0 }
		, AscendTime { 1 }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	bool IsFalling;
	double Height;
	int RemainingDistance;
	double FallPercent;
	double FallSpeed;
	Leptons DetonationDistance;
	int DetonationHeight;
	bool EarlyDetonation;
	Leptons TargetSnapDistance;
	bool FreeFallOnTarget;
	bool LeadTimeCalculate;
	CoordStruct OffsetCoord;
	int RotateCoord;
	bool MirrorCoord;
	bool UseDisperseBurst;
	CoordStruct AxisOfRotation;
	bool SubjectToGround;
	CoordStruct LastTargetCoord;
	int CountOfBurst;
	int CurrentBurst;
	double RotateAngle;
	int AscendTime;

private:
	bool BulletDetonatePreCheck(BulletClass* pBullet, HouseClass* pOwner);
	void CalculateLeadTime(BulletClass* pBullet);
	void CalculateDisperseBurst(BulletClass* pBullet, BulletVelocity& pVelocity);
	void CalculateBulletVelocity(BulletVelocity& pVelocity);

private:
	template <typename T>
	void Serialize(T& Stm);
};
