#pragma once

#include "../PhobosActualTrajectory.h"

enum class ParabolaFireMode : unsigned char
{
	Speed = 0,
	Height = 1,
	Angle = 2,
	SpeedAndHeight = 3,
	HeightAndAngle = 4,
	SpeedAndAngle = 5
};

class ParabolaTrajectoryType final : public ActualTrajectoryType
{
public:
	ParabolaTrajectoryType() : ActualTrajectoryType()
		, OpenFireMode { ParabolaFireMode::Speed }
		, ThrowHeight { 600 }
		, LaunchAngle { 30.0 }
		, DetonationAngle { -90.0 }
		, BounceTimes { 0 }
		, BounceOnTarget { AffectedTarget::Land }
		, BounceOnHouses { AffectedHouse::All }
		, BounceDetonate { false }
		, BounceAttenuation { 0.8 }
		, BounceCoefficient { 0.8 }
	{ }

	Valueable<ParabolaFireMode> OpenFireMode;
	Valueable<int> ThrowHeight;
	Valueable<double> LaunchAngle;
	Valueable<double> DetonationAngle;
	Valueable<int> BounceTimes;
	Valueable<AffectedTarget> BounceOnTarget;
	Valueable<AffectedHouse> BounceOnHouses;
	Valueable<bool> BounceDetonate;
	Valueable<double> BounceAttenuation;
	Valueable<double> BounceCoefficient;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual std::unique_ptr<PhobosTrajectory> CreateInstance(BulletClass* pBullet) const override;
	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Parabola; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParabolaTrajectory final : public ActualTrajectory
{
public:
	static constexpr int Attempts = 10;
	static constexpr double Delta = 1e-5;

	ParabolaTrajectory(noinit_t) { }
	ParabolaTrajectory(ParabolaTrajectoryType const* pTrajType, BulletClass* pBullet)
		: ActualTrajectory(pTrajType, pBullet)
		, Type { pTrajType }
		, ThrowHeight { pTrajType->ThrowHeight > 0 ? pTrajType->ThrowHeight : 600 }
		, BounceTimes { pTrajType->BounceTimes }
		, LastVelocity {}
	{ }

	const ParabolaTrajectoryType* Type;
	int ThrowHeight;
	int BounceTimes;
	BulletVelocity LastVelocity;

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Parabola; }
	virtual void OnUnlimbo() override;
	virtual bool OnVelocityCheck() override;
	virtual TrajectoryCheckReturnType OnDetonateUpdate(const CoordStruct& position) override;
	virtual void OnPreDetonate() override;
	virtual const PhobosTrajectoryType* GetType() const override { return this->Type; }
	virtual void OpenFire() override;
	virtual void FireTrajectory() override;
	virtual bool GetCanHitGround() const override { return this->BounceTimes <= 0; }
	virtual void MultiplyBulletVelocity(const double ratio, const bool shouldDetonate) override;

private:
	void CalculateBulletVelocityRightNow(const CoordStruct& pSourceCoords, const double gravity);
	void CalculateBulletVelocityLeadTime(const CoordStruct& pSourceCoords, const double gravity);
	double SearchVelocity(const double horizontalDistance, int distanceCoordsZ, const double radian, const double gravity);
	double CheckVelocityEquation(const double horizontalDistance, int distanceCoordsZ, const double velocity, const double radian, const double gravity);
	double SolveFixedSpeedMeetTime(const CoordStruct& source, const CoordStruct& target, const CoordStruct& offset, const double horizontalSpeed);
	double SearchFixedHeightMeetTime(const CoordStruct& source, const CoordStruct& target, const CoordStruct& offset, const double gravity);
	double CheckFixedHeightEquation(const CoordStruct& source, const CoordStruct& target, const CoordStruct& offset, const double meetTime, const double gravity);
	double SearchFixedAngleMeetTime(const CoordStruct& source, const CoordStruct& target, const CoordStruct& offset, const double radian, const double gravity);
	double CheckFixedAngleEquation(const CoordStruct& source, const CoordStruct& target, const CoordStruct& offset, const double meetTime, const double radian, const double gravity);
	bool CalculateBulletVelocityAfterBounce(CellClass* const pCell, const CoordStruct& position);
	BulletVelocity GetGroundNormalVector(CellClass* const pCell, const CoordStruct& position);

	static inline bool CheckBulletHitCliff(short X, short Y, int bulletHeight, int lastCellHeight)
	{
		if (const auto pCell = MapClass::Instance.TryGetCellAt(CellStruct{ X, Y }))
		{
			const auto cellHeight = pCell->Level * Unsorted::LevelHeight;

			// (384 -> (4 * Unsorted::LevelHeight - 32(error range)))
			if (bulletHeight < cellHeight && (cellHeight - lastCellHeight) > 384)
				return true;
		}

		return false;
	}

	static constexpr double SqrtConstexpr(double x, double curr = 1.0, double prev = 0.0)
	{
		return curr == prev ? curr : SqrtConstexpr(x, 0.5 * (curr + x / curr), curr);
	}

	template <typename T>
	void Serialize(T& Stm);
};
