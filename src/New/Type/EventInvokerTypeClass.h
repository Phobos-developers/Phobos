#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include "EventTypeClass.h"
#include "Affiliated/HandlerFilterClass.h"

class EventHandlerTypeClass;

class EventInvokerTypeClass final : public Enumerable<EventInvokerTypeClass>
{
public:
	Valueable<bool> loaded;
	ValueableVector<EventTypeClass*> EventTypes;
	std::unique_ptr<HandlerFilterClass> Filter;
	std::unique_ptr<HandlerFilterClass> NegFilter;
	ValueableVector<EventHandlerTypeClass*> ExtraEventHandlers;
	Valueable<bool> Target_PassDown_Passengers;
	Valueable<bool> Target_PassDown_MindControlled;

	EventInvokerTypeClass(const char* pTitle = NONE_STR) : Enumerable<EventInvokerTypeClass>(pTitle)
		, loaded { false }
		, EventTypes {}
		, Filter {}
		, NegFilter {}
		, ExtraEventHandlers {}
		, Target_PassDown_Passengers { false }
		, Target_PassDown_MindControlled { false }
	{};

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromINI(INI_EX& exINI);
	void TryExecute(HouseClass* pHouse, std::map<EventScopeType, TechnoClass*>* pParticipants);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	static void LoadTypeListFromINI(INI_EX& exINI, const char* pSection, const char* pHeader, ValueableVector<EventInvokerTypeClass*>* vec);
private:
	template <typename T>
	void Serialize(T& Stm);
	void LoadFromINIPrivate(INI_EX& exINI, const char* pSection);
	bool CheckFilters(HouseClass* pHouse, TechnoClass* pTarget) const;
	void TryExecuteSingle(HouseClass* pHouse, std::map<EventScopeType, TechnoClass*>* pParticipants, TechnoClass* pTarget);
	void TryPassDown(HouseClass* pHouse, std::map<EventScopeType, TechnoClass*>* pParticipants, TechnoClass* pRoot);
};
