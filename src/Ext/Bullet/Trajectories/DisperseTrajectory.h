#pragma once

#include "PhobosTrajectory.h"

class DisperseTrajectoryType final : public PhobosTrajectoryType
{
public:
	DisperseTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Disperse)
		, UniqueCurve { false }
		, PreAimCoord { { 0, 0, 0 } }
		, RotateCoord { 0 }
		, MirrorCoord { true }
		, FacingCoord { false }
		, ReduceCoord { true }
		, UseDisperseBurst { false }
		, AxisOfRotation { { 0, 0, 1 } }
		, LaunchSpeed { 0 }
		, Acceleration { 10.0 }
		, ROT { 30.0 }
		, LockDirection { false }
		, CruiseEnable { false }
		, CruiseUnableRange { Leptons(128) }
		, LeadTimeCalculate { true }
		, TargetSnapDistance { Leptons(128) }
		, RetargetAllies { false }
		, RetargetRadius { 0 }
		, SuicideShortOfROT { true }
		, SuicideAboveRange { 0 }
		, SuicideIfNoWeapon { true }
		, Weapon {}
		, WeaponBurst {}
		, WeaponCount { 0 }
		, WeaponDelay { 1 }
		, WeaponTimer { 0 }
		, WeaponScope { Leptons(0) }
		, WeaponSeparate { false }
		, WeaponRetarget { false }
		, WeaponLocation { false }
		, WeaponTendency { false }
		, WeaponToAllies { false }
		, WeaponToGround { false }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	Valueable<bool> UniqueCurve;
	Valueable<CoordStruct> PreAimCoord;
	Valueable<double> RotateCoord;
	Valueable<bool> MirrorCoord;
	Valueable<bool> FacingCoord;
	Valueable<bool> ReduceCoord;
	Valueable<bool> UseDisperseBurst;
	Valueable<CoordStruct> AxisOfRotation;
	Valueable<double> LaunchSpeed;
	Valueable<double> Acceleration;
	Valueable<double> ROT;
	Valueable<bool> LockDirection;
	Valueable<bool> CruiseEnable;
	Valueable<Leptons> CruiseUnableRange;
	Valueable<bool> LeadTimeCalculate;
	Valueable<Leptons> TargetSnapDistance;
	Valueable<bool> RetargetAllies;
	Valueable<double> RetargetRadius;
	Valueable<bool> SuicideShortOfROT;
	Valueable<double> SuicideAboveRange;
	Valueable<bool> SuicideIfNoWeapon;
	ValueableVector<WeaponTypeClass*> Weapon;
	ValueableVector<int> WeaponBurst;
	Valueable<int> WeaponCount;
	Valueable<int> WeaponDelay;
	Valueable<int> WeaponTimer;
	Valueable<Leptons> WeaponScope;
	Valueable<bool> WeaponSeparate;
	Valueable<bool> WeaponRetarget;
	Valueable<bool> WeaponLocation;
	Valueable<bool> WeaponTendency;
	Valueable<bool> WeaponToAllies;
	Valueable<bool> WeaponToGround;
};

class DisperseTrajectory final : public PhobosTrajectory
{
public:
	DisperseTrajectory() : PhobosTrajectory(TrajectoryFlag::Disperse)
		, UniqueCurve { false }
		, PreAimCoord {}
		, RotateCoord { 0 }
		, MirrorCoord { true }
		, FacingCoord { false }
		, ReduceCoord { true }
		, UseDisperseBurst { false }
		, AxisOfRotation {}
		, LaunchSpeed { 0 }
		, Acceleration { 10.0 }
		, ROT { 30.0 }
		, LockDirection { false }
		, CruiseEnable { false }
		, CruiseUnableRange { Leptons(128) }
		, LeadTimeCalculate { true }
		, TargetSnapDistance { Leptons(128) }
		, RetargetAllies { false }
		, RetargetRadius { 0 }
		, SuicideShortOfROT { true }
		, SuicideAboveRange { 0 }
		, SuicideIfNoWeapon { true }
		, Weapon {}
		, WeaponBurst {}
		, WeaponCount { 0 }
		, WeaponDelay { 1 }
		, WeaponTimer {}
		, WeaponScope { Leptons(0) }
		, WeaponSeparate { false }
		, WeaponRetarget { false }
		, WeaponLocation { false }
		, WeaponTendency { false }
		, WeaponToAllies { false }
		, WeaponToGround { false }
		, InStraight { false }
		, Accelerate { true }
		, TargetInTheAir { false }
		, TargetIsTechno { false }
		, OriginalDistance { 0 }
		, CurrentBurst { 0 }
		, ThisWeaponIndex { 0 }
		, LastTargetCoord {}
		, PreAimDistance { 0 }
		, LastReviseMult { 0 }
		, FirepowerMult { 1.0 }
	{}

	DisperseTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory(TrajectoryFlag::Disperse)
		, UniqueCurve { false }
		, PreAimCoord {}
		, RotateCoord { 0 }
		, MirrorCoord { true }
		, FacingCoord { false }
		, ReduceCoord { true }
		, UseDisperseBurst { false }
		, AxisOfRotation {}
		, LaunchSpeed { 0 }
		, Acceleration { 10.0 }
		, ROT { 30.0 }
		, LockDirection { false }
		, CruiseEnable { false }
		, CruiseUnableRange { Leptons(128) }
		, LeadTimeCalculate { true }
		, TargetSnapDistance { Leptons(128) }
		, RetargetAllies { false }
		, RetargetRadius { 0 }
		, SuicideShortOfROT { true }
		, SuicideAboveRange { 0 }
		, SuicideIfNoWeapon { true }
		, Weapon {}
		, WeaponBurst {}
		, WeaponCount { 0 }
		, WeaponDelay { 1 }
		, WeaponTimer {}
		, WeaponScope { Leptons(0) }
		, WeaponSeparate { false }
		, WeaponRetarget { false }
		, WeaponLocation { false }
		, WeaponTendency { false }
		, WeaponToAllies { false }
		, WeaponToGround { false }
		, InStraight { false }
		, Accelerate { true }
		, TargetInTheAir { false }
		, TargetIsTechno { false }
		, OriginalDistance { 0 }
		, CurrentBurst { 0 }
		, ThisWeaponIndex { 0 }
		, LastTargetCoord {}
		, PreAimDistance { 0 }
		, LastReviseMult { 0 }
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
	double RotateCoord;
	bool MirrorCoord;
	bool FacingCoord;
	bool ReduceCoord;
	bool UseDisperseBurst;
	CoordStruct AxisOfRotation;
	double LaunchSpeed;
	double Acceleration;
	double ROT;
	bool LockDirection;
	bool CruiseEnable;
	Leptons CruiseUnableRange;
	bool LeadTimeCalculate;
	Leptons TargetSnapDistance;
	bool RetargetAllies;
	double RetargetRadius;
	bool SuicideShortOfROT;
	double SuicideAboveRange;
	bool SuicideIfNoWeapon;
	std::vector<WeaponTypeClass*> Weapon;
	std::vector<int> WeaponBurst;
	int WeaponCount;
	int WeaponDelay;
	CDTimerClass WeaponTimer;
	Leptons WeaponScope;
	bool WeaponSeparate;
	bool WeaponRetarget;
	bool WeaponLocation;
	bool WeaponTendency;
	bool WeaponToAllies;
	bool WeaponToGround;
	bool InStraight;
	bool Accelerate;
	bool TargetInTheAir;
	bool TargetIsTechno;
	int OriginalDistance;
	int CurrentBurst;
	int ThisWeaponIndex;
	CoordStruct LastTargetCoord;
	double PreAimDistance;
	double LastReviseMult;
	double FirepowerMult;

private:
	void InitializeBulletNotCurve(BulletClass* pBullet);
	BulletVelocity RotateAboutTheAxis(BulletVelocity TheSpeed, BulletVelocity TheAxis, double TheRadian);
	bool CalculateBulletVelocity(BulletClass* pBullet, double StraightSpeed);
	bool BulletRetargetTechno(BulletClass* pBullet, HouseClass* pOwner);
	bool CheckTechnoIsInvalid(TechnoClass* pTechno);
	bool CurveVelocityChange(BulletClass* pBullet);
	bool NotCurveVelocityChange(BulletClass* pBullet, HouseClass* pOwner);
	bool StandardVelocityChange(BulletClass* pBullet);
	bool ChangeBulletVelocity(BulletClass* pBullet, CoordStruct TargetLocation, double TurningRadius, bool Curve);
	bool PrepareDisperseWeapon(BulletClass* pBullet, HouseClass* pOwner);
	std::vector<TechnoClass*> GetValidTechnosInSame(std::vector<TechnoClass*> Technos, HouseClass* pOwner, WarheadTypeClass* pWH, AbstractClass* pTarget);
	void CreateDisperseBullets(BulletClass* pBullet, WeaponTypeClass* pWeapon, AbstractClass* pTarget, HouseClass* pOwner, int CurBurst, int MaxBurst);
};
