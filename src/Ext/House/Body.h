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
	class ExtData final : public Extension<HouseClass>
	{
	public:
		std::map<BuildingTypeExt::ExtData*, int> BuildingCounter;
		std::map<BuildingClass*, BuildingExt::ExtData*> OwnedLimboDeliveredBuildings;
		std::vector<TechnoExt::ExtData*> OwnedTimedAutoDeathObjects;

		BuildingClass* Factory_BuildingType;
		BuildingClass* Factory_InfantryType;
		BuildingClass* Factory_VehicleType;
		BuildingClass* Factory_NavyType;
		BuildingClass* Factory_AircraftType;

		ExtData(HouseClass* OwnerObject) : Extension<HouseClass>(OwnerObject)
			, BuildingCounter {}
			, OwnedLimboDeliveredBuildings {}
			, OwnedTimedAutoDeathObjects {}
			, Factory_BuildingType { nullptr }
			, Factory_InfantryType { nullptr }
			, Factory_VehicleType { nullptr }
			, Factory_NavyType { nullptr }
			, Factory_AircraftType { nullptr }
		{ }

		bool OwnsLimboDeliveredBuilding(BuildingClass* pBuilding);
		void UpdateAutoDeathObjectsInLimbo();

		virtual ~ExtData() = default;

		//virtual void Initialize() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			AnnounceInvalidPointer(Factory_AircraftType, ptr);
			AnnounceInvalidPointer(Factory_NavyType, ptr);
			AnnounceInvalidPointer(Factory_InfantryType, ptr);
			AnnounceInvalidPointer(Factory_VehicleType, ptr);
			AnnounceInvalidPointer(Factory_BuildingType, ptr);
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<HouseExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static int ActiveHarvesterCount(HouseClass* pThis);
	static int TotalHarvesterCount(HouseClass* pThis);
	static HouseClass* GetHouseKind(OwnerHouseKind kind, bool allowRandom, HouseClass* pDefault, HouseClass* pInvoker = nullptr, HouseClass* pVictim = nullptr);
};
