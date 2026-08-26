#include "Body.h"

#include <cmath>

#include <Ext/TechnoType/Body.h>
#include <New/Type/RadTypeClass.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/DigitalDisplayTypeClass.h>
#include <New/Type/AttachEffectTypeClass.h>
#include <New/Type/BannerTypeClass.h>
#include <New/Type/InsigniaTypeClass.h>
#include <New/Type/SelectBoxTypeClass.h>
#include <New/Type/ResourceTypeClass.h>

std::unique_ptr<RulesExt::ExtData> RulesExt::Data = nullptr;

void RulesExt::Allocate(RulesClass* pThis)
{
	Data = std::make_unique<RulesExt::ExtData>(pThis);
}

void RulesExt::Remove(RulesClass* pThis)
{
	Data = nullptr;
}

void RulesExt::LoadFromINIFile(RulesClass* pThis, CCINIClass* pINI)
{
	Data->LoadFromINI(pINI);
}

void RulesExt::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	ResourceTypeClass::LoadFromINIList(pINI);
	ResourceTypeClass::LoadGlobalsFromINI(pINI);
	DigitalDisplayTypeClass::LoadFromINIList(pINI);
	SelectBoxTypeClass::LoadFromINIList(pINI);
	RadTypeClass::LoadFromINIList(pINI);
	ShieldTypeClass::LoadFromINIList(pINI);
	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art);
	AttachEffectTypeClass::LoadFromINIList(pINI);
	BannerTypeClass::LoadFromINIList(pINI);
	InsigniaTypeClass::LoadFromINIList(pINI);

	Data->LoadBeforeTypeData(pThis, pINI);
}

void RulesExt::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	for (const auto& pTechnoType : TechnoTypeClass::Array)
	{
		if (const auto pTechnoTypeExt = TechnoTypeExt::TryFetch(pTechnoType))
		{
			// Spawner range
			if (pTechnoTypeExt->Spawner_LimitRange)
				pTechnoTypeExt->CalculateSpawnerRange();

			pTechnoTypeExt->UpdateAdditionalAttributes();
		}
	}

	if (pINI == CCINIClass::INI_Rules)
		Data->InitializeAfterTypeData(pThis);

	Data->LoadAfterTypeData(pThis, pINI);
}

void RulesExt::ExtData::InitializeConstants()
{

}

// earliest loader - can't really do much because nothing else is initialized yet, so lookups won't work
void RulesExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	INI_EX exINI(pINI);

	this->DefaultToGuardArea.Read(exINI, GameStrings::General, "DefaultToGuardArea");
	this->LeptonMindControlOffset.Read(exINI, GameStrings::AudioVisual, "LeptonMindControlOffset");
	this->MindControlRingOffset.Read(exINI, GameStrings::AudioVisual, "MindControlRingOffset");
}

