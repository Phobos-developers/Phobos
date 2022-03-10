#pragma once
#include <BuildingTypeClass.h>
#include <SuperWeaponTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class BuildingTypeExt
{
public:
	using base_type = BuildingTypeClass;

	class ExtData final : public Extension<BuildingTypeClass>
	{
	public:
		Valueable<AffectedHouse> PowersUp_Owner;
		ValueableVector<BuildingTypeClass*> PowersUp_Buildings;
		DynamicVectorClass<SuperWeaponTypeClass*> SuperWeapons;

		ValueableVector<BuildingTypeClass*> PowerPlantEnhancer_Buildings;
		Nullable<int> PowerPlantEnhancer_Amount;
		Nullable<float> PowerPlantEnhancer_Factor;

		DynamicVectorClass<Point2D> OccupierMuzzleFlashes;

		Valueable<bool> Refinery_UseStorage;

		Valueable<bool> Grinding_AllowAllies;
		Valueable<bool> Grinding_AllowOwner;
		ValueableVector<TechnoTypeClass*> Grinding_AllowTypes;
		ValueableVector<TechnoTypeClass*> Grinding_DisallowTypes;
		NullableIdx<VocClass> Grinding_Sound;
		Nullable<WeaponTypeClass*> Grinding_Weapon;

		Valueable<bool> PackupSound_PlayGlobal;
		Valueable<bool> DisableDamageSound;
		Nullable<float> BuildingOccupyDamageMult;
		Nullable<float> BuildingOccupyROFMult;
		Nullable<float> BuildingBunkerDamageMult;
		Nullable<float> BuildingBunkerROFMult;
		NullableIdx<VocClass> BunkerWallsUpSound;
		NullableIdx<VocClass> BunkerWallsDownSound;

		ExtData(BuildingTypeClass* OwnerObject) : Extension<BuildingTypeClass>(OwnerObject)
			, PowersUp_Owner{ AffectedHouse::Owner }
			, PowersUp_Buildings{}
			, PowerPlantEnhancer_Buildings{}
			, PowerPlantEnhancer_Amount{}
			, PowerPlantEnhancer_Factor{}
			, OccupierMuzzleFlashes()
			, Refinery_UseStorage{ false }
			, Grinding_AllowAllies{ false }
			, Grinding_AllowOwner{ true }
			, Grinding_AllowTypes{}
			, Grinding_DisallowTypes{}
			, Grinding_Sound{}
			, Grinding_Weapon{}
			, PackupSound_PlayGlobal { false }
			, DisableDamageSound { false }
			, BuildingOccupyDamageMult {}
			, BuildingOccupyROFMult {}
			, BuildingBunkerDamageMult {}
			, BuildingBunkerROFMult {}
			, BunkerWallsUpSound {}
			, BunkerWallsDownSound {}
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass * pINI) override;
		virtual void Initialize() override;
		virtual void CompleteInitialization();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {
		}

		virtual void LoadFromStream(PhobosStreamReader & Stm) override;
		virtual void SaveToStream(PhobosStreamWriter & Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BuildingTypeExt> {
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
	static bool CanGrindTechno(BuildingClass* pBuilding, TechnoClass* pTechno);
};