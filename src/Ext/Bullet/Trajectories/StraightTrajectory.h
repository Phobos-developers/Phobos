#pragma once

#include "PhobosTrajectory.h"

class StraightTrajectoryType final : public PhobosTrajectoryType
{
public:
	StraightTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Straight)
		, DetonationDistance { Leptons(102) }
		, TargetSnapDistance { Leptons(128) }
		, ApplyRangeModifiers { false }
		, PassThrough { false }
		, PassDetonate { false }
		, PassDetonateWarhead {}
		, PassDetonateDamage { 0 }
		, PassDetonateDelay { 1 }
		, PassDetonateTimer { 0 }
		, PassDetonateLocal { false }
		, LeadTimeCalculate { false }
		, OffsetCoord { { 0, 0, 0 } }
		, RotateCoord { 0 }
		, MirrorCoord { true }
		, UseDisperseBurst { false }
		, AxisOfRotation { { 0, 0, 1 } }
		, ProximityImpact { 0 }
		, ProximityWarhead {}
		, ProximityDamage { 0 }
		, ProximityRadius { Leptons(179) }
		, ProximityDirect { false }
		, ProximityMedial { false }
		, ProximityAllies { false }
		, ProximityFlight { false }
		, ThroughVehicles { true }
		, ThroughBuilding { true }
		, SubjectToGround { false }
		, ConfineAtHeight { 0 }
		, EdgeAttenuation { 1.0 }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;
	virtual PhobosTrajectory* CreateInstance() const override;
	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	Valueable<Leptons> DetonationDistance;
	Valueable<Leptons> TargetSnapDistance;
	Valueable<bool> ApplyRangeModifiers;
	Valueable<bool> PassThrough;
	Valueable<bool> PassDetonate;
	Valueable<WarheadTypeClass*> PassDetonateWarhead;
	Valueable<int> PassDetonateDamage;
	Valueable<int> PassDetonateDelay;
	Valueable<int> PassDetonateTimer;
	Valueable<bool> PassDetonateLocal;
	Valueable<bool> LeadTimeCalculate;
	Valueable<CoordStruct> OffsetCoord;
	Valueable<double> RotateCoord;
	Valueable<bool> MirrorCoord;
	Valueable<bool> UseDisperseBurst;
	Valueable<CoordStruct> AxisOfRotation;
	Valueable<int> ProximityImpact;
	Valueable<WarheadTypeClass*> ProximityWarhead;
	Valueable<int> ProximityDamage;
	Valueable<Leptons> ProximityRadius;
	Valueable<bool> ProximityDirect;
	Valueable<bool> ProximityMedial;
	Valueable<bool> ProximityAllies;
	Valueable<bool> ProximityFlight;
	Valueable<bool> ThroughVehicles;
	Valueable<bool> ThroughBuilding;
	Valueable<bool> SubjectToGround;
	Valueable<int> ConfineAtHeight;
	Valueable<double> EdgeAttenuation;

private:
	template <typename T>
	void Serialize(T& Stm);
};

class StraightTrajectory final : public PhobosTrajectory
{
public:
	StraightTrajectory(noinit_t) :PhobosTrajectory { noinit_t{} } { }

	StraightTrajectory(PhobosTrajectoryType const* pType) : PhobosTrajectory(TrajectoryFlag::Straight)
		, Type { static_cast<StraightTrajectoryType*>(const_cast<PhobosTrajectoryType*>(pType)) }
		, DetonationDistance { Type->DetonationDistance }
		, PassDetonateTimer {}
		, OffsetCoord { static_cast<CoordStruct>(Type->OffsetCoord) }
		, UseDisperseBurst { Type->UseDisperseBurst }
		, ProximityImpact { Type->ProximityImpact }
		, RemainingDistance { 1 }
		, ExtraCheck { nullptr }
		, LastCasualty {}
		, FirepowerMult { 1.0 }
		, AttenuationRange { 0 }
		, LastTargetCoord {}
		, CurrentBurst { 0 }
		, CountOfBurst { 0 }
		, WaitOneFrame {}
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	struct CasualtyData
	{
		TechnoClass* pCasualty; // Cannot be used, only for comparison purposes
		int RemainTime;
	};

	StraightTrajectoryType* Type;
	Leptons DetonationDistance;
	CDTimerClass PassDetonateTimer;
	CoordStruct OffsetCoord;
	bool UseDisperseBurst;
	int ProximityImpact;
	int RemainingDistance;
	TechnoClass* ExtraCheck;
	std::vector<CasualtyData> LastCasualty;
	double FirepowerMult;
	int AttenuationRange;
	CoordStruct LastTargetCoord;
	int CurrentBurst;
	int CountOfBurst;
	CDTimerClass WaitOneFrame;

private:
	template <typename T>
	void Serialize(T& Stm);

	void PrepareForOpenFire(BulletClass* pBullet);
	int GetVelocityZ(BulletClass* pBullet);
	bool CalculateBulletVelocity(BulletClass* pBullet, double straightSpeed);
	bool BulletPrepareCheck(BulletClass* pBullet);
	bool BulletDetonatePreCheck(BulletClass* pBullet, double straightSpeed);
	void BulletDetonateLastCheck(BulletClass* pBullet, HouseClass* pOwner, double straightSpeed);
	bool CheckThroughAndSubjectInCell(BulletClass* pBullet, CellClass* pCell, HouseClass* pOwner);
	void PassWithDetonateAt(BulletClass* pBullet, HouseClass* pOwner);
	void PrepareForDetonateAt(BulletClass* pBullet, HouseClass* pOwner);
	std::vector<CellClass*> GetCellsInProximityRadius(BulletClass* pBullet);
	std::vector<CellStruct> GetCellsInRectangle(CellStruct bottomStaCell, CellStruct leftMidCell, CellStruct rightMidCell, CellStruct topEndCell);
	int GetTheTrueDamage(int damage, BulletClass* pBullet, TechnoClass* pTechno, bool self);
	double GetExtraDamageMultiplier(BulletClass* pBullet, TechnoClass* pTechno, double edgeAttenuation);
	bool PassAndConfineAtHeight(BulletClass* pBullet, double straightSpeed);
	int GetFirerZPosition(BulletClass* pBullet);
	int GetTargetZPosition(BulletClass* pBullet);
	bool ElevationDetonationCheck(BulletClass* pBullet);
};
