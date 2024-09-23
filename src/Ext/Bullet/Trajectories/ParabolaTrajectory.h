#pragma once

#include "PhobosTrajectory.h"

enum class ParabolaFireMode
{
	Speed = 0,
	Height = 1,
	Angle = 2,
	SpeedAndHeight = 3,
	HeightAndAngle = 4,
	SpeedAndAngle = 5,
};

class ParabolaTrajectoryType final : public PhobosTrajectoryType
{
public:
	ParabolaTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Parabola)
		, DetonationDistance { Leptons(102) }
		, TargetSnapDistance { Leptons(128) }
		, OpenFireMode { ParabolaFireMode::Speed }
		, ThrowHeight { 600 }
		, LaunchAngle { 30.0 }
		, LeadTimeCalculate { false }
		, LeadTimeSimplify { false }
		, LeadTimeMultiplier { 1.0 }
		, DetonationAngle { -90.0 }
		, DetonationHeight { -1 }
		, BounceTimes { 0 }
		, BounceOnWater { false }
		, BounceDetonate { false }
		, BounceAttenuation { 0.8 }
		, BounceCoefficient { 0.8 }
		, OffsetCoord { { 0, 0, 0 } }
		, RotateCoord { 0 }
		, MirrorCoord { true }
		, UseDisperseBurst { false }
		, AxisOfRotation { { 0, 0, 1 } }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual PhobosTrajectory* CreateInstance() const override;
	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	Valueable<Leptons> DetonationDistance;
	Valueable<Leptons> TargetSnapDistance;
	Valueable<ParabolaFireMode> OpenFireMode;
	Valueable<int> ThrowHeight;
	Valueable<double> LaunchAngle;
	Valueable<bool> LeadTimeCalculate;
	Valueable<bool> LeadTimeSimplify;
	Valueable<double> LeadTimeMultiplier;
	Valueable<double> DetonationAngle;
	Valueable<int> DetonationHeight;
	Valueable<int> BounceTimes;
	Valueable<bool> BounceOnWater;
	Valueable<bool> BounceDetonate;
	Valueable<double> BounceAttenuation;
	Valueable<double> BounceCoefficient;
	Valueable<CoordStruct> OffsetCoord;
	Valueable<int> RotateCoord;
	Valueable<bool> MirrorCoord;
	Valueable<bool> UseDisperseBurst;
	Valueable<CoordStruct> AxisOfRotation;

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParabolaTrajectory final : public PhobosTrajectory
{
public:
	ParabolaTrajectory(noinit_t) :PhobosTrajectory { noinit_t{} } { }

	ParabolaTrajectory(PhobosTrajectoryType const* pType) : PhobosTrajectory(TrajectoryFlag::Parabola)
		, DetonationDistance { Leptons(102) }
		, TargetSnapDistance { Leptons(128) }
		, OpenFireMode { ParabolaFireMode::Speed }
		, ThrowHeight { 600 }
		, LaunchAngle { 30.0 }
		, LeadTimeCalculate { false }
		, LeadTimeSimplify { false }
		, LeadTimeMultiplier { 1.0 }
		, DetonationAngle { -90.0 }
		, DetonationHeight { -1 }
		, BounceTimes { 0 }
		, BounceOnWater { false }
		, BounceDetonate { false }
		, BounceAttenuation { 0.8 }
		, BounceCoefficient { 0.8 }
		, OffsetCoord {}
		, RotateCoord { 0 }
		, MirrorCoord { true }
		, UseDisperseBurst { false }
		, AxisOfRotation {}
		, ShouldDetonate { false }
		, ShouldBounce { false }
		, NeedExtraCheck { false }
		, LastTargetCoord {}
		, CurrentBurst { 0 }
		, CountOfBurst { 0 }
		, WaitOneFrame {}
		, LastVelocity {}
	{
		auto const pFinalType = static_cast<const ParabolaTrajectoryType*>(pType);

		this->DetonationDistance = pFinalType->DetonationDistance;
		this->TargetSnapDistance = pFinalType->TargetSnapDistance;
		this->OpenFireMode = pFinalType->OpenFireMode;
		this->ThrowHeight = pFinalType->ThrowHeight > 0 ? pFinalType->ThrowHeight : 600;
		this->LaunchAngle = pFinalType->LaunchAngle;
		this->LeadTimeCalculate = pFinalType->LeadTimeCalculate;
		this->LeadTimeSimplify = pFinalType->LeadTimeSimplify;
		this->LeadTimeMultiplier = pFinalType->LeadTimeMultiplier;
		this->DetonationAngle = pFinalType->DetonationAngle;
		this->DetonationHeight = pFinalType->DetonationHeight;
		this->BounceTimes = pFinalType->BounceTimes;
		this->BounceOnWater = pFinalType->BounceOnWater;
		this->BounceDetonate = pFinalType->BounceDetonate;
		this->BounceAttenuation = pFinalType->BounceAttenuation;
		this->BounceCoefficient = pFinalType->BounceCoefficient;
		this->OffsetCoord = pFinalType->OffsetCoord;
		this->RotateCoord = pFinalType->RotateCoord;
		this->MirrorCoord = pFinalType->MirrorCoord;
		this->UseDisperseBurst = pFinalType->UseDisperseBurst;
		this->AxisOfRotation = pFinalType->AxisOfRotation;
	}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	Leptons DetonationDistance;
	Leptons TargetSnapDistance;
	ParabolaFireMode OpenFireMode;
	int ThrowHeight;
	double LaunchAngle;
	bool LeadTimeCalculate;
	bool LeadTimeSimplify;
	double LeadTimeMultiplier;
	double DetonationAngle;
	int DetonationHeight;
	int BounceTimes;
	bool BounceOnWater;
	bool BounceDetonate;
	double BounceAttenuation;
	double BounceCoefficient;
	CoordStruct OffsetCoord;
	int RotateCoord;
	bool MirrorCoord;
	bool UseDisperseBurst;
	CoordStruct AxisOfRotation;
	bool ShouldDetonate;
	bool ShouldBounce;
	bool NeedExtraCheck;
	CoordStruct LastTargetCoord;
	int CurrentBurst;
	int CountOfBurst;
	CDTimerClass WaitOneFrame;
	BulletVelocity LastVelocity;

private:
	template <typename T>
	void Serialize(T& Stm);

	void PrepareForOpenFire(BulletClass* pBullet);
	bool BulletPrepareCheck(BulletClass* pBullet);
	void CalculateBulletVelocityRightNow(BulletClass* pBullet, CoordStruct* pSourceCoords, double gravity);
	void CalculateBulletVelocityLeadTime(BulletClass* pBullet, CoordStruct* pSourceCoords, double gravity);
	void CheckIfNeedExtraCheck(BulletClass* pBullet);
	double SearchVelocity(double horizontalDistance, int distanceCoordsZ, double radian, double gravity);
	double CheckVelocityEquation(double horizontalDistance, int distanceCoordsZ, double velocity, double radian, double gravity);
	double SolveFixedSpeedMeetTime(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double horizontalSpeed);
	double SearchFixedHeightMeetTime(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double gravity);
	double CheckFixedHeightEquation(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double meetTime, double gravity);
	double SearchFixedAngleMeetTime(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double radian, double gravity);
	double CheckFixedAngleEquation(CoordStruct* pSourceCrd, CoordStruct* pTargetCrd, CoordStruct* pOffsetCrd, double meetTime, double radian, double gravity);
	bool CalculateBulletVelocityAfterBounce(BulletClass* pBullet, CellClass* pCell, double gravity);
	BulletVelocity GetGroundNormalVector(BulletClass* pBullet, CellClass* pCell);
	bool CheckBulletHitCliff(short X, short Y, int bulletHeight, int lastCellHeight);
	bool BulletDetonatePreCheck(BulletClass* pBullet);
	bool BulletDetonateLastCheck(BulletClass* pBullet, double gravity);
	void BulletDetonateEffectuate(BulletClass* pBullet, double velocityMult);
};
