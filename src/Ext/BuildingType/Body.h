#pragma once
#include <Ext/TechnoType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <BuildingTypeClass.h>

class BuildingTypeExt final : public TechnoTypeExt
{
public:
	using base_type = BuildingTypeClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = BuildingTypeExt;

	static constexpr DWORD Canary = 0x11111111;

public:
	Valueable<AffectedHouse> PowersUp_Owner;
	ValueableVector<BuildingTypeClass*> PowersUp_Buildings;
	ValueableIdxVector<SuperWeaponTypeClass> SuperWeapons;

	Valueable<double> PowerPlant_DamageFactor;
	ValueableVector<BuildingTypeClass*> PowerPlantEnhancer_Buildings;
	Valueable<Leptons> PowerPlantEnhancer_Range;
	Valueable<int> PowerPlantEnhancer_Amount;
	Nullable<float> PowerPlantEnhancer_Factor;
	Valueable<int> PowerPlantEnhancer_MaxCount;

	std::vector<Point2D> OccupierMuzzleFlashes;
	Valueable<bool> Powered_KillSpawns;
	Valueable<bool> CanC4_AllowZeroDamage;
	Valueable<bool> Refinery_UseStorage;
	Valueable<PartialVector2D<double>> InitialStrength_Cloning;
	Valueable<bool> Cloning_Powered;
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
	Nullable<int> DisplayIncome_Delay;
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
	Nullable<bool> Explodes_DuringBuildup;

	Valueable<bool> IsAnimDelayedBurst;

	std::vector<std::optional<DirType>> AircraftDockingDirs;
		Nullable<bool> AircraftDockingDir_DefaultToPoseDir;

	ValueableVector<TechnoTypeClass*> FactoryPlant_AllowTypes;
	ValueableVector<TechnoTypeClass*> FactoryPlant_DisallowTypes;
	Valueable<int> FactoryPlant_MaxCount;

	Nullable<double> Units_RepairRate;
	Nullable<int> Units_RepairStep;
	Nullable<double> Units_RepairPercent;
	Nullable<bool> Units_UseRepairCost;

	Valueable<bool> NoBuildAreaOnBuildup;
	Nullable<bool> NoAlphaImageOnBuildup;
	ValueableVector<BuildingTypeClass*> Adjacent_Allowed;
	ValueableVector<BuildingTypeClass*> Adjacent_Disallowed;
	Valueable<bool> Adjacent_Disallowed_Prohibit;
	Valueable<int> Adjacent_Disallowed_ProhibitDistance;

	Nullable<Point2D> BarracksExitCell;

	Valueable<int> Overpower_KeepOnline;
	Valueable<int> Overpower_ChargeWeapon;

	Valueable<bool> DisableDamageSound;
	Nullable<float> BuildingOccupyDamageMult;
	Nullable<float> BuildingOccupyROFMult;
	Nullable<float> BuildingBunkerDamageMult;
	Nullable<float> BuildingBunkerROFMult;
	NullableIdx<VocClass> BunkerWallsUpSound;
	NullableIdx<VocClass> BunkerWallsDownSound;
	Nullable<int> BunkerStateUpdateDelay;

	NullableIdx<VocClass> BuildingRepairedSound;

	Valueable<bool> Refinery_UseNormalActiveAnim;

	ValueableVector<bool> HasPowerUpAnim;

	Valueable<bool> UndeploysInto_Sellable;

	Nullable<bool> BuildingRadioLink_SyncOwner;

	Nullable<PartialVector2D<int>> GuardRetryDelay;

	Valueable<int> TurretAnim_IdleFrames;
	Valueable<int> TurretAnim_LowPowerIdleFrames;
	Valueable<int> TurretAnim_FiringFrames;
	Valueable<int> TurretAnim_LowPowerFiringFrames;
	Valueable<int> TurretAnim_IdleRate;
	Valueable<int> TurretAnim_FiringRate;

	Nullable<int> StartFacing;
	Nullable<bool> StartFacing_Random;

	Valueable<int> SetTabBySelecting;

	Nullable<int> RevealToAll_Radius;

	Nullable<int> DeployFireDelay;

	Valueable<AnimTypeClass*> RoofProductionAnim;
	Valueable<AnimTypeClass*> RoofProductionAnimDamaged;
	Valueable<AnimTypeClass*> RoofProductionAnimGarrisoned;
	Nullable<int> RoofProductionAnimX;
	Nullable<int> RoofProductionAnimY;
	Nullable<int> RoofProductionAnimZAdjust;
	Nullable<int> RoofProductionAnimYSort;
	Nullable<bool> RoofProductionAnimPowered;
	Nullable<bool> RoofProductionAnimPoweredLight;
	Nullable<bool> RoofProductionAnimPoweredEffect;
	Nullable<bool> RoofProductionAnimPoweredSpecial;

	// Ares 0.2
	Valueable<bool> CloningFacility;

	// Ares 0.A
	Valueable<BuildingTypeClass*> RubbleIntact;
	Valueable<bool> RubbleIntactRemove;

	// Ares 0.E
	Valueable<bool> Tunnel; // temporarily bool: Ares stores TunnelType name (string -> index), not mapped here

	// Ares 3.0
	Nullable<bool> UnitSell;

	Nullable<int> NewEvaVoice_Index;
	Valueable<int> NewEvaVoice_Priority;
	Valueable<bool> NewEvaVoice_RecheckOnDeath;
	NullableIdx<VoxClass> NewEvaVoice_InitialMessage;

