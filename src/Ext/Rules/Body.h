#pragma once

#include <CCINIClass.h>
#include <RulesClass.h>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>

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
		Valueable<Vector3D<int>> Pips_Shield;
		Valueable<Vector3D<int>> Pips_Shield_Buildings;
		Valueable<int> RadApplicationDelay_Building;
		PhobosFixedString<32u> MissingCameo;
		DynamicVectorClass<DynamicVectorClass<TechnoTypeClass*>> AITargetTypesLists;
		DynamicVectorClass<DynamicVectorClass<ScriptTypeClass*>> AIScriptsLists;

		Valueable<double> JumpjetCrash;
		Valueable<bool> JumpjetNoWobbles;
		Valueable<bool> JumpjetAllowLayerDeviation;
		Valueable<int> Storage_TiberiumIndex;

		SHPStruct* SHP_SelectBrdSHP_INF;
		ConvertClass* SHP_SelectBrdPAL_INF;
		SHPStruct* SHP_SelectBrdSHP_UNIT;
		ConvertClass* SHP_SelectBrdPAL_UNIT;

		Valueable<bool> UseSelectBrd;
		PhobosFixedString<32U> SelectBrd_SHP_Infantry;
		PhobosFixedString<32U> SelectBrd_PAL_Infantry;
		Nullable<Vector3D<int>> SelectBrd_Frame_Infantry;
		Nullable<Vector2D<int>> SelectBrd_DrawOffset_Infantry;
		PhobosFixedString<32U> SelectBrd_SHP_Unit;
		PhobosFixedString<32U> SelectBrd_PAL_Unit;
		Nullable<Vector3D<int>> SelectBrd_Frame_Unit;
		Nullable<Vector2D<int>> SelectBrd_DrawOffset_Unit;
		Nullable<int> SelectBrd_DefaultTranslucentLevel;
		Valueable<bool> SelectBrd_DefaultShowEnemy;

		ExtData(RulesClass* OwnerObject) : Extension<RulesClass>(OwnerObject)
			, Pips_Shield { { -1,-1,-1 } }
			, Pips_Shield_Buildings { { -1,-1,-1 } }
			, RadApplicationDelay_Building { 0 }
			, MissingCameo { "xxicon.shp" }
			, JumpjetCrash { 5.0 }
			, JumpjetNoWobbles { false }
			, JumpjetAllowLayerDeviation { true }
			, Storage_TiberiumIndex { -1 }
			, SHP_SelectBrdSHP_INF { nullptr }
			, SHP_SelectBrdPAL_INF { nullptr }
			, SHP_SelectBrdSHP_UNIT { nullptr }
			, SHP_SelectBrdPAL_UNIT { nullptr }
			, UseSelectBrd { false }
			, SelectBrd_SHP_Infantry { "select.shp" }
			, SelectBrd_PAL_Infantry { "palette.pal" }
			, SelectBrd_Frame_Infantry { {0,0,0} }
			, SelectBrd_DrawOffset_Infantry { {0,0} }
			, SelectBrd_SHP_Unit { "select.shp" }
			, SelectBrd_PAL_Unit { "palette.pal" }
			, SelectBrd_Frame_Unit { {3,3,3} }
			, SelectBrd_DrawOffset_Unit { {0,0} }
			, SelectBrd_DefaultTranslucentLevel { 0 }
			, SelectBrd_DefaultShowEnemy { true }
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
