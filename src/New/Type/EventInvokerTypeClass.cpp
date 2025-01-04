#include "EventInvokerTypeClass.h"
#include <nameof/nameof.h>
#include <Ext/TechnoType/Body.h>

template<>

const char* Enumerable<EventInvokerTypeClass>::GetMainSection()
{
	return "EventInvokerTypes";
}

void EventInvokerTypeClass::LoadFromINI(CCINIClass* pINI)
{
	if (this->loaded.Get())
		return;
	this->loaded = true;

	const char* pSection = this->Name;
	if (strcmp(pSection, NONE_STR) == 0)
		return;

	INI_EX exINI(pINI);
	LoadFromINIPrivate(exINI, pSection);
}

void EventInvokerTypeClass::LoadFromINI(INI_EX& exINI)
{
	if (this->loaded.Get())
		return;
	this->loaded = true;

	const char* pSection = this->Name;
	if (strcmp(pSection, NONE_STR) == 0)
		return;

	LoadFromINIPrivate(exINI, pSection);
}

void EventInvokerTypeClass::LoadFromINIPrivate(INI_EX& exINI, const char* pSection)
{
	char tempBuffer[32];

	// read event types
	Nullable<EventTypeClass*> eventTypeNullable;
	for (size_t i = 0; ; ++i)
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "EventType%d", i);
		eventTypeNullable.Reset();
		eventTypeNullable.Read<true>(exINI, pSection, tempBuffer);
		if (eventTypeNullable.isset())
		{
			this->EventTypes.push_back(eventTypeNullable.Get());
		}
		else
		{
			break;
		}
	}

	// read single event type
	if (this->EventTypes.empty())
	{
		eventTypeNullable.Reset();
		eventTypeNullable.Read<true>(exINI, pSection, "EventType");
		if (eventTypeNullable.isset())
		{
			this->EventTypes.push_back(eventTypeNullable.Get());
		}
	}

	this->Filter = HandlerFilterClass::Parse(exINI, pSection, "Target", "Filter");
	this->NegFilter = HandlerFilterClass::Parse(exINI, pSection, "Target", "NegFilter");
}

bool EventInvokerTypeClass::CheckFilters(HouseClass* pHouse, TechnoClass* pTarget) const
{
	if (this->Filter)
	{
		if (!this->Filter.get()->Check(pHouse, pTarget, false))
		{
			return false;
		}
	}

	if (this->NegFilter)
	{
		if (!this->NegFilter.get()->Check(pHouse, pTarget, true))
		{
			return false;
		}
	}

	return true;
}

void EventInvokerTypeClass::TryExecute(HouseClass* pHouse, std::map<EventScopeType, TechnoClass*>* pParticipants) const
{
	auto pTarget = pParticipants->at(EventScopeType::Me);
	if (!CheckFilters(pHouse, pTarget))
		return;
	auto pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType());

	for (auto pEventType : EventTypes)
	{
		pTargetTypeExt->InvokeEvent(pEventType, pParticipants);
	}
}

template <typename T>
void EventInvokerTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->loaded)
		.Process(this->EventTypes)
		.Process(this->Filter)
		.Process(this->NegFilter)
		;
}

void EventInvokerTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void EventInvokerTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
