#pragma once

#include <Ext/Techno/Body.h>
#include <FootClass.h>

// Intermediate base mirroring FootClass in the extension hierarchy; holds the
// data shared by units, infantry and aircraft (but not buildings).
// UnitExt / InfantryExt / AircraftExt derive from this.
class FootExt : public TechnoExt
{
public:
	bool LastKillWasTeamTarget;
	double LastWarpDistance;
	int JumpjetSpeed;
	bool IsInTunnel;
	bool IsBurrowed;
	bool UndergroundTracked;
	HouseClass* OriginalPassengerOwner;
	bool HasRemainingWarpInDelay;          // Converted from object with Teleport Locomotor to one with a different Locomotor while still phasing in OR set if ChronoSphereDelay > 0.
	int LastWarpInDelay;                   // Last-warp in delay for this unit, used by HasCarryoverWarpInDelay.
	bool IsBeingChronoSphered;             // Set to true on units currently being ChronoSphered, does not apply to Ares-ChronoSphere'd buildings or Chrono reinforcements.
	CellStruct LastSensorsMapCoords;
	CDTimerClass TiberiumEater_Timer;
	bool ResetLocomotor;
	bool JumpjetStraightAscend; // Is set to true jumpjet units will ascend straight and do not adjust rotation or position during it.
	int AttackMoveFollowerTempCount;
	bool IsOwnerChangeFromRevertOnExit;
	bool FlyingProduction_Descending;
	int FlyingProduction_TargetHeight;
	AbstractClass* FlyingProduction_RallyTarget;
	BuildingClass* FlyingProduction_SpawnBuilding;
	DirStruct FlyingProduction_ExitFacing;

	explicit FootExt(FootClass* const OwnerObject) : TechnoExt(OwnerObject)
		, LastKillWasTeamTarget { false }
		, LastWarpDistance {}
		, JumpjetSpeed { 14 } // 0x7115B8
		, IsInTunnel { false }
		, IsBurrowed { false }
		, UndergroundTracked { false }
		, OriginalPassengerOwner {}
		, HasRemainingWarpInDelay { false }
		, LastWarpInDelay { 0 }
		, IsBeingChronoSphered { false }
		, LastSensorsMapCoords { CellStruct::Empty }
		, TiberiumEater_Timer {}
		, ResetLocomotor { false }
		, JumpjetStraightAscend { false }
		, AttackMoveFollowerTempCount { 0 }
		, IsOwnerChangeFromRevertOnExit { false }
		, FlyingProduction_Descending { false }
		, FlyingProduction_TargetHeight { 0 }
		, FlyingProduction_RallyTarget { nullptr }
		, FlyingProduction_SpawnBuilding { nullptr }
		, FlyingProduction_ExitFacing { 0 }
	{ }

	FootClass* OwnerObject() const
	{
		return static_cast<FootClass*>(this->GetAttachedObject());
	}

	static FootExt* Fetch(const FootClass* pThis)
	{
		return AbstractExt::Fetch<FootExt>(pThis);
	}

	static FootExt* TryFetch(const FootClass* pThis)
	{
		return AbstractExt::TryFetch<FootExt>(pThis);
	}

	virtual bool IsInTunnelState() const override { return this->IsInTunnel; }
	virtual bool IsBurrowedState() const override { return this->IsBurrowed; }

	void UpdateTiberiumEater();
	void UpdateWarpInDelay();
	void UpdateOnTunnelEnter();
	void UpdateOnTunnelExit();
	void UpdateTypeData(TechnoTypeClass* pCurrentType);
	void HealthAutoConvertActions();
	void AmmoAutoConvertActions();
	void UpdateFlyingProductionDescent();
	void FlyingProduction_DispatchMove(AbstractClass* pRallyTarget, BuildingClass* pSpawnBuilding, DirStruct facing);

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);
};
