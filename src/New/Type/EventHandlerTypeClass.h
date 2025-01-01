#pragma once
#include <set>
#include <unordered_map>

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDef.h>
#include "EventTypeClass.h"
#include "Affiliated/HandlerFilterClass.h"
#include "Affiliated/HandlerEffectClass.h"

class EventHandlerTypeClass final : public Enumerable<EventHandlerTypeClass>
{
public:
	Valueable<EventTypeClass*> EventType;
	std::map<EventScopeType, std::unique_ptr<HandlerFilterClass>> Filter;
	std::map<EventScopeType, std::unique_ptr<HandlerFilterClass>> Transport_Filter;
	std::map<EventScopeType, std::unique_ptr<HandlerFilterClass>> NegFilter;
	std::map<EventScopeType, std::unique_ptr<HandlerFilterClass>> Transport_NegFilter;
	std::map<EventScopeType, std::unique_ptr<HandlerEffectClass>> Effect;
	std::map<EventScopeType, std::unique_ptr<HandlerEffectClass>> Transport_Effect;

	EventHandlerTypeClass(const char* pTitle = NONE_STR) : Enumerable<EventHandlerTypeClass>(pTitle)
		, EventType {}
		, Filter {}
		, Transport_Filter {}
		, NegFilter {}
		, Transport_NegFilter {}
		, Effect {}
		, Transport_Effect {}
	{};

	void LoadFromINI(CCINIClass* pINI);
	void LoadForScope(INI_EX& exINI, const char* pSection, const EventScopeType scope);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	void HandleEvent(TechnoClass* pOwner, std::map<EventScopeType, TechnoClass*> participants);

private:
	template <typename T>
	void Serialize(T& Stm);

	bool CheckFilters(TechnoClass* pOwner, EventScopeType scope, TechnoClass* pTarget) const;
	void ExecuteEffects(TechnoClass* pOwner, EventScopeType scope, TechnoClass* pTarget) const;
};
