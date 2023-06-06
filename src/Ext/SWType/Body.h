#pragma once
#include <WarheadTypeClass.h>
#include <WeaponTypeClass.h>
#include <SuperWeaponTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Building/Body.h>

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
		Valueable<double> SW_RangeMinimum;
		Valueable<double> SW_RangeMaximum;
		DWORD SW_RequiredHouses;
		DWORD SW_ForbiddenHouses;
		ValueableVector<BuildingTypeClass*> SW_AuxBuildings;
		ValueableVector<BuildingTypeClass*> SW_NegBuildings;
		Valueable<bool> SW_InitialReady;

		Valueable<CSFText> UIDescription;
		Valueable<int> CameoPriority;
		ValueableVector<BuildingTypeClass*> LimboDelivery_Types;
		ValueableVector<int> LimboDelivery_IDs;
		ValueableVector<float> LimboDelivery_RollChances;
		Valueable<AffectedHouse> LimboKill_Affected;
		ValueableVector<int> LimboKill_IDs;
		Valueable<double> RandomBuffer;
		ValueableIdxVector<SuperWeaponTypeClass> SW_Next;
		Valueable<bool> SW_Next_RealLaunch;
		Valueable<bool> SW_Next_IgnoreInhibitors;
		Valueable<bool> SW_Next_IgnoreDesignators;
		ValueableVector<float> SW_Next_RollChances;

		Valueable<int> ShowTimer_Priority;

		Nullable<WarheadTypeClass*> Detonate_Warhead;
		Nullable<WeaponTypeClass*> Detonate_Weapon;
		Nullable<int> Detonate_Damage;
		Valueable<bool> Detonate_AtFirer;

		std::vector<ValueableVector<int>> LimboDelivery_RandomWeightsData;
		std::vector<ValueableVector<int>> SW_Next_RandomWeightsData;

		std::vector<std::tuple<ValueableVector<TechnoTypeClass*>, NullableIdx<TechnoTypeClass>, Nullable<AffectedHouse>>> Convert_Pairs;

		ExtData(SuperWeaponTypeClass* OwnerObject) : Extension<SuperWeaponTypeClass>(OwnerObject)
			, Money_Amount { 0 }
			, SW_Inhibitors {}
			, SW_AnyInhibitor { false }
			, SW_Designators { }
			, SW_AnyDesignator { false }
			, SW_RangeMinimum { -1.0 }
			, SW_RangeMaximum { -1.0 }
			, SW_RequiredHouses { 0xFFFFFFFFu }
			, SW_ForbiddenHouses { 0u }
			, SW_AuxBuildings {}
			, SW_NegBuildings {}
			, SW_InitialReady { false }
			, UIDescription {}
			, CameoPriority { 0 }
			, LimboDelivery_Types {}
			, LimboDelivery_IDs {}
			, LimboDelivery_RollChances {}
			, LimboDelivery_RandomWeightsData {}
			, LimboKill_Affected { AffectedHouse::Owner }
			, LimboKill_IDs {}
			, RandomBuffer { 0.0 }
			, Detonate_Warhead {}
			, Detonate_Weapon {}
			, Detonate_Damage {}
			, Detonate_AtFirer { false }
			, SW_Next {}
			, SW_Next_RealLaunch { true }
			, SW_Next_IgnoreInhibitors { false }
			, SW_Next_IgnoreDesignators { true }
			, SW_Next_RollChances {}
			, SW_Next_RandomWeightsData {}
			, ShowTimer_Priority { 0 }
			, Convert_Pairs {}
		{ }

		// Ares 0.A functions
		bool IsInhibitor(HouseClass* pOwner, TechnoClass* pTechno) const;
		bool HasInhibitor(HouseClass* pOwner, const CellStruct& coords) const;
		bool IsInhibitorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const;
		bool IsDesignator(HouseClass* pOwner, TechnoClass* pTechno) const;
		bool HasDesignator(HouseClass* pOwner, const CellStruct& coords) const;
		bool IsDesignatorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const;
		bool IsLaunchSiteEligible(const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const;
		bool IsLaunchSite(BuildingClass* pBuilding) const;
		std::pair<double, double> GetLaunchSiteRange(BuildingClass* pBuilding = nullptr) const;
		bool IsAvailable(HouseClass* pHouse) const;

		void ApplyLimboDelivery(HouseClass* pHouse);
		void ApplyLimboKill(HouseClass* pHouse);
		void ApplyDetonation(HouseClass* pHouse, const CellStruct& cell);
		void ApplySWNext(SuperClass* pSW, const CellStruct& cell);
		void ApplyTypeConversion(SuperClass* pSW);

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	private:
		std::vector<int> WeightedRollsHandler(ValueableVector<float>* chances, std::vector<ValueableVector<int>>* weights, size_t size);

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
