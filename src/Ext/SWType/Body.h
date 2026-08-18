#pragma once
#include <Ext/Building/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <New/Type/Affiliated/TypeConvertGroup.h>

class SWTypeExt final : public AbstractTypeExt
{
public:
	using base_type = SuperWeaponTypeClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = SWTypeExt;

	static constexpr DWORD Canary = 0x11111111;

public:
	// typed owner accessor
	SuperWeaponTypeClass* OwnerObject() const
	{
		return static_cast<SuperWeaponTypeClass*>(this->GetAttachedObject());
	}


	PhobosFixedString<0x20> TypeID;

	//Ares 0.A
	Valueable<int> Money_Amount;
	ValueableIdx<VoxClass> EVA_Impatient;
	ValueableIdx<VoxClass> EVA_InsufficientFunds;
	ValueableIdx<VoxClass> EVA_SelectTarget;
	Valueable<bool> SW_UseAITargeting;
	Valueable<bool> SW_AutoFire;
	Valueable<bool> SW_ManualFire;
	Valueable<bool> SW_ShowCameo;
	Valueable<bool> SW_Unstoppable;
	Valueable<bool> SW_AllowPlayer;
	Valueable<bool> SW_AllowAI;
	ValueableVector<TechnoTypeClass*> SW_Inhibitors;
	Valueable<bool> SW_AnyInhibitor;
	ValueableVector<TechnoTypeClass*> SW_Designators;
	Valueable<bool> SW_AnyDesignator;
	Valueable<double> SW_RangeMinimum;
	Valueable<double> SW_RangeMaximum;
	Valueable<int> SW_Shots;

	DWORD SW_RequiredHouses;
	DWORD SW_ForbiddenHouses;
	ValueableVector<BuildingTypeClass*> SW_AuxBuildings;
	ValueableVector<BuildingTypeClass*> SW_NegBuildings;
	ValueableVector<TechnoTypeClass*> SW_AuxTechnos;
	ValueableVector<TechnoTypeClass*> SW_NegTechnos;
	Valueable<int> SW_TechLevel;
	Valueable<bool> SW_InitialReady;
	ValueableIdx<SuperWeaponTypeClass> SW_PostDependent;
	Valueable<int> SW_MaxCount;

	Valueable<CSFText> Message_CannotFire;
	Valueable<CSFText> Message_InsufficientFunds;
	ValueableIdx<ColorScheme> Message_ColorScheme;
	Valueable<bool> Message_FirerColor;

	Valueable<CSFText> UIDescription;
	Valueable<int> CameoPriority;
	ValueableVector<BuildingTypeClass*> LimboDelivery_Types;
	ValueableVector<int> LimboDelivery_IDs;
	ValueableVector<float> LimboDelivery_RollChances;
	Valueable<AffectedHouse> LimboKill_AffectsHouse;
	ValueableVector<int> LimboKill_IDs;
	ValueableVector<int> LimboKill_Counts;
	Valueable<double> RandomBuffer;
	ValueableIdxVector<SuperWeaponTypeClass> SW_Next;
	Valueable<bool> SW_Next_RealLaunch;
	Valueable<bool> SW_Next_IgnoreInhibitors;
	Valueable<bool> SW_Next_IgnoreDesignators;
	ValueableVector<float> SW_Next_RollChances;

	Valueable<int> ShowTimer_Priority;
	Nullable<bool> ShowTimer_Percentage;

	Valueable<WarheadTypeClass*> Detonate_Warhead;
	Valueable<WeaponTypeClass*> Detonate_Weapon;
	Nullable<int> Detonate_Damage;
	Valueable<bool> Detonate_Warhead_Full;
	Valueable<bool> Detonate_AtFirer;
	Valueable<bool> ShowDesignatorRange;

	Valueable<int> TabIndex;

	Nullable<bool> SuperWeaponSidebar_Allow;
	DWORD SuperWeaponSidebar_PriorityHouses;
	DWORD SuperWeaponSidebar_RequiredHouses;
	Valueable<int> SuperWeaponSidebar_Significance;

	CustomPalette SidebarPal;
	PhobosPCXFile SidebarPCX;

	std::vector<ValueableVector<int>> LimboDelivery_RandomWeightsData;
	std::vector<ValueableVector<int>> SW_Next_RandomWeightsData;
	std::vector<ValueableVector<int>> SW_Link_RandomWeightsData;

