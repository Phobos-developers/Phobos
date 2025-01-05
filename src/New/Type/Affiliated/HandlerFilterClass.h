#pragma once

#include <New/Type/AttachEffectTypeClass.h>
#include <New/Type/ShieldTypeClass.h>

class HandlerFilterClass
{
public:
	HandlerFilterClass();

	Nullable<AffectedTarget> Abstract;
	Nullable<bool> IsInAir;
	ValueableVector<TechnoTypeClass*> TechnoTypes;
	ValueableVector<AttachEffectTypeClass*> AttachedEffects;
	ValueableVector<ShieldTypeClass*> ShieldTypes;
	Nullable<VeterancyType> Veterancy;
	Nullable<HPPercentageType> HPPercentage;

	Nullable<bool> IsBunkered;
	Nullable<bool> IsMindControlled;
	Nullable<bool> IsMindControlled_Perma;
	Nullable<bool> MindControlling_Any;
	ValueableVector<TechnoTypeClass*> MindControlling_Type;
	Nullable<bool> Passengers_Any;
	ValueableVector<TechnoTypeClass*> Passengers_Type;
	Nullable<bool> Upgrades_Any;
	ValueableVector<BuildingTypeClass*> Upgrades_Type;

	Nullable<AffectedHouse> Owner_House;
	ValueableVector<SideClass*> Owner_Sides;
	ValueableVector<HouseTypeClass*> Owner_Countries;
	ValueableVector<BuildingTypeClass*> Owner_Buildings;
	Nullable<bool> Owner_IsHuman;
	Nullable<bool> Owner_IsAI;

	static std::unique_ptr<HandlerFilterClass> Parse(INI_EX& exINI, const char* pSection, const char* scopeName, const char* filterName);

	void LoadFromINI(INI_EX& exINI, const char* pSection, const char* scopeName, const char* filterName);

	bool Check(HouseClass* pHouse, TechnoClass* pTarget, bool negative = false) const;
	bool CheckForHouse(HouseClass* pHouse, HouseClass* pTargetHouse, bool negative = false) const;

	bool IsDefined() const;
	bool IsDefinedAnyTechnoCheck() const;
	bool IsDefinedAnyHouseCheck() const;

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;
private:
	template <typename T>
	bool Serialize(T& stm);
};
