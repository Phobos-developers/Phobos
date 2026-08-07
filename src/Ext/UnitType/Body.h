#pragma once

#include <Ext/TechnoType/Body.h>
#include <UnitTypeClass.h>

// Concrete leaf extension for UnitTypeClass.
class UnitTypeExt final : public TechnoTypeExt
{
public:
	using base_type = UnitTypeClass;

	static constexpr DWORD Canary = 0xE5E6E7E8;

	Nullable<int> SinkSpeed;
	Nullable<bool> Sinkable;
	Nullable<bool> Sinkable_SquidGrab;
	Nullable<double> DamagedSpeed;

	Valueable<bool> Harvester_CanGuardArea;
	Valueable<bool> Harvester_CanGuardArea_RequireTarget;
	Nullable<bool> HarvesterScanAfterUnload;
	Nullable<int> HarvesterLoadRate;
	Nullable<double> HarvesterDumpRate;
	Nullable<float> HarvesterDumpAmount;

	ValueableVector<AnimTypeClass*> OreGathering_Anims;
	ValueableVector<int> OreGathering_Tiberiums;
	ValueableVector<int> OreGathering_FramesPerDir;

	Valueable<int> Ammo_AddOnDeploy;
	Valueable<int> Ammo_AutoDeployMinimumAmount;
	Valueable<int> Ammo_AutoDeployMaximumAmount;
	Valueable<int> Ammo_DeployUnlockMinimumAmount;
	Valueable<int> Ammo_DeployUnlockMaximumAmount;

	int SubterraneanSpeed;
	Nullable<int> SubterraneanHeight;
	Nullable<bool> Parasite_AllowWaterExit;

	NullableVector<TerrainTypeClass*> DefaultMirageDisguises;

	Valueable<bool> IsSimpleDeployer_ConsiderPathfinding;
	Nullable<LandTypeFlags> IsSimpleDeployer_DisallowedLandTypes;
	Nullable<FacingType> DeployDir;
	ValueableVector<AnimTypeClass*> DeployingAnims;
	Valueable<bool> DeployingAnim_KeepUnitVisible;
	Valueable<bool> DeployingAnim_ReverseForUndeploy;
	Valueable<bool> DeployingAnim_UseUnitDrawer;

	Nullable<bool> JumpjetTilt;
	Nullable<double> JumpjetTilt_ForwardAccelFactor;
	Nullable<double> JumpjetTilt_ForwardSpeedFactor;
	Nullable<double> JumpjetTilt_SidewaysRotationFactor;
	Nullable<double> JumpjetTilt_SidewaysSpeedFactor;

	Nullable<bool> TiltsWhenCrushes_Vehicles;
	Nullable<bool> TiltsWhenCrushes_Overlays;
	Nullable<double> CrushForwardTiltPerFrame;
	Valueable<double> CrushOverlayExtraForwardTilt;
	Nullable<double> CrushSlowdownMultiplier;
	Nullable<bool> SkipCrushSlowdown;

	Valueable<double> CrateGoodie_RerollChance;
	Nullable<bool> NoTurret_TrackTarget;
	Nullable<UnitTypeClass*> WaterImage_ConditionYellow;
	Nullable<UnitTypeClass*> WaterImage_ConditionRed;

	Valueable<int> FireUp;
	Valueable<bool> FireUp_ResetInRetarget;
	Nullable<bool> TurretResponse;
	Valueable<bool> Deploy_SkipPassengerUnload;
	Valueable<bool> Deploy_NoPassenger;
	Valueable<bool> Deploy_NoTiberium;
	Nullable<bool> HoverDrownable;

	SHPStruct* TurretShape;

	Nullable<bool> BarrelOverTurret;
	Valueable<int> BarrelOffset;
	Valueable<int> ExtraBarrelCount;
	std::vector<int> ExtraBarrelOffsets;
	Valueable<int> ExtraTurretCount;
	std::vector<CoordStruct> ExtraTurretOffsets;
	Valueable<int> BurstPerTurret;

	explicit UnitTypeExt(UnitTypeClass* const OwnerObject) : TechnoTypeExt(OwnerObject)
		, SinkSpeed {}
		, Sinkable {}
		, Sinkable_SquidGrab {}
		, DamagedSpeed {}
		, Harvester_CanGuardArea { false }
		, Harvester_CanGuardArea_RequireTarget { false }
		, HarvesterScanAfterUnload {}
		, HarvesterLoadRate {}
		, HarvesterDumpRate {}
		, HarvesterDumpAmount {}
		, OreGathering_Anims {}
		, OreGathering_Tiberiums {}
		, OreGathering_FramesPerDir {}
		, Ammo_AddOnDeploy { 0 }
		, Ammo_AutoDeployMinimumAmount { -1 }
		, Ammo_AutoDeployMaximumAmount { -1 }
		, Ammo_DeployUnlockMinimumAmount { -1 }
		, Ammo_DeployUnlockMaximumAmount { -1 }
		, SubterraneanSpeed { -1 }
		, SubterraneanHeight {}
		, Parasite_AllowWaterExit {}
		, DefaultMirageDisguises {}
		, IsSimpleDeployer_ConsiderPathfinding { false }
		, IsSimpleDeployer_DisallowedLandTypes {}
		, DeployDir {}
		, DeployingAnims {}
		, DeployingAnim_KeepUnitVisible { false }
		, DeployingAnim_ReverseForUndeploy { true }
		, DeployingAnim_UseUnitDrawer { true }
		, JumpjetTilt {}
		, JumpjetTilt_ForwardAccelFactor {}
		, JumpjetTilt_ForwardSpeedFactor {}
		, JumpjetTilt_SidewaysRotationFactor {}
		, JumpjetTilt_SidewaysSpeedFactor {}
		, TiltsWhenCrushes_Vehicles {}
		, TiltsWhenCrushes_Overlays {}
		, CrushForwardTiltPerFrame {}
		, CrushOverlayExtraForwardTilt { 0.02 }
		, CrushSlowdownMultiplier {}
		, SkipCrushSlowdown {}
		, CrateGoodie_RerollChance { 0.0 }
		, NoTurret_TrackTarget {}
		, WaterImage_ConditionYellow {}
		, WaterImage_ConditionRed {}
		, FireUp { -1 }
		, FireUp_ResetInRetarget { true }
		, TurretResponse {}
		, Deploy_SkipPassengerUnload { false }
		, Deploy_NoPassenger { false }
		, Deploy_NoTiberium { false }
		, HoverDrownable {}
		, TurretShape { nullptr }
		, BarrelOverTurret { }
		, BarrelOffset { 0 }
		, ExtraBarrelCount { 0 }
		, ExtraBarrelOffsets { }
		, ExtraTurretCount { 0 }
		, ExtraTurretOffsets { }
		, BurstPerTurret { 0 }
	{ }

	UnitTypeClass* OwnerObject() const
	{
		return static_cast<UnitTypeClass*>(this->GetAttachedObject());
	}

	class ExtContainer final : public Container<UnitTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static UnitTypeExt* Fetch(const UnitTypeClass* pThis)
	{
		return AbstractExt::Fetch<UnitTypeExt>(pThis);
	}

	static UnitTypeExt* TryFetch(const UnitTypeClass* pThis)
	{
		return AbstractExt::TryFetch<UnitTypeExt>(pThis);
	}

	virtual void LoadFromINIFile(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	void ApplyTurretOffsetUnit(Matrix3D* mtx, double factor, int turIdx);

private:
	template <typename T>
	void Serialize(T& Stm);
};