	BuildingTypeExt(BuildingTypeClass* OwnerObject) : TechnoTypeExt(OwnerObject)
		, PowersUp_Owner { AffectedHouse::Owner }
		, PowersUp_Buildings {}
		, PowerPlant_DamageFactor { 1.0 }
		, PowerPlantEnhancer_Buildings {}
		, PowerPlantEnhancer_Range { Leptons(0) }
		, PowerPlantEnhancer_Amount { 0 }
		, PowerPlantEnhancer_Factor { 1.0f }
		, PowerPlantEnhancer_MaxCount { -1 }
		, OccupierMuzzleFlashes()
		, Powered_KillSpawns { false }
		, CanC4_AllowZeroDamage { false }
		, InitialStrength_Cloning { { 1.0 } }
		, Cloning_Powered { true }
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
		, DisplayIncome_Delay { }
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
		, FactoryPlant_MaxCount { -1 }
		, IsAnimDelayedBurst { true }
		, IsDestroyableObstacle { false }
		, Explodes_DuringBuildup {}
		, Units_RepairRate {}
		, Units_RepairStep {}
		, Units_RepairPercent {}
		, Units_UseRepairCost {}
		, NoBuildAreaOnBuildup { false }
		, NoAlphaImageOnBuildup {}
		, Adjacent_Allowed {}
		, Adjacent_Disallowed {}
		, Adjacent_Disallowed_Prohibit { false }
		, Adjacent_Disallowed_ProhibitDistance { 0 }
		, BarracksExitCell {}
		, Overpower_KeepOnline { 2 }
		, Overpower_ChargeWeapon { 1 }
		, DisableDamageSound { false }
		, BuildingOccupyDamageMult {}
		, BuildingOccupyROFMult {}
		, BuildingBunkerDamageMult {}
		, BuildingBunkerROFMult {}
		, BunkerWallsUpSound {}
		, BunkerWallsDownSound {}
		, BunkerStateUpdateDelay {}
		, BuildingRepairedSound {}
		, Refinery_UseNormalActiveAnim { false }
		, HasPowerUpAnim {}
		, UndeploysInto_Sellable { false }
		, BuildingRadioLink_SyncOwner {}
		, GuardRetryDelay {}
		, TurretAnim_IdleFrames { 1 }
		, TurretAnim_LowPowerIdleFrames { 0 }
		, TurretAnim_FiringFrames { 0 }
		, TurretAnim_LowPowerFiringFrames { 0 }
		, TurretAnim_IdleRate { 1 }
		, TurretAnim_FiringRate { 1 }
		, StartFacing{}
		, StartFacing_Random{}
		, SetTabBySelecting { -1 }
		, RevealToAll_Radius {}
		, DeployFireDelay {}

		, RoofProductionAnim { nullptr }
		, RoofProductionAnimDamaged { nullptr }
		, RoofProductionAnimGarrisoned { nullptr }
		, RoofProductionAnimX {}
		, RoofProductionAnimY {}
		, RoofProductionAnimZAdjust {}
		, RoofProductionAnimYSort {}
		, RoofProductionAnimPowered { }
		, RoofProductionAnimPoweredLight { }
		, RoofProductionAnimPoweredEffect { }
		, RoofProductionAnimPoweredSpecial { }

		// Ares 0.2
		, CloningFacility { false }

		// Ares 0.A
		, RubbleIntact { nullptr }
		, RubbleIntactRemove { false }

		// Ares 0.E
		, Tunnel { false }

		// Ares 3.0
		, UnitSell {}

		, NewEvaVoice_Index {}
		, NewEvaVoice_Priority { 0 }
		, NewEvaVoice_RecheckOnDeath { false }
		, NewEvaVoice_InitialMessage { }
	{ }

	// typed owner accessor (shadows the TechnoTypeClass one from the base)
	BuildingTypeClass* OwnerObject() const
	{
		return static_cast<BuildingTypeClass*>(this->TechnoTypeExt::OwnerObject());
	}

	// Ares 0.A functions
	int GetSuperWeaponCount() const;
	int GetSuperWeaponIndex(int index, HouseClass* pHouse) const;
	int GetSuperWeaponIndex(int index) const;

	virtual ~BuildingTypeExt() = default;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;
	virtual void Initialize() override;
	virtual void CompleteInitialization();

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:

	class ExtContainer final : public Container<BuildingTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static BuildingTypeExt* Fetch(const BuildingTypeClass* pThis)
	{
		return AbstractExt::Fetch<BuildingTypeExt>(pThis);
	}

	static BuildingTypeExt* TryFetch(const BuildingTypeClass* pThis)
	{
		return AbstractExt::TryFetch<BuildingTypeExt>(pThis);
	}
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void PlayBunkerSound(BuildingClass const* pThis, bool buildUp = false);
	static bool IsPoweredAnimBlocked(BuildingClass* pBuilding, bool powered, bool poweredLight, bool poweredEffect, bool poweredSpecial);

	static std::pair<int, int> GetEnhancedPower(BuildingTypeClass* pBuilding, int output, HouseClass* pHouse, BuildingClass* pPowerPlant = nullptr);
	static bool CanUpgrade(BuildingClass* pBuilding, BuildingTypeClass* pUpgradeType, HouseClass* pUpgradeOwner);
	static int CountOwnedNowWithDeployOrUpgrade(BuildingTypeClass* pBuilding, HouseClass* pHouse);
	static int GetUpgradesAmount(BuildingTypeClass* pBuilding, HouseClass* pHouse);
};