void RulesExt::ExtData::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	INI_EX exINI(pINI);

	this->Storage_TiberiumIndex.Read(exINI, GameStrings::General, "Storage.TiberiumIndex");
	this->HarvesterDumpAmount.Read(exINI, GameStrings::General, "HarvesterDumpAmount");
	this->InfantryGainSelfHealCap.Read(exINI, GameStrings::General, "InfantryGainSelfHealCap");
	this->UnitsGainSelfHealCap.Read(exINI, GameStrings::General, "UnitsGainSelfHealCap");
	this->GainSelfHealAllowMultiplayPassive.Read(exINI, GameStrings::General, "GainSelfHealAllowMultiplayPassive");
	this->GainSelfHealFromPlayerControl.Read(exINI, GameStrings::General, "GainSelfHealFromPlayerControl");
	this->GainSelfHealFromAllies.Read(exINI, GameStrings::General, "GainSelfHealFromAllies");
	this->EnemyInsignia.Read(exINI, GameStrings::General, "EnemyInsignia");
	this->DisguiseBlinkingVisibility.Read(exINI, GameStrings::General, "DisguiseBlinkingVisibility");
	this->ChronoSparkleDisplayDelay.Read(exINI, GameStrings::General, "ChronoSparkleDisplayDelay");
	this->ChronoSparkleBuildingDisplayPositions.Read(exINI, GameStrings::General, "ChronoSparkleBuildingDisplayPositions");
	this->ChronoSpherePreDelay.Read(exINI, GameStrings::General, "ChronoSpherePreDelay");
	this->ChronoSphereDelay.Read(exINI, GameStrings::General, "ChronoSphereDelay");
	this->AIChronoSphereSW.Read(exINI, GameStrings::General, "AIChronoSphereSW");
	this->AIChronoWarpSW.Read(exINI, GameStrings::General, "AIChronoWarpSW");

	exINI.ReadSpeed(GameStrings::General, "SubterraneanSpeed", &this->SubterraneanSpeed);
	this->SubterraneanHeight.Read(exINI, GameStrings::General, "SubterraneanHeight");
	this->AISuperWeaponDelay.Read(exINI, GameStrings::General, "AISuperWeaponDelay");
	this->UseGlobalRadApplicationDelay.Read(exINI, GameStrings::Radiation, "UseGlobalRadApplicationDelay");
	this->RadApplicationDelay_Building.Read(exINI, GameStrings::Radiation, "RadApplicationDelay.Building");
	this->RadBuildingDamageMaxCount.Read(exINI, GameStrings::Radiation, "RadBuildingDamageMaxCount");
	this->RadSiteWarhead_Detonate.Read(exINI, GameStrings::Radiation, "RadSiteWarhead.Detonate");
	this->RadSiteWarhead_Detonate_Full.Read(exINI, GameStrings::Radiation, "RadSiteWarhead.Detonate.Full");
	this->RadHasOwner.Read(exINI, GameStrings::Radiation, "RadHasOwner");
	this->RadHasInvoker.Read(exINI, GameStrings::Radiation, "RadHasInvoker");
	this->ShieldApplyArmorMult.Read(exINI, GameStrings::CombatDamage, "ShieldApplyArmorMult");
	this->VeinholeWarhead.Read<true>(exINI, GameStrings::CombatDamage, "VeinholeWarhead");
	this->MissingCameo.Read(pINI, GameStrings::AudioVisual, "MissingCameo");

	this->PlacementGrid_Translucency.Read(exINI, GameStrings::AudioVisual, "PlacementGrid.Translucency");
	this->PlacementGrid_TranslucencyWithPreview.Read(exINI, GameStrings::AudioVisual, "PlacementGrid.TranslucencyWithPreview");
	this->PlacementPreview.Read(exINI, GameStrings::AudioVisual, "PlacementPreview");
	this->PlacementPreview_Translucency.Read(exINI, GameStrings::AudioVisual, "PlacementPreview.Translucency");

	this->SuperWeaponTimer_Percentage.Read(exINI, GameStrings::AudioVisual, "SuperWeaponTimer.Percentage");
	this->SuperWeaponSidebar_AllowByDefault.Read(exINI, GameStrings::AudioVisual, "SuperWeaponSidebar.AllowByDefault");

	this->ConditionYellow_Terrain.Read(exINI, GameStrings::AudioVisual, "ConditionYellow.Terrain");
	this->Shield_ConditionYellow.Read(exINI, GameStrings::AudioVisual, "Shield.ConditionYellow");
	this->Shield_ConditionRed.Read(exINI, GameStrings::AudioVisual, "Shield.ConditionRed");
	this->Pips_Shield.Read(exINI, GameStrings::AudioVisual, "Pips.Shield");
	this->Pips_Shield_Background.Read(exINI, GameStrings::AudioVisual, "Pips.Shield.Background");
	this->Pips_Shield_Building.Read(exINI, GameStrings::AudioVisual, "Pips.Shield.Building");
	this->Pips_Shield_Building_Empty.Read(exINI, GameStrings::AudioVisual, "Pips.Shield.Building.Empty");
	this->Pips_SelfHeal_Infantry.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Infantry");
	this->Pips_SelfHeal_Units.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Units");
	this->Pips_SelfHeal_Buildings.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Buildings");
	this->Pips_SelfHeal_Infantry_Offset.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Infantry.Offset");
	this->Pips_SelfHeal_Units_Offset.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Units.Offset");
	this->Pips_SelfHeal_Buildings_Offset.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Buildings.Offset");
	this->Pips_Generic_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Generic.Size");
	this->Pips_Generic_Buildings_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Generic.Buildings.Size");
	this->Pips_Ammo_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Ammo.Size");
	this->Pips_Ammo_Buildings_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Ammo.Buildings.Size");
	this->Pips_Tiberiums_Frames.Read(exINI, GameStrings::AudioVisual, "Pips.Tiberiums.Frames");
	this->Pips_Tiberiums_EmptyFrame.Read(exINI, GameStrings::AudioVisual, "Pips.Tiberiums.EmptyFrame");
	this->Pips_Tiberiums_DisplayOrder.Read(exINI, GameStrings::AudioVisual, "Pips.Tiberiums.DisplayOrder");
	this->Pips_Tiberiums_WeedFrame.Read(exINI, GameStrings::AudioVisual, "Pips.Tiberiums.WeedFrame");
	this->Pips_Tiberiums_WeedEmptyFrame.Read(exINI, GameStrings::AudioVisual, "Pips.Tiberiums.WeedEmptyFrame");
	this->ToolTip_Background_Color.Read(exINI, GameStrings::AudioVisual, "ToolTip.Background.Color");
	this->ToolTip_Background_Opacity.Read(exINI, GameStrings::AudioVisual, "ToolTip.Background.Opacity");
	this->ToolTip_Background_BlurSize.Read(exINI, GameStrings::AudioVisual, "ToolTip.Background.BlurSize");
	this->RadialIndicatorVisibility.Read(exINI, GameStrings::AudioVisual, "RadialIndicatorVisibility");
	this->DrawTurretShadow.Read(exINI, GameStrings::AudioVisual, "DrawTurretShadow");
	this->AnimRemapDefaultColorScheme.Read(exINI, GameStrings::AudioVisual, "AnimRemapDefaultColorScheme");
	this->TimerBlinkColorScheme.Read(exINI, GameStrings::AudioVisual, "TimerBlinkColorScheme");
	this->ShowDesignatorRange.Read(exINI, GameStrings::AudioVisual, "ShowDesignatorRange");
	this->ShowPowerPlantEnhancerRange.Read(exINI, GameStrings::AudioVisual, "ShowPowerPlantEnhancerRange");

	this->ShowGameTime.Read(exINI, GameStrings::General, "ShowGameTime");

	Nullable<double>AirShadowBaseScale;
	AirShadowBaseScale.Read(exINI, GameStrings::AudioVisual, "AirShadowBaseScale");
	if (AirShadowBaseScale.isset() && AirShadowBaseScale.Get() > 0)
		this->AirShadowBaseScale_log = -std::log(std::min(AirShadowBaseScale.Get(), 1.0));

	this->HeightShadowScaling.Read(exINI, GameStrings::AudioVisual, "HeightShadowScaling");
	if (AirShadowBaseScale.isset() && AirShadowBaseScale.Get() > 0.98 && this->HeightShadowScaling.Get())
		this->HeightShadowScaling = false;
	this->HeightShadowScaling_MinScale.Read(exINI, GameStrings::AudioVisual, "HeightShadowScaling.MinScale");

	this->ExtendedAircraftMissions.Read(exINI, GameStrings::General, "ExtendedAircraftMissions");
	this->ExtendedAircraftMissions_UnlandDamage.Read(exINI, GameStrings::General, "ExtendedAircraftMissions.UnlandDamage");
	this->AircraftSpawnFromEdge.Read(exINI, GameStrings::General, "AircraftSpawnFromEdge");
	this->AircraftRetreatToEdge.Read(exINI, GameStrings::General, "AircraftRetreatToEdge");
	this->AmphibiousEnter.Read(exINI, GameStrings::General, "AmphibiousEnter");
	this->AmphibiousUnload.Read(exINI, GameStrings::General, "AmphibiousUnload");
	this->NoQueueUpToEnter.Read(exINI, GameStrings::General, "NoQueueUpToEnter");
	this->NoQueueUpToEnter_BoardDistance.Read(exINI, GameStrings::General, "NoQueueUpToEnter.BoardDistance");
	this->NoQueueUpToUnload.Read(exINI, GameStrings::General, "NoQueueUpToUnload");
	this->NoQueueUpToEnter_Buildings.Read(exINI, GameStrings::General, "NoQueueUpToEnter.Buildings");
	this->NoQueueUpToUnload_Buildings.Read(exINI, GameStrings::General, "NoQueueUpToUnload.Buildings");

	this->JumpjetTilt.Read(exINI, GameStrings::AudioVisual, "JumpjetTilt");
	this->JumpjetTilt_ForwardAccelFactor.Read(exINI, GameStrings::AudioVisual, "JumpjetTilt.ForwardAccelFactor");
	this->JumpjetTilt_ForwardSpeedFactor.Read(exINI, GameStrings::AudioVisual, "JumpjetTilt.ForwardSpeedFactor");
	this->JumpjetTilt_SidewaysRotationFactor.Read(exINI, GameStrings::AudioVisual, "JumpjetTilt.SidewaysRotationFactor");
	this->JumpjetTilt_SidewaysSpeedFactor.Read(exINI, GameStrings::AudioVisual, "JumpjetTilt.SidewaysSpeedFactor");
	this->Spawner_AttackImmediately.Read(exINI, GameStrings::General, "Spawner.AttackImmediately");
	this->Spawner_UseTurretFacing.Read(exINI, GameStrings::General, "Spawner.UseTurretFacing");
	this->Spawner_RecycleRange.Read(exINI, GameStrings::General, "Spawner.RecycleRange");
	this->Spawner_RecycleOnTurret.Read(exINI, GameStrings::General, "Spawner.RecycleOnTurret");
	this->Promote_IncludeSpawns.Read(exINI, GameStrings::General, "Promote.IncludeSpawns");
	this->RadarJamHouses.Read(exINI, GameStrings::General, "RadarJamHouses");
	this->RadarJamDelay.Read(exINI, GameStrings::General, "RadarJamDelay");
	this->MindControl_IgnoreSize.Read(exINI, GameStrings::General, "MindControl.IgnoreSize");
	this->MultiMindControl_ReleaseVictim.Read(exINI, GameStrings::General, "MultiMindControl.ReleaseVictim");
	this->MindControlLink_VisibleToHouse.Read(exINI, GameStrings::General, "MindControlLink.VisibleToHouse");
	this->AlternateFLH_OnTurret.Read(exINI, GameStrings::General, "AlternateFLH.OnTurret");
	this->AlternateFLH_ApplyVehicle.Read(exINI, GameStrings::General, "AlternateFLH.ApplyVehicle");
	this->DestroyAnim_Random.Read(exINI, GameStrings::General, "DestroyAnim.Random");
	this->UseDisguiseMovementSpeed.Read(exINI, GameStrings::General, "UseDisguiseMovementSpeed");
	this->Convert_ResetMindControl.Read(exINI, GameStrings::General, "Convert.ResetMindControl");
	this->BuildLimitGroup_ContentIfAnyMatch.Read(exINI, GameStrings::General, "BuildLimitGroup.ContentIfAnyMatch");
	this->BuildLimitGroup_NotBuildableIfQueueMatch.Read(exINI, GameStrings::General, "BuildLimitGroup.NotBuildableIfQueueMatch");
	this->DigitalDisplay_Health_FakeAtDisguise.Read(exINI, GameStrings::AudioVisual, "DigitalDisplay.Health.FakeAtDisguise");
	this->Overload_ParticleSysCount.Read(exINI, GameStrings::CombatDamage, "Overload.ParticleSysCount");
	this->FallingDownDamage.Read(exINI, GameStrings::CombatDamage, "FallingDownDamage");
	this->FallingDownDamage_AllowEMP.Read(exINI, GameStrings::CombatDamage, "FallingDownDamage.AllowEMP");

	this->ForceWeapon_InRange_TechnoOnly.Read(exINI, GameStrings::General, "ForceWeapon.InRange.TechnoOnly");
	this->ForceWeapon_InRange_ApplyRangeModifiers.Read(exINI, GameStrings::General, "ForceWeapon.InRange.ApplyRangeModifiers");
	this->ForceAAWeapon_InRange_ApplyRangeModifiers.Read(exINI, GameStrings::General, "ForceAAWeapon.InRange.ApplyRangeModifiers");

	this->BuildingProductionQueue.Read(exINI, GameStrings::General, "BuildingProductionQueue");

	this->AllowParallelAIQueues.Read(exINI, "GlobalControls", "AllowParallelAIQueues");
	this->ForbidParallelAIQueues_Aircraft.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Aircraft");
	this->ForbidParallelAIQueues_Building.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Building");
	this->ForbidParallelAIQueues_Infantry.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Infantry");
	this->ForbidParallelAIQueues_Navy.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Navy");
	this->ForbidParallelAIQueues_Vehicle.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Vehicle");

	this->EnablePowerSurplus.Read(exINI, GameStrings::AI, "EnablePowerSurplus");
	this->PowerSurplus_ScaleToDrainAmount.Read(exINI, GameStrings::AI, "PowerSurplus.ScaleToDrainAmount");

	this->AllowDeployControlledMCV.Read(exINI, GameStrings::General, "AllowDeployControlledMCV");

	this->TypeSelectUseIFVMode.Read(exINI, GameStrings::General, "TypeSelectUseIFVMode");

	this->IronCurtain_KeptOnDeploy.Read(exINI, GameStrings::CombatDamage, "IronCurtain.KeptOnDeploy");
	this->IronCurtain_EffectOnOrganics.Read(exINI, GameStrings::CombatDamage, "IronCurtain.EffectOnOrganics");
	this->IronCurtain_KillOrganicsWarhead.Read<true>(exINI, GameStrings::CombatDamage, "IronCurtain.KillOrganicsWarhead");
	this->ForceShield_KeptOnDeploy.Read(exINI, GameStrings::CombatDamage, "ForceShield.KeptOnDeploy");
	this->ForceShield_EffectOnOrganics.Read(exINI, GameStrings::CombatDamage, "ForceShield.EffectOnOrganics");
	this->ForceShield_KillOrganicsWarhead.Read<true>(exINI, GameStrings::CombatDamage, "ForceShield.KillOrganicsWarhead");
	this->AllowWeaponSelectAgainstWalls.Read(exINI, GameStrings::CombatDamage, "AllowWeaponSelectAgainstWalls");

	this->IronCurtain_ExtraTintIntensity.Read(exINI, GameStrings::AudioVisual, "IronCurtain.ExtraTintIntensity");
	this->ForceShield_ExtraTintIntensity.Read(exINI, GameStrings::AudioVisual, "ForceShield.ExtraTintIntensity");
	this->ColorAddUse8BitRGB.Read(exINI, GameStrings::AudioVisual, "ColorAddUse8BitRGB");
	this->AirstrikeLineColor.Read(exINI, GameStrings::AudioVisual, "AirstrikeLineColor");
	this->AirstrikeLineZAdjust.Read(exINI, GameStrings::AudioVisual, "AirstrikeLineZAdjust");

	this->Strafing_SimulateBurst.Read(exINI, GameStrings::General, "Strafing.SimulateBurst");
	this->Strafing_UseAmmoPerShot.Read(exINI, GameStrings::General, "Strafing.UseAmmoPerShot");
	this->Strafing_TargetCell.Read(exINI, GameStrings::General, "Strafing.TargetCell");
	this->OmniFire_TurnToTarget.Read(exINI, GameStrings::General, "OmniFire.TurnToTarget");
	this->AmbientDamage_IgnoreTarget.Read(exINI, GameStrings::General, "AmbientDamage.IgnoreTarget");
	this->KeepRange_AllowAI.Read(exINI, GameStrings::General, "KeepRange.AllowAI");
	this->KeepRange_AllowPlayer.Read(exINI, GameStrings::General, "KeepRange.AllowPlayer");
	this->KeepRange_EarlyStopFrame.Read(exINI, GameStrings::General, "KeepRange.EarlyStopFrame");
	this->AircraftWeapon_KickOutPassengers.Read(exINI, GameStrings::General, "AircraftWeapon.KickOutPassengers");
	this->CrushSlowdownMultiplier.Read(exINI, GameStrings::General, "CrushSlowdownMultiplier");
	this->SkipCrushSlowdown.Read(exINI, GameStrings::General, "SkipCrushSlowdown");

	this->LaserPositionUpdate_StopOnFirerConvert.Read(exINI, GameStrings::AudioVisual, "LaserPositionUpdate.StopOnFirerConvert");
	this->LaserZAdjust.Read(exINI, GameStrings::AudioVisual, "LaserZAdjust");
	this->EBoltZAdjust.Read(exINI, GameStrings::AudioVisual, "EBoltZAdjust");
	this->EBoltZAdjust_ClampInitialDepthForBuilding.Read(exINI, GameStrings::AudioVisual, "EBoltZAdjust.ClampInitialDepthForBuilding");

	this->CrateOnlyOnLand.Read(exINI, GameStrings::CrateRules, "CrateOnlyOnLand");
	this->UnitCrateVehicleCap.Read(exINI, GameStrings::CrateRules, "UnitCrateVehicleCap");
	this->FreeMCV_CreditsThreshold.Read(exINI, GameStrings::CrateRules, "FreeMCV.CreditsThreshold");

	this->ROF_RandomDelay.Read(exINI, GameStrings::CombatDamage, "ROF.RandomDelay");

	this->DisplayIncome.Read(exINI, GameStrings::AudioVisual, "DisplayIncome");
	this->DisplayIncome_Delay.Read(exINI, GameStrings::AudioVisual, "DisplayIncome.Delay");
	if (!this->DisplayIncome_Delay)
	{
		Debug::Log("[Developer warning] [AudioVisual] DisplayIncome.Delay is set 0 which would cause a crash, set to 1 instead.\n");
		this->DisplayIncome_Delay = 1;
	}
	this->DisplayIncome_Houses.Read(exINI, GameStrings::AudioVisual, "DisplayIncome.Houses");
	this->DisplayIncome_AllowAI.Read(exINI, GameStrings::AudioVisual, "DisplayIncome.AllowAI");

	this->DrainMoneyDisplay.Read(exINI, GameStrings::AudioVisual, "DrainMoneyDisplay");
	this->DrainMoneyDisplay_Houses.Read(exINI, GameStrings::AudioVisual, "DrainMoneyDisplay.Houses");
	this->DrainMoneyDisplay_OnTarget.Read(exINI, GameStrings::AudioVisual, "DrainMoneyDisplay.OnTarget");
	this->DrainMoneyDisplay_OnTarget_UseDisplayIncome.Read(exINI, GameStrings::AudioVisual, "DrainMoneyDisplay.OnTarget.UseDisplayIncome");

	this->IsVoiceCreatedGlobal.Read(exINI, GameStrings::AudioVisual, "IsVoiceCreatedGlobal");
	this->SetTabBySelectingFactory.Read(exINI, GameStrings::General, "SetTabBySelectingFactory");
	this->SelectionFlashDuration.Read(exINI, GameStrings::AudioVisual, "SelectionFlashDuration");
	this->SetRecruitableOnLiberate.Read(exINI, GameStrings::General, "SetRecruitableOnLiberate");
	this->DrawInsignia_OnlyOnSelected.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.OnlyOnSelected");
	this->DrawInsignia_AdjustPos_Infantry.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.AdjustPos.Infantry");
	this->DrawInsignia_AdjustPos_Buildings.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.AdjustPos.Buildings");
	this->DrawInsignia_AdjustPos_BuildingsAnchor.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.AdjustPos.BuildingsAnchor");
	this->DrawInsignia_AdjustPos_Units.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.AdjustPos.Units");
	this->DrawInsignia_UsePixelSelectionBracketDelta.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.UsePixelSelectionBracketDelta");
	this->Promote_VeteranAnimation.Read(exINI, GameStrings::AudioVisual, "Promote.VeteranAnimation");
	this->Promote_EliteAnimation.Read(exINI, GameStrings::AudioVisual, "Promote.EliteAnimation");

	this->DropPodTrailer.Read(exINI, GameStrings::General, "DropPodTrailer");
	this->DropPodDefaultTrailer = AnimTypeClass::Find("SMOKEY");
	this->PodImage = FileSystem::LoadSHPFile("POD.SHP");

	this->BuildingWaypoints.Read(exINI, GameStrings::General, "BuildingWaypoints");

	this->VisualScatter_Min.Read(exINI, GameStrings::AudioVisual, "VisualScatter.Min");
	this->VisualScatter_Max.Read(exINI, GameStrings::AudioVisual, "VisualScatter.Max");

	this->Buildings_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Buildings.DefaultDigitalDisplayTypes");
	this->Infantry_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Infantry.DefaultDigitalDisplayTypes");
	this->Vehicles_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Vehicles.DefaultDigitalDisplayTypes");
	this->Aircraft_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Aircraft.DefaultDigitalDisplayTypes");

	this->DefaultInfantrySelectBox.Read(exINI, GameStrings::AudioVisual, "DefaultInfantrySelectBox");
	this->DefaultUnitSelectBox.Read(exINI, GameStrings::AudioVisual, "DefaultUnitSelectBox");

	this->JumpjetClimbPredictHeight.Read(exINI, GameStrings::General, "JumpjetClimbPredictHeight");
	this->JumpjetClimbWithoutCutOut.Read(exINI, GameStrings::General, "JumpjetClimbWithoutCutOut");
	this->JumpjetClimbIgnoreBuilding.Read(exINI, GameStrings::General, "JumpjetClimbIgnoreBuilding");

	this->DamageOwnerMultiplier.Read(exINI, GameStrings::CombatDamage, "DamageOwnerMultiplier");
	this->DamageAlliesMultiplier.Read(exINI, GameStrings::CombatDamage, "DamageAlliesMultiplier");
	this->DamageEnemiesMultiplier.Read(exINI, GameStrings::CombatDamage, "DamageEnemiesMultiplier");
	this->DamageOwnerMultiplier_NotAffectsEnemies.Read(exINI, GameStrings::CombatDamage, "DamageOwnerMultiplier.NotAffectsEnemies");
	this->DamageAlliesMultiplier_NotAffectsEnemies.Read(exINI, GameStrings::CombatDamage, "DamageAlliesMultiplier.NotAffectsEnemies");
	this->DamageOwnerMultiplier_Berzerk.Read(exINI, GameStrings::CombatDamage, "DamageOwnerMultiplier.Berzerk");
	this->DamageAlliesMultiplier_Berzerk.Read(exINI, GameStrings::CombatDamage, "DamageAlliesMultiplier.Berzerk");
	this->DamageEnemiesMultiplier_Berzerk.Read(exINI, GameStrings::CombatDamage, "DamageEnemiesMultiplier.Berzerk");

	this->AircraftLevelLightMultiplier.Read(exINI, GameStrings::AudioVisual, "AircraftLevelLightMultiplier");
	this->JumpjetLevelLightMultiplier.Read(exINI, GameStrings::AudioVisual, "JumpjetLevelLightMultiplier");

	this->VoxelLightSource.Read(exINI, GameStrings::AudioVisual, "VoxelLightSource");
	// this->VoxelShadowLightSource.Read(exINI, GameStrings::AudioVisual, "VoxelShadowLightSource");

	this->CombatAlert.Read(exINI, GameStrings::AudioVisual, "CombatAlert");
	this->CombatAlert_Default.Read(exINI, GameStrings::AudioVisual, "CombatAlert.Default");
	this->CombatAlert_IgnoreBuilding.Read(exINI, GameStrings::AudioVisual, "CombatAlert.IgnoreBuilding");
	this->CombatAlert_SuppressIfInScreen.Read(exINI, GameStrings::AudioVisual, "CombatAlert.SuppressIfInScreen");
	this->CombatAlert_Interval.Read(exINI, GameStrings::AudioVisual, "CombatAlert.Interval");
	this->CombatAlert_SuppressIfAllyDamage.Read(exINI, GameStrings::AudioVisual, "CombatAlert.SuppressIfAllyDamage");
	this->CombatAlert_MakeAVoice.Read(exINI, GameStrings::AudioVisual, "CombatAlert.MakeAVoice");
	this->CombatAlert_UseFeedbackVoice.Read(exINI, GameStrings::AudioVisual, "CombatAlert.UseFeedbackVoice");
	this->CombatAlert_UseAttackVoice.Read(exINI, GameStrings::AudioVisual, "CombatAlert.UseAttackVoice");
	this->CombatAlert_UseEVA.Read(exINI, GameStrings::AudioVisual, "CombatAlert.UseEVA");

	this->ReplaceVoxelLightSources();

	this->UseFixedVoxelLighting.Read(exINI, GameStrings::AudioVisual, "UseFixedVoxelLighting");

	this->AIAutoDeployMCV.Read(exINI, GameStrings::AI, "AIAutoDeployMCV");
	this->AISetBaseCenter.Read(exINI, GameStrings::AI, "AISetBaseCenter");
	this->AIBiasSpawnCell.Read(exINI, GameStrings::AI, "AIBiasSpawnCell");
	this->AIForbidConYard.Read(exINI, GameStrings::AI, "AIForbidConYard");
	this->AINodeWallsOnly.Read(exINI, GameStrings::AI, "AINodeWallsOnly");
	this->AICleanWallNode.Read(exINI, GameStrings::AI, "AICleanWallNode");

	this->AttackMove_Aggressive.Read(exINI, GameStrings::General, "AttackMove.Aggressive");
	this->AttackMove_UpdateTarget.Read(exINI, GameStrings::General, "AttackMove.UpdateTarget");

	this->MindControl_ThreatDelay.Read(exINI, GameStrings::General, "MindControl.ThreatDelay");

	this->RecountBurst.Read(exINI, GameStrings::General, "RecountBurst");
	this->NoRearm_UnderEMP.Read(exINI, GameStrings::General, "NoRearm.UnderEMP");
	this->NoRearm_Temporal.Read(exINI, GameStrings::General, "NoRearm.Temporal");
	this->NoReload_UnderEMP.Read(exINI, GameStrings::General, "NoReload.UnderEMP");
	this->NoReload_Temporal.Read(exINI, GameStrings::General, "NoReload.Temporal");

	this->VeteranReload.Read(exINI, GameStrings::General, "VeteranReload");
	this->VeteranEmptyReload.Read(exINI, GameStrings::General, "VeteranEmptyReload");

	this->NoTurret_TrackTarget.Read(exINI, GameStrings::General, "NoTurret.TrackTarget");

	this->GatherWhenMCVDeploy.Read(exINI, GameStrings::General, "GatherWhenMCVDeploy");
	this->AIFireSale.Read(exINI, GameStrings::General, "AIFireSale");
	this->AIFireSaleDelay.Read(exINI, GameStrings::General, "AIFireSaleDelay");
	this->AIAllToHunt.Read(exINI, GameStrings::General, "AIAllToHunt");
	this->RepairBaseNodes.Read(exINI, GameStrings::Basic, "RepairBaseNodes");

	this->FixRepairStepCost.Read(exINI, GameStrings::General, "FixRepairStepCost");

	this->WarheadParticleAlphaImageIsLightFlash.Read(exINI, GameStrings::AudioVisual, "WarheadParticleAlphaImageIsLightFlash");
	this->CombatLightDetailLevel.Read(exINI, GameStrings::AudioVisual, "CombatLightDetailLevel");
	this->CombatLightDetailLevel_CheckColored.Read(exINI, GameStrings::AudioVisual, "CombatLightDetailLevel.CheckColored");
	this->LightFlashAlphaImageDetailLevel.Read(exINI, GameStrings::AudioVisual, "LightFlashAlphaImageDetailLevel");
	this->BuildingTypeSelectable.Read(exINI, GameStrings::General, "BuildingTypeSelectable");

	this->UseRetintFix.Read(exINI, GameStrings::AudioVisual, "UseRetintFix");

	this->ProneSpeed_Crawls.Read(exINI, GameStrings::General, "ProneSpeed.Crawls");
	this->ProneSpeed_NoCrawls.Read(exINI, GameStrings::General, "ProneSpeed.NoCrawls");

	this->DamagedSpeed.Read(exINI, GameStrings::General, "DamagedSpeed");

	this->HarvesterScanAfterUnload.Read(exINI, GameStrings::General, "HarvesterScanAfterUnload");

	this->AnimCraterDestroyTiberium.Read(exINI, GameStrings::General, "AnimCraterDestroyTiberium");

	this->BerzerkTargeting.Read(exINI, GameStrings::CombatDamage, "BerzerkTargeting");
	this->AllowBerzerkOnAllies.Read(exINI, GameStrings::CombatDamage, "AllowBerzerkOnAllies");

	this->AttackMove_IgnoreWeaponCheck.Read(exINI, GameStrings::General, "AttackMove.IgnoreWeaponCheck");

	this->Parasite_GrappleAnim.Read(exINI, GameStrings::AudioVisual, "Parasite.GrappleAnim");
	this->Parasite_AllowWaterExit.Read(exINI, GameStrings::General, "Parasite.AllowWaterExit");

	this->AINormalTargetingDelay.Read(exINI, GameStrings::General, "AINormalTargetingDelay");
	this->PlayerNormalTargetingDelay.Read(exINI, GameStrings::General, "PlayerNormalTargetingDelay");
	this->AIGuardAreaTargetingDelay.Read(exINI, GameStrings::General, "AIGuardAreaTargetingDelay");
	this->PlayerGuardAreaTargetingDelay.Read(exINI, GameStrings::General, "PlayerGuardAreaTargetingDelay");
	this->AIAttackMoveTargetingDelay.Read(exINI, GameStrings::General, "AIAttackMoveTargetingDelay");
	this->PlayerAttackMoveTargetingDelay.Read(exINI, GameStrings::General, "PlayerAttackMoveTargetingDelay");
	this->DistributeTargetingFrame.Read(exINI, GameStrings::General, "DistributeTargetingFrame");
	this->DistributeTargetingFrame_AIOnly.Read(exINI, GameStrings::General, "DistributeTargetingFrame.AIOnly");

	this->CanTargetAI_IronCurtained.Read(exINI, GameStrings::CombatDamage, "CanTargetAI.IronCurtained");
	this->CanTarget_IronCurtained.Read(exINI, GameStrings::CombatDamage, "CanTarget.IronCurtained");
	this->AutoTarget_IronCurtained.Read(exINI, GameStrings::CombatDamage, "AutoTarget.IronCurtained");

	this->InfantryAutoDeploy.Read(exINI, GameStrings::General, "InfantryAutoDeploy");

	this->AdjacentWallDamage.Read(exINI, GameStrings::CombatDamage, "AdjacentWallDamage");

	this->WarheadAnimZAdjust.Read(exINI, GameStrings::AudioVisual, "WarheadAnimZAdjust");

	this->IvanBombAttachToCenter.Read(exINI, GameStrings::CombatDamage, "IvanBombAttachToCenter");
	this->IvanBomb_Visibility.Read(exINI, GameStrings::AudioVisual, "IvanIconVisibility");
	this->MissileSpawnAttackCell.Read(exINI, GameStrings::CombatDamage, "MissileSpawnAttackCell");

	this->FallingDownTargetingFix.Read(exINI, GameStrings::General, "FallingDownTargetingFix");
	this->AIAirTargetingFix.Read(exINI, GameStrings::General, "AIAirTargetingFix");

	this->ReloadInTransport.Read(exINI, GameStrings::General, "ReloadInTransport");
	this->OpenTopped_IgnoreRangefinding.Read(exINI, GameStrings::General, "OpenTopped.IgnoreRangefinding");
	this->OpenTopped_AllowFiringIfDeactivated.Read(exINI, GameStrings::General, "OpenTopped.AllowFiringIfDeactivated");
	this->OpenTopped_AllowFiringIfAttackedByLocomotor.Read(exINI, GameStrings::General, "OpenTopped.AllowFiringIfAttackedByLocomotor");
	this->OpenTopped_ShareTransportTarget.Read(exINI, GameStrings::General, "OpenTopped.ShareTransportTarget");
	this->OpenTopped_UseTransportRangeModifiers.Read(exINI, GameStrings::General, "OpenTopped.UseTransportRangeModifiers");
	this->OpenTopped_CheckTransportDisableWeapons.Read(exINI, GameStrings::General, "OpenTopped.CheckTransportDisableWeapons");
	this->OpenTopped_DecloakToFire.Read(exINI, GameStrings::General, "OpenTopped.DecloakToFire");
	this->OpenTopped_FireWhileMoving.Read(exINI, GameStrings::General, "OpenTopped.FireWhileMoving");
	this->OpenTransport_RangeBonus.Read(exINI, GameStrings::CombatDamage, "OpenTransport.RangeBonus");
	this->OpenTransport_DamageMultiplier.Read(exINI, GameStrings::CombatDamage, "OpenTransport.DamageMultiplier");
	this->OpenTransport_FireWhileMoving.Read(exINI, GameStrings::General, "OpenTransport.FireWhileMoving");

	this->Passengers_SyncOwner.Read(exINI, GameStrings::General, "Passengers.SyncOwner");
	this->Passengers_SyncOwner_RevertOnExit.Read(exINI, GameStrings::General, "Passengers.SyncOwner.RevertOnExit");

	this->Explodes_KillPassengers.Read(exINI, GameStrings::General, "Explodes.KillPassengers");
	this->Explodes_DuringBuildup.Read(exINI, GameStrings::General, "Explodes.DuringBuildup");

	this->AircraftFiringForceScatter.Read(exINI, GameStrings::General, "AircraftFiringForceScatter");

	this->HoverDrownable.Read(exINI, GameStrings::General, "HoverDrownable");

	this->Arcing_AllowElevationInaccuracy.Read(exINI, GameStrings::CombatDamage, "Arcing.AllowElevationInaccuracy");

	this->Terrain_IsPassable.Read(exINI, GameStrings::General, "Terrain.IsPassable");
	this->Tibtree_IsPassable.Read(exINI, GameStrings::General, "Tibtree.IsPassable");
	this->Terrain_CanBeBuiltOn.Read(exINI, GameStrings::General, "Terrain.CanBeBuiltOn");
	this->Tibtree_CanBeBuiltOn.Read(exINI, GameStrings::General, "Tibtree.CanBeBuiltOn");

	this->Sinkable.Read(exINI, GameStrings::General, "Sinkable");
	this->Sinkable_SquidGrab.Read(exINI, GameStrings::General, "Sinkable.SquidGrab");
	this->SinkSpeed.Read(exINI, GameStrings::General, "SinkSpeed");

	this->CreateAnimsOnZeroDamage.Read(exINI, GameStrings::General, "CreateAnimsOnZeroDamage");
	this->Conventional_IgnoreUnits.Read(exINI, GameStrings::General, "Conventional.IgnoreUnits");
	this->DecloakDamagedTargets.Read(exINI, GameStrings::General, "DecloakDamagedTargets");
	this->ShakeIsLocal.Read(exINI, GameStrings::General, "ShakeIsLocal");
	this->ApplyModifiersOnNegativeDamage.Read(exINI, GameStrings::General, "ApplyModifiersOnNegativeDamage");
	this->AllowDamageOnSelf.Read(exINI, GameStrings::General, "AllowDamageOnSelf");
	this->Debris_Conventional.Read(exINI, GameStrings::General, "Debris.Conventional");
	this->Parasite_DisableParticleSystem.Read(exINI, GameStrings::CombatDamage, "Parasite.DisableParticleSystem");

	this->ProjectileInterceptable.Read(exINI, GameStrings::CombatDamage, "ProjectileInterceptable");
	this->Interceptor_GuardRange_IsCylindrical.Read(exINI, GameStrings::CombatDamage, "Interceptor.GuardRange.IsCylindrical");
	this->Interceptor_ApplyFirepowerMult.Read(exINI, GameStrings::CombatDamage, "Interceptor.ApplyFirepowerMult");

	this->SortCameoByName.Read(exINI, GameStrings::General, "SortCameoByName");

	this->MergeBuildingDamage.Read(exINI, GameStrings::CombatDamage, "MergeBuildingDamage");

	this->ApplyPerTargetEffectsOnDetonate.Read(exINI, GameStrings::CombatDamage, "ApplyPerTargetEffectsOnDetonate");

	this->AffectsInvokerOnly_IgnoreInvokerState.Read(exINI, GameStrings::CombatDamage, "AffectsInvokerOnly.IgnoreInvokerState");

	this->BuildingRadioLink_SyncOwner.Read(exINI, GameStrings::General, "BuildingRadioLink.SyncOwner");

	this->ExtraRange_TargetMoving.Read(exINI, GameStrings::General, "ExtraRange.TargetMoving");
	this->ExtraRange_TargetMoving_CloseRangeOnly.Read(exINI, GameStrings::General, "ExtraRange.TargetMoving.CloseRangeOnly");
	this->ExtraRange_FirerMoving.Read(exINI, GameStrings::General, "ExtraRange.FirerMoving");
	this->ExtraRange_Prefiring.Read(exINI, GameStrings::General, "ExtraRange.Prefiring");
	this->ExtraRange_Prefiring_IncludeBurst.Read(exINI, GameStrings::General, "ExtraRange.Prefiring.IncludeBurst");

	this->AutoTarget_NoThreatBuildings.Read(exINI, GameStrings::General, "AutoTarget.NoThreatBuildings");
	this->AutoTargetAI_NoThreatBuildings.Read(exINI, GameStrings::General, "AutoTargetAI.NoThreatBuildings");

	this->ParadropMission.Read(exINI, GameStrings::General, "ParadropMission");
	this->AIParadropMission.Read(exINI, GameStrings::General, "AIParadropMission");
	this->ParadropDelay.Read(exINI, GameStrings::General, "ParadropDelay");
	this->ParadropEndDelay.Read(exINI, GameStrings::General, "ParadropEndDelay");

	this->CylinderRangefinding.Read(exINI, GameStrings::General, "CylinderRangefinding");

	this->PenetratesTransport_Level.Read(exINI, GameStrings::CombatDamage, "PenetratesTransport.Level");

	this->UnitsUnsellable.Read(exINI, GameStrings::General, "UnitsUnsellable");

	this->DisableOveroptimizationInTargeting.Read(exINI, GameStrings::General, "DisableOveroptimizationInTargeting");

	this->DriverKilled_KeptPassengers.Read(exINI, GameStrings::CombatDamage, "DriverKilled.KeptPassengers");
	this->DriverKilled_KillPassengers.Read(exINI, GameStrings::CombatDamage, "DriverKilled.KillPassengers");
	this->ExtraThreat_IsThreat.Read(exINI, GameStrings::General, "ExtraThreat.IsThreat");
	this->ExtraThreat_InRange.Read(exINI, GameStrings::General, "ExtraThreat.InRange");
	this->ExtraThreatCoefficient_InRangeDistance.Read(exINI, GameStrings::General, "ExtraThreatCoefficient.InRangeDistance");
	this->ExtraThreatCoefficient_Facing.Read(exINI, GameStrings::General, "ExtraThreatCoefficient.Facing");
	this->ExtraThreatCoefficient_DistanceToLastTarget.Read(exINI, GameStrings::General, "ExtraThreatCoefficient.DistanceToLastTarget");
	this->BalloonHoverPathingFix.Read(exINI, GameStrings::General, "BalloonHoverPathingFix");
	Phobos::Optimizations::DisableBalloonHoverPathingFix = !this->BalloonHoverPathingFix;

	this->WalkLocomotorMakesWake.Read(exINI, GameStrings::AudioVisual, "WalkLocomotorMakesWake");
	this->DriveLocomotorMakesWake.Read(exINI, GameStrings::AudioVisual, "DriveLocomotorMakesWake");
	this->HoverLocomotorMakesWake.Read(exINI, GameStrings::AudioVisual, "HoverLocomotionClassMakesWake");
	this->ShipLocomotorMakesWake.Read(exINI, GameStrings::AudioVisual, "ShipLocomotionClassMakesWake");

	this->FiringAnim_Update.Read(exINI, GameStrings::AudioVisual, "FiringAnim.Update");
	this->ExtendedPlayerRepair.Read(exINI, GameStrings::General, "ExtendedPlayerRepair");

	this->Psychedelic_StackingMode.Read(exINI, GameStrings::CombatDamage, "Psychedelic.StackingMode");

	this->Shrapnel_AffectsGround.Read(exINI, GameStrings::CombatDamage, "Shrapnel.AffectsGround");
	this->Shrapnel_AffectsBuildings.Read(exINI, GameStrings::CombatDamage, "Shrapnel.AffectsBuildings");
	this->Shrapnel_UseWeaponTargeting.Read(exINI, GameStrings::CombatDamage, "Shrapnel.UseWeaponTargeting");
	this->Shrapnel_IgnoreHitBuildings.Read(exINI, GameStrings::CombatDamage, "Shrapnel.IgnoreHitBuildings");
	this->Shrapnel_ObeyWarheadTriggerConditions.Read(exINI, GameStrings::CombatDamage, "Shrapnel.ObeyWarheadTriggerConditions");

	this->ReturnWeapon_ApplyFirepowerMult.Read(exINI, GameStrings::CombatDamage, "ReturnWeapon.ApplyFirepowerMult");

	this->Splits_TargetingDistance_Cylindrical.Read(exINI, GameStrings::CombatDamage, "Splits.TargetingDistance.Cylindrical");
	this->Splits_AllowRepeatTargets.Read(exINI, GameStrings::CombatDamage, "Splits.AllowRepeatTargets");
	this->Splits_UseWeaponTargeting.Read(exINI, GameStrings::CombatDamage, "Splits.UseWeaponTargeting");
	this->Airburst_UseCluster.Read(exINI, GameStrings::CombatDamage, "Airburst.UseCluster");
	this->Airburst_TargetAsSource_SkipHeight.Read(exINI, GameStrings::CombatDamage, "Airburst.TargetAsSource.SkipHeight");
	this->AirburstWeapon_ApplyFirepowerMult.Read(exINI, GameStrings::CombatDamage, "AirburstWeapon.ApplyFirepowerMult");
	this->AirburstWeapon_UseFiringEffects.Read(exINI, GameStrings::CombatDamage, "AirburstWeapon.UseFiringEffects");
	this->AirburstWeapon_HeadToTarget.Read(exINI, GameStrings::CombatDamage, "AirburstWeapon.HeadToTarget");

	this->AnimDamage_DealtByInvoker.Read(exINI, GameStrings::CombatDamage, "AnimDamage.DealtByInvoker");
	this->AnimDamage_ApplyFirepowerMult.Read(exINI, GameStrings::CombatDamage, "AnimDamage.ApplyFirepowerMult");

	this->Crit_ApplyChancePerTarget.Read(exINI, GameStrings::CombatDamage, "Crit.ApplyChancePerTarget");
	this->Crit_ExtraDamage_ApplyFirepowerMult.Read(exINI, GameStrings::CombatDamage, "Crit.ExtraDamage.ApplyFirepowerMult");
	this->Crit_AnimOnAffectedTargets.Read(exINI, GameStrings::CombatDamage, "Crit.AnimOnAffectedTargets");
	this->Crit_SuppressWhenIntercepted.Read(exINI, GameStrings::CombatDamage, "Crit.SuppressWhenIntercepted");
	this->ReturnWarhead_ApplyChancePerTarget.Read(exINI, GameStrings::CombatDamage, "ReturnWarhead.ApplyChancePerTarget");

	this->BuildingGuardRetryDelay.Read(exINI, GameStrings::General, "BuildingGuardRetryDelay");

	this->Vertical_AircraftFix.Read(exINI, GameStrings::General, "Vertical.AircraftFix");

	this->Temporal_ApplyVersus.Read(exINI, GameStrings::CombatDamage, "Temporal.ApplyVersus");
	this->Temporal_ApplyMultiplier.Read(exINI, GameStrings::CombatDamage, "Temporal.ApplyMultiplier");

	ValueableIdx<VocClass> deploySound { pThis->DeploySound };
	deploySound.Read(exINI, GameStrings::AudioVisual, "DeploySound");
	pThis->DeploySound = deploySound;

	this->DiscardOn_Sequences_Immediate.Read(exINI, GameStrings::General, "DiscardOn.Sequences.Immediate");
	this->DiscardOn_MoveBasedOnDestination.Read(exINI, GameStrings::General, "DiscardOn.MoveBasedOnDestination");
	this->DiscardOn_ConsiderHarvestingAsStationary.Read(exINI, GameStrings::General, "DiscardOn.ConsiderHarvestingAsStationary");

	this->RemoveMindControl_Silent.Read(exINI, GameStrings::AudioVisual, "RemoveMindControl.Silent");
	this->MindControl_Permanent_ReplaceSilent.Read(exINI, GameStrings::AudioVisual, "MindControl.Permanent.ReplaceSilent");

	this->FlyNoWobbles.Read(exINI, GameStrings::AudioVisual, "FlyNoWobbles");

	this->DefaultLandingAnim.Read(exINI, GameStrings::AudioVisual, "DefaultLandingAnim");
	this->DefaultLandingAnim_Dropship.Read(exINI, GameStrings::AudioVisual, "DefaultLandingAnim.Dropship");
	this->DefaultLandingAnim_Carryall.Read(exINI, GameStrings::AudioVisual, "DefaultLandingAnim.Carryall");

	this->TeamDelays_DynamicType.Read(exINI, GameStrings::General, "TeamDelays.DynamicType");

	char tempBuffer[40];
	for (size_t i = 0; i < 8; i++)
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "TeamDelays.Count%d", i + 1);
		this->TeamDelays_Count[i].Read(exINI, GameStrings::General, tempBuffer);
	}

	this->BerzerkMission.Read(exINI, GameStrings::CombatDamage, "BerzerkMission");

	this->BunkerStateUpdateDelay.Read(exINI, GameStrings::General, "BunkerStateUpdateDelay");

	this->AllowChatBoxInSinglePlayer.Read(exINI, GameStrings::General, "AllowChatBoxInSinglePlayer");

	this->NotHuman_RandomDeathSequence.Read(exINI, GameStrings::General, "NotHuman.RandomDeathSequence");
	this->OnlyUseLandSequences.Read(exINI, GameStrings::General, "OnlyUseLandSequences");
	this->SecondaryFireSequenceLandOnly.Read(exINI, GameStrings::General, "SecondaryFireSequenceLandOnly");
	this->AutoRemoveEarliestBeacon.Read(exINI, GameStrings::General, "AutoRemoveEarliestBeacon");
	this->AllowBeaconHotKeyInSinglePlayer.Read(exINI, GameStrings::General, "AllowBeaconHotKeyInSinglePlayer");
	this->StartFacing.Read(exINI, GameStrings::General, "BuildingStartFacing");
	this->StartFacing_Random.Read(exINI, GameStrings::General, "BuildingStartFacing.Random");

	this->AutoDeath_AllowLimboed.Read(exINI, GameStrings::CombatDamage, "AutoDeath.AllowLimboed");
	this->AutoDeath_OnOwnerChange_IgnoreRevertOnExit.Read(exINI, GameStrings::CombatDamage, "AutoDeath.OnOwnerChange.IgnoreRevertOnExit");
	this->AutoDeath_TechnosDontExist_AllowLimboed.Read(exINI, GameStrings::CombatDamage, "AutoDeath.TechnosDontExist.AllowLimboed");
	this->AutoDeath_TechnosExist_AllowLimboed.Read(exINI, GameStrings::CombatDamage, "AutoDeath.TechnosExist.AllowLimboed");

	this->AircraftDockingDir_DefaultToPoseDir.Read(exINI, GameStrings::AudioVisual, "AircraftDockingDir.DefaultToPoseDir");
	this->PoseDir_Production.Read(exINI, GameStrings::AudioVisual, "PoseDir.Production");
	this->PoseDir_Field.Read(exINI, GameStrings::AudioVisual, "PoseDir.Field");

	if (exINI.ReadString(GameStrings::General, "AttackMove.StopWhenTargetAcquired") > 0)
	{
		Debug::Log("[Developer warning][%s] AttackMove.StopWhenTargetAcquired is deprecated and has been replaced by ApproachTarget.StopWhenInRange! If both are set, the latter will be used.\n", GameStrings::General);
	}
	this->ApproachTarget_StopWhenInRange.Read(exINI, GameStrings::General, "AttackMove.StopWhenTargetAcquired");
	this->ApproachTarget_StopWhenInRange.Read(exINI, GameStrings::General, "ApproachTarget.StopWhenInRange");

	this->NoAlphaImageOnBuildup.Read(exINI, GameStrings::AudioVisual, "NoAlphaImageOnBuildup");
	this->ReadyToNextMission_MovingCheck.Read(exINI, GameStrings::General, "ReadyToNextMission.MovingCheck");

	this->Warhead_PreventScatter.Read(exINI, GameStrings::CombatDamage, "Warhead.PreventScatter");

	this->ProjectileRange_ApplyModifiers.Read(exINI, GameStrings::CombatDamage, "ProjectileRange.ApplyModifiers");

	this->KeepAlive_Infantry.Read(exINI, GameStrings::General, "KeepAlive.Infantry");
	this->KeepAlive_Units.Read(exINI, GameStrings::General, "KeepAlive.Units");
	this->KeepAlive_Aircraft.Read(exINI, GameStrings::General, "KeepAlive.Aircraft");
	this->KeepAlive_Buildings.Read(exINI, GameStrings::General, "KeepAlive.Buildings");
	this->KeepAlive_Defenses.Read(exINI, GameStrings::General, "KeepAlive.Defenses");

	this->AutoTarget_InsignificantWhenMindControlled.Read(exINI, GameStrings::CombatDamage, "AutoTarget.InsignificantWhenMindControlled");

	// Section AITargetTypes
	int itemsCount = pINI->GetKeyCount("AITargetTypes");
	for (int i = 0; i < itemsCount; ++i)
	{
		std::vector<TechnoTypeClass*> objectsList;
		char* context = nullptr;
		pINI->ReadString("AITargetTypes", pINI->GetKeyName("AITargetTypes", i), "", Phobos::readBuffer);

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			TechnoTypeClass* buffer;
			if (Parser<TechnoTypeClass*>::TryParse(cur, &buffer))
				objectsList.emplace_back(buffer);
			else
				Debug::Log("[Developer warning] AITargetTypes (Count: %d): Error parsing [%s]\n", this->AITargetTypesLists.size(), cur);
		}

		this->AITargetTypesLists.emplace_back(std::move(objectsList));
	}

	// Section AIScriptsList
	int scriptitemsCount = pINI->GetKeyCount("AIScriptsList");
	for (int i = 0; i < scriptitemsCount; ++i)
	{
		std::vector<ScriptTypeClass*> objectsList;

		char* context = nullptr;
		pINI->ReadString("AIScriptsList", pINI->GetKeyName("AIScriptsList", i), "", Phobos::readBuffer);

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			ScriptTypeClass* pNewScript = ScriptTypeClass::FindOrAllocate(cur);
			objectsList.emplace_back(pNewScript);
		}

		this->AIScriptsLists.emplace_back(std::move(objectsList));
	}
}

