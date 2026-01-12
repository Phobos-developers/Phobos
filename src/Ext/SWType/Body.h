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
	Valueable<bool> SW_AITargeting_PsyDom_SkipChecks;
	Valueable<bool> SW_AITargeting_PsyDom_AllowAir;
	Valueable<bool> SW_AITargeting_PsyDom_AllowInvulnerable;
	ValueableVector<TechnoTypeClass*> SW_AITargeting_PsyDom_AllowTypes;
	ValueableVector<TechnoTypeClass*> SW_AITargeting_PsyDom_DisallowTypes;
	Valueable<bool> SW_AITargeting_Random_SnapOnDesignators;
	Valueable<bool> SW_AITargeting_Random_PickFirstDesignator;
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
	Nullable<AffectedTarget> SW_AIRequiresTarget;
	Nullable<AffectedHouse> SW_AIRequiresHouse;

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
		, SW_AITargeting_PsyDom_SkipChecks { false }
		, SW_AITargeting_PsyDom_AllowAir { false }
		, SW_AITargeting_PsyDom_AllowInvulnerable { false }
		, SW_AITargeting_PsyDom_AllowTypes {}
		, SW_AITargeting_PsyDom_DisallowTypes {}
		, SW_AITargeting_Random_SnapOnDesignators { false }
		, SW_AITargeting_Random_PickFirstDesignator { false }
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
		, SW_AIRequiresTarget {}
		, SW_AIRequiresHouse {}
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

	void ApplyLinkedSW(SuperClass* pSW);

	void ApplyActivatedMessage(SuperClass* pSW) const;
	void ApplyActivatedEva(SuperClass* pSW) const;

	bool PickDesignatorCell(HouseClass* pOwner, bool pickFirst, CellStruct& targetCell, bool& isSuccessful) const;

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

	static SuperClass* CurrentAIEvaluatedSW;

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
	static bool EligibleTargetForPsyDomSW(TechnoClass* pTechno);
	static bool HandleAITargetingOverrides(SuperClass* pSuper, SuperWeaponAITargetingMode aiTargetingType, CellStruct& targetCell, bool& isSuccessful);
};

