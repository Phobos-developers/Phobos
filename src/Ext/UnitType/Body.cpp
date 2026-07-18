#include "Body.h"

UnitTypeExt::ExtContainer UnitTypeExt::ExtMap;

// =============================
// load / save

void UnitTypeExt::LoadFromINIFile(CCINIClass* const pINI)
{
	TechnoTypeExt::LoadFromINIFile(pINI);

	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	INI_EX exINI(pINI);

	this->SinkSpeed.Read(exINI, pSection, "SinkSpeed");
	this->Sinkable.Read(exINI, pSection, "Sinkable");
	this->Sinkable_SquidGrab.Read(exINI, pSection, "Sinkable.SquidGrab");
	this->DamagedSpeed.Read(exINI, pSection, "DamagedSpeed");

	this->Harvester_CanGuardArea.Read(exINI, pSection, "Harvester.CanGuardArea");
	this->Harvester_CanGuardArea_RequireTarget.Read(exINI, pSection, "Harvester.CanGuardArea.RequireTarget");
	this->HarvesterScanAfterUnload.Read(exINI, pSection, "HarvesterScanAfterUnload");
	this->HarvesterLoadRate.Read(exINI, pSection, "HarvesterLoadRate");
	this->HarvesterDumpRate.Read(exINI, pSection, "HarvesterDumpRate");
	this->HarvesterDumpAmount.Read(exINI, pSection, "HarvesterDumpAmount");

	this->OreGathering_Anims.Read(exINI, pSection, "OreGathering.Anims");
	this->OreGathering_Tiberiums.Read(exINI, pSection, "OreGathering.Tiberiums");
	this->OreGathering_FramesPerDir.Read(exINI, pSection, "OreGathering.FramesPerDir");

	this->Ammo_AddOnDeploy.Read(exINI, pSection, "Ammo.AddOnDeploy");
	this->Ammo_AutoDeployMinimumAmount.Read(exINI, pSection, "Ammo.AutoDeployMinimumAmount");
	this->Ammo_AutoDeployMaximumAmount.Read(exINI, pSection, "Ammo.AutoDeployMaximumAmount");
	this->Ammo_DeployUnlockMinimumAmount.Read(exINI, pSection, "Ammo.DeployUnlockMinimumAmount");
	this->Ammo_DeployUnlockMaximumAmount.Read(exINI, pSection, "Ammo.DeployUnlockMaximumAmount");

	exINI.ReadSpeed(pSection, "SubterraneanSpeed", &this->SubterraneanSpeed);
	this->SubterraneanHeight.Read(exINI, pSection, "SubterraneanHeight");
	this->Parasite_AllowWaterExit.Read(exINI, pSection, "Parasite.AllowWaterExit");

	this->DefaultMirageDisguises.Read(exINI, pSection, "DefaultMirageDisguises");

	this->IsSimpleDeployer_ConsiderPathfinding.Read(exINI, pSection, "IsSimpleDeployer.ConsiderPathfinding");
	this->IsSimpleDeployer_DisallowedLandTypes.Read<false, true>(exINI, pSection, "IsSimpleDeployer.DisallowedLandTypes");
	this->DeployDir.Read(exINI, pSection, "DeployDir");
	this->DeployingAnims.Read(exINI, pSection, "DeployingAnims");
	this->DeployingAnim_KeepUnitVisible.Read(exINI, pSection, "DeployingAnim.KeepUnitVisible");
	this->DeployingAnim_ReverseForUndeploy.Read(exINI, pSection, "DeployingAnim.ReverseForUndeploy");
	this->DeployingAnim_UseUnitDrawer.Read(exINI, pSection, "DeployingAnim.UseUnitDrawer");

	this->JumpjetTilt.Read(exINI, pSection, "JumpjetTilt");
	this->JumpjetTilt_ForwardAccelFactor.Read(exINI, pSection, "JumpjetTilt.ForwardAccelFactor");
	this->JumpjetTilt_ForwardSpeedFactor.Read(exINI, pSection, "JumpjetTilt.ForwardSpeedFactor");
	this->JumpjetTilt_SidewaysRotationFactor.Read(exINI, pSection, "JumpjetTilt.SidewaysRotationFactor");
	this->JumpjetTilt_SidewaysSpeedFactor.Read(exINI, pSection, "JumpjetTilt.SidewaysSpeedFactor");

	this->TiltsWhenCrushes_Vehicles.Read(exINI, pSection, "TiltsWhenCrushes.Vehicles");
	this->TiltsWhenCrushes_Overlays.Read(exINI, pSection, "TiltsWhenCrushes.Overlays");
	this->CrushForwardTiltPerFrame.Read(exINI, pSection, "CrushForwardTiltPerFrame");
	this->CrushOverlayExtraForwardTilt.Read(exINI, pSection, "CrushOverlayExtraForwardTilt");
	this->CrushSlowdownMultiplier.Read(exINI, pSection, "CrushSlowdownMultiplier");
	this->SkipCrushSlowdown.Read(exINI, pSection, "SkipCrushSlowdown");

	this->CrateGoodie_RerollChance.Read(exINI, pSection, "CrateGoodie.RerollChance");
	this->NoTurret_TrackTarget.Read(exINI, pSection, "NoTurret.TrackTarget");
	this->WaterImage_ConditionYellow.Read(exINI, pSection, "WaterImage.ConditionYellow");
	this->WaterImage_ConditionRed.Read(exINI, pSection, "WaterImage.ConditionRed");

	this->NeedDamagedImage |= this->WaterImage_ConditionYellow.isset() || this->WaterImage_ConditionRed.isset();

	this->TurretResponse.Read(exINI, pSection, "TurretResponse");
	this->Deploy_SkipPassengerUnload.Read(exINI, pSection, "Deploy.SkipPassengerUnload");
	this->Deploy_NoPassenger.Read(exINI, pSection, "Deploy.NoPassenger");
	this->Deploy_NoTiberium.Read(exINI, pSection, "Deploy.NoTiberium");
	this->HoverDrownable.Read(exINI, pSection, "HoverDrownable");

	const auto pArtINI = &CCINIClass::INI_Art;
	INI_EX exArtINI(pArtINI);
	auto pArtSection = pThis->ImageFile;

	this->FireUp.Read(exArtINI, pArtSection, "FireUp");
	this->FireUp_ResetInRetarget.Read(exArtINI, pArtSection, "FireUp.ResetInRetarget");
}

