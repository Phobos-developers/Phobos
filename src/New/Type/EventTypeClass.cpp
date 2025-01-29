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
EventTypeClass* EventTypeClass::WhenIntercept = nullptr;
EventTypeClass* EventTypeClass::WhenIntercepted = nullptr;
EventTypeClass* EventTypeClass::WhenLoad = nullptr;
EventTypeClass* EventTypeClass::WhenUnload = nullptr;
EventTypeClass* EventTypeClass::WhenBoard = nullptr;
EventTypeClass* EventTypeClass::WhenUnboard = nullptr;
EventTypeClass* EventTypeClass::WhenUpgraded = nullptr;
#pragma endregion

#pragma region TechnoLayerEvents
EventTypeClass* EventTypeClass::EnterLayer_None = nullptr;
EventTypeClass* EventTypeClass::EnterLayer_Underground = nullptr;
EventTypeClass* EventTypeClass::EnterLayer_Surface = nullptr;
EventTypeClass* EventTypeClass::EnterLayer_Ground = nullptr;
EventTypeClass* EventTypeClass::EnterLayer_Air = nullptr;
EventTypeClass* EventTypeClass::EnterLayer_Top = nullptr;
EventTypeClass* EventTypeClass::QuitLayer_None = nullptr;
EventTypeClass* EventTypeClass::QuitLayer_Underground = nullptr;
EventTypeClass* EventTypeClass::QuitLayer_Surface = nullptr;
EventTypeClass* EventTypeClass::QuitLayer_Ground = nullptr;
EventTypeClass* EventTypeClass::QuitLayer_Air = nullptr;
EventTypeClass* EventTypeClass::QuitLayer_Top = nullptr;
#pragma endregion

#pragma region AttachedEffectEvents
EventTypeClass* EventTypeClass::WhenInitialize = nullptr;
EventTypeClass* EventTypeClass::WhenAttach = nullptr;
EventTypeClass* EventTypeClass::WhenDetach = nullptr;
EventTypeClass* EventTypeClass::WhenExpired = nullptr;
EventTypeClass* EventTypeClass::WhenRemoved = nullptr;
EventTypeClass* EventTypeClass::WhenObjectDied = nullptr;
EventTypeClass* EventTypeClass::WhenDiscarded = nullptr;
#pragma endregion

#pragma region SuperWeaponEvents
EventTypeClass* EventTypeClass::WhenLaunch = nullptr;
EventTypeClass* EventTypeClass::WhenImpact = nullptr;
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
	EventTypeClass::WhenIntercept = FindOrAllocate("WhenIntercept");
	EventTypeClass::WhenIntercepted = FindOrAllocate("WhenIntercepted");
	EventTypeClass::WhenLoad = FindOrAllocate("WhenLoad");
	EventTypeClass::WhenUnload = FindOrAllocate("WhenUnload");
	EventTypeClass::WhenBoard = FindOrAllocate("WhenBoard");
	EventTypeClass::WhenUnboard = FindOrAllocate("WhenUnboard");
	EventTypeClass::WhenUpgraded = FindOrAllocate("WhenUpgraded");
#pragma endregion

#pragma region TechnoLayerEvents
	EventTypeClass::EnterLayer_None = FindOrAllocate("EnterLayer_None");
	EventTypeClass::EnterLayer_Underground = FindOrAllocate("EnterLayer_Underground");
	EventTypeClass::EnterLayer_Surface = FindOrAllocate("EnterLayer_Surface");
	EventTypeClass::EnterLayer_Ground = FindOrAllocate("EnterLayer_Ground");
	EventTypeClass::EnterLayer_Air = FindOrAllocate("EnterLayer_Air");
	EventTypeClass::EnterLayer_Top = FindOrAllocate("EnterLayer_Top");
	EventTypeClass::QuitLayer_None = FindOrAllocate("QuitLayer_None");
	EventTypeClass::QuitLayer_Underground = FindOrAllocate("QuitLayer_Underground");
	EventTypeClass::QuitLayer_Surface = FindOrAllocate("QuitLayer_Surface");
	EventTypeClass::QuitLayer_Ground = FindOrAllocate("QuitLayer_Ground");
	EventTypeClass::QuitLayer_Air = FindOrAllocate("QuitLayer_Air");
	EventTypeClass::QuitLayer_Top = FindOrAllocate("QuitLayer_Top");
#pragma endregion

#pragma region AttachedEffectEvents
	EventTypeClass::WhenInitialize = FindOrAllocate("WhenInitialize");
	EventTypeClass::WhenAttach = FindOrAllocate("WhenAttach");
	EventTypeClass::WhenDetach = FindOrAllocate("WhenDetach");
	EventTypeClass::WhenExpired = FindOrAllocate("WhenExpired");
	EventTypeClass::WhenRemoved = FindOrAllocate("WhenRemoved");
	EventTypeClass::WhenObjectDied = FindOrAllocate("WhenObjectDied");
	EventTypeClass::WhenDiscarded = FindOrAllocate("WhenDiscarded");
#pragma endregion

#pragma region SuperWeaponEvents
	EventTypeClass::WhenLaunch = FindOrAllocate("WhenLaunch");
	EventTypeClass::WhenImpact = FindOrAllocate("WhenImpact");
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

EventTypeClass* EventTypeClass::EnterLayerEventType(Layer layer)
{
	switch (layer)
	{
	case Layer::None:
		return EventTypeClass::EnterLayer_None;
	case Layer::Underground:
		return EventTypeClass::EnterLayer_Underground;
	case Layer::Surface:
		return EventTypeClass::EnterLayer_Surface;
	case Layer::Ground:
		return EventTypeClass::EnterLayer_Ground;
	case Layer::Air:
		return EventTypeClass::EnterLayer_Air;
	case Layer::Top:
		return EventTypeClass::EnterLayer_Top;
	}
	return nullptr;
}

EventTypeClass* EventTypeClass::QuitLayerEventType(Layer layer)
{
	switch (layer)
	{
	case Layer::None:
		return EventTypeClass::QuitLayer_None;
	case Layer::Underground:
		return EventTypeClass::QuitLayer_Underground;
	case Layer::Surface:
		return EventTypeClass::QuitLayer_Surface;
	case Layer::Ground:
		return EventTypeClass::QuitLayer_Ground;
	case Layer::Air:
		return EventTypeClass::QuitLayer_Air;
	case Layer::Top:
		return EventTypeClass::QuitLayer_Top;
	}
	return nullptr;
}
