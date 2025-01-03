#pragma once

#include <CCINIClass.h>
#include <RulesClass.h>
#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Utilities/Enum.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>
#include <Utilities/Anchor.h>

class AnimTypeClass;
class MouseCursor;
class SuperWeaponTypeClass;
class TechnoTypeClass;
class VocClass;
class WarheadTypeClass;
class DigitalDisplayTypeClass;

class RulesExt
{
public:
	using base_type = RulesClass;

	static constexpr DWORD Canary = 0x12341234;

	class ExtData final : public Extension<RulesClass>
	{
	public:
		std::vector<std::vector<TechnoTypeClass*>> AITargetTypesLists;
		std::vector<std::vector<ScriptTypeClass*>> AIScriptsLists;
		ValueableVector<TechnoTypeClass*> HarvesterTypes;

		Valueable<int> Storage_TiberiumIndex;
		Nullable<int> InfantryGainSelfHealCap;
		Nullable<int> UnitsGainSelfHealCap;
		Valueable<bool> GainSelfHealAllowMultiplayPassive;
		Valueable<bool> EnemyInsignia;
		Valueable<AffectedHouse> DisguiseBlinkingVisibility;
		Valueable<int> ChronoSparkleDisplayDelay;
		Valueable<ChronoSparkleDisplayPosition> ChronoSparkleBuildingDisplayPositions;
		Valueable<int> ChronoSpherePreDelay;
		Valueable<int> ChronoSphereDelay;
		ValueableIdx<SuperWeaponTypeClass> AIChronoSphereSW;
		ValueableIdx<SuperWeaponTypeClass> AIChronoWarpSW;
		int SubterraneanSpeed;
		Valueable<int> SubterraneanHeight;
		Nullable<int> AISuperWeaponDelay;
		Valueable<bool> UseGlobalRadApplicationDelay;
		Valueable<int> RadApplicationDelay_Building;
		Valueable<int> RadBuildingDamageMaxCount;
		Valueable<bool> RadSiteWarhead_Detonate;
		Valueable<bool> RadSiteWarhead_Detonate_Full;
		Valueable<bool> RadHasOwner;
		Valueable<bool> RadHasInvoker;
		Valueable<double> JumpjetCrash;
		Valueable<bool> JumpjetNoWobbles;

		Nullable<WarheadTypeClass*> VeinholeWarhead;

		PhobosFixedString<32u> MissingCameo;

		TranslucencyLevel PlacementGrid_Translucency;
		Nullable<TranslucencyLevel> PlacementGrid_TranslucencyWithPreview;
		Valueable<bool> PlacementPreview;
		TranslucencyLevel PlacementPreview_Translucency;

		Nullable<double> ConditionYellow_Terrain;
		Nullable<double> Shield_ConditionYellow;
		Nullable<double> Shield_ConditionRed;
		Valueable<Vector3D<int>> Pips_Shield;
		Nullable<SHPStruct*> Pips_Shield_Background;
		Valueable<Vector3D<int>> Pips_Shield_Building;
		Nullable<int> Pips_Shield_Building_Empty;
		Valueable<Point2D> Pips_SelfHeal_Infantry;
		Valueable<Point2D> Pips_SelfHeal_Units;
		Valueable<Point2D> Pips_SelfHeal_Buildings;
		Valueable<Point2D> Pips_SelfHeal_Infantry_Offset;
		Valueable<Point2D> Pips_SelfHeal_Units_Offset;
		Valueable<Point2D> Pips_SelfHeal_Buildings_Offset;
		Valueable<Point2D> Pips_Generic_Size;
		Valueable<Point2D> Pips_Generic_Buildings_Size;
		Valueable<Point2D> Pips_Ammo_Size;
		Valueable<Point2D> Pips_Ammo_Buildings_Size;
		ValueableVector<int> Pips_Tiberiums_Frames;
		Valueable<int> Pips_Tiberiums_EmptyFrame;
		ValueableVector<int> Pips_Tiberiums_DisplayOrder;
		Valueable<int> Pips_Tiberiums_WeedFrame;
		Valueable<int> Pips_Tiberiums_WeedEmptyFrame;

		Valueable<bool> HeightShadowScaling;
		Valueable<double> HeightShadowScaling_MinScale;
		double AirShadowBaseScale_log;

		Valueable<bool> ExtendedAircraftMissions;

		Valueable<bool> AllowParallelAIQueues;
		Valueable<bool> ForbidParallelAIQueues_Aircraft;
		Valueable<bool> ForbidParallelAIQueues_Building;
		Valueable<bool> ForbidParallelAIQueues_Infantry;
		Valueable<bool> ForbidParallelAIQueues_Navy;
		Valueable<bool> ForbidParallelAIQueues_Vehicle;

		Valueable<bool> EnablePowerSurplus;

		Valueable<bool> DisplayIncome;
		Valueable<bool> DisplayIncome_AllowAI;
		Valueable<AffectedHouse> DisplayIncome_Houses;

