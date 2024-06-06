#pragma once
#include <BuildingClass.h>
#include <HouseClass.h>
#include <TiberiumClass.h>
#include <FactoryClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Misc/FlyingStrings.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>

class BuildingExt
{
public:
	using base_type = BuildingClass;

	static constexpr DWORD Canary = 0x87654321;
	static constexpr size_t ExtPointerOffset = 0x6FC;

	class ExtData final : public Extension<BuildingClass>
	{
	public:
		BuildingTypeExt::ExtData* TypeExtData;
		TechnoExt::ExtData* TechnoExtData;
		bool DeployedTechno;
		bool IsCreatedFromMapFile;
		int LimboID;
		int GrindingWeapon_LastFiredFrame;
		int GrindingWeapon_AccumulatedCredits;
		BuildingClass* CurrentAirFactory;
		int AccumulatedIncome;
		OptionalStruct<int, true> CurrentLaserWeaponIndex;

		/**
		*  If this building was built by the AI for it to reach an expansion
		*  point, this records the expansion point that the building helped reach.
		*/
		CellStruct AssignedExpansionPoint;

		ExtData(BuildingClass* OwnerObject) : Extension<BuildingClass>(OwnerObject)
			, TypeExtData { nullptr }
			, TechnoExtData { nullptr }
			, DeployedTechno { false }
			, IsCreatedFromMapFile { false }
			, LimboID { -1 }
			, GrindingWeapon_LastFiredFrame { 0 }
			, GrindingWeapon_AccumulatedCredits { 0 }
			, CurrentAirFactory { nullptr }
			, AccumulatedIncome { 0 }
			, CurrentLaserWeaponIndex {}
			, AssignedExpansionPoint {}
		{ }

		void DisplayIncomeString();
		void ApplyPoweredKillSpawns();
		bool HasSuperWeapon(int index, bool withUpgrades) const;

		void UpdatePrimaryFactoryAI();

		static BuildingClass* OurBuildings[1000];
		static size_t OurBuildingCount;
		static CellStruct AttackCell;

		virtual ~ExtData() = default;

		// virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			AnnounceInvalidPointer(CurrentAirFactory, ptr);
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BuildingExt>
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
			default:
				return true;
			}
		}
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void StoreTiberium(BuildingClass* pThis, float amount, int idxTiberiumType, int idxStorageTiberiumType);

	static int CountOccupiedDocks(BuildingClass* pBuilding);
	static bool HasFreeDocks(BuildingClass* pBuilding);
	static bool CanGrindTechno(BuildingClass* pBuilding, TechnoClass* pTechno);
	static bool DoGrindingExtras(BuildingClass* pBuilding, TechnoClass* pTechno, int refund);
	static bool HandleInfiltrate(BuildingClass* pBuilding, HouseClass* pInfiltratorHouse);
	static bool CanUndeployOnSell(BuildingClass* pThis);

	static HouseClass* Find_Closest_Opponent(const HouseClass* pHouse);
	static int Get_Distance_To_Primary_Enemy(CellStruct cell, HouseClass* pHouse);
	static void Mark_Expansion_As_Done(HouseClass* pHouse);
	static int Try_Place(BuildingClass* pBuilding, CellStruct cell);
	static RectangleStruct Get_Base_Rect(HouseClass* pHouse, int adjacency, int width, int height);
	static bool Should_Evaluate_Cell_For_Placement(CellStruct cell, BuildingClass* pBuilding, int adjacencyBonus);
	static int inline Modify_Rating_By_Terrain_Passability(CellStruct cell, BuildingClass* pBuilding, int originalValue);
	static CellStruct Find_Best_Building_Placement_Cell(RectangleStruct baseArea, BuildingClass* pBuilding, int (*valueGenerator)(CellStruct, BuildingClass*), int adjacencyBonus = 0);
	static int inline Modify_Rating_By_Allied_Building_Proximity(CellStruct cell, BuildingClass* pBuilding, int originalValue);
	static int Refinery_Placement_Cell_Value(CellStruct cell, BuildingClass* pBuilding);
	static CellStruct Get_Best_Refinery_Placement_Position(BuildingClass* pBuilding);
	static int Near_Base_Center_Placement_Position_Value(CellStruct cell, BuildingClass* pBuilding);
	static int Near_Base_Center_Defense_Placement_Position_Value(CellStruct cell, BuildingClass* pBuilding);
	static int Near_Enemy_Placement_Position_Value(CellStruct cell, BuildingClass* pBuilding);
	static int Near_Refinery_Placement_Position_Value(CellStruct cell, BuildingClass* pBuilding);
	static int Near_ConYard_Placement_Position_Value(CellStruct cell, BuildingClass* pBuilding);
	static int Far_From_Enemy_Placement_Position_Value(CellStruct cell, BuildingClass* pBuilding);
	static CellStruct Get_Best_SuperWeapon_Building_Placement_Position(BuildingClass* pBuilding);
	static int Towards_Expansion_Placement_Cell_Value(CellStruct cell, BuildingClass* pBuilding);
	static CellStruct Get_Best_Expansion_Placement_Position(BuildingClass* pBuilding);
	static int Barracks_Placement_Cell_Value(CellStruct cell, BuildingClass* pBuilding);
	static int NavalYard_Placement_Cell_Value(CellStruct cell, BuildingClass* pBuilding);
	static int WarFactory_Placement_Cell_Value(CellStruct cell, BuildingClass* pBuilding);
	static int Helipad_Placement_Cell_Value(CellStruct cell, BuildingClass* pBuilding);
	static CellStruct Get_Best_Factory_Placement_Position(BuildingClass* pBuilding);
	static int Near_AttackCell_Cell_Value(CellStruct cell, BuildingClass* pBuilding);
	static CellStruct Get_Best_Defense_Placement_Position(BuildingClass* pBuilding);
	static CellStruct Get_Best_Sensor_Placement_Position(BuildingClass* pBuilding);
	static CellStruct Get_Best_Placement_Position(BuildingClass* pBuilding);
	static int Exit_Object_Custom_Position(BuildingClass* pBuilding);
};
