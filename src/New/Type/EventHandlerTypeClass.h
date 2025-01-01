#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include "EventTypeClass.h"
#include "Affiliated/HandlerCompClass.h"

class EventHandlerTypeClass final : public Enumerable<EventHandlerTypeClass>
{
public:
	Valueable<bool> loaded;
	Valueable<EventTypeClass*> EventType;
	std::vector<std::unique_ptr<HandlerCompClass>> HandlerComps;

	EventHandlerTypeClass(const char* pTitle = NONE_STR) : Enumerable<EventHandlerTypeClass>(pTitle)
		, loaded { false }
		, EventType {}
		, HandlerComps {}
	{};

	void LoadFromINI(CCINIClass* pINI);
	void LoadForScope(INI_EX& exINI, const char* pSection, const EventScopeType scopeType, const char* scopeName);
	void LoadForExtendedScope(INI_EX& exINI, const char* pSection, const EventScopeType scopeType, const EventExtendedScopeType extendedScopeType, const char* scopeName, const char* extendedScopeName);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	void HandleEvent(TechnoClass* pOwner, std::map<EventScopeType, TechnoClass*> participants);

private:
	template <typename T>
	void Serialize(T& Stm);

	bool CheckFilters(EventScopeType scopeType, TechnoClass* pOwner, TechnoClass* pTarget) const;
	void ExecuteEffects(EventScopeType scopeType, TechnoClass* pOwner, TechnoClass* pTarget) const;
};