// this should load everything that TypeData is not dependant on
// i.e. InfantryElectrocuted= can go here since nothing refers to it
// but [GenericPrerequisites] have to go earlier because they're used in parsing TypeData
void RulesExt::ExtData::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	INI_EX exINI(pINI);

}

// this runs between the before and after type data loading methods for rules ini
void RulesExt::ExtData::InitializeAfterTypeData(RulesClass* const pThis)
{

}

void RulesExt::ExtData::InitializeAfterAllLoaded()
{
	const auto pRules = RulesClass::Instance;

	// tint color
	this->TintColorIronCurtain = GeneralUtils::GetColorFromColorAdd(pRules->IronCurtainColor);
	this->TintColorForceShield = GeneralUtils::GetColorFromColorAdd(pRules->ForceShieldColor);
	this->TintColorBerserk = GeneralUtils::GetColorFromColorAdd(pRules->BerserkColor);
}

// =============================
// load / save

template <typename T>
void RulesExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->AITargetTypesLists)
		.Process(this->AIScriptsLists)
		.Process(this->Storage_TiberiumIndex)
		.Process(this->HarvesterDumpAmount)
		.Process(this->InfantryGainSelfHealCap)
		.Process(this->UnitsGainSelfHealCap)
		.Process(this->GainSelfHealAllowMultiplayPassive)
		.Process(this->GainSelfHealFromPlayerControl)
		.Process(this->GainSelfHealFromAllies)
		.Process(this->EnemyInsignia)
		.Process(this->DisguiseBlinkingVisibility)
		.Process(this->ChronoSparkleDisplayDelay)
		.Process(this->ChronoSparkleBuildingDisplayPositions)
		.Process(this->ChronoSpherePreDelay)
		.Process(this->ChronoSphereDelay)
		.Process(this->AIChronoSphereSW)
		.Process(this->AIChronoWarpSW)
		.Process(this->SubterraneanSpeed)
		.Process(this->SubterraneanHeight)
		.Process(this->AISuperWeaponDelay)
		.Process(this->UseGlobalRadApplicationDelay)
		.Process(this->RadApplicationDelay_Building)
		.Process(this->RadBuildingDamageMaxCount)
		.Process(this->RadSiteWarhead_Detonate)
		.Process(this->RadSiteWarhead_Detonate_Full)
		.Process(this->RadHasOwner)
		.Process(this->RadHasInvoker)
		.Process(this->ShieldApplyArmorMult)
		.Process(this->JumpjetCrash)
		.Process(this->JumpjetNoWobbles)
		.Process(this->JumpjetRotateOnCrash)
		.Process(this->VeinholeWarhead)
		.Process(this->MissingCameo)
		.Process(this->PlacementGrid_Translucency)
		.Process(this->PlacementGrid_TranslucencyWithPreview)
		.Process(this->PlacementPreview)
		.Process(this->PlacementPreview_Translucency)
		.Process(this->SuperWeaponTimer_Percentage)
		.Process(this->SuperWeaponSidebar_AllowByDefault)
		.Process(this->ConditionYellow_Terrain)
		.Process(this->Shield_ConditionYellow)
		.Process(this->Shield_ConditionRed)
		.Process(this->Pips_Shield)
		.Process(this->Pips_Shield_Background)
		.Process(this->Pips_Shield_Building)
		.Process(this->Pips_Shield_Building_Empty)
		.Process(this->Pips_SelfHeal_Infantry)
		.Process(this->Pips_SelfHeal_Units)
		.Process(this->Pips_SelfHeal_Buildings)
		.Process(this->Pips_SelfHeal_Infantry_Offset)
		.Process(this->Pips_SelfHeal_Units_Offset)
		.Process(this->Pips_SelfHeal_Buildings_Offset)
		.Process(this->Pips_Generic_Size)
		.Process(this->Pips_Generic_Buildings_Size)
		.Process(this->Pips_Ammo_Size)
		.Process(this->Pips_Ammo_Buildings_Size)
		.Process(this->Pips_Tiberiums_Frames)
		.Process(this->Pips_Tiberiums_EmptyFrame)
		.Process(this->Pips_Tiberiums_DisplayOrder)
		.Process(this->Pips_Tiberiums_WeedFrame)
		.Process(this->Pips_Tiberiums_WeedEmptyFrame)
		.Process(this->AirShadowBaseScale_log)
		.Process(this->HeightShadowScaling)
		.Process(this->HeightShadowScaling_MinScale)
		.Process(this->ExtendedAircraftMissions)
		.Process(this->ExtendedAircraftMissions_UnlandDamage)
		.Process(this->AircraftSpawnFromEdge)
		.Process(this->AircraftRetreatToEdge)
		.Process(this->AmphibiousEnter)
		.Process(this->AmphibiousUnload)
		.Process(this->NoQueueUpToEnter)
		.Process(this->NoQueueUpToEnter_BoardDistance)
		.Process(this->NoQueueUpToUnload)
		.Process(this->NoQueueUpToEnter_Buildings)
		.Process(this->NoQueueUpToUnload_Buildings)
		.Process(this->JumpjetTilt)
		.Process(this->JumpjetTilt_ForwardAccelFactor)
		.Process(this->JumpjetTilt_ForwardSpeedFactor)
		.Process(this->JumpjetTilt_SidewaysRotationFactor)
		.Process(this->JumpjetTilt_SidewaysSpeedFactor)
		.Process(this->Spawner_AttackImmediately)
		.Process(this->Spawner_UseTurretFacing)
		.Process(this->Spawner_RecycleRange)
		.Process(this->Spawner_RecycleOnTurret)
		.Process(this->Promote_IncludeSpawns)
		.Process(this->RadarJamHouses)
		.Process(this->RadarJamDelay)
		.Process(this->MindControl_IgnoreSize)
		.Process(this->MultiMindControl_ReleaseVictim)
		.Process(this->MindControlLink_VisibleToHouse)
		.Process(this->AlternateFLH_OnTurret)
		.Process(this->AlternateFLH_ApplyVehicle)
		.Process(this->DestroyAnim_Random)
		.Process(this->UseDisguiseMovementSpeed)
		.Process(this->Convert_ResetMindControl)
		.Process(this->BuildLimitGroup_ContentIfAnyMatch)
		.Process(this->BuildLimitGroup_NotBuildableIfQueueMatch)
		.Process(this->ForceWeapon_InRange_TechnoOnly)
		.Process(this->ForceWeapon_InRange_ApplyRangeModifiers)
		.Process(this->ForceAAWeapon_InRange_ApplyRangeModifiers)
		.Process(this->DigitalDisplay_Health_FakeAtDisguise)
		.Process(this->Overload_ParticleSysCount)
		.Process(this->FallingDownDamage)
		.Process(this->FallingDownDamage_AllowEMP)
		.Process(this->BuildingProductionQueue)
		.Process(this->AllowParallelAIQueues)
		.Process(this->ForbidParallelAIQueues_Aircraft)
		.Process(this->ForbidParallelAIQueues_Building)
		.Process(this->ForbidParallelAIQueues_Infantry)
		.Process(this->ForbidParallelAIQueues_Navy)
		.Process(this->ForbidParallelAIQueues_Vehicle)
		.Process(this->EnablePowerSurplus)
		.Process(this->PowerSurplus_ScaleToDrainAmount)
		.Process(this->AllowDeployControlledMCV)
		.Process(this->TypeSelectUseIFVMode)
		.Process(this->IronCurtain_KeptOnDeploy)
		.Process(this->IronCurtain_EffectOnOrganics)
		.Process(this->IronCurtain_KillOrganicsWarhead)
		.Process(this->ForceShield_KeptOnDeploy)
		.Process(this->ForceShield_EffectOnOrganics)
		.Process(this->ForceShield_KillOrganicsWarhead)
		.Process(this->IronCurtain_ExtraTintIntensity)
		.Process(this->ForceShield_ExtraTintIntensity)
		.Process(this->AllowWeaponSelectAgainstWalls)
		.Process(this->ColorAddUse8BitRGB)
		.Process(this->AirstrikeLineColor)
		.Process(this->AirstrikeLineZAdjust)
		.Process(this->Strafing_SimulateBurst)
		.Process(this->Strafing_UseAmmoPerShot)
		.Process(this->Strafing_TargetCell)
		.Process(this->OmniFire_TurnToTarget)
		.Process(this->AmbientDamage_IgnoreTarget)
		.Process(this->KeepRange_AllowAI)
		.Process(this->KeepRange_AllowPlayer)
		.Process(this->KeepRange_EarlyStopFrame)
		.Process(this->AircraftWeapon_KickOutPassengers)
		.Process(this->CrushSlowdownMultiplier)
		.Process(this->SkipCrushSlowdown)
		.Process(this->LaserPositionUpdate_StopOnFirerConvert)
		.Process(this->LaserZAdjust)
		.Process(this->EBoltZAdjust)
		.Process(this->EBoltZAdjust_ClampInitialDepthForBuilding)
		.Process(this->ROF_RandomDelay)
		.Process(this->ToolTip_Background_Color)
		.Process(this->ToolTip_Background_Opacity)
		.Process(this->ToolTip_Background_BlurSize)
		.Process(this->DisplayIncome)
		.Process(this->DisplayIncome_Delay)
		.Process(this->DisplayIncome_AllowAI)
		.Process(this->DisplayIncome_Houses)
		.Process(this->DrainMoneyDisplay)
		.Process(this->DrainMoneyDisplay_Houses)
		.Process(this->DrainMoneyDisplay_OnTarget)
		.Process(this->DrainMoneyDisplay_OnTarget_UseDisplayIncome)
		.Process(this->CrateOnlyOnLand)
		.Process(this->UnitCrateVehicleCap)
		.Process(this->FreeMCV_CreditsThreshold)
		.Process(this->RadialIndicatorVisibility)
		.Process(this->DrawTurretShadow)
		.Process(this->IsVoiceCreatedGlobal)
		.Process(this->SetTabBySelectingFactory)
		.Process(this->SelectionFlashDuration)
		.Process(this->SetRecruitableOnLiberate)
		.Process(this->DrawInsignia_OnlyOnSelected)
		.Process(this->DrawInsignia_AdjustPos_Infantry)
		.Process(this->DrawInsignia_AdjustPos_Buildings)
		.Process(this->DrawInsignia_AdjustPos_BuildingsAnchor)
		.Process(this->DrawInsignia_AdjustPos_Units)
		.Process(this->DrawInsignia_UsePixelSelectionBracketDelta)
		.Process(this->Promote_VeteranAnimation)
		.Process(this->Promote_EliteAnimation)
		.Process(this->AnimRemapDefaultColorScheme)
		.Process(this->TimerBlinkColorScheme)
		.Process(this->Buildings_DefaultDigitalDisplayTypes)
		.Process(this->Infantry_DefaultDigitalDisplayTypes)
		.Process(this->Vehicles_DefaultDigitalDisplayTypes)
		.Process(this->Aircraft_DefaultDigitalDisplayTypes)
		.Process(this->DefaultInfantrySelectBox)
		.Process(this->DefaultUnitSelectBox)
		.Process(this->VisualScatter_Min)
		.Process(this->VisualScatter_Max)
		.Process(this->ShowDesignatorRange)
		.Process(this->ShowPowerPlantEnhancerRange)
		.Process(this->ShowGameTime)
		.Process(this->DropPodTrailer)
		.Process(this->DropPodDefaultTrailer)
		.Process(this->PodImage)
		.Process(this->JumpjetClimbPredictHeight)
		.Process(this->JumpjetClimbWithoutCutOut)
		.Process(this->JumpjetClimbIgnoreBuilding)
		.Process(this->DamageOwnerMultiplier)
		.Process(this->DamageAlliesMultiplier)
		.Process(this->DamageEnemiesMultiplier)
		.Process(this->DamageOwnerMultiplier_NotAffectsEnemies)
		.Process(this->DamageAlliesMultiplier_NotAffectsEnemies)
		.Process(this->DamageOwnerMultiplier_Berzerk)
		.Process(this->DamageAlliesMultiplier_Berzerk)
		.Process(this->DamageEnemiesMultiplier_Berzerk)
		.Process(this->AircraftLevelLightMultiplier)
		.Process(this->JumpjetLevelLightMultiplier)
		.Process(this->VoxelLightSource)
		// .Process(this->VoxelShadowLightSource)
		.Process(this->BuildingWaypoints)
		.Process(this->CombatAlert)
		.Process(this->CombatAlert_Default)
		.Process(this->CombatAlert_IgnoreBuilding)
		.Process(this->CombatAlert_SuppressIfInScreen)
		.Process(this->CombatAlert_Interval)
		.Process(this->CombatAlert_SuppressIfAllyDamage)
		.Process(this->CombatAlert_MakeAVoice)
		.Process(this->CombatAlert_UseFeedbackVoice)
		.Process(this->CombatAlert_UseAttackVoice)
		.Process(this->CombatAlert_UseEVA)
		.Process(this->UseFixedVoxelLighting)
		.Process(this->AIAutoDeployMCV)
		.Process(this->AISetBaseCenter)
		.Process(this->AIBiasSpawnCell)
		.Process(this->AIForbidConYard)
		.Process(this->AINodeWallsOnly)
		.Process(this->AICleanWallNode)
		.Process(this->AttackMove_Aggressive)
		.Process(this->AttackMove_UpdateTarget)
		.Process(this->MindControl_ThreatDelay)
		.Process(this->RecountBurst)
		.Process(this->NoRearm_UnderEMP)
		.Process(this->NoRearm_Temporal)
		.Process(this->NoReload_UnderEMP)
		.Process(this->NoReload_Temporal)
		.Process(this->VeteranReload)
		.Process(this->VeteranEmptyReload)
		.Process(this->NoTurret_TrackTarget)
		.Process(this->GatherWhenMCVDeploy)
		.Process(this->AIFireSale)
		.Process(this->AIFireSaleDelay)
		.Process(this->AIAllToHunt)
		.Process(this->RepairBaseNodes)
		.Process(this->FixRepairStepCost)
		.Process(this->WarheadParticleAlphaImageIsLightFlash)
		.Process(this->CombatLightDetailLevel)
		.Process(this->CombatLightDetailLevel_CheckColored)
		.Process(this->LightFlashAlphaImageDetailLevel)
		.Process(this->UseRetintFix)
		.Process(this->AINormalTargetingDelay)
		.Process(this->PlayerNormalTargetingDelay)
		.Process(this->AIGuardAreaTargetingDelay)
		.Process(this->PlayerGuardAreaTargetingDelay)
		.Process(this->AIAttackMoveTargetingDelay)
		.Process(this->PlayerAttackMoveTargetingDelay)
		.Process(this->DistributeTargetingFrame)
		.Process(this->DistributeTargetingFrame_AIOnly)
		.Process(this->CanTargetAI_IronCurtained)
		.Process(this->CanTarget_IronCurtained)
		.Process(this->AutoTarget_IronCurtained)
		.Process(this->BuildingTypeSelectable)
		.Process(this->ProneSpeed_Crawls)
		.Process(this->ProneSpeed_NoCrawls)
		.Process(this->DamagedSpeed)
		.Process(this->HarvesterScanAfterUnload)
		.Process(this->AnimCraterDestroyTiberium)
		.Process(this->BerzerkTargeting)
		.Process(this->AllowBerzerkOnAllies)
		.Process(this->TintColorIronCurtain)
		.Process(this->TintColorForceShield)
		.Process(this->TintColorBerserk)
		.Process(this->AttackMove_IgnoreWeaponCheck)
		.Process(this->Parasite_GrappleAnim)
		.Process(this->Parasite_AllowWaterExit)
		.Process(this->InfantryAutoDeploy)
		.Process(this->AdjacentWallDamage)
		.Process(this->WarheadAnimZAdjust)
		.Process(this->IvanBombAttachToCenter)
		.Process(this->IvanBomb_Visibility)
		.Process(this->MissileSpawnAttackCell)
		.Process(this->FallingDownTargetingFix)
		.Process(this->AIAirTargetingFix)
		.Process(this->ReloadInTransport)
		.Process(this->OpenTopped_IgnoreRangefinding)
		.Process(this->OpenTopped_AllowFiringIfDeactivated)
		.Process(this->OpenTopped_AllowFiringIfAttackedByLocomotor)
		.Process(this->OpenTopped_ShareTransportTarget)
		.Process(this->OpenTopped_UseTransportRangeModifiers)
		.Process(this->OpenTopped_CheckTransportDisableWeapons)
		.Process(this->OpenTopped_DecloakToFire)
		.Process(this->OpenTopped_FireWhileMoving)
		.Process(this->OpenTransport_RangeBonus)
		.Process(this->OpenTransport_DamageMultiplier)
		.Process(this->OpenTransport_FireWhileMoving)
		.Process(this->Passengers_SyncOwner)
		.Process(this->Passengers_SyncOwner_RevertOnExit)
		.Process(this->Explodes_KillPassengers)
		.Process(this->Explodes_DuringBuildup)
		.Process(this->AircraftFiringForceScatter)
		.Process(this->HoverDrownable)
		.Process(this->Arcing_AllowElevationInaccuracy)
		.Process(this->Terrain_IsPassable)
		.Process(this->Tibtree_IsPassable)
		.Process(this->Terrain_CanBeBuiltOn)
		.Process(this->Tibtree_CanBeBuiltOn)
		.Process(this->Sinkable)
		.Process(this->Sinkable_SquidGrab)
		.Process(this->SinkSpeed)
		.Process(this->CreateAnimsOnZeroDamage)
		.Process(this->Conventional_IgnoreUnits)
		.Process(this->DecloakDamagedTargets)
		.Process(this->ShakeIsLocal)
		.Process(this->ApplyModifiersOnNegativeDamage)
		.Process(this->AllowDamageOnSelf)
		.Process(this->Debris_Conventional)
		.Process(this->Parasite_DisableParticleSystem)
		.Process(this->ProjectileInterceptable)
		.Process(this->Interceptor_GuardRange_IsCylindrical)
		.Process(this->Interceptor_ApplyFirepowerMult)
		.Process(this->SortCameoByName)
		.Process(this->MergeBuildingDamage)
		.Process(this->BuildingRadioLink_SyncOwner)
		.Process(this->ApplyPerTargetEffectsOnDetonate)
		.Process(this->AffectsInvokerOnly_IgnoreInvokerState)
		.Process(this->ExtraRange_TargetMoving)
		.Process(this->ExtraRange_TargetMoving_CloseRangeOnly)
		.Process(this->ExtraRange_FirerMoving)
		.Process(this->ExtraRange_Prefiring)
		.Process(this->ExtraRange_Prefiring_IncludeBurst)
		.Process(this->AutoTarget_NoThreatBuildings)
		.Process(this->AutoTargetAI_NoThreatBuildings)
		.Process(this->ParadropMission)
		.Process(this->AIParadropMission)
		.Process(this->ParadropDelay)
		.Process(this->ParadropEndDelay)
		.Process(this->DefaultToGuardArea)
		.Process(this->LeptonMindControlOffset)
		.Process(this->MindControlRingOffset)
		.Process(this->CylinderRangefinding)
		.Process(this->PenetratesTransport_Level)
		.Process(this->UnitsUnsellable)
		.Process(this->DriverKilled_KeptPassengers)
		.Process(this->DriverKilled_KillPassengers)
		.Process(this->DisableOveroptimizationInTargeting)
		.Process(this->ExtraThreat_IsThreat)
		.Process(this->ExtraThreat_InRange)
		.Process(this->ExtraThreatCoefficient_InRangeDistance)
		.Process(this->ExtraThreatCoefficient_Facing)
		.Process(this->ExtraThreatCoefficient_DistanceToLastTarget)
		.Process(this->BalloonHoverPathingFix)
		.Process(this->WalkLocomotorMakesWake)
		.Process(this->DriveLocomotorMakesWake)
		.Process(this->HoverLocomotorMakesWake)
		.Process(this->ShipLocomotorMakesWake)
		.Process(this->FiringAnim_Update)
		.Process(this->ExtendedPlayerRepair)
		.Process(this->Psychedelic_StackingMode)
		.Process(this->Shrapnel_AffectsGround)
		.Process(this->Shrapnel_AffectsBuildings)
		.Process(this->Shrapnel_UseWeaponTargeting)
		.Process(this->Shrapnel_IgnoreHitBuildings)
		.Process(this->Shrapnel_ObeyWarheadTriggerConditions)
		.Process(this->ReturnWeapon_ApplyFirepowerMult)
		.Process(this->Splits_TargetingDistance_Cylindrical)
		.Process(this->Splits_AllowRepeatTargets)
		.Process(this->Splits_UseWeaponTargeting)
		.Process(this->Airburst_UseCluster)
		.Process(this->Airburst_TargetAsSource_SkipHeight)
		.Process(this->AirburstWeapon_ApplyFirepowerMult)
		.Process(this->AirburstWeapon_UseFiringEffects)
		.Process(this->AirburstWeapon_HeadToTarget)
		.Process(this->AnimDamage_DealtByInvoker)
		.Process(this->AnimDamage_ApplyFirepowerMult)
		.Process(this->Crit_ApplyChancePerTarget)
		.Process(this->Crit_ExtraDamage_ApplyFirepowerMult)
		.Process(this->Crit_AnimOnAffectedTargets)
		.Process(this->Crit_SuppressWhenIntercepted)
		.Process(this->ReturnWarhead_ApplyChancePerTarget)
		.Process(this->BuildingGuardRetryDelay)
		.Process(this->Vertical_AircraftFix)
		.Process(this->Temporal_ApplyVersus)
		.Process(this->Temporal_ApplyMultiplier)
		.Process(this->DiscardOn_Sequences_Immediate)
		.Process(this->DiscardOn_MoveBasedOnDestination)
		.Process(this->DiscardOn_ConsiderHarvestingAsStationary)
		.Process(this->RemoveMindControl_Silent)
		.Process(this->MindControl_Permanent_ReplaceSilent)
		.Process(this->FlyNoWobbles)
		.Process(this->DefaultLandingAnim)
		.Process(this->DefaultLandingAnim_Dropship)
		.Process(this->DefaultLandingAnim_Carryall)
		.Process(this->TeamDelays_DynamicType)
		.Process(this->TeamDelays_Count)
		.Process(this->BerzerkMission)
		.Process(this->BunkerStateUpdateDelay)
		.Process(this->AllowChatBoxInSinglePlayer)
		.Process(this->NotHuman_RandomDeathSequence)
		.Process(this->OnlyUseLandSequences)
		.Process(this->SecondaryFireSequenceLandOnly)
		.Process(this->AutoRemoveEarliestBeacon)
		.Process(this->AllowBeaconHotKeyInSinglePlayer)
		.Process(this->StartFacing)
		.Process(this->StartFacing_Random)
		.Process(this->AutoDeath_AllowLimboed)
		.Process(this->AutoDeath_OnOwnerChange_IgnoreRevertOnExit)
		.Process(this->AutoDeath_TechnosDontExist_AllowLimboed)
		.Process(this->AutoDeath_TechnosExist_AllowLimboed)
		.Process(this->AircraftDockingDir_DefaultToPoseDir)
		.Process(this->PoseDir_Production)
		.Process(this->PoseDir_Field)
		.Process(this->ApproachTarget_StopWhenInRange)
		.Process(this->NoAlphaImageOnBuildup)
		.Process(this->ReadyToNextMission_MovingCheck)
		.Process(this->Warhead_PreventScatter)
		.Process(this->ProjectileRange_ApplyModifiers)
		.Process(this->KeepAlive_Infantry)
		.Process(this->KeepAlive_Units)
		.Process(this->KeepAlive_Aircraft)
		.Process(this->KeepAlive_Buildings)
		.Process(this->KeepAlive_Defenses)
		.Process(this->AutoTarget_InsignificantWhenMindControlled)
		;
}

void RulesExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<RulesClass>::LoadFromStream(Stm);
	this->Serialize(Stm);

	this->ReplaceVoxelLightSources();
	Phobos::Optimizations::DisableBalloonHoverPathingFix = !this->BalloonHoverPathingFix;
}

void RulesExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<RulesClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

void RulesExt::ExtData::ReplaceVoxelLightSources()
{
	bool needCacheFlush = false;

	if (this->VoxelLightSource.isset())
	{
		needCacheFlush = true;
		auto source = this->VoxelLightSource.Get().Normalized();
		Game::VoxelLightSource = Matrix3D::VoxelDefaultMatrix * source;
	}

	/*
	// doesn't really impact anything from my testing - Kerbiter
	if (this->VoxelShadowLightSource.isset())
	{
		needCacheFlush = true;
		auto source = this->VoxelShadowLightSource.Get().Normalized();
		Game::VoxelShadowLightSource = Matrix3D::VoxelDefaultMatrix * source;
	}
	*/

	if (needCacheFlush)
		Game::DestroyVoxelCaches();
}

// =============================
// container hooks

DEFINE_HOOK(0x667A1D, RulesClass_CTOR, 0x5)
{
	GET(RulesClass*, pItem, ESI);

	RulesExt::Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x667A30, RulesClass_DTOR, 0x5)
{
	GET(RulesClass*, pItem, ECX);

	RulesExt::Remove(pItem);

	return 0;
}