		Valueable<bool> IronCurtain_KeptOnDeploy;
		Valueable<IronCurtainEffect> IronCurtain_EffectOnOrganics;
		Nullable<WarheadTypeClass*> IronCurtain_KillOrganicsWarhead;
		Valueable<bool> ForceShield_KeptOnDeploy;
		Valueable<IronCurtainEffect> ForceShield_EffectOnOrganics;
		Nullable<WarheadTypeClass*> ForceShield_KillOrganicsWarhead;

		Valueable<double> IronCurtain_ExtraTintIntensity;
		Valueable<double> ForceShield_ExtraTintIntensity;
		Valueable<bool> ColorAddUse8BitRGB;

		Valueable<PartialVector2D<int>> ROF_RandomDelay;
		Valueable<ColorStruct> ToolTip_Background_Color;
		Valueable<int> ToolTip_Background_Opacity;
		Valueable<float> ToolTip_Background_BlurSize;

		Valueable<bool> CrateOnlyOnLand;
		Valueable<int> UnitCrateVehicleCap;
		Valueable<int> FreeMCV_CreditsThreshold;
		Valueable<AffectedHouse> RadialIndicatorVisibility;
		Valueable<bool> DrawTurretShadow;
		ValueableIdx<ColorScheme> AnimRemapDefaultColorScheme;
		ValueableIdx<ColorScheme> TimerBlinkColorScheme;

		ValueableVector<DigitalDisplayTypeClass*> Buildings_DefaultDigitalDisplayTypes;
		ValueableVector<DigitalDisplayTypeClass*> Infantry_DefaultDigitalDisplayTypes;
		ValueableVector<DigitalDisplayTypeClass*> Vehicles_DefaultDigitalDisplayTypes;
		ValueableVector<DigitalDisplayTypeClass*> Aircraft_DefaultDigitalDisplayTypes;

		Valueable<bool> ShowDesignatorRange;
		Valueable<bool> IsVoiceCreatedGlobal;
		Valueable<int> SelectionFlashDuration;
		AnimTypeClass* DropPodTrailer;
		SHPStruct* PodImage;
		Valueable<bool> DrawInsignia_OnlyOnSelected;
		Valueable<Point2D> DrawInsignia_AdjustPos_Infantry;
		Valueable<Point2D> DrawInsignia_AdjustPos_Buildings;
		Nullable<BuildingSelectBracketPosition> DrawInsignia_AdjustPos_BuildingsAnchor;
		Valueable<Point2D> DrawInsignia_AdjustPos_Units;
		Valueable<AnimTypeClass*> Promote_VeteranAnimation;
		Valueable<AnimTypeClass*> Promote_EliteAnimation;

		Valueable<double> AircraftLevelLightMultiplier;
		Valueable<double> JumpjetLevelLightMultiplier;

		Nullable<Vector3D<float>> VoxelLightSource;
		// Nullable<Vector3D<float>> VoxelShadowLightSource;
		Valueable<bool> UseFixedVoxelLighting;

		Valueable<bool> GatherWhenMCVDeploy;
		Valueable<bool> AIFireSale;
		Valueable<int> AIFireSaleDelay;
		Valueable<bool> AIAllToHunt;
		Valueable<bool> RepairBaseNodes;

		Valueable<bool> WarheadParticleAlphaImageIsLightFlash;
		Valueable<int> CombatLightDetailLevel;
		Valueable<int> LightFlashAlphaImageDetailLevel;

		ExtData(RulesClass* OwnerObject) : Extension<RulesClass>(OwnerObject)
			, Storage_TiberiumIndex { -1 }
			, InfantryGainSelfHealCap {}
			, UnitsGainSelfHealCap {}
			, GainSelfHealAllowMultiplayPassive { true }
			, EnemyInsignia { true }
			, DisguiseBlinkingVisibility { AffectedHouse::Owner }
			, ChronoSparkleDisplayDelay { 24 }
			, ChronoSparkleBuildingDisplayPositions { ChronoSparkleDisplayPosition::OccupantSlots }
			, ChronoSpherePreDelay { 60 }
			, ChronoSphereDelay { 0 }
			, AIChronoSphereSW {}
			, AIChronoWarpSW {}
			, SubterraneanSpeed { 19 }
			, SubterraneanHeight { -256 }
			, AISuperWeaponDelay {}
			, UseGlobalRadApplicationDelay { true }
			, RadApplicationDelay_Building { 0 }
			, RadBuildingDamageMaxCount { -1 }
			, RadSiteWarhead_Detonate { false }
			, RadSiteWarhead_Detonate_Full { true }
			, RadHasOwner { false }
			, RadHasInvoker { false }
			, JumpjetCrash { 5.0 }
			, JumpjetNoWobbles { false }
			, VeinholeWarhead {}
			, MissingCameo { GameStrings::XXICON_SHP() }

			, PlacementGrid_Translucency { 0 }
			, PlacementGrid_TranslucencyWithPreview { }
			, PlacementPreview { false }
			, PlacementPreview_Translucency { 75 }

