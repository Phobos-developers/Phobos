#pragma once

#include <Utilities/Template.h>
#include "TypeConvertGroup.h"
#include <New/Type/EventInvokerTypeClass.h>

class EventInvokerTypeClass;

class HandlerEffectClass
{
public:
	HandlerEffectClass();

	Valueable<bool> HasAnyTechnoEffect;

	Nullable<WeaponTypeClass*> Weapon;
	Nullable<EventActorType> Weapon_Firer;
	Nullable<EventExtendedActorType> Weapon_FirerExt;
	Valueable<bool> Weapon_SpawnProj;

	std::vector<TypeConvertGroup> Convert_Pairs;

	Nullable<double> Soylent_Mult;
	Valueable<bool> Soylent_IncludePassengers;
	Nullable<EventActorType> Soylent_Receptant;
	Nullable<EventExtendedActorType> Soylent_ReceptantExt;
	Valueable<bool> Soylent_Display;
	Valueable<AffectedHouse> Soylent_Display_Houses;
	Valueable<Point2D> Soylent_Display_Offset;

	Valueable<bool> Passengers_Eject;
	Valueable<bool> Passengers_Kill;
	Valueable<bool> Passengers_Kill_Score;
	Nullable<EventActorType> Passengers_Kill_Scorer;
	Nullable<EventExtendedActorType> Passengers_Kill_ScorerExt;
	ValueableVector<TechnoTypeClass*> Passengers_Create_Types;
	ValueableVector<int> Passengers_Create_Nums;
	Nullable<EventActorType> Passengers_Create_Owner;
	Nullable<EventExtendedActorType> Passengers_Create_OwnerExt;

	Nullable<VeterancyType> Veterancy_Set;
	Nullable<double> Veterancy_Add;

	NullableIdx<VocClass> Voice;
	Valueable<bool> Voice_Persist;
	Valueable<bool> Voice_Global;

	Nullable<OwnerHouseKind> Transfer_To_House;
	Nullable<EventActorType> Transfer_To_Actor;
	Nullable<EventExtendedActorType> Transfer_To_ActorExt;

	Nullable<Mission> Command;
	Nullable<EventActorType> Command_Target;
	Nullable<EventExtendedActorType> Command_TargetExt;

	Valueable<bool> HasAnyHouseEffect;

	NullableIdx<VocClass> EVA;

	Valueable<bool> HasAnyGenericEffect;

	ValueableVector<EventHandlerTypeClass*> EventHandlers;
	ValueableVector<EventInvokerTypeClass*> EventInvokers;

	static std::unique_ptr<HandlerEffectClass> Parse(INI_EX& exINI, const char* pSection, const char* actorName, const char* effectName);

	void LoadFromINI(INI_EX& exINI, const char* pSection, const char* actorName, const char* effectName);

	void Execute(std::map<EventActorType, AbstractClass*>* pParticipants, AbstractClass* pTarget) const;

	bool IsDefined() const;

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;
private:
	template <typename T>
	bool Serialize(T& stm);

	void ExecuteForTechno(AbstractClass* pOwner, HouseClass* pOwnerHouse, std::map<EventActorType, AbstractClass*>* pParticipants, TechnoClass* pTarget) const;
	void ExecuteForHouse(AbstractClass* pOwner, HouseClass* pOwnerHouse, std::map<EventActorType, AbstractClass*>* pParticipants, HouseClass* pTarget) const;
	void ExecuteGeneric(AbstractClass* pOwner, HouseClass* pOwnerHouse, std::map<EventActorType, AbstractClass*>* pParticipants, AbstractClass* pTarget) const;

	bool IsDefinedAnyTechnoEffect() const;
	bool IsDefinedAnyHouseEffect() const;
	bool IsDefinedAnyGenericEffect() const;

	void CreatePassengers(TechnoClass* pToWhom, HouseClass* pPassengerOwner) const;
	void TransferOwnership(TechnoClass* pTarget, HouseClass* pNewOnwer) const;
};
