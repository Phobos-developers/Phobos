#include "EventTypeClass.h"

template<>

const char* Enumerable<EventTypeClass>::GetMainSection()
{
	return "EventTypes";
}

void EventTypeClass::AddDefaults()
{
	EventTypeClass::WhenCreated = FindOrAllocate("WhenCreated");
	EventTypeClass::WhenCaptured = FindOrAllocate("WhenCaptured");
	EventTypeClass::WhenCrush = FindOrAllocate("WhenCrush");
	EventTypeClass::WhenCrushed = FindOrAllocate("WhenCrushed");
	EventTypeClass::WhenInfiltrate = FindOrAllocate("WhenInfiltrate");
	EventTypeClass::WhenInfiltrated = FindOrAllocate("WhenInfiltrated");
	EventTypeClass::WhenLoad = FindOrAllocate("WhenLoad");
	EventTypeClass::WhenUnload = FindOrAllocate("WhenUnload");
	EventTypeClass::WhenBoard = FindOrAllocate("WhenBoard");
	EventTypeClass::WhenUnboard = FindOrAllocate("WhenUnboard");
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