	std::vector<TypeConvertGroup> Convert_Pairs;

	Valueable<bool> UseWeeds;
	Valueable<int> UseWeeds_Amount;
	Valueable<bool> UseWeeds_StorageTimer;
	Valueable<double> UseWeeds_ReadinessAnimationPercentage;

	Valueable<int> EMPulse_WeaponIndex;
	Valueable<bool> EMPulse_SuspendOthers;
	ValueableVector<BuildingTypeClass*> EMPulse_Cannons;
	Valueable<bool> EMPulse_TargetSelf;

	Nullable<bool> DropshipLoadout_OpenWindow;
	Valueable<bool> DropshipLoadout_Launch;
	Valueable<bool> DropshipLoadout_PersistentCargo;
	Nullable<bool> DropshipLoadout_PreloadCargo;
	Nullable<bool> DropshipLoadout_AddUnusedMoneyToPlayer;
	Valueable<bool> DropshipLoadout_RememberPurchasedCargo;
	ConvertClass* DropshipLoadout_Palette;
	Nullable<TechnoTypeClass*> DropshipLoadout_Carrier;
	ValueableVector<TechnoTypeClass*> DropshipLoadout_AllowableUnits;
	ValueableVector<int> DropshipLoadout_AllowableUnitMaximums;
	Nullable<int> DropshipLoadout_Money;
	Nullable<int> DropshipLoadout_Theme;
	NullableIdx<VoxClass> DropshipLoadout_StartEVA;
	Nullable<int> DropshipLoadout_SizeLimit;
	Nullable<PhobosPCXFile> DropshipLoadout_BackgroundPCX;
	std::string DropshipLoadout_BackgroundPCXPattern;
	Nullable<PhobosPCXFile> DropshipLoadout_UpArrowPCX;
	Nullable<PhobosPCXFile> DropshipLoadout_DownArrowPCX;
	Nullable<Point2D> DropshipLoadout_UpArrowLocation;
	Nullable<Point2D> DropshipLoadout_DownArrowLocation;
	Nullable<int> DropshipLoadout_SidebarCameosCount;
	ValueableVector<Point2D> DropshipLoadout_SidebarCameosLocations;
	Nullable<PhobosPCXFile> DropshipLoadout_PilotLitPCX;
	Nullable<Point2D> DropshipLoadout_PilotLitLocation;
	Nullable<PhobosPCXFile> DropshipLoadout_LoadoutPCX;
	Nullable<Point2D> DropshipLoadout_LoadoutLocation;
	ValueableVector<PhobosPCXFile> DropshipLoadout_DGreenListPCX;
	Nullable<SHPStruct*> DropshipLoadout_Background;
	Nullable<SHPStruct*> DropshipLoadout_UpArrow;
	Nullable<SHPStruct*> DropshipLoadout_DownArrow;
	Nullable<SHPStruct*> DropshipLoadout_Loadout;
	Nullable<SHPStruct*> DropshipLoadout_PilotLit;
	std::vector<SHPStruct*> DropshipLoadout_DGreenList;
	Nullable<int> DropshipLoadout_DGreenAnimationsCount;
	ValueableVector<Point2D> DropshipLoadout_DGreenLocations;
	Nullable<int> DropshipLoadout_DropshipCameosCount;
	ValueableVector<Point2D> DropshipLoadout_DropshipCameosLocations;
	ValueableVector<TechnoTypeClass*> DropshipLoadout_FixedUnits;
	ValueableVector<TechnoTypeClass*> DropshipLoadout_InitialUnits;
	NullableIdx<VoxClass> DropshipLoadout_BuyClickSound;
	NullableIdx<VoxClass> DropshipLoadout_SellClickSound;
	NullableIdx<VoxClass> DropshipLoadout_ArrowsClickSound;
	NullableIdx<VoxClass> DropshipLoadout_StartingDragDropSound;
	NullableIdx<VoxClass> DropshipLoadout_EndingDragDropSound;
	Nullable<int> DropshipLoadout_VeteranLevel;

	ValueableIdxVector<SuperWeaponTypeClass> SW_Link;
	Valueable<bool> SW_Link_Grant;
	Valueable<bool> SW_Link_Ready;
	Valueable<bool> SW_Link_Reset;
	ValueableVector<float> SW_Link_RollChances;
	Valueable<CSFText> Message_LinkedSWAcquired;
	NullableIdx<VoxClass> EVA_LinkedSWAcquired;
	Valueable<CSFText> Message_Activated_Owner;
	Valueable<CSFText> Message_Activated_Allies;
	Valueable<CSFText> Message_Activated_Enemies;
	ValueableIdx<VoxClass> EVA_Activated_Owner;
	ValueableIdx<VoxClass> EVA_Activated_Allies;
	ValueableIdx<VoxClass> EVA_Activated_Enemies;

	SWTypeExt(SuperWeaponTypeClass* OwnerObject) : AbstractTypeExt(OwnerObject)
		, TypeID { "" }
		, Money_Amount { 0 }
		, EVA_Impatient { -1 }
		, EVA_InsufficientFunds { -1 }
		, EVA_SelectTarget { -1 }
		, SW_UseAITargeting { false }
		, SW_AutoFire { false }
		, SW_ManualFire { true }
		, SW_ShowCameo { true }
		, SW_Unstoppable { false }
		, SW_AllowPlayer { true }
		, SW_AllowAI { true }
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
		, SW_AuxTechnos {}
		, SW_NegTechnos {}
		, SW_TechLevel { 0 }
		, SW_InitialReady { false }
		, SW_PostDependent {}
		, SW_MaxCount { -1 }
		, SW_Shots { -1 }
		, Message_CannotFire {}
		, Message_InsufficientFunds {}
		, Message_ColorScheme { -1 }
		, Message_FirerColor { false }
		, UIDescription {}
		, CameoPriority { 0 }
		, LimboDelivery_Types {}
		, LimboDelivery_IDs {}
		, LimboDelivery_RollChances {}
		, LimboDelivery_RandomWeightsData {}
		, LimboKill_AffectsHouse { AffectedHouse::Owner }
		, LimboKill_IDs {}
		, LimboKill_Counts {}
		, RandomBuffer { 0.0 }
		, Detonate_Warhead {}
		, Detonate_Weapon {}
		, Detonate_Damage {}
		, Detonate_Warhead_Full { true }
		, Detonate_AtFirer { false }
		, SW_Next {}
		, SW_Next_RealLaunch { true }
		, SW_Next_IgnoreInhibitors { false }
		, SW_Next_IgnoreDesignators { true }
		, SW_Next_RollChances {}
		, SW_Next_RandomWeightsData {}
		, ShowTimer_Priority { 0 }
		, ShowTimer_Percentage { false }
		, Convert_Pairs {}
		, ShowDesignatorRange { true }
		, TabIndex { 1 }
		, SuperWeaponSidebar_Allow {}
		, SuperWeaponSidebar_PriorityHouses { 0u }
		, SuperWeaponSidebar_RequiredHouses { 0xFFFFFFFFu }
		, SuperWeaponSidebar_Significance { 0 }
		, SidebarPal {}
		, SidebarPCX {}
		, UseWeeds { false }
		, UseWeeds_Amount { RulesClass::Instance->WeedCapacity }
		, UseWeeds_StorageTimer { false }
		, UseWeeds_ReadinessAnimationPercentage { 0.9 }
		, EMPulse_WeaponIndex { 0 }
		, EMPulse_SuspendOthers { false }
		, EMPulse_Cannons {}
		, EMPulse_TargetSelf { false }
		, SW_Link {}
		, SW_Link_Grant { false }
		, SW_Link_Ready { false }
		, SW_Link_Reset { false }
		, SW_Link_RollChances {}
		, SW_Link_RandomWeightsData {}
		, Message_LinkedSWAcquired {}
		, EVA_LinkedSWAcquired {}
		, Message_Activated_Owner {}
		, Message_Activated_Allies {}
		, Message_Activated_Enemies {}
		, EVA_Activated_Owner { -1 }
		, EVA_Activated_Allies { -1 }
		, EVA_Activated_Enemies { -1 }
		, DropshipLoadout_OpenWindow {}
		, DropshipLoadout_Launch { false }
		, DropshipLoadout_PersistentCargo { false }
		, DropshipLoadout_PreloadCargo {}
		, DropshipLoadout_AddUnusedMoneyToPlayer {}
		, DropshipLoadout_RememberPurchasedCargo { false }
		, DropshipLoadout_Palette { nullptr }
		, DropshipLoadout_Carrier {}
		, DropshipLoadout_AllowableUnits {}
		, DropshipLoadout_AllowableUnitMaximums {}
		, DropshipLoadout_Money {}
		, DropshipLoadout_Theme {}
		, DropshipLoadout_StartEVA {}
		, DropshipLoadout_SizeLimit {}
		, DropshipLoadout_BackgroundPCX {}
		, DropshipLoadout_BackgroundPCXPattern { "" }
		, DropshipLoadout_UpArrowPCX {}
		, DropshipLoadout_DownArrowPCX {}
		, DropshipLoadout_UpArrowLocation {}
		, DropshipLoadout_DownArrowLocation {}
		, DropshipLoadout_SidebarCameosCount {}
		, DropshipLoadout_SidebarCameosLocations {}
		, DropshipLoadout_PilotLitPCX {}
		, DropshipLoadout_PilotLitLocation {}
		, DropshipLoadout_LoadoutPCX {}
		, DropshipLoadout_LoadoutLocation {}
		, DropshipLoadout_DGreenListPCX {}
		, DropshipLoadout_Background {}
		, DropshipLoadout_UpArrow {}
		, DropshipLoadout_DownArrow {}
		, DropshipLoadout_Loadout {}
		, DropshipLoadout_PilotLit {}
		, DropshipLoadout_DGreenList {}
		, DropshipLoadout_DGreenAnimationsCount {}
		, DropshipLoadout_DGreenLocations {}
		, DropshipLoadout_DropshipCameosCount {}
		, DropshipLoadout_DropshipCameosLocations {}
		, DropshipLoadout_FixedUnits {}
		, DropshipLoadout_InitialUnits {}
		, DropshipLoadout_BuyClickSound {}
		, DropshipLoadout_SellClickSound {}
		, DropshipLoadout_ArrowsClickSound {}
		, DropshipLoadout_StartingDragDropSound {}
		, DropshipLoadout_EndingDragDropSound {}
		, DropshipLoadout_VeteranLevel {}
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
	void PrintMessage(const CSFText& message, HouseClass* pFirer) const;

