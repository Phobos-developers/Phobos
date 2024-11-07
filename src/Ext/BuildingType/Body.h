#pragma once
#include <BuildingTypeClass.h>
#include <SuperClass.h>
#include <SuperWeaponTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class BuildingTypeExt
{
public:
	using base_type = BuildingTypeClass;

	static constexpr DWORD Canary = 0x11111111;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<BuildingTypeClass>
	{
	public:
		Valueable<AffectedHouse> PowersUp_Owner;
		ValueableVector<BuildingTypeClass*> PowersUp_Buildings;
		ValueableIdxVector<SuperWeaponTypeClass> SuperWeapons;

		ValueableVector<BuildingTypeClass*> PowerPlantEnhancer_Buildings;
		Valueable<int> PowerPlantEnhancer_Amount;
		Nullable<float> PowerPlantEnhancer_Factor;

		std::vector<Point2D> OccupierMuzzleFlashes;
		Valueable<bool> Powered_KillSpawns;
		Nullable<bool> AllowAirstrike;
		Valueable<bool> CanC4_AllowZeroDamage;
		Valueable<bool> Refinery_UseStorage;
		Valueable<PartialVector2D<double>> InitialStrength_Cloning;
		Valueable<bool> ExcludeFromMultipleFactoryBonus;

		ValueableIdx<VocClass> Grinding_Sound;
		Valueable<WeaponTypeClass*> Grinding_Weapon;
		Valueable<int> Grinding_Weapon_RequiredCredits;
		ValueableVector<TechnoTypeClass*> Grinding_AllowTypes;
		ValueableVector<TechnoTypeClass*> Grinding_DisallowTypes;
		Valueable<bool> Grinding_AllowAllies;
		Valueable<bool> Grinding_AllowOwner;
		Valueable<bool> Grinding_PlayDieSound;

		Nullable<bool> DisplayIncome;
		Nullable<AffectedHouse> DisplayIncome_Houses;
		Valueable<Point2D> DisplayIncome_Offset;

		Valueable<bool> PlacementPreview;
		TheaterSpecificSHP PlacementPreview_Shape;
		Nullable<int> PlacementPreview_ShapeFrame;
		Valueable<CoordStruct> PlacementPreview_Offset;
		Valueable<bool> PlacementPreview_Remap;
		CustomPalette PlacementPreview_Palette;
		Nullable<TranslucencyLevel> PlacementPreview_Translucency;

		Valueable<bool> SpyEffect_Custom;
		ValueableIdx<SuperWeaponTypeClass> SpyEffect_VictimSuperWeapon;
		ValueableIdx<SuperWeaponTypeClass> SpyEffect_InfiltratorSuperWeapon;

		Nullable<bool> ConsideredVehicle;
		Valueable<bool> ZShapePointMove_OnBuildup;
		Valueable<int> SellBuildupLength;
		Valueable<bool> IsDestroyableObstacle;

		std::vector<std::optional<DirType>> AircraftDockingDirs;

		ValueableVector<TechnoTypeClass*> FactoryPlant_AllowTypes;
		ValueableVector<TechnoTypeClass*> FactoryPlant_DisallowTypes;

		Nullable<double> Units_RepairRate;
		Nullable<int> Units_RepairStep;
		Nullable<double> Units_RepairPercent;
		Nullable<bool> Units_UseRepairCost;

		Valueable<bool> NoBuildAreaOnBuildup;
		ValueableVector<BuildingTypeClass*> Adjacent_Allowed;
		ValueableVector<BuildingTypeClass*> Adjacent_Disallowed;

		ExtData(BuildingTypeClass* OwnerObject) : Extension<BuildingTypeClass>(OwnerObject)
			, PowersUp_Owner { AffectedHouse::Owner }
			, PowersUp_Buildings {}
			, PowerPlantEnhancer_Buildings {}
			, PowerPlantEnhancer_Amount { 0 }
			, PowerPlantEnhancer_Factor { 1.0 }
			, OccupierMuzzleFlashes()
			, Powered_KillSpawns { false }
			, AllowAirstrike {}
			, CanC4_AllowZeroDamage { false }
			, InitialStrength_Cloning { { 1.0, 0.0 } }
			, ExcludeFromMultipleFactoryBonus { false }
			, Refinery_UseStorage { false }
			, Grinding_AllowAllies { false }
			, Grinding_AllowOwner { true }
			, Grinding_AllowTypes {}
			, Grinding_DisallowTypes {}
			, Grinding_Sound {}
			, Grinding_PlayDieSound { true }
			, Grinding_Weapon {}
			, Grinding_Weapon_RequiredCredits { 0 }
			, DisplayIncome { }
			, DisplayIncome_Houses { }
			, DisplayIncome_Offset { { 0,0 } }
			, PlacementPreview { true }
			, PlacementPreview_Shape {}
			, PlacementPreview_ShapeFrame {}
			, PlacementPreview_Remap { true }
			, PlacementPreview_Offset { {0,-15,1} }
			, PlacementPreview_Palette {}
			, PlacementPreview_Translucency {}
			, SpyEffect_Custom { false }
			, SpyEffect_VictimSuperWeapon {}
			, SpyEffect_InfiltratorSuperWeapon {}
			, ConsideredVehicle {}
			, ZShapePointMove_OnBuildup { false }
			, SellBuildupLength { 23 }
			, AircraftDockingDirs {}
			, FactoryPlant_AllowTypes {}
			, FactoryPlant_DisallowTypes {}
			, IsDestroyableObstacle { false }
			, Units_RepairRate {}
			, Units_RepairStep {}
			, Units_RepairPercent {}
			, Units_UseRepairCost {}
			, NoBuildAreaOnBuildup { false }
			, Adjacent_Allowed {}
			, Adjacent_Disallowed {}
		{ }

		// Ares 0.A functions
		int GetSuperWeaponCount() const;
		int GetSuperWeaponIndex(int index, HouseClass* pHouse) const;
		int GetSuperWeaponIndex(int index) const;

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void CompleteInitialization();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BuildingTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool Load(BuildingTypeClass* pThis, IStream* pStm) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static int GetEnhancedPower(BuildingClass* pBuilding, HouseClass* pHouse);
	static bool CanUpgrade(BuildingClass* pBuilding, BuildingTypeClass* pUpgradeType, HouseClass* pUpgradeOwner);
	static int GetUpgradesAmount(BuildingTypeClass* pBuilding, HouseClass* pHouse);
};
