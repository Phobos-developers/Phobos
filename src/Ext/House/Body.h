#pragma once
#include <HouseClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Building/Body.h>

#include <map>

class HouseExt
{
public:
	using base_type = HouseClass;

	static constexpr DWORD Canary = 0x11111111;
	static constexpr size_t ExtPointerOffset = 0x16098;

	class ExtData final : public Extension<HouseClass>
	{
	public:
		std::map<BuildingTypeExt::ExtData*, int> PowerPlantEnhancers;
		std::vector<BuildingClass*> OwnedLimboDeliveredBuildings;
		std::vector<TechnoExt::ExtData*> OwnedAutoDeathObjects;
		std::vector<TechnoExt::ExtData*> OwnedTransportReloaders; // Objects that can reload ammo in limbo

		CounterClass LimboAircraft;  // Currently owned aircraft in limbo
		CounterClass LimboBuildings; // Currently owned buildings in limbo
		CounterClass LimboInfantry;  // Currently owned infantry in limbo
		CounterClass LimboVehicles;  // Currently owned vehicles in limbo

		BuildingClass* Factory_BuildingType;
		BuildingClass* Factory_InfantryType;
		BuildingClass* Factory_VehicleType;
		BuildingClass* Factory_NavyType;
		BuildingClass* Factory_AircraftType;

		//Read from INI
		bool RepairBaseNodes[3];

		int LastBuiltNavalVehicleType;
		int ProducingNavalUnitTypeIndex;

		/**
		 *  If we are currently expanding our base towards a resourceful location,
		 *  this records the cell that we are expanding towards.
		 */
		CellStruct NextExpansionPointLocation;

		/**
		 *  Locations that we should never expand towards.
		 *  Basically, locations that are unreachable.
		 */
		CellStruct PermanentlyBlockedExpansionPointLocations[20];

		/**
		 *  Records whether the AI has reached its expansion point.
		 *  If yes, the AI should build a refinery.
		 */
		bool ShouldBuildRefinery;

		/**
		 *  Set when the AI has built its first barracks during the game.
		 *  Used to figure out whether the AI should reset its TeamDelay
		 *  timer when it has built a barracks.
		 */
		bool HasBuiltFirstBarracks;

		/**
		 *  Records when the AI last checked for excess refineries.
		 */
		int LastExcessRefineryCheckFrame;

		/**
		 *  Records when the AI last checked for sleeping harvesters.
		 */
		int LastSleepingHarvesterCheckFrame;

		/**
		 *  Defines whether the AI has already performed a final "desperate vehicle charge".
		 *  If it has been done, then there is no need to do it again.
		 */
		bool HasPerformedVehicleCharge;

		/**
		 *  Records a value whether the current structure build choice
		 *  was made under threat of getting rushed early in the game.
		 */
		bool IsUnderStartRushThreat;

		ExtData(HouseClass* OwnerObject) : Extension<HouseClass>(OwnerObject)
			, PowerPlantEnhancers {}
			, OwnedLimboDeliveredBuildings {}
			, OwnedAutoDeathObjects {}
			, OwnedTransportReloaders {}
			, LimboAircraft {}
			, LimboBuildings {}
			, LimboInfantry {}
			, LimboVehicles {}
			, Factory_BuildingType { nullptr }
			, Factory_InfantryType { nullptr }
			, Factory_VehicleType { nullptr }
			, Factory_NavyType { nullptr }
			, Factory_AircraftType { nullptr }
			, RepairBaseNodes { false,false,false }
			, LastBuiltNavalVehicleType { -1 }
			, ProducingNavalUnitTypeIndex { -1 }
		{ }

		bool OwnsLimboDeliveredBuilding(BuildingClass* pBuilding);
		void UpdateAutoDeathObjectsInLimbo();
		void UpdateTransportReloaders();
		void AddToLimboTracking(TechnoTypeClass* pTechnoType);
		void RemoveFromLimboTracking(TechnoTypeClass* pTechnoType);
		int CountOwnedPresentAndLimboed(TechnoTypeClass* pTechnoType);

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		//virtual void Initialize() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;

		void UpdateVehicleProduction();

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
		bool UpdateHarvesterProduction();
	};

	class ExtContainer final : public Container<HouseExt> {
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();

			switch (abs)
			{
			case AbstractType::Building:
				return false;
			}

			return true;
		}
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static int ActiveHarvesterCount(HouseClass* pThis);
	static int TotalHarvesterCount(HouseClass* pThis);
	static HouseClass* GetHouseKind(OwnerHouseKind kind, bool allowRandom, HouseClass* pDefault, HouseClass* pInvoker = nullptr, HouseClass* pVictim = nullptr);
	static CellClass* GetEnemyBaseGatherCell(HouseClass* pTargetHouse, HouseClass* pCurrentHouse, CoordStruct defaultCurrentCoords, SpeedType speedTypeZone, int extraDistance = 0);

	static void SetSkirmishHouseName(HouseClass* pHouse);

	static bool AdvAI_House_Search_For_Next_Expansion_Point(HouseClass* pHouse);
	static bool AdvAI_Can_Build_Building(HouseClass* pHouse, BuildingTypeClass* pBuildingType, bool checkPrereqs);
	static bool AdvAI_Is_Recently_Attacked(HouseClass* pHouse);
	static bool AdvAI_Is_Under_Start_Rush_Threat(HouseClass* pHouse, int enemyAircraftCount);
	static int AdvAI_Calculate_Enemy_Aircraft_Count(HouseClass* pHouse);
	static const BuildingTypeClass* AdvAI_Evaluate_Get_Best_Building(HouseClass* pHouse);
	static const BuildingTypeClass* AdvAI_Get_Building_To_Build(HouseClass* pHouse);
	static void AdvAI_Raise_Money(HouseClass* pHouse);
	static void AdvAI_Economy_Upkeep(HouseClass* pHouse);
	static void AdvAI_Awaken_Sleeping_Harvesters(HouseClass* pHouse);
	static void AdvAI_Sell_Extra_ConYards(HouseClass* pHouse);
	static void Vinifera_HouseClass_AI_Building(HouseClass* pHouse);
	static void AdvAI_HouseClass_Expert_AI(HouseClass* pHouse);

	static bool IsDisabledFromShell(
	HouseClass const* pHouse, BuildingTypeClass const* pItem);

	static size_t FindOwnedIndex(
	HouseClass const* pHouse, int idxParentCountry,
	Iterator<TechnoTypeClass const*> items, size_t start = 0);

	static size_t FindBuildableIndex(
		HouseClass const* pHouse, int idxParentCountry,
		Iterator<TechnoTypeClass const*> items, size_t start = 0);

	template <typename T>
	static T* FindOwned(
		HouseClass const* const pHouse, int const idxParent,
		Iterator<T*> const items, size_t const start = 0)
	{
		auto const index = FindOwnedIndex(pHouse, idxParent, items, start);
		return index < items.size() ? items[index] : nullptr;
	}

	template <typename T>
	static T* FindBuildable(
		HouseClass const* const pHouse, int const idxParent,
		Iterator<T*> const items, size_t const start = 0)
	{
		auto const index = FindBuildableIndex(pHouse, idxParent, items, start);
		return index < items.size() ? items[index] : nullptr;
	}

	static std::vector<int> AIProduction_CreationFrames;
	static std::vector<int> AIProduction_Values;
	static std::vector<int> AIProduction_BestChoices;
	static std::vector<int> AIProduction_BestChoicesNaval;
};
