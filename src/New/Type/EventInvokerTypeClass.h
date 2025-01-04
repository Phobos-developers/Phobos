#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include "EventTypeClass.h"
#include "Affiliated/HandlerFilterClass.h"

class EventInvokerTypeClass final : public Enumerable<EventInvokerTypeClass>
{
public:
	Valueable<bool> loaded;
	ValueableVector<EventTypeClass*> EventTypes;
	std::unique_ptr<HandlerFilterClass> Filter;
	std::unique_ptr<HandlerFilterClass> NegFilter;

	EventInvokerTypeClass(const char* pTitle = NONE_STR) : Enumerable<EventInvokerTypeClass>(pTitle)
		, loaded { false }
		, EventTypes {}
		, Filter {}
		, NegFilter {}
	{};

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromINI(INI_EX& exINI);
	void TryExecute(HouseClass* pHouse, std::map<EventScopeType, TechnoClass*>* pParticipants) const;
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);
private:
	template <typename T>
	void Serialize(T& Stm);
	void LoadFromINIPrivate(INI_EX& exINI, const char* pSection);
	bool CheckFilters(HouseClass* pHouse, TechnoClass* pTarget) const;
};
