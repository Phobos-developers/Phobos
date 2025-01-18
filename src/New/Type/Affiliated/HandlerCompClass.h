#pragma once

#include <Utilities/Template.h>
#include "HandlerFilterClass.h"
#include "HandlerEffectClass.h"

class HandlerEffectClass;

class HandlerCompClass
{
public:
	HandlerCompClass();

	Valueable<EventActorType> ScopeType;
	Nullable<EventExtendedActorType> ExtendedScopeType;
	std::unique_ptr<HandlerFilterClass> Filter;
	std::unique_ptr<HandlerFilterClass> NegFilter;
	std::unique_ptr<HandlerEffectClass> Effect;

	static std::unique_ptr<HandlerCompClass> Parse(INI_EX& exINI, const char* pSection, EventActorType ScopeType, const char* scopeName, bool includeEffects = true);

	static std::unique_ptr<HandlerCompClass> Parse(INI_EX& exINI, const char* pSection, EventActorType ScopeType, EventExtendedActorType ExtendedScopeType, const char* scopeName, const char* extendedScopeName, bool includeEffects = true);

	void LoadFromINI(INI_EX& exINI, const char* pSection, const char* scopeName, const char* extendedScopeName, bool includeEffects = true);

	bool IsDefined() const;

	static AbstractClass* GetTrueTarget(AbstractClass* pTarget, Nullable<EventExtendedActorType> ExtendedScopeType);
	static HouseClass* GetOwningHouseOfActor(AbstractClass* pTarget);
	static TechnoClass* GetTransportingTechno(TechnoClass* pTarget);
	static TechnoClass* GetParasiteTechno(TechnoClass* pTarget);
	static TechnoClass* GetHostTechno(TechnoClass* pTarget);
	bool CheckFilters(HouseClass* pHouse, AbstractClass* pTarget) const;
	bool CheckFilters(std::map<EventActorType, AbstractClass*>* pParticipants) const;
	void ExecuteEffects(std::map<EventActorType, AbstractClass*>* pParticipants) const;

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;
private:
	template <typename T>
	bool Serialize(T& stm);
};
#pragma once
