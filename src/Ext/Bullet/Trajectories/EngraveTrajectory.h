#pragma once

#include "PhobosTrajectory.h"

class EngraveTrajectoryType final : public PhobosTrajectoryType
{
public:
	EngraveTrajectoryType() : PhobosTrajectoryType()
		, SourceCoord { { 0, 0 } }
		, TargetCoord { { 0, 0 } }
		, MirrorCoord { true }
		, UseDisperseCoord { false }
		, ApplyRangeModifiers { false }
		, AllowFirerTurning { true }
		, Duration { 0 }
		, IsLaser { true }
		, IsIntense { false }
		, IsHouseColor { false }
		, IsSingleColor { false }
		, LaserInnerColor { { 0, 0, 0 } }
		, LaserOuterColor { { 0, 0, 0 } }
		, LaserOuterSpread { { 0, 0, 0 } }
		, LaserThickness { 3 }
		, LaserDuration { 1 }
		, LaserDelay { 1 }
		, DamageDelay { 2 }
		, ProximityImpact { 0 }
		, ProximityWarhead {}
		, ProximityDamage { 0 }
		, ProximityRadius { Leptons(179) }
		, ProximityDirect { false }
		, ProximityMedial { false }
		, ProximityAllies { false }
		, ProximitySuicide { false }
	{ }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual std::unique_ptr<PhobosTrajectory> CreateInstance() const override;
	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Engrave; }

	Valueable<Point2D> SourceCoord;
	Valueable<Point2D> TargetCoord;
	Valueable<bool> MirrorCoord;
	Valueable<bool> UseDisperseCoord;
	Valueable<bool> ApplyRangeModifiers;
	Valueable<bool> AllowFirerTurning;
	Valueable<int> Duration;
	Valueable<bool> IsLaser;
	Valueable<bool> IsIntense;
	Valueable<bool> IsHouseColor;
	Valueable<bool> IsSingleColor;
	Valueable<ColorStruct> LaserInnerColor;
	Valueable<ColorStruct> LaserOuterColor;
	Valueable<ColorStruct> LaserOuterSpread;
	Valueable<int> LaserThickness;
	Valueable<int> LaserDuration;
	Valueable<int> LaserDelay;
	Valueable<int> DamageDelay;
	Valueable<int> ProximityImpact;
	Valueable<WarheadTypeClass*> ProximityWarhead;
	Valueable<int> ProximityDamage;
	Valueable<Leptons> ProximityRadius;
	Valueable<bool> ProximityDirect;
	Valueable<bool> ProximityMedial;
	Valueable<bool> ProximityAllies;
	Valueable<bool> ProximitySuicide;

private:
	template <typename T>
	void Serialize(T& Stm);
};

class EngraveTrajectory final : public PhobosTrajectory
{
public:
	EngraveTrajectory(noinit_t) { }

	EngraveTrajectory(EngraveTrajectoryType const* trajType) : Type { trajType }
		, SourceCoord { trajType->SourceCoord.Get() }
		, TargetCoord { trajType->TargetCoord.Get() }
		, Duration { trajType->Duration }
		, LaserTimer {}
		, DamageTimer {}
		, TechnoInTransport { false }
		, NotMainWeapon { false }
		, FLHCoord {}
		, BuildingCoord {}
		, StartCoord {}
		, ProximityImpact { trajType->ProximityImpact }
		, TheCasualty {}
	{ }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual TrajectoryFlag Flag() const override { return TrajectoryFlag::Engrave; }
	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	const EngraveTrajectoryType* Type;
	Point2D SourceCoord;
	Point2D TargetCoord;
	int Duration;
	CDTimerClass LaserTimer;
	CDTimerClass DamageTimer;
	bool TechnoInTransport;
	bool NotMainWeapon;
	CoordStruct FLHCoord;
	CoordStruct BuildingCoord;
	CoordStruct StartCoord;
	int ProximityImpact;
	std::map<int, int> TheCasualty; // Only for recording existence

	void SetEngraveDirection(BulletClass* pBullet, double rotateAngle);
private:
	template <typename T>
	void Serialize(T& Stm);

	void GetTechnoFLHCoord(BulletClass* pBullet, TechnoClass* pTechno);
	inline void CheckMirrorCoord(TechnoClass* pTechno);
	bool InvalidFireCondition(BulletClass* pBullet, TechnoClass* pTechno);
	int GetFloorCoordHeight(BulletClass* pBullet, const CoordStruct& coord);
	bool PlaceOnCorrectHeight(BulletClass* pBullet);
	void DrawEngraveLaser(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner);
	inline void DetonateLaserWarhead(BulletClass* pBullet, TechnoClass* pTechno, HouseClass* pOwner);
	void PrepareForDetonateAt(BulletClass* pBullet, HouseClass* pOwner);
};
