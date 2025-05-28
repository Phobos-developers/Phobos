#pragma once

#include "PhobosTrajectory.h"

class ActualTrajectoryType : public PhobosTrajectoryType
{
public:
	ActualTrajectoryType() : PhobosTrajectoryType()
		, RotateCoord { 0 }
		, OffsetCoord { { 0, 0, 0 } }
		, AxisOfRotation { { 0, 0, 1 } }
		, LeadTimeMaximum { 0 }
		, LeadTimeCalculate {}
		, SubjectToGround { false }
		, EarlyDetonation { false }
		, DetonationHeight { -1 }
		, DetonationDistance { Leptons(102) }
		, TargetSnapDistance { Leptons(128) }
	{ }

	Valueable<double> RotateCoord; // The maximum rotation angle of the initial velocity vector on the axis of rotation
	Valueable<CoordStruct> OffsetCoord; // Offset of target position, refers to the initial target position on Missile
	Valueable<CoordStruct> AxisOfRotation; // RotateCoord's rotation axis
	Valueable<int> LeadTimeMaximum; // Maximum prediction time
	Nullable<bool> LeadTimeCalculate; // Predict the moving direction of the target
	bool SubjectToGround; // Auto set
	Valueable<bool> EarlyDetonation; // Calculating DetonationHeight in the rising phase rather than the falling phase
	Valueable<int> DetonationHeight; // At what height did it detonate in advance
	Valueable<Leptons> DetonationDistance; // Explode at a distance from the target, different on AAA and BBB
	Valueable<Leptons> TargetSnapDistance; // Snap to target when detonating with a distance less than this

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
//	virtual void Read(CCINIClass* const pINI, const char* pSection) override; // Read separately

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ActualTrajectory : public PhobosTrajectory
{
public:
	ActualTrajectory() { }
	ActualTrajectory(ActualTrajectoryType const* trajType, BulletClass* pBullet)
		: PhobosTrajectory(trajType, pBullet)
		, LastTargetCoord { CoordStruct::Empty }
		, WaitOneFrame { 0 }
	{ }

	// TODO If we could calculate this before firing, perhaps it can solve the problem of one frame delay and not so correct turret orientation.
	CoordStruct LastTargetCoord; // The target is located in the previous frame, used to calculate the lead time
	int WaitOneFrame; // Attempts to launch when update

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual void OnUnlimbo() override;
	virtual bool OnEarlyUpdate() override;
	virtual void OnPreDetonate() override;
	virtual void FireTrajectory() { this->OpenFire(); } // New

	inline void CheckProjectileRange()
	{
		if (this->GetType()->Ranged)
		{
			const auto pBullet = this->Bullet;
			pBullet->Range -= Game::F2I(this->MovingSpeed);

			if (pBullet->Range <= 0)
				this->ShouldDetonate = true;
		}
	}
	inline double GetLeadTime(const double defaultTime)
	{
		const double maximum = static_cast<double>(static_cast<const ActualTrajectoryType*>(this->GetType())->LeadTimeMaximum.Get());

		return (maximum > 0.0 && defaultTime > maximum) ? maximum : defaultTime;
	}

	bool BulletPrepareCheck();
	CoordStruct GetOnlyStableOffsetCoords(const double rotateRadian);
	CoordStruct GetInaccurateTargetCoords(const CoordStruct& baseCoord, const double distance);
	void DisperseBurstSubstitution(const double baseRadian);

private:
	template <typename T>
	void Serialize(T& Stm);
};