	void ApplyLimboDelivery(HouseClass* pHouse);
	void ApplyLimboKill(HouseClass* pHouse);
	void ApplyDetonation(HouseClass* pHouse, const CellStruct& cell);
	void ApplySWNext(SuperClass* pSW, const CellStruct& cell);
	void ApplyTypeConversion(SuperClass* pSW);
	void HandleEMPulseLaunch(SuperClass* pSW, const CellStruct& cell) const;
	std::vector<BuildingClass*> GetEMPulseCannons(HouseClass* pOwner, const CellStruct& cell) const;
	std::pair<double, double> GetEMPulseCannonRange(BuildingClass* pBuilding) const;

	void ApplyDropshipLoadoutLaunch(HouseClass* pHouse, const CellStruct& cell);
	void ApplyLinkedSW(SuperClass* pSW);

	void ApplyActivatedMessage(SuperClass* pSW) const;
	void ApplyActivatedEva(SuperClass* pSW) const;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;
	virtual void Initialize() override;

	virtual ~SWTypeExt() = default;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;

	virtual void SaveToStream(PhobosStreamWriter& Stm) override;
private:
	std::vector<int> WeightedRollsHandler(ValueableVector<float>* chances, std::vector<ValueableVector<int>>* weights, size_t size);

	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<SWTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static void FireSuperWeaponExt(SuperClass* pSW, const CellStruct& cell);

	static ExtContainer ExtMap;

	static SWTypeExt* Fetch(const SuperWeaponTypeClass* pThis)
	{
		return AbstractExt::Fetch<SWTypeExt>(pThis);
	}

	static SWTypeExt* TryFetch(const SuperWeaponTypeClass* pThis)
	{
		return AbstractExt::TryFetch<SWTypeExt>(pThis);
	}
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool Activate(SuperClass* pSuper, CellStruct cell, bool isPlayer);
	static SuperClass* __stdcall IsSuperAvailable(int swIdx, HouseClass* pHouse);

};

