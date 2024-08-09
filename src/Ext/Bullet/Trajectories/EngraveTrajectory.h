#pragma once

#include "PhobosTrajectory.h"

class EngraveTrajectoryType final : public PhobosTrajectoryType
{
public:
	EngraveTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Engrave)
		, ApplyRangeModifiers { false }
		, SourceCoord { { 0, 0 } }
		, TargetCoord { { 0, 0 } }
		, MirrorCoord { true }
		, TheDuration { 0 }
		, IsLaser { true }
		, IsSupported { false }
		, IsHouseColor { false }
		, IsSingleColor { false }
		, LaserInnerColor { { 0, 0, 0 } }
		, LaserOuterColor { { 0, 0, 0 } }
		, LaserOuterSpread { { 0, 0, 0 } }
		, LaserThickness { 3 }
		, LaserDuration { 1 }
		, LaserDelay { 1 }
		, DamageDelay { 2 }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual PhobosTrajectory* CreateInstance() const override;
	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	Valueable<bool> ApplyRangeModifiers;
	Valueable<Point2D> SourceCoord;
	Valueable<Point2D> TargetCoord;
	Valueable<bool> MirrorCoord;
	Valueable<int> TheDuration;
	Valueable<bool> IsLaser;
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
		, IsLaser { true }
		, IsSupported { false }
		, IsHouseColor { false }
		, IsSingleColor { false }
		, LaserInnerColor {}
		, LaserOuterColor {}
		, LaserOuterSpread {}
		, LaserThickness { 3 }
		, LaserDuration { 1 }
		, LaserDelay { 1 }
		, DamageDelay { 2 }
		, LaserTimer {}
		, DamageTimer {}
		, TechnoInLimbo { false }
		, NotMainWeapon { false }
		, FLHCoord {}
		, BuildingCoord {}
		, TemporaryCoord {}
	{}

	EngraveTrajectory(PhobosTrajectoryType const* pType) : PhobosTrajectory(TrajectoryFlag::Engrave)
		, SourceCoord {}
		, TargetCoord {}
		, MirrorCoord { true }
		, TheDuration { 0 }
		, IsLaser { true }
		, IsSupported { false }
		, IsHouseColor { false }
		, IsSingleColor { false }
		, LaserInnerColor {}
		, LaserOuterColor {}
		, LaserOuterSpread {}
		, LaserThickness { 3 }
		, LaserDuration { 1 }
		, LaserDelay { 1 }
		, DamageDelay { 2 }
		, LaserTimer {}
		, DamageTimer {}
		, TechnoInLimbo { false }
		, NotMainWeapon { false }
		, FLHCoord {}
		, BuildingCoord {}
		, TemporaryCoord {}
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
	bool IsLaser;
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
	CDTimerClass LaserTimer;
	CDTimerClass DamageTimer;
	bool TechnoInLimbo;
	bool NotMainWeapon;
	CoordStruct FLHCoord;
	CoordStruct BuildingCoord;
	CoordStruct TemporaryCoord;

private:
	void GetTechnoFLHCoord(BulletClass* pBullet, TechnoClass* pTechno);
	void CheckMirrorCoord(TechnoClass* pTechno);
	void SetEngraveDirection(BulletClass* pBullet, CoordStruct theSource, CoordStruct theTarget);
	int GetFloorCoordHeight(BulletClass* pBullet, CoordStruct coord);
	bool PlaceOnCorrectHeight(BulletClass* pBullet);
	void DrawEngraveLaser(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner);
	void DetonateLaserWarhead(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner);
};
