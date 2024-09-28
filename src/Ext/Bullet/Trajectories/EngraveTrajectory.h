#pragma once

#include "PhobosTrajectory.h"

class EngraveTrajectoryType final : public PhobosTrajectoryType
{
public:
	EngraveTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Engrave)
		, SourceCoord { { 0, 0 } }
		, TargetCoord { { 0, 0 } }
		, MirrorCoord { true }
		, ApplyRangeModifiers { false }
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

	Valueable<Point2D> SourceCoord;
	Valueable<Point2D> TargetCoord;
	Valueable<bool> MirrorCoord;
	Valueable<bool> ApplyRangeModifiers;
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

private:
	template <typename T>
	void Serialize(T& Stm);
};

class EngraveTrajectory final : public PhobosTrajectory
{
public:
	EngraveTrajectory(noinit_t) :PhobosTrajectory { noinit_t{} } { }

	EngraveTrajectory(PhobosTrajectoryType const* pType) : PhobosTrajectory(TrajectoryFlag::Engrave)
		, Type { static_cast<EngraveTrajectoryType*>(const_cast<PhobosTrajectoryType*>(pType)) }
		, SourceCoord { static_cast<Point2D>(Type->SourceCoord) }
		, TargetCoord { static_cast<Point2D>(Type->TargetCoord) }
		, TheDuration { Type->TheDuration }
		, LaserTimer {}
		, DamageTimer {}
		, TechnoInLimbo { false }
		, NotMainWeapon { false }
		, FLHCoord {}
		, BuildingCoord {}
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	EngraveTrajectoryType* Type;
	Point2D SourceCoord;
	Point2D TargetCoord;
	int TheDuration;
	CDTimerClass LaserTimer;
	CDTimerClass DamageTimer;
	bool TechnoInLimbo;
	bool NotMainWeapon;
	CoordStruct FLHCoord;
	CoordStruct BuildingCoord;

private:
	template <typename T>
	void Serialize(T& Stm);

	void GetTechnoFLHCoord(BulletClass* pBullet, TechnoClass* pTechno);
	void CheckMirrorCoord(TechnoClass* pTechno);
	void SetEngraveDirection(BulletClass* pBullet, CoordStruct theSource, CoordStruct theTarget);
	int GetFloorCoordHeight(BulletClass* pBullet, CoordStruct coord);
	bool PlaceOnCorrectHeight(BulletClass* pBullet);
	void DrawEngraveLaser(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner);
	void DetonateLaserWarhead(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner);
};
