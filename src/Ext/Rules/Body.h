#pragma once

#include <CCINIClass.h>
#include <RulesClass.h>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Utilities/Anchor.h>
#include <Utilities/Debug.h>

class AnimTypeClass;
class MouseCursor;
class TechnoTypeClass;
class VocClass;
class WarheadTypeClass;

class RulesExt
{
public:
	using base_type = RulesClass;

	class ExtData final : public Extension<RulesClass>
	{
	public:
		DynamicVectorClass<DynamicVectorClass<TechnoTypeClass*>> AITargetTypesLists;
		DynamicVectorClass<DynamicVectorClass<ScriptTypeClass*>> AIScriptsLists;

		Valueable<int> Storage_TiberiumIndex;
		Nullable<int> InfantryGainSelfHealCap;
		Nullable<int> UnitsGainSelfHealCap;
		Valueable<int> RadApplicationDelay_Building;
		Valueable<bool> RadWarhead_Detonate;
		Valueable<bool> RadHasOwner;
		Valueable<bool> RadHasInvoker;
		Valueable<double> JumpjetCrash;
		Valueable<bool> JumpjetNoWobbles;
		Valueable<bool> JumpjetAllowLayerDeviation;
		Valueable<bool> JumpjetTurnToTarget;
		PhobosFixedString<32u> MissingCameo;
		Valueable<int> PlacementGrid_TranslucentLevel;
		Valueable<int> BuildingPlacementPreview_TranslucentLevel;
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
		Valueable<bool> IronCurtain_KeptOnDeploy;


		class HugeBarData
		{
		public:

			Valueable<double> HugeBar_RectWidthPercentage;
			Valueable<Point2D> HugeBar_RectWH;
			Damageable<ColorStruct> HugeBar_Pips_Color1;
			Damageable<ColorStruct> HugeBar_Pips_Color2;

			Valueable<SHPStruct*> HugeBar_Shape;
			Valueable<SHPStruct*> HugeBar_Pips_Shape;
			CustomPalette HugeBar_Palette;
			CustomPalette HugeBar_Pips_Palette;
			Damageable<int> HugeBar_Frame;
			Damageable<int> HugeBar_Pips_Frame;
			Valueable<int> HugeBar_Pips_Interval;

			Valueable<Point2D> HugeBar_Offset;
			Nullable<Point2D> HugeBar_Pips_Offset;
			Valueable<int> HugeBar_Pips_Num;

			Damageable<ColorStruct> Value_Text_Color;

			Valueable<SHPStruct*> Value_Shape;
			CustomPalette Value_Palette;
			Valueable<int> Value_Num_BaseFrame;
			Valueable<int> Value_Sign_BaseFrame;
			Valueable<int> Value_Shape_Interval;

			Valueable<bool> DisplayValue;
			Valueable<bool> Value_Percentage;
			Valueable<Point2D> Value_Offset;
			Anchor Anchor;
			DisplayInfoType InfoType;

			HugeBarData() = default;
			HugeBarData(DisplayInfoType infoType);

			void LoadFromINI(CCINIClass* pINI);

			void Load(PhobosStreamReader& stm);
			void Save(PhobosStreamWriter& stm);

		private:

			template <typename T>
			void Serialize(T& stm);
		};

		std::vector<std::unique_ptr<HugeBarData>> HugeBar_Config;

		ExtData(RulesClass* OwnerObject) : Extension<RulesClass>(OwnerObject)
			, Storage_TiberiumIndex { -1 }
			, InfantryGainSelfHealCap {}
			, UnitsGainSelfHealCap {}
			, RadApplicationDelay_Building { 0 }
			, RadWarhead_Detonate { false }
			, RadHasOwner { false }
			, RadHasInvoker { false }
			, JumpjetCrash { 5.0 }
			, JumpjetNoWobbles { false }
			, JumpjetAllowLayerDeviation { true }
			, JumpjetTurnToTarget { false }
			, MissingCameo { "xxicon.shp" }
			, PlacementGrid_TranslucentLevel { 0 }
			, BuildingPlacementPreview_TranslucentLevel { 3 }
			, Pips_Shield_Background { }
			, Pips_Shield_Building { { -1,-1,-1 } }
			, Pips_Shield_Building_Empty { }
			, Pips_SelfHeal_Infantry {{ 13, 20 }}
			, Pips_SelfHeal_Units {{ 13, 20 }}
			, Pips_SelfHeal_Buildings {{ 13, 20 }}
			, Pips_SelfHeal_Infantry_Offset {{ 25, -35 }}
			, Pips_SelfHeal_Units_Offset {{ 33, -32 }}
			, Pips_SelfHeal_Buildings_Offset {{ 15, 10 }}
			, IronCurtain_KeptOnDeploy { true }
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

	static bool DetailsCurrentlyEnabled();
	static bool DetailsCurrentlyEnabled(int minDetailLevel);
};