			, Shield_ConditionYellow { }
			, Shield_ConditionRed { }
			, Pips_Shield_Background { }
			, Pips_Shield_Building { { -1,-1,-1 } }
			, Pips_Shield_Building_Empty { }
			, Pips_SelfHeal_Infantry { { 13, 20 } }
			, Pips_SelfHeal_Units { { 13, 20 } }
			, Pips_SelfHeal_Buildings { { 13, 20 } }
			, Pips_SelfHeal_Infantry_Offset { { 25, -35 } }
			, Pips_SelfHeal_Units_Offset { { 33, -32 } }
			, Pips_SelfHeal_Buildings_Offset { { 15, 10 } }
			, Pips_Generic_Size { { 4, 0 } }
			, Pips_Generic_Buildings_Size { { 4, 2 } }
			, Pips_Ammo_Size { { 4, 0 } }
			, Pips_Ammo_Buildings_Size { { 4, 2 } }
			, Pips_Tiberiums_Frames {}
			, Pips_Tiberiums_EmptyFrame { 0 }
			, Pips_Tiberiums_DisplayOrder {}
			, Pips_Tiberiums_WeedFrame { 1 }
			, Pips_Tiberiums_WeedEmptyFrame { 0 }

			, HeightShadowScaling { false }
			, HeightShadowScaling_MinScale { 0.0 }
			, AirShadowBaseScale_log { 0.693376137 }

			, ExtendedAircraftMissions { false }

			, AllowParallelAIQueues { true }
			, ForbidParallelAIQueues_Aircraft { false }
			, ForbidParallelAIQueues_Building { false }
			, ForbidParallelAIQueues_Infantry { false }
			, ForbidParallelAIQueues_Navy { false }
			, ForbidParallelAIQueues_Vehicle { false }

			, EnablePowerSurplus { false }

			, IronCurtain_KeptOnDeploy { true }
			, IronCurtain_EffectOnOrganics { IronCurtainEffect::Kill }
			, IronCurtain_KillOrganicsWarhead { }
			, ForceShield_KeptOnDeploy { false }
			, ForceShield_EffectOnOrganics { IronCurtainEffect::Kill }
			, ForceShield_KillOrganicsWarhead { }
			, IronCurtain_ExtraTintIntensity { 0.0 }
			, ForceShield_ExtraTintIntensity { 0.0 }
			, ColorAddUse8BitRGB { false }
			, ROF_RandomDelay { { 0 ,2  } }
			, ToolTip_Background_Color { { 0, 0, 0 } }
			, ToolTip_Background_Opacity { 100 }
			, ToolTip_Background_BlurSize { 0.0f }
			, DisplayIncome { false }
			, DisplayIncome_AllowAI { true }
			, DisplayIncome_Houses { AffectedHouse::All }
			, CrateOnlyOnLand { false }
			, UnitCrateVehicleCap { 50 }
			, FreeMCV_CreditsThreshold { 1500 }
			, RadialIndicatorVisibility { AffectedHouse::Allies }
			, DrawTurretShadow { false }
			, IsVoiceCreatedGlobal { false }
			, SelectionFlashDuration { 0 }
			, DrawInsignia_OnlyOnSelected { false }
			, DrawInsignia_AdjustPos_Infantry { { 5, 2  } }
			, DrawInsignia_AdjustPos_Buildings { { 10, 6  } }
			, DrawInsignia_AdjustPos_BuildingsAnchor {}
			, DrawInsignia_AdjustPos_Units { { 10, 6  } }
			, Promote_VeteranAnimation {}
			, Promote_EliteAnimation {}
			, AnimRemapDefaultColorScheme { 0 }
			, TimerBlinkColorScheme { 5 }
			, Buildings_DefaultDigitalDisplayTypes {}
			, Infantry_DefaultDigitalDisplayTypes {}
			, Vehicles_DefaultDigitalDisplayTypes {}
			, Aircraft_DefaultDigitalDisplayTypes {}
			, ShowDesignatorRange { true }
			, DropPodTrailer { }
			, PodImage { }
			, AircraftLevelLightMultiplier { 1.0 }
			, JumpjetLevelLightMultiplier { 0.0 }
			, VoxelLightSource { }
			// , VoxelShadowLightSource { }
			, UseFixedVoxelLighting { false }
			, GatherWhenMCVDeploy { true }
			, AIFireSale { true }
			, AIFireSaleDelay { 0 }
			, AIAllToHunt { true }
			, RepairBaseNodes { false }
			, WarheadParticleAlphaImageIsLightFlash { false }
			, CombatLightDetailLevel { 0 }
			, LightFlashAlphaImageDetailLevel { 0 }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI);
		virtual void LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI);
		virtual void InitializeConstants() override;
		void InitializeAfterTypeData(RulesClass* pThis);

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void ReplaceVoxelLightSources();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

private:
	static std::unique_ptr<ExtData> Data;

public:
	static IStream* g_pStm;

	static void Allocate(RulesClass* pThis);
	static void Remove(RulesClass* pThis);

	static void LoadFromINIFile(RulesClass* pThis, CCINIClass* pINI);
	static void LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI);

	static ExtData* Global()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(RulesClass::Instance);
	}

	static void PointerGotInvalid(void* ptr, bool removed)
	{
		Global()->InvalidatePointer(ptr, removed);
	}

};
