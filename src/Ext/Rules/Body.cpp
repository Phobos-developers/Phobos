#include "Body.h"
#include <Ext/Side/Body.h>
#include <Utilities/TemplateDef.h>
#include <FPSCounter.h>
#include <GameOptionsClass.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/DigitalDisplayTypeClass.h>
#include <New/Type/AttachEffectTypeClass.h>

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
	DigitalDisplayTypeClass::LoadFromINIList(pINI);
	RadTypeClass::LoadFromINIList(pINI);
	ShieldTypeClass::LoadFromINIList(pINI);
	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art);
	AttachEffectTypeClass::LoadFromINIList(pINI);

	Data->LoadBeforeTypeData(pThis, pINI);
}

void RulesExt::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
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

}

void RulesExt::ExtData::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	RulesExt::ExtData* pData = RulesExt::Global();

	if (!pData)
		return;

	INI_EX exINI(pINI);

	this->Storage_TiberiumIndex.Read(exINI, GameStrings::General, "Storage.TiberiumIndex");
	this->InfantryGainSelfHealCap.Read(exINI, GameStrings::General, "InfantryGainSelfHealCap");
	this->UnitsGainSelfHealCap.Read(exINI, GameStrings::General, "UnitsGainSelfHealCap");
	this->GainSelfHealAllowMultiplayPassive.Read(exINI, GameStrings::General, "GainSelfHealAllowMultiplayPassive");
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
	this->VeinholeWarhead.Read<true>(exINI, GameStrings::CombatDamage, "VeinholeWarhead");
	this->MissingCameo.Read(pINI, GameStrings::AudioVisual, "MissingCameo");

	this->PlacementGrid_Translucency.Read(exINI, GameStrings::AudioVisual, "PlacementGrid.Translucency");
	this->PlacementGrid_TranslucencyWithPreview.Read(exINI, GameStrings::AudioVisual, "PlacementGrid.TranslucencyWithPreview");
	this->PlacementPreview.Read(exINI, GameStrings::AudioVisual, "PlacementPreview");
	this->PlacementPreview_Translucency.Read(exINI, GameStrings::AudioVisual, "PlacementPreview.Translucency");

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
	Nullable<double>AirShadowBaseScale;
	AirShadowBaseScale.Read(exINI, GameStrings::AudioVisual, "AirShadowBaseScale");
	if (AirShadowBaseScale.isset() && AirShadowBaseScale.Get() > 0)
		this->AirShadowBaseScale_log = -std::log(std::min(AirShadowBaseScale.Get(), 1.0));

	this->HeightShadowScaling.Read(exINI, GameStrings::AudioVisual, "HeightShadowScaling");
	if (AirShadowBaseScale.isset() && AirShadowBaseScale.Get() > 0.98 && this->HeightShadowScaling.Get())
		this->HeightShadowScaling = false;
	this->HeightShadowScaling_MinScale.Read(exINI, GameStrings::AudioVisual, "HeightShadowScaling.MinScale");

	this->ExtendedAircraftMissions.Read(exINI, GameStrings::General, "ExtendedAircraftMissions");

	this->AllowParallelAIQueues.Read(exINI, "GlobalControls", "AllowParallelAIQueues");
	this->ForbidParallelAIQueues_Aircraft.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Aircraft");
	this->ForbidParallelAIQueues_Building.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Building");
	this->ForbidParallelAIQueues_Infantry.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Infantry");
	this->ForbidParallelAIQueues_Navy.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Navy");
	this->ForbidParallelAIQueues_Vehicle.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Vehicle");

	this->EnablePowerSurplus.Read(exINI, GameStrings::AI, "EnablePowerSurplus");

	this->IronCurtain_KeptOnDeploy.Read(exINI, GameStrings::CombatDamage, "IronCurtain.KeptOnDeploy");
	this->IronCurtain_EffectOnOrganics.Read(exINI, GameStrings::CombatDamage, "IronCurtain.EffectOnOrganics");
	this->IronCurtain_KillOrganicsWarhead.Read<true>(exINI, GameStrings::CombatDamage, "IronCurtain.KillOrganicsWarhead");
	this->ForceShield_KeptOnDeploy.Read(exINI, GameStrings::CombatDamage, "ForceShield.KeptOnDeploy");
	this->ForceShield_EffectOnOrganics.Read(exINI, GameStrings::CombatDamage, "ForceShield.EffectOnOrganics");
	this->ForceShield_KillOrganicsWarhead.Read<true>(exINI, GameStrings::CombatDamage, "ForceShield.KillOrganicsWarhead");

	this->IronCurtain_ExtraTintIntensity.Read(exINI, GameStrings::AudioVisual, "IronCurtain.ExtraTintIntensity");
	this->ForceShield_ExtraTintIntensity.Read(exINI, GameStrings::AudioVisual, "ForceShield.ExtraTintIntensity");
	this->ColorAddUse8BitRGB.Read(exINI, GameStrings::AudioVisual, "ColorAddUse8BitRGB");

	this->CrateOnlyOnLand.Read(exINI, GameStrings::CrateRules, "CrateOnlyOnLand");
	this->UnitCrateVehicleCap.Read(exINI, GameStrings::CrateRules, "UnitCrateVehicleCap");
	this->FreeMCV_CreditsThreshold.Read(exINI, GameStrings::CrateRules, "FreeMCV.CreditsThreshold");

	this->ROF_RandomDelay.Read(exINI, GameStrings::CombatDamage, "ROF.RandomDelay");

	this->DisplayIncome.Read(exINI, GameStrings::AudioVisual, "DisplayIncome");
	this->DisplayIncome_Houses.Read(exINI, GameStrings::AudioVisual, "DisplayIncome.Houses");
	this->DisplayIncome_AllowAI.Read(exINI, GameStrings::AudioVisual, "DisplayIncome.AllowAI");

	this->IsVoiceCreatedGlobal.Read(exINI, GameStrings::AudioVisual, "IsVoiceCreatedGlobal");
	this->SelectionFlashDuration.Read(exINI, GameStrings::AudioVisual, "SelectionFlashDuration");
	this->DrawInsignia_OnlyOnSelected.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.OnlyOnSelected");
	this->DrawInsignia_AdjustPos_Infantry.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.AdjustPos.Infantry");
	this->DrawInsignia_AdjustPos_Buildings.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.AdjustPos.Buildings");
	this->DrawInsignia_AdjustPos_BuildingsAnchor.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.AdjustPos.BuildingsAnchor");
	this->DrawInsignia_AdjustPos_Units.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.AdjustPos.Units");
	this->Promote_VeteranAnimation.Read(exINI, GameStrings::AudioVisual, "Promote.VeteranAnimation");
	this->Promote_EliteAnimation.Read(exINI, GameStrings::AudioVisual, "Promote.EliteAnimation");

	Nullable<AnimTypeClass*> droppod_trailer {};
	droppod_trailer.Read(exINI, GameStrings::General, "DropPodTrailer");
	this->DropPodTrailer = droppod_trailer.Get(AnimTypeClass::Find("SMOKEY"));// Ares convention
	this->PodImage = FileSystem::LoadSHPFile("POD.SHP");

	this->Buildings_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Buildings.DefaultDigitalDisplayTypes");
	this->Infantry_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Infantry.DefaultDigitalDisplayTypes");
	this->Vehicles_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Vehicles.DefaultDigitalDisplayTypes");
	this->Aircraft_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Aircraft.DefaultDigitalDisplayTypes");

	this->AircraftLevelLightMultiplier.Read(exINI, GameStrings::AudioVisual, "AircraftLevelLightMultiplier");
	this->JumpjetLevelLightMultiplier.Read(exINI, GameStrings::AudioVisual, "JumpjetLevelLightMultiplier");

	this->VoxelLightSource.Read(exINI, GameStrings::AudioVisual, "VoxelLightSource");
	// this->VoxelShadowLightSource.Read(exINI, GameStrings::AudioVisual, "VoxelShadowLightSource");

	this->ReplaceVoxelLightSources();

	this->UseFixedVoxelLighting.Read(exINI, GameStrings::AudioVisual, "UseFixedVoxelLighting");

	this->GatherWhenMCVDeploy.Read(exINI, GameStrings::General, "GatherWhenMCVDeploy");
	this->AIFireSale.Read(exINI, GameStrings::General, "AIFireSale");
	this->AIFireSaleDelay.Read(exINI, GameStrings::General, "AIFireSaleDelay");
	this->AIAllToHunt.Read(exINI, GameStrings::General, "AIAllToHunt");
	this->RepairBaseNodes.Read(exINI, GameStrings::Basic, "RepairBaseNodes");

	this->WarheadParticleAlphaImageIsLightFlash.Read(exINI, GameStrings::AudioVisual, "WarheadParticleAlphaImageIsLightFlash");
	this->CombatLightDetailLevel.Read(exINI, GameStrings::AudioVisual, "CombatLightDetailLevel");
	this->LightFlashAlphaImageDetailLevel.Read(exINI, GameStrings::AudioVisual, "LightFlashAlphaImageDetailLevel");

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

