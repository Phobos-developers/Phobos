#pragma once

#include <New/Type/AttachEffectTypeClass.h>
#include <New/Type/ShieldTypeClass.h>

class AttachEffectTypeClass;

class PlayerEmblemTypeClass;

class HandlerFilterClass
{
public:
	HandlerFilterClass();

#pragma region TechnoFilters
	Valueable<bool> HasAnyTechnoCheck;
	Nullable<AffectedTarget> Abstract;
	Nullable<bool> IsInAir;
	ValueableVector<TechnoTypeClass*> TechnoTypes;
	ValueableVector<AttachEffectTypeClass*> AttachedEffects;
	ValueableVector<AttachEffectTypeClass*> AEStacks_Types;
	ValueableVector<ComparatorType> AEStacks_Methods;
	ValueableVector<int> AEStacks_Values;
	ValueableVector<ShieldTypeClass*> ShieldTypes;
	Nullable<VeterancyType> Veterancy;
	Nullable<HPPercentageType> HPPercentage;
	Nullable<bool> IsPassenger;
	Nullable<bool> IsParasited;
	Nullable<bool> IsParasiting;
	Nullable<bool> IsBunkered;
	Nullable<bool> IsMindControlled;
	Nullable<bool> IsMindControlled_Perma;
	Nullable<bool> MindControlling_Any;
	ValueableVector<TechnoTypeClass*> MindControlling_Type;
	Nullable<bool> Passengers_Any;
	ValueableVector<TechnoTypeClass*> Passengers_Type;
	Nullable<bool> Upgrades_Any;
	ValueableVector<BuildingTypeClass*> Upgrades_Type;
#pragma endregion

#pragma region HouseFilters
	Valueable<bool> HasAnyHouseCheck;
	Nullable<AffectedHouse> House;
	ValueableVector<SideClass*> Sides;
	ValueableVector<HouseTypeClass*> Countries;
	ValueableVector<PlayerEmblemTypeClass*> Emblems;
	ValueableVector<BuildingTypeClass*> Buildings;
	Nullable<bool> IsHuman;
	Nullable<bool> IsAI;
	Nullable<bool> IsLowPower;
	ValueableVector<HouseParameterType> HouseCompare_Params;
	ValueableVector<ComparatorType> HouseCompare_Methods;
	ValueableVector<int> HouseCompare_Values;
#pragma endregion

	static std::unique_ptr<HandlerFilterClass> Parse(INI_EX& exINI, const char* pSection, const char* actorName, const char* filterName);

	void LoadFromINI(INI_EX& exINI, const char* pSection, const char* actorName, const char* filterName);

	bool Check(HouseClass* pHouse, AbstractClass* pTarget, bool negative = false) const;

	bool IsDefined() const;
	bool IsDefinedAnyTechnoCheck() const;
	bool IsDefinedAnyHouseCheck() const;

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;
private:
	template <typename T>
	bool Serialize(T& stm);

	bool CheckForTechno(HouseClass* pHouse, TechnoClass* pTarget, bool negative = false) const;
	bool CheckForHouse(HouseClass* pHouse, HouseClass* pTargetHouse, bool negative = false) const;
};
