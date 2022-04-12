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

		Valueable<bool> Buildings_ShowHP;
		Valueable<bool> Buildings_ShowShield;
		Valueable<Vector3D<int>> Buildings_ShowColorHPHigh;
		Valueable<Vector3D<int>> Buildings_ShowColorHPMid;
		Valueable<Vector3D<int>> Buildings_ShowColorHPLow;
		Valueable<Vector3D<int>> Buildings_ShowColorShieldHigh;
		Valueable<Vector3D<int>> Buildings_ShowColorShieldMid;
		Valueable<Vector3D<int>> Buildings_ShowColorShieldLow;
		Valueable<Vector2D<int>> Buildings_ShowHPOffset;
		Valueable<Vector2D<int>> Buildings_ShowShieldOffset;
		Valueable<bool> Buildings_ShowBackground;
		Valueable<Vector2D<int>> Buildings_ShowHPOffset_WithoutShield;
		Valueable<bool> Units_ShowHP;
		Valueable<bool> Units_ShowShield;
		Valueable<Vector3D<int>> Units_ShowColorHPHigh;
		Valueable<Vector3D<int>> Units_ShowColorHPMid;
		Valueable<Vector3D<int>> Units_ShowColorHPLow;
		Valueable<Vector3D<int>> Units_ShowColorShieldHigh;
		Valueable<Vector3D<int>> Units_ShowColorShieldMid;
		Valueable<Vector3D<int>> Units_ShowColorShieldLow;
		Valueable<Vector2D<int>> Units_ShowHPOffset;
		Valueable<Vector2D<int>> Units_ShowShieldOffset;
		Valueable<bool> Units_ShowBackground;
		Valueable<Vector2D<int>> Units_ShowHPOffset_WithoutShield;

		Valueable<bool> Buildings_UseSHPShowHP;
		Valueable<bool> Buildings_UseSHPShowShield;
		PhobosFixedString<0x20> Buildings_HPNumberSHP;
		PhobosFixedString<0x20> Buildings_ShieldNumberSHP;
		PhobosFixedString<0x20> Buildings_HPNumberPAL;
		PhobosFixedString<0x20> Buildings_ShieldNumberPAL;
		Valueable<int> Buildings_HPNumberInterval;
		Valueable<int> Buildings_ShieldNumberInterval;
		Valueable<bool> Units_UseSHPShowHP;
		Valueable<bool> Units_UseSHPShowShield;
		PhobosFixedString<0x20> Units_HPNumberSHP;
		PhobosFixedString<0x20> Units_ShieldNumberSHP;
		PhobosFixedString<0x20> Units_HPNumberPAL;
		PhobosFixedString<0x20> Units_ShieldNumberPAL;
		Valueable<int> Units_HPNumberInterval;
		Valueable<int> Units_ShieldNumberInterval;

		ExtData(RulesClass* OwnerObject) : Extension<RulesClass>(OwnerObject)
			, Pips_Shield { { -1,-1,-1 } }
			, Pips_Shield_Buildings { { -1,-1,-1 } }
			, RadApplicationDelay_Building { 0 }
			, MissingCameo { "xxicon.shp" }
			, JumpjetCrash { 5.0 }
			, JumpjetNoWobbles { false }
			, JumpjetAllowLayerDeviation { true }
			, Storage_TiberiumIndex { -1 }
			, Buildings_ShowHP { false }
			, Buildings_ShowShield { false }
			, Buildings_ShowColorHPHigh { { 0, 255, 0 } }
			, Buildings_ShowColorHPMid { { 255, 255, 0 } }
			, Buildings_ShowColorHPLow { { 255, 0, 0 } }
			, Buildings_ShowColorShieldHigh { { 0, 0, 255 } }
			, Buildings_ShowColorShieldMid { { 0, 0, 255 } }
			, Buildings_ShowColorShieldLow { { 0, 0, 255 } }
			, Buildings_ShowHPOffset { { 0,0 } }
			, Buildings_ShowShieldOffset { { 0, 0 } }
			, Buildings_ShowBackground { false }
			, Buildings_ShowHPOffset_WithoutShield { { 0, 0 } }
			, Units_ShowHP { false }
			, Units_ShowShield { false }
			, Units_ShowColorHPHigh { { 0, 255, 0 } }
			, Units_ShowColorHPMid { { 255,255,0 } }
			, Units_ShowColorHPLow { { 255, 0, 0 } }
			, Units_ShowColorShieldHigh { { 0, 0, 255 } }
			, Units_ShowColorShieldMid { { 0, 0, 255 } }
			, Units_ShowColorShieldLow { { 0, 0, 255 } }
			, Units_ShowHPOffset { { 0, 0 } }
			, Units_ShowShieldOffset { { 0, 0 } }
			, Units_ShowBackground { false }
			, Units_ShowHPOffset_WithoutShield { { 0, 0 } }
			, Buildings_UseSHPShowHP { false }
			, Buildings_UseSHPShowShield { false }
			, Buildings_HPNumberSHP { "number.shp" }
			, Buildings_HPNumberPAL { "" }
			, Buildings_ShieldNumberSHP { "number.shp" }
			, Buildings_ShieldNumberPAL { "" }
			, Buildings_HPNumberInterval { 8 }
			, Buildings_ShieldNumberInterval { 8 }
			, Units_UseSHPShowHP { false }
			, Units_UseSHPShowShield { false }
			, Units_HPNumberSHP { "number.shp" }
			, Units_HPNumberPAL { "" }
			, Units_ShieldNumberSHP { "number.shp" }
			, Units_ShieldNumberPAL { "" }
			, Units_HPNumberInterval { 8 }
			, Units_ShieldNumberInterval { 8 }
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
