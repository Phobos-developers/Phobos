#pragma once
#include <SuperWeaponTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class SWTypeExt
{
public:
	using base_type = SuperWeaponTypeClass;

	class ExtData final : public Extension<SuperWeaponTypeClass>
	{
	public:

		//Ares 0.A
		Valueable<int> Money_Amount;
		ValueableVector<TechnoTypeClass*> SW_Inhibitors;
		Valueable<bool> SW_AnyInhibitor;
		ValueableVector<TechnoTypeClass*> SW_Designators;
		Valueable<bool> SW_AnyDesignator;

		Valueable<CSFText> UIDescription;
		Valueable<int> CameoPriority;
		ValueableVector<TechnoTypeClass*> LimboDelivery_Types;
		ValueableVector<int> LimboDelivery_IDs;
		ValueableVector<float> LimboDelivery_RollChances;
		Valueable<AffectedHouse> LimboKill_Affected;
		ValueableVector<int> LimboKill_IDs;
		Valueable<double> RandomBuffer;


		ValueableVector<ValueableVector<int>> LimboDelivery_RandomWeightsData;

		ExtData(SuperWeaponTypeClass* OwnerObject) : Extension<SuperWeaponTypeClass>(OwnerObject)
			, Money_Amount { 0 }
			, SW_Inhibitors {}
			, SW_AnyInhibitor { false }
			, SW_Designators { }
			, SW_AnyDesignator { false }
			, UIDescription {}
			, CameoPriority { 0 }
			, LimboDelivery_Types {}
			, LimboDelivery_IDs {}
			, LimboDelivery_RollChances {}
			, LimboDelivery_RandomWeightsData {}
			, LimboKill_Affected { AffectedHouse::Owner }
			, LimboKill_IDs {}
			, RandomBuffer { 0.0 }
		{ }

		// Ares 0.A functions
		bool IsInhibitor(HouseClass* pOwner, TechnoClass* pTechno) const;
		bool HasInhibitor(HouseClass* pOwner, const CellStruct& coords) const;
		bool IsInhibitorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const;

		bool IsDesignator(HouseClass* pOwner, TechnoClass* pTechno) const;
		bool HasDesignator(HouseClass* pOwner, const CellStruct& coords) const;
		bool IsDesignatorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const;

		void ApplyLimboDelivery(HouseClass* pHouse);
		void ApplyLimboKill(HouseClass* pHouse);

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SWTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static void FireSuperWeaponExt(SuperClass* pSW, const CellStruct& cell);

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};
