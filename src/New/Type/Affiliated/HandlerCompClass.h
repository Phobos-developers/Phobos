#pragma once

#include <Utilities/Template.h>
#include "HandlerFilterClass.h"
#include "HandlerEffectClass.h"

class HandlerCompClass
{
public:
	HandlerCompClass();

	Valueable<EventScopeType> ScopeType;
	Nullable<EventExtendedScopeType> ExtendedScopeType;
	std::unique_ptr<HandlerFilterClass> Filter;
	std::unique_ptr<HandlerFilterClass> NegFilter;
	std::unique_ptr<HandlerEffectClass> Effect;

	static std::unique_ptr<HandlerCompClass> Parse(INI_EX& exINI, const char* pSection, EventScopeType ScopeType, const char* scopeName);

	static std::unique_ptr<HandlerCompClass> Parse(INI_EX& exINI, const char* pSection, EventScopeType ScopeType, EventExtendedScopeType ExtendedScopeType, const char* scopeName, const char* extendedScopeName);

	void LoadFromINI(INI_EX& exINI, const char* pSection, const char* scopeName, const char* extendedScopeName);

	bool IsDefined() const;

	static TechnoClass* GetTrueTarget(TechnoClass* pTarget, Nullable<EventExtendedScopeType> ExtendedScopeType);
	static TechnoClass* GetTransportingTechno(TechnoClass* pTarget);
	bool CheckFilters(std::map<EventScopeType, TechnoClass*>* pParticipants, TechnoClass* pOwner, TechnoClass* pTarget) const;
	void ExecuteEffects(std::map<EventScopeType, TechnoClass*>* pParticipants, TechnoClass* pOwner, TechnoClass* pTarget) const;

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;
private:
	template <typename T>
	bool Serialize(T& stm);
};
#pragma once