template <typename T>
void UnitTypeExt::Serialize(T& Stm)
{
	Stm
		.Process(this->SinkSpeed)
		.Process(this->Sinkable)
		.Process(this->Sinkable_SquidGrab)
		.Process(this->DamagedSpeed)
		.Process(this->Harvester_CanGuardArea)
		.Process(this->Harvester_CanGuardArea_RequireTarget)
		.Process(this->HarvesterScanAfterUnload)
		.Process(this->HarvesterLoadRate)
		.Process(this->HarvesterDumpRate)
		.Process(this->HarvesterDumpAmount)
		.Process(this->OreGathering_Anims)
		.Process(this->OreGathering_Tiberiums)
		.Process(this->OreGathering_FramesPerDir)
		.Process(this->Ammo_AddOnDeploy)
		.Process(this->Ammo_AutoDeployMinimumAmount)
		.Process(this->Ammo_AutoDeployMaximumAmount)
		.Process(this->Ammo_DeployUnlockMinimumAmount)
		.Process(this->Ammo_DeployUnlockMaximumAmount)
		.Process(this->SubterraneanSpeed)
		.Process(this->SubterraneanHeight)
		.Process(this->Parasite_AllowWaterExit)
		.Process(this->DefaultMirageDisguises)
		.Process(this->IsSimpleDeployer_ConsiderPathfinding)
		.Process(this->IsSimpleDeployer_DisallowedLandTypes)
		.Process(this->DeployDir)
		.Process(this->DeployingAnims)
		.Process(this->DeployingAnim_KeepUnitVisible)
		.Process(this->DeployingAnim_ReverseForUndeploy)
		.Process(this->DeployingAnim_UseUnitDrawer)
		.Process(this->JumpjetTilt)
		.Process(this->JumpjetTilt_ForwardAccelFactor)
		.Process(this->JumpjetTilt_ForwardSpeedFactor)
		.Process(this->JumpjetTilt_SidewaysRotationFactor)
		.Process(this->JumpjetTilt_SidewaysSpeedFactor)
		.Process(this->TiltsWhenCrushes_Vehicles)
		.Process(this->TiltsWhenCrushes_Overlays)
		.Process(this->CrushForwardTiltPerFrame)
		.Process(this->CrushOverlayExtraForwardTilt)
		.Process(this->CrushSlowdownMultiplier)
		.Process(this->SkipCrushSlowdown)
		.Process(this->CrateGoodie_RerollChance)
		.Process(this->NoTurret_TrackTarget)
		.Process(this->WaterImage_ConditionYellow)
		.Process(this->WaterImage_ConditionRed)
		.Process(this->FireUp)
		.Process(this->FireUp_ResetInRetarget)
		.Process(this->TurretResponse)
		.Process(this->Deploy_SkipPassengerUnload)
		.Process(this->Deploy_NoPassenger)
		.Process(this->Deploy_NoTiberium)
		.Process(this->HoverDrownable)
		.Process(this->TurretShape)
		;
}

void UnitTypeExt::LoadFromStream(PhobosStreamReader& Stm)
{
	TechnoTypeExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void UnitTypeExt::SaveToStream(PhobosStreamWriter& Stm)
{
	TechnoTypeExt::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

UnitTypeExt::ExtContainer::ExtContainer() : Container("UnitTypeClass") { }
UnitTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x7470E3, UnitTypeClass_CTOR, 0x6)
{
	GET(UnitTypeClass*, pItem, ESI);

	UnitTypeExt::ExtMap.Allocate(pItem);

	return 0;
}

// Hooked after the base destructor call in both destructor bodies; the second site
// is the tail of the standalone body (pop/pop/retn, safe to steal - the bytes after
// it are alignment padding that is never executed).
DEFINE_HOOK_AGAIN(0x747366, UnitTypeClass_DTOR, 0x3)
DEFINE_HOOK(0x748206, UnitTypeClass_DTOR, 0x5)
{
	GET(UnitTypeClass*, pItem, ESI);

	UnitTypeExt::ExtMap.Remove(pItem);

	return 0;
}
