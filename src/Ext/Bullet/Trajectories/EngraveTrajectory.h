#pragma once

#include "PhobosTrajectory.h"

class EngraveTrajectoryType final : public PhobosTrajectoryType
{
public:
	EngraveTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Engrave)
		, SourceCoord { { 0, 0 } }
		, TargetCoord { { 0, 0 } }
		, MirrorCoord { true }
		, TheDuration { 0 }
		, IsSupported { false }
		, IsHouseColor { false }
		, IsSingleColor { false }
		, LaserInnerColor { { 0, 0, 0 } }
		, LaserOuterColor { { 0, 0, 0 } }
		, LaserOuterSpread { { 0, 0, 0 } }
		, LaserThickness { 3 }
		, LaserDuration { 1 }
		, LaserDelay { 1 }
		, DamageDelay { 10 }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	Valueable<Point2D> SourceCoord;
	Valueable<Point2D> TargetCoord;
	Valueable<bool> MirrorCoord;
	Valueable<int> TheDuration;
	Valueable<bool> IsSupported;
	Valueable<bool> IsHouseColor;
	Valueable<bool> IsSingleColor;
	Valueable<ColorStruct> LaserInnerColor;
	Valueable<ColorStruct> LaserOuterColor;
	Valueable<ColorStruct> LaserOuterSpread;
	Valueable<int> LaserThickness;
	Valueable<int> LaserDuration;
	Valueable<int> LaserDelay;
	Valueable<int> DamageDelay;
};

class EngraveTrajectory final : public PhobosTrajectory
{
public:
	EngraveTrajectory() : PhobosTrajectory(TrajectoryFlag::Engrave)
		, SourceCoord {}
		, TargetCoord {}
		, MirrorCoord { true }
		, TheDuration { 0 }
		, IsSupported { false }
		, IsHouseColor { false }
		, IsSingleColor { false }
		, LaserInnerColor {}
		, LaserOuterColor {}
		, LaserOuterSpread {}
		, LaserThickness { 3 }
		, LaserDuration { 1 }
		, LaserDelay { 1 }
		, DamageDelay { 10 }
		, LaserTimer { 0 }
		, DamageTimer { 0 }
		, SourceHeight { 0 }
		, SetItsLocation { false }
		, TechnoInLimbo { false }
		, FirepowerMult { 1.0 }
		, FLHCoord {}
	{}

	EngraveTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory(TrajectoryFlag::Engrave)
		, SourceCoord {}
		, TargetCoord {}
		, MirrorCoord { true }
		, TheDuration { 0 }
		, IsSupported { false }
		, IsHouseColor { false }
		, IsSingleColor { false }
		, LaserInnerColor {}
		, LaserOuterColor {}
		, LaserOuterSpread {}
		, LaserThickness { 3 }
		, LaserDuration { 1 }
		, LaserDelay { 1 }
		, DamageDelay { 10 }
		, LaserTimer { 0 }
		, DamageTimer { 0 }
		, SourceHeight { 0 }
		, SetItsLocation { false }
		, TechnoInLimbo { false }
		, FirepowerMult { 1.0 }
		, FLHCoord {}
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	Point2D SourceCoord;
	Point2D TargetCoord;
	bool MirrorCoord;
	int TheDuration;
	bool IsSupported;
	bool IsHouseColor;
	bool IsSingleColor;
	ColorStruct LaserInnerColor;
	ColorStruct LaserOuterColor;
	ColorStruct LaserOuterSpread;
	int LaserThickness;
	int LaserDuration;
	int LaserDelay;
	int DamageDelay;
	int LaserTimer;
	int DamageTimer;
	int SourceHeight;
	bool SetItsLocation;
	bool TechnoInLimbo;
	double FirepowerMult;
	CoordStruct FLHCoord;

private:
	int GetFloorCoordHeight(CoordStruct Coord);
};
