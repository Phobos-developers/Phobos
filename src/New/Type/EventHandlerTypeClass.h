#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include "EventTypeClass.h"
#include "Affiliated/HandlerCompClass.h"

class EventHandlerTypeClass final : public Enumerable<EventHandlerTypeClass>
{
public:
	Valueable<EventTypeClass*> EventType;
	std::vector<std::unique_ptr<HandlerCompClass>> HandlerComps;

	EventHandlerTypeClass(const char* pTitle = NONE_STR) : Enumerable<EventHandlerTypeClass>(pTitle)
		, EventType {}
		, HandlerComps {}
	{};

	void LoadFromINI(CCINIClass* pINI);
	void LoadForScope(INI_EX& exINI, const char* pSection, const EventScopeType scopeType);
	void LoadForExtendedScope(INI_EX& exINI, const char* pSection, const EventScopeType scopeType, const EventExtendedScopeType extendedScopeType);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	void HandleEvent(TechnoClass* pOwner, std::map<EventScopeType, TechnoClass*> participants);

private:
	template <typename T>
	void Serialize(T& Stm);

	bool CheckFilters(EventScopeType scopeType, TechnoClass* pOwner, TechnoClass* pTarget) const;
	void ExecuteEffects(EventScopeType scopeType, TechnoClass* pOwner, TechnoClass* pTarget) const;
};
