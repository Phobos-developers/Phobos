#include "EventTypeClass.h"

template<>

const char* Enumerable<EventTypeClass>::GetMainSection()
{
	return "EventTypes";
}

#pragma region TechnoEvents
EventTypeClass* EventTypeClass::WhenCreated = nullptr;
EventTypeClass* EventTypeClass::WhenCaptured = nullptr;
EventTypeClass* EventTypeClass::WhenPromoted = nullptr;
EventTypeClass* EventTypeClass::WhenDemoted = nullptr;
EventTypeClass* EventTypeClass::WhenProduce = nullptr;
EventTypeClass* EventTypeClass::WhenProduced = nullptr;
EventTypeClass* EventTypeClass::WhenGrind = nullptr;
EventTypeClass* EventTypeClass::WhenGrinded = nullptr;
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
#pragma endregion

#pragma region AttachedEffectEvents
EventTypeClass* EventTypeClass::WhenAttach = nullptr;
EventTypeClass* EventTypeClass::WhenDetach = nullptr;
EventTypeClass* EventTypeClass::WhenExpired = nullptr;
EventTypeClass* EventTypeClass::WhenRemoved = nullptr;
EventTypeClass* EventTypeClass::WhenObjectDied = nullptr;
EventTypeClass* EventTypeClass::WhenDiscarded = nullptr;
#pragma endregion

void EventTypeClass::AddDefaults()
{
#pragma region TechnoEvents
	EventTypeClass::WhenCreated = FindOrAllocate("WhenCreated");
	EventTypeClass::WhenCaptured = FindOrAllocate("WhenCaptured");
	EventTypeClass::WhenPromoted = FindOrAllocate("WhenPromoted");
	EventTypeClass::WhenDemoted = FindOrAllocate("WhenDemoted");
	EventTypeClass::WhenProduce = FindOrAllocate("WhenProduce");
	EventTypeClass::WhenProduced = FindOrAllocate("WhenProduced");
	EventTypeClass::WhenGrind = FindOrAllocate("WhenGrind");
	EventTypeClass::WhenGrinded = FindOrAllocate("WhenGrinded");
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
#pragma endregion

#pragma region AttachedEffectEvents
	EventTypeClass::WhenAttach = FindOrAllocate("WhenAttach");
	EventTypeClass::WhenDetach = FindOrAllocate("WhenDetach");
	EventTypeClass::WhenExpired = FindOrAllocate("WhenExpired");
	EventTypeClass::WhenRemoved = FindOrAllocate("WhenRemoved");
	EventTypeClass::WhenObjectDied = FindOrAllocate("WhenObjectDied");
	EventTypeClass::WhenDiscarded = FindOrAllocate("WhenDiscarded");
#pragma endregion
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