// this runs between the before and after type data loading methods for rules ini
void RulesExt::ExtData::InitializeAfterTypeData(RulesClass* const pThis)
{

}

// this should load everything that TypeData is not dependant on
// i.e. InfantryElectrocuted= can go here since nothing refers to it
// but [GenericPrerequisites] have to go earlier because they're used in parsing TypeData
void RulesExt::ExtData::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	RulesExt::ExtData* pData = RulesExt::Global();

	if (!pData)
		return;

	INI_EX exINI(pINI);
}

// =============================
// load / save

template <typename T>
void RulesExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->AITargetTypesLists)
		.Process(this->AIScriptsLists)
		.Process(this->HarvesterTypes)
		.Process(this->Storage_TiberiumIndex)
		.Process(this->InfantryGainSelfHealCap)
		.Process(this->UnitsGainSelfHealCap)
		.Process(this->GainSelfHealAllowMultiplayPassive)
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
		.Process(this->JumpjetCrash)
		.Process(this->JumpjetNoWobbles)
		.Process(this->VeinholeWarhead)
		.Process(this->MissingCameo)
		.Process(this->PlacementGrid_Translucency)
		.Process(this->PlacementGrid_TranslucencyWithPreview)
		.Process(this->PlacementPreview)
		.Process(this->PlacementPreview_Translucency)
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
		.Process(this->AllowParallelAIQueues)
		.Process(this->ForbidParallelAIQueues_Aircraft)
		.Process(this->ForbidParallelAIQueues_Building)
		.Process(this->ForbidParallelAIQueues_Infantry)
		.Process(this->ForbidParallelAIQueues_Navy)
		.Process(this->ForbidParallelAIQueues_Vehicle)
		.Process(this->EnablePowerSurplus)
		.Process(this->IronCurtain_KeptOnDeploy)
		.Process(this->IronCurtain_EffectOnOrganics)
		.Process(this->IronCurtain_KillOrganicsWarhead)
		.Process(this->ForceShield_KeptOnDeploy)
		.Process(this->ForceShield_EffectOnOrganics)
		.Process(this->ForceShield_KillOrganicsWarhead)
		.Process(this->IronCurtain_ExtraTintIntensity)
		.Process(this->ForceShield_ExtraTintIntensity)
		.Process(this->ColorAddUse8BitRGB)
		.Process(this->ROF_RandomDelay)
		.Process(this->ToolTip_Background_Color)
		.Process(this->ToolTip_Background_Opacity)
		.Process(this->ToolTip_Background_BlurSize)
		.Process(this->DisplayIncome)
		.Process(this->DisplayIncome_AllowAI)
		.Process(this->DisplayIncome_Houses)
		.Process(this->CrateOnlyOnLand)
		.Process(this->UnitCrateVehicleCap)
		.Process(this->FreeMCV_CreditsThreshold)
		.Process(this->RadialIndicatorVisibility)
		.Process(this->DrawTurretShadow)
		.Process(this->IsVoiceCreatedGlobal)
		.Process(this->SelectionFlashDuration)
		.Process(this->DrawInsignia_OnlyOnSelected)
		.Process(this->DrawInsignia_AdjustPos_Infantry)
		.Process(this->DrawInsignia_AdjustPos_Buildings)
		.Process(this->DrawInsignia_AdjustPos_BuildingsAnchor)
		.Process(this->DrawInsignia_AdjustPos_Units)
		.Process(this->Promote_VeteranAnimation)
		.Process(this->Promote_EliteAnimation)
		.Process(this->AnimRemapDefaultColorScheme)
		.Process(this->TimerBlinkColorScheme)
		.Process(this->Buildings_DefaultDigitalDisplayTypes)
		.Process(this->Infantry_DefaultDigitalDisplayTypes)
		.Process(this->Vehicles_DefaultDigitalDisplayTypes)
		.Process(this->Aircraft_DefaultDigitalDisplayTypes)
		.Process(this->ShowDesignatorRange)
		.Process(this->DropPodTrailer)
		.Process(this->PodImage)
		.Process(this->AircraftLevelLightMultiplier)
		.Process(this->JumpjetLevelLightMultiplier)
		.Process(this->VoxelLightSource)
		// .Process(this->VoxelShadowLightSource)
		.Process(this->UseFixedVoxelLighting)
		.Process(this->GatherWhenMCVDeploy)
		.Process(this->AIFireSale)
		.Process(this->AIFireSaleDelay)
		.Process(this->AIAllToHunt)
		.Process(this->RepairBaseNodes)
		.Process(this->WarheadParticleAlphaImageIsLightFlash)
		.Process(this->CombatLightDetailLevel)
		.Process(this->LightFlashAlphaImageDetailLevel)
		;
}

void RulesExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<RulesClass>::LoadFromStream(Stm);
	this->Serialize(Stm);

	this->ReplaceVoxelLightSources();
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
		Game::VoxelLightSource = Matrix3D::VoxelDefaultMatrix() * source;
	}

	/*
	// doesn't really impact anything from my testing - Kerbiter
	if (this->VoxelShadowLightSource.isset())
	{
		needCacheFlush = true;
		auto source = this->VoxelShadowLightSource.Get().Normalized();
		Game::VoxelShadowLightSource = Matrix3D::VoxelDefaultMatrix() * source;
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
	RulesClass* pItem = RulesClass::Instance();
	GET(CCINIClass*, pINI, ESI);

	RulesExt::LoadAfterTypeData(pItem, pINI);

	return 0;
}

// Reenable obsolete [JumpjetControls] in RA2/YR
// Author: Uranusian
DEFINE_HOOK(0x7115AE, TechnoTypeClass_CTOR_JumpjetControls, 0xA)
{
	GET(TechnoTypeClass*, pThis, ESI);
	auto pRules = RulesClass::Instance();
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

	return 0;
}

// skip vanilla JumpjetControls and make it earlier load
// DEFINE_JUMP(LJMP, 0x668EB5, 0x668EBD); // RulesClass_Process_SkipJumpjetControls // Really necessary? won't hurt to read again
