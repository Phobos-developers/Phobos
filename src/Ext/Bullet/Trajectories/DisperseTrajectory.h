#pragma once

#include "PhobosTrajectory.h"

class DisperseTrajectoryType final : public PhobosTrajectoryType
{
public:
	DisperseTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Disperse)
		, UniqueCurve { false }
		, PreAimCoord { { 0, 0, 0 } }
		, LaunchSpeed { 0 }
		, Acceleration { 10.0 }
		, RateofTurning { 10.0 }
		, LockDirection { false }
		, CruiseEnable { false }
		, CruiseUnableRange { Leptons(128) }
		, LeadTimeCalculate { false }
		, TargetSnapDistance { Leptons(128) }
		, RetargetAllies { false }
		, RetargetRadius { 0 }
		, SuicideAboveRange { 0 }
		, SuicideIfNoWeapon { false }
		, Weapon {}
		, WeaponBurst {}
		, WeaponCount { 0 }
		, WeaponDelay { 1 }
		, WeaponTimer { 0 }
		, WeaponScope { Leptons(0) }
		, WeaponRetarget { false }
		, WeaponLocation { false }
		, WeaponTendency { false }
		, WeaponToAllies { false }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	Valueable<bool> UniqueCurve;
	Valueable<CoordStruct> PreAimCoord;
	Valueable<double> LaunchSpeed;
	Valueable<double> Acceleration;
	Valueable<double> RateofTurning;
	Valueable<bool> LockDirection;
	Valueable<bool> CruiseEnable;
	Valueable<Leptons> CruiseUnableRange;
	Valueable<bool> LeadTimeCalculate;
	Valueable<Leptons> TargetSnapDistance;
	Valueable<bool> RetargetAllies;
	Valueable<double> RetargetRadius;
	Valueable<double> SuicideAboveRange;
	Valueable<bool> SuicideIfNoWeapon;
	ValueableVector<WeaponTypeClass*> Weapon;
	ValueableVector<int> WeaponBurst;
	Valueable<int> WeaponCount;
	Valueable<int> WeaponDelay;
	Valueable<int> WeaponTimer;
	Valueable<Leptons> WeaponScope;
	Valueable<bool> WeaponRetarget;
	Valueable<bool> WeaponLocation;
	Valueable<bool> WeaponTendency;
	Valueable<bool> WeaponToAllies;
};

class DisperseTrajectory final : public PhobosTrajectory
{
public:
	DisperseTrajectory() : PhobosTrajectory(TrajectoryFlag::Disperse)
		, UniqueCurve { false }
		, PreAimCoord {}
		, LaunchSpeed { 0 }
		, Acceleration { 10.0 }
		, RateofTurning { 10.0 }
		, LockDirection { false }
		, CruiseEnable { false }
		, CruiseUnableRange { Leptons(128) }
		, LeadTimeCalculate { false }
		, TargetSnapDistance { Leptons(128) }
		, RetargetAllies { false }
		, RetargetRadius { 0 }
		, SuicideAboveRange { 0 }
		, SuicideIfNoWeapon { false }
		, Weapon {}
		, WeaponBurst {}
		, WeaponCount { 0 }
		, WeaponDelay { 1 }
		, WeaponTimer { 0 }
		, WeaponScope { Leptons(0) }
		, WeaponRetarget { false }
		, WeaponLocation { false }
		, WeaponTendency { false }
		, WeaponToAllies { false }
		, InStraight { false }
		, Accelerate { true }
		, TargetInAir { false }
		, FinalHeight { 0 }
		, LastTargetCoord {}
		, FirepowerMult { 1.0 }
	{}

	DisperseTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory(TrajectoryFlag::Disperse)
		, UniqueCurve { false }
		, PreAimCoord {}
		, LaunchSpeed { 0 }
		, Acceleration { 10.0 }
		, RateofTurning { 10.0 }
		, LockDirection { false }
		, CruiseEnable { false }
		, CruiseUnableRange { Leptons(128) }
		, LeadTimeCalculate { false }
		, TargetSnapDistance { Leptons(128) }
		, RetargetAllies { false }
		, RetargetRadius { 0 }
		, SuicideAboveRange { 0 }
		, SuicideIfNoWeapon { false }
		, Weapon {}
		, WeaponBurst {}
		, WeaponCount { 0 }
		, WeaponDelay { 1 }
		, WeaponTimer { 0 }
		, WeaponScope { Leptons(0) }
		, WeaponRetarget { false }
		, WeaponLocation { false }
		, WeaponTendency { false }
		, WeaponToAllies { false }
		, InStraight { false }
		, Accelerate { true }
		, TargetInAir { false }
		, FinalHeight { 0 }
		, LastTargetCoord {}
		, FirepowerMult { 1.0 }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	bool UniqueCurve;
	CoordStruct PreAimCoord;
	double LaunchSpeed;
	double Acceleration;
	double RateofTurning;
	bool LockDirection;
	bool CruiseEnable;
	Leptons CruiseUnableRange;
	bool LeadTimeCalculate;
	Leptons TargetSnapDistance;
	bool RetargetAllies;
	double RetargetRadius;
	double SuicideAboveRange;
	bool SuicideIfNoWeapon;
	std::vector<WeaponTypeClass*> Weapon;
	std::vector<int> WeaponBurst;
	int WeaponCount;
	int WeaponDelay;
	int WeaponTimer;
	Leptons WeaponScope;
	bool WeaponRetarget;
	bool WeaponLocation;
	bool WeaponTendency;
	bool WeaponToAllies;
	bool InStraight;
	bool Accelerate;
	bool TargetInAir;
	int FinalHeight;
	CoordStruct LastTargetCoord;
	double FirepowerMult;

private:
	bool CalculateBulletVelocity(BulletClass* pBullet, double StraightSpeed);
	bool BulletDetonatePreCheck(BulletClass* pBullet);
	bool BulletRetargetTechno(BulletClass* pBullet, HouseClass* pOwner);
	void CurveVelocityChange(BulletClass* pBullet);
	void StandardVelocityChange(BulletClass* pBullet);
	void ChangeBulletVelocity(BulletClass* pBullet, CoordStruct TargetLocation, double TurningRadius, bool Curve);
	bool PrepareDisperseWeapon(BulletClass* pBullet, HouseClass* pOwner);
	std::vector<TechnoClass*> GetValidTechnosInSame(std::vector<TechnoClass*> Technos, HouseClass* pOwner, WarheadTypeClass* pWH, bool Mode);
	void CreateDisperseBullets(BulletClass* pBullet, WeaponTypeClass* pWeapon, AbstractClass* BulletTarget, HouseClass* pOwner);
};
