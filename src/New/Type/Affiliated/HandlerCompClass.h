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

	static std::unique_ptr<HandlerCompClass> Parse(INI_EX& exINI, const char* pSection, EventScopeType ScopeType);

	static std::unique_ptr<HandlerCompClass> Parse(INI_EX& exINI, const char* pSection, EventScopeType ScopeType, EventExtendedScopeType ExtendedScopeType);

	void LoadFromINI(INI_EX& exINI, const char* pSection);

	bool IsDefined() const;

	TechnoClass* GetTrueTarget(TechnoClass* pTarget) const;
	bool CheckFilters(TechnoClass* pOwner, TechnoClass* pTarget) const;
	void ExecuteEffects(TechnoClass* pOwner, TechnoClass* pTarget) const;

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;
private:
	template <typename T>
	bool Serialize(T& stm);
};
#pragma once
