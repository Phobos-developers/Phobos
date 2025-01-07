#include "EventTypeClass.h"

template<>

const char* Enumerable<EventTypeClass>::GetMainSection()
{
	return "EventTypes";
}

EventTypeClass* EventTypeClass::WhenCreated = nullptr;
EventTypeClass* EventTypeClass::WhenCaptured = nullptr;
EventTypeClass* EventTypeClass::WhenPromoted = nullptr;
EventTypeClass* EventTypeClass::WhenDemoted = nullptr;
EventTypeClass* EventTypeClass::WhenKill = nullptr;
EventTypeClass* EventTypeClass::WhenKilled = nullptr;
EventTypeClass* EventTypeClass::WhenCrush = nullptr;
EventTypeClass* EventTypeClass::WhenCrushed = nullptr;
EventTypeClass* EventTypeClass::WhenInfiltrate = nullptr;
EventTypeClass* EventTypeClass::WhenInfiltrated = nullptr;
EventTypeClass* EventTypeClass::WhenLoad = nullptr;
EventTypeClass* EventTypeClass::WhenUnload = nullptr;
EventTypeClass* EventTypeClass::WhenBoard = nullptr;
EventTypeClass* EventTypeClass::WhenUnboard = nullptr;
EventTypeClass* EventTypeClass::WhenUpgraded = nullptr;

void EventTypeClass::AddDefaults()
{
	EventTypeClass::WhenCreated = FindOrAllocate("WhenCreated");
	EventTypeClass::WhenCaptured = FindOrAllocate("WhenCaptured");
	EventTypeClass::WhenPromoted = FindOrAllocate("WhenPromoted");
	EventTypeClass::WhenDemoted = FindOrAllocate("WhenDemoted");
	EventTypeClass::WhenKill = FindOrAllocate("WhenKill");
	EventTypeClass::WhenKilled = FindOrAllocate("WhenKilled");
	EventTypeClass::WhenCrush = FindOrAllocate("WhenCrush");
	EventTypeClass::WhenCrushed = FindOrAllocate("WhenCrushed");
	EventTypeClass::WhenInfiltrate = FindOrAllocate("WhenInfiltrate");
	EventTypeClass::WhenInfiltrated = FindOrAllocate("WhenInfiltrated");
	EventTypeClass::WhenLoad = FindOrAllocate("WhenLoad");
	EventTypeClass::WhenUnload = FindOrAllocate("WhenUnload");
	EventTypeClass::WhenBoard = FindOrAllocate("WhenBoard");
	EventTypeClass::WhenUnboard = FindOrAllocate("WhenUnboard");
	EventTypeClass::WhenUpgraded = FindOrAllocate("WhenUpgraded");
}

void EventTypeClass::LoadFromINI(CCINIClass* pINI)
{
}

template <typename T>
void EventTypeClass::Serialize(T& Stm)
{
}

void EventTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void EventTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

void EventTypeClass::LoadTypeListFromINI(INI_EX& exINI, const char* pSection, const char* pHeader, ValueableVector<EventTypeClass*>* vec)
{
	char tempBuffer[32];

	Nullable<EventTypeClass*> eventTypeNullable;
	for (size_t i = 0; ; ++i)
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s%d", pHeader, i);
		eventTypeNullable.Reset();
		eventTypeNullable.Read<true>(exINI, pSection, tempBuffer);
		if (eventTypeNullable.isset())
		{
			vec->push_back(eventTypeNullable.Get());
		}
		else
		{
			break;
		}
	}

	// read single event type
	if (vec->empty())
	{
		eventTypeNullable.Reset();
		eventTypeNullable.Read<true>(exINI, pSection, pHeader);
		if (eventTypeNullable.isset())
		{
			vec->push_back(eventTypeNullable.Get());
		}
	}
}