IStream* RulesExt::g_pStm = nullptr;

DEFINE_HOOK_AGAIN(0x674730, RulesClass_SaveLoad_Prefix, 0x6)
DEFINE_HOOK(0x675210, RulesClass_SaveLoad_Prefix, 0x5)
{
	//GET(RulesClass*, pItem, ECX);
	GET_STACK(IStream*, pStm, 0x4);

	RulesExt::g_pStm = pStm;

	return 0;
}

DEFINE_HOOK(0x678841, RulesClass_Load_Suffix, 0x7)
{
	auto buffer = RulesExt::Global();

	PhobosByteStream Stm(0);
	if (Stm.ReadBlockFromStream(RulesExt::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(RulesExt::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x675205, RulesClass_Save_Suffix, 0x8)
{
	auto buffer = RulesExt::Global();
	PhobosByteStream saver(sizeof(*buffer));
	PhobosStreamWriter writer(saver);

	writer.Expect(RulesExt::Canary);
	writer.RegisterChange(buffer);

	buffer->SaveToStream(writer);
	saver.WriteBlockToStream(RulesExt::g_pStm);

	return 0;
}

// DEFINE_HOOK(0x52D149, InitRules_PostInit, 0x5)
// {
// 	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art.get());
// 	return 0;
// }

DEFINE_HOOK(0x668BF0, RulesClass_Addition, 0x5)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

	//	RulesClass::Initialized = false;
	RulesExt::LoadFromINIFile(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x679A15, RulesData_LoadBeforeTypeData, 0x6)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

	//	RulesClass::Initialized = true;
	RulesExt::LoadBeforeTypeData(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x679CAF, RulesData_LoadAfterTypeData, 0x5)
{
	RulesClass* pItem = RulesClass::Instance;
	GET(CCINIClass*, pINI, ESI);

	RulesExt::LoadAfterTypeData(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x668F6A, RulesData_InitializeAfterAllLoaded, 0x5)
{
	RulesExt::Global()->InitializeAfterAllLoaded();
	return 0;
}

// Reenable obsolete [JumpjetControls] in RA2/YR
// Author: Uranusian
DEFINE_HOOK(0x7115AE, TechnoTypeClass_CTOR_JumpjetControls, 0xA)
{
	GET(TechnoTypeClass*, pThis, ESI);
	auto pRules = RulesClass::Instance;
	auto pRulesExt = RulesExt::Global();

	pThis->JumpjetTurnRate = pRules->TurnRate;
	pThis->JumpjetSpeed = pRules->Speed;
	pThis->JumpjetClimb = static_cast<float>(pRules->Climb);
	pThis->JumpjetCrash = static_cast<float>(pRulesExt->JumpjetCrash);
	pThis->JumpjetHeight = pRules->CruiseHeight;
	pThis->JumpjetAccel = static_cast<float>(pRules->Acceleration);
	pThis->JumpjetWobbles = static_cast<float>(pRules->WobblesPerSecond);
	pThis->JumpjetNoWobbles = pRulesExt->JumpjetNoWobbles;
	pThis->JumpjetDeviation = pRules->WobbleDeviation;

	return 0x711601;
}

DEFINE_HOOK(0x6744E4, RulesClass_ReadJumpjetControls_Extra, 0x7)
{
	auto pRulesExt = RulesExt::Global();
	if (!pRulesExt)
		return 0;

	GET(CCINIClass*, pINI, EDI);
	INI_EX exINI(pINI);

	pRulesExt->JumpjetCrash.Read(exINI, GameStrings::JumpjetControls, "Crash");
	pRulesExt->JumpjetNoWobbles.Read(exINI, GameStrings::JumpjetControls, "NoWobbles");
	pRulesExt->JumpjetRotateOnCrash.Read(exINI, GameStrings::JumpjetControls, "RotateOnCrash");

	return 0;
}

namespace
{
	template <typename T>
	void ReadSpecialWeaponType(T& value, INI_EX& parser, CCINIClass* pINI, const char* pKey)
	{
		using base_type = std::remove_pointer_t<T>;

		if (!parser.ReadString("SpecialWeapons", pKey))
			return;

		auto pValue = parser.value();
		if (INIClass::IsBlank(pValue))
		{
			value = nullptr;
			return;
		}

		if (auto pType = base_type::Find(pValue))
		{
			value = pType;
			return;
		}

		if (auto pType = GameCreate<base_type>(pValue))
		{
			pType->LoadFromINI(pINI);
			value = pType;
			return;
		}

		Debug::INIParseFailed("SpecialWeapons", pKey, pValue);
	}
}

DEFINE_JUMP(LJMP, 0x66919B, 0x6691B7) // Don't read warhead here!
DEFINE_HOOK(0x668FDB, RulesClass_Read_SpecialWeapons, 0x6)
{
	GET(RulesClass*, pRules, ESI);
	GET(CCINIClass*, pINI, EDI);
	INI_EX exINI(pINI);

	ReadSpecialWeaponType(pRules->NukeWarhead, exINI, pINI, "NukeWarhead");
	ReadSpecialWeaponType(pRules->NukeProjectile, exINI, pINI, "NukeProjectile");
	ReadSpecialWeaponType(pRules->NukeDown, exINI, pINI, "NukeDown");
	ReadSpecialWeaponType(pRules->MutateWarhead, exINI, pINI, "MutateWarhead");
	ReadSpecialWeaponType(pRules->MutateExplosionWarhead, exINI, pINI, "MutateExplosionWarhead");
	ReadSpecialWeaponType(pRules->EMPulseWarhead, exINI, pINI, "EMPulseWarhead");
	ReadSpecialWeaponType(pRules->EMPulseProjectile, exINI, pINI, "EMPulseProjectile");

	return 0x6691B7;
}

// skip vanilla JumpjetControls and make it earlier load
// DEFINE_JUMP(LJMP, 0x668EB5, 0x668EBD); // RulesClass_Process_SkipJumpjetControls // Really necessary? won't hurt to read again
