#pragma once

#include <CCINIClass.h>
#include <RulesClass.h>
#include <GameStrings.h>
#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Utilities/Enum.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Debug.h>

class AnimTypeClass;
class MouseCursor;
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
		Valueable<bool> UseGlobalRadApplicationDelay;
		Valueable<int> RadApplicationDelay_Building;
		Valueable<bool> RadWarhead_Detonate;
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

		Valueable<bool> AllowParallelAIQueues;
		Valueable<bool> ForbidParallelAIQueues_Aircraft;
		Valueable<bool> ForbidParallelAIQueues_Building;
		Valueable<bool> ForbidParallelAIQueues_Infantry;
		Valueable<bool> ForbidParallelAIQueues_Navy;
		Valueable<bool> ForbidParallelAIQueues_Vehicle;

		Valueable<bool> DisplayIncome;
		Valueable<bool> DisplayIncome_AllowAI;
		Valueable<AffectedHouse> DisplayIncome_Houses;

		Valueable<bool> IronCurtain_KeptOnDeploy;
		Valueable<IronCurtainEffect> IronCurtain_EffectOnOrganics;
		Nullable<WarheadTypeClass*> IronCurtain_KillOrganicsWarhead;

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

		ExtData(RulesClass* OwnerObject) : Extension<RulesClass>(OwnerObject)
			, Storage_TiberiumIndex { -1 }
			, InfantryGainSelfHealCap {}
			, UnitsGainSelfHealCap {}
			, GainSelfHealAllowMultiplayPassive { true }
			, EnemyInsignia { true }
			, DisguiseBlinkingVisibility { AffectedHouse::Owner }
			, ChronoSparkleDisplayDelay { 24 }
			, ChronoSparkleBuildingDisplayPositions { ChronoSparkleDisplayPosition::OccupantSlots }
			, UseGlobalRadApplicationDelay { true }
			, RadApplicationDelay_Building { 0 }
			, RadWarhead_Detonate { false }
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

			, AllowParallelAIQueues { true }
			, ForbidParallelAIQueues_Aircraft { false }
			, ForbidParallelAIQueues_Building { false }
			, ForbidParallelAIQueues_Infantry { false }
			, ForbidParallelAIQueues_Navy { false }
			, ForbidParallelAIQueues_Vehicle { false }
			, IronCurtain_KeptOnDeploy { true }
			, IronCurtain_EffectOnOrganics { IronCurtainEffect::Kill }
			, IronCurtain_KillOrganicsWarhead { }
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
			, AnimRemapDefaultColorScheme { 0 }
			, TimerBlinkColorScheme { 5 }
			, Buildings_DefaultDigitalDisplayTypes {}
			, Infantry_DefaultDigitalDisplayTypes {}
			, Vehicles_DefaultDigitalDisplayTypes {}
			, Aircraft_DefaultDigitalDisplayTypes {}
			, ShowDesignatorRange { true }
			, DropPodTrailer { }
			, PodImage { }
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

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};
