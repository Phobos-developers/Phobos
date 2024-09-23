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
		, SourceCoord {}
		, TargetCoord {}
		, MirrorCoord { true }
		, ApplyRangeModifiers { false }
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
	{
		auto const pFinalType = static_cast<const EngraveTrajectoryType*>(pType);

		this->SourceCoord = pFinalType->SourceCoord;
		this->TargetCoord = pFinalType->TargetCoord;
		this->MirrorCoord = pFinalType->MirrorCoord;
		this->ApplyRangeModifiers = pFinalType->ApplyRangeModifiers;
		this->TheDuration = pFinalType->TheDuration;
		this->IsLaser = pFinalType->IsLaser;
		this->IsSupported = pFinalType->IsSupported;
		this->IsHouseColor = pFinalType->IsHouseColor;
		this->IsSingleColor = pFinalType->IsSingleColor;
		this->LaserInnerColor = pFinalType->LaserInnerColor;
		this->LaserOuterColor = pFinalType->LaserOuterColor;
		this->LaserOuterSpread = pFinalType->LaserOuterSpread;
		this->LaserThickness = pFinalType->LaserThickness > 0 ? pFinalType->LaserThickness : 1;
		this->LaserDuration = pFinalType->LaserDuration > 0 ? pFinalType->LaserDuration : 1;
		this->LaserDelay = pFinalType->LaserDelay > 0 ? pFinalType->LaserDelay : 1;
		this->DamageDelay = pFinalType->DamageDelay > 0 ? pFinalType->DamageDelay : 1;
	}

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
	bool ApplyRangeModifiers;
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
