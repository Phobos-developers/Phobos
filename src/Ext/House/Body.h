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
	static constexpr bool ShouldConsiderInvalidatePointer = true;

	class ExtData final : public Extension<HouseClass>
	{
	public:
		std::map<BuildingTypeExt::ExtData*, int> PowerPlantEnhancers;
		std::vector<BuildingClass*> OwnedLimboDeliveredBuildings;

		CounterClass LimboAircraft;  // Currently owned aircraft in limbo
		CounterClass LimboBuildings; // Currently owned buildings in limbo
		CounterClass LimboInfantry;  // Currently owned infantry in limbo
		CounterClass LimboVehicles;  // Currently owned vehicles in limbo

		BuildingClass* Factory_BuildingType;
		BuildingClass* Factory_InfantryType;
		BuildingClass* Factory_VehicleType;
		BuildingClass* Factory_NavyType;
		BuildingClass* Factory_AircraftType;

		CDTimerClass AISuperWeaponDelayTimer;
		CDTimerClass AIFireSaleDelayTimer;

		//Read from INI
		Nullable<bool> RepairBaseNodes[3];

		// FactoryPlants with Allow/DisallowTypes set.
		std::vector<BuildingClass*> RestrictedFactoryPlants;

		int LastBuiltNavalVehicleType;
		int ProducingNavalUnitTypeIndex;

		// Factories that exist but don't count towards multiple factory bonus.
		int NumAirpads_NonMFB;
		int NumBarracks_NonMFB;
		int NumWarFactories_NonMFB;
		int NumConYards_NonMFB;
		int NumShipyards_NonMFB;

		std::map<SuperClass*, std::vector<SuperClass*>> SuspendedEMPulseSWs;
		// standalone? no need and not a good idea
		struct SWExt
		{
			int ShotCount;
		};
		std::vector<SWExt> SuperExts;

		ExtData(HouseClass* OwnerObject) : Extension<HouseClass>(OwnerObject)
			, PowerPlantEnhancers {}
			, OwnedLimboDeliveredBuildings {}
			, LimboAircraft {}
			, LimboBuildings {}
			, LimboInfantry {}
			, LimboVehicles {}
			, Factory_BuildingType { nullptr }
			, Factory_InfantryType { nullptr }
			, Factory_VehicleType { nullptr }
			, Factory_NavyType { nullptr }
			, Factory_AircraftType { nullptr }
			, AISuperWeaponDelayTimer {}
			, RepairBaseNodes { }
			, RestrictedFactoryPlants {}
			, LastBuiltNavalVehicleType { -1 }
			, ProducingNavalUnitTypeIndex { -1 }
			, NumAirpads_NonMFB { 0 }
			, NumBarracks_NonMFB { 0 }
			, NumWarFactories_NonMFB { 0 }
			, NumConYards_NonMFB { 0 }
			, NumShipyards_NonMFB { 0 }
			, AIFireSaleDelayTimer {}
			, SuspendedEMPulseSWs {}
			, SuperExts(SuperWeaponTypeClass::Array->Count)
		{ }

		bool OwnsLimboDeliveredBuilding(BuildingClass* pBuilding);
		void AddToLimboTracking(TechnoTypeClass* pTechnoType);
		void RemoveFromLimboTracking(TechnoTypeClass* pTechnoType);
		int CountOwnedPresentAndLimboed(TechnoTypeClass* pTechnoType);
		void UpdateNonMFBFactoryCounts(AbstractType rtti, bool remove, bool isNaval);
		int GetFactoryCountWithoutNonMFB(AbstractType rtti, bool isNaval);
		float GetRestrictedFactoryPlantMult(TechnoTypeClass* pTechnoType) const;

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

	class ExtContainer final : public Container<HouseExt>
	{
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
	static void GetAIChronoshiftSupers(HouseClass* pThis, SuperClass*& pSuperCSphere, SuperClass*& pSuperCWarp);

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

	static CanBuildResult BuildLimitGroupCheck(const HouseClass* pThis, const TechnoTypeClass* pItem, bool buildLimitOnly, bool includeQueued);
	static bool ReachedBuildLimit(const HouseClass* pHouse, const TechnoTypeClass* pType, bool ignoreQueued);
};
