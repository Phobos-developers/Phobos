#pragma once

#include <Utilities/Template.h>
#include "TypeConvertGroup.h"

class HandlerEffectClass
{
public:
	HandlerEffectClass();

	Nullable<WeaponTypeClass*> Weapon;
	Nullable<EventScopeType> Weapon_Firer_Scope;
	Nullable<EventExtendedScopeType> Weapon_Firer_ExtScope;
	Valueable<bool> Weapon_SpawnProj;

	std::vector<TypeConvertGroup> Convert_Pairs;

	Nullable<double> Soylent_Mult;
	Valueable<bool> Soylent_IncludePassengers;
	Nullable<EventScopeType> Soylent_Scope;
	Nullable<EventExtendedScopeType> Soylent_ExtScope;
	Valueable<bool> Soylent_Display;
	Valueable<AffectedHouse> Soylent_Display_Houses;
	Valueable<Point2D> Soylent_Display_Offset;

	Valueable<bool> Passengers_Eject;
	Valueable<bool> Passengers_Kill;
	Valueable<bool> Passengers_Kill_Score;
	Nullable<EventScopeType> Passengers_Kill_Score_Scope;
	Nullable<EventExtendedScopeType> Passengers_Kill_Score_ExtScope;
	ValueableVector<TechnoTypeClass*> Passengers_Create_Types;
	ValueableVector<int> Passengers_Create_Nums;
	Nullable<EventScopeType> Passengers_Create_Owner_Scope;
	Nullable<EventExtendedScopeType> Passengers_Create_Owner_ExtScope;

	Nullable<VeterancyType> Veterancy_Set;
	Nullable<double> Veterancy_Add;

	NullableIdx<VocClass> Voice;
	Valueable<bool> Voice_Persist;
	Valueable<bool> Voice_Global;
	NullableIdx<VocClass> EVA;

	static std::unique_ptr<HandlerEffectClass> Parse(INI_EX& exINI, const char* pSection, const char* scopeName, const char* effectName);

	void LoadFromINI(INI_EX& exINI, const char* pSection, const char* scopeName, const char* effectName);

	void Execute(std::map<EventScopeType, TechnoClass*>* pParticipants, TechnoClass* pOwner, TechnoClass* pTarget) const;

	bool IsDefined() const;

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;
private:
	template <typename T>
	bool Serialize(T& stm);

	void CreatePassengers(TechnoClass* pToWhom, TechnoClass* pPassengerOwnerScope) const;
};
