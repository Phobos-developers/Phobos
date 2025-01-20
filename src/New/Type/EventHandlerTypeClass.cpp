#include "EventHandlerTypeClass.h"
#include <nameof/nameof.h>

template<>

const char* Enumerable<EventHandlerTypeClass>::GetMainSection()
{
	return "EventHandlerTypes";
}

void EventHandlerTypeClass::LoadFromINI(CCINIClass* pINI)
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

void EventHandlerTypeClass::LoadFromINI(INI_EX& exINI)
{
	if (this->loaded.Get())
		return;
	this->loaded = true;

	const char* pSection = this->Name;
	if (strcmp(pSection, NONE_STR) == 0)
		return;

	LoadFromINIPrivate(exINI, pSection);
}

void EventHandlerTypeClass::LoadFromINIPrivate(INI_EX& exINI, const char* pSection)
{
	LoadForActor(exINI, pSection, EventActorType::Me, "Me");
	LoadForActor(exINI, pSection, EventActorType::They, "They");
	LoadForActor(exINI, pSection, EventActorType::Enchanter, "Enchanter");
	LoadForActor(exINI, pSection, EventActorType::Scoper, "Scoper");
	this->Next.Read<true>(exINI, pSection, "Next");
	if (Next.isset())
	{
		Next.Get()->LoadFromINI(exINI);
	}
}

void EventHandlerTypeClass::LoadForActor(INI_EX& exINI, const char* pSection, const EventActorType actorType, const char* actorName)
{
	auto comp = HandlerCompClass::Parse(exINI, pSection, actorType, actorName);
	if (comp)
	{
		this->HandlerComps.push_back(std::move(comp));
	}

	LoadForExtendedActor(exINI, pSection, actorType, EventExtendedActorType::Owner, actorName, "Owner");
	LoadForExtendedActor(exINI, pSection, actorType, EventExtendedActorType::Transport, actorName, "Transport");
	LoadForExtendedActor(exINI, pSection, actorType, EventExtendedActorType::Bunker, actorName, "Bunker");
	LoadForExtendedActor(exINI, pSection, actorType, EventExtendedActorType::MindController, actorName, "MindController");
	LoadForExtendedActor(exINI, pSection, actorType, EventExtendedActorType::Parasite, actorName, "Parasite");
	LoadForExtendedActor(exINI, pSection, actorType, EventExtendedActorType::Host, actorName, "Host");
}

void EventHandlerTypeClass::LoadForExtendedActor(INI_EX& exINI, const char* pSection, const EventActorType actorType, const EventExtendedActorType extendedActorType, const char* actorName, const char* extendedActorName)
{
	auto comp = HandlerCompClass::Parse(exINI, pSection, actorType, extendedActorType, actorName, extendedActorName);
	if (comp)
	{
		this->HandlerComps.push_back(std::move(comp));
	}
}

template <typename T>
void EventHandlerTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->loaded)
		.Process(this->HandlerComps)
		.Process(this->Next)
		;
}

void EventHandlerTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void EventHandlerTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

void EventHandlerTypeClass::LoadTypeListFromINI(INI_EX& exINI, const char* pSection, const char* pHeader, ValueableVector<EventHandlerTypeClass*>* vec)
{
	char tempBuffer[32];

	// read event handlers
	Nullable<EventHandlerTypeClass*> eventHandlerNullable;
	for (size_t i = 0; ; ++i)
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s%d", pHeader, i);
		eventHandlerNullable.Reset();
		eventHandlerNullable.Read<true>(exINI, pSection, tempBuffer);
		if (eventHandlerNullable.isset())
		{
			eventHandlerNullable.Get()->LoadFromINI(exINI);
			vec->push_back(eventHandlerNullable.Get());
		}
		else
		{
			break;
		}
	}

	// read single event handler
	if (vec->empty())
	{
		eventHandlerNullable.Reset();
		eventHandlerNullable.Read<true>(exINI, pSection, pHeader);
		if (eventHandlerNullable.isset())
		{
			eventHandlerNullable.Get()->LoadFromINI(exINI);
			vec->push_back(eventHandlerNullable.Get());
		}
	}
}

void EventHandlerTypeClass::LoadTypeMapFromINI(INI_EX & exINI, const char* pSection, const char* pHeader, PhobosMap<EventTypeClass*, std::vector<EventHandlerTypeClass*>>*map)
{
	char tempBuffer[32];

	// read event handlers
	Nullable<EventTypeClass*> eventTypeNullable;
	Nullable<EventHandlerTypeClass*> eventHandlerNullable;
	for (size_t i = 0; ; ++i)
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s%d.EventType", pHeader, i);
		eventTypeNullable.Reset();
		eventTypeNullable.Read<true>(exINI, pSection, tempBuffer);
		if (eventTypeNullable.isset())
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s%d.EventHandler", pHeader, i);
			eventHandlerNullable.Reset();
			eventHandlerNullable.Read<true>(exINI, pSection, tempBuffer);
			if (eventHandlerNullable.isset())
			{
				eventHandlerNullable.Get()->LoadFromINI(exINI);
				map->operator[](eventTypeNullable.Get()).push_back(eventHandlerNullable.Get());
			}
		}
		else
		{
			break;
		}
	}

	// read single event handler
	if (map->empty())
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.EventType", pHeader);
		eventTypeNullable.Reset();
		eventTypeNullable.Read<true>(exINI, pSection, tempBuffer);
		if (eventTypeNullable.isset())
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.EventHandler", pHeader);
			eventHandlerNullable.Reset();
			eventHandlerNullable.Read<true>(exINI, pSection, tempBuffer);
			if (eventHandlerNullable.isset())
			{
				eventHandlerNullable.Get()->LoadFromINI(exINI);
				map->operator[](eventTypeNullable.Get()).push_back(eventHandlerNullable.Get());
			}
		}
	}
}

void EventHandlerTypeClass::HandleEvent(std::map<EventActorType, AbstractClass*>* pParticipants)
{
	bool passedFilters = true;
	for (auto it = pParticipants->begin(); it != pParticipants->end(); ++it)
	{
		if (!CheckFilters(pParticipants, it->first))
		{
			passedFilters = false;
			break;
		}
	}

	if (passedFilters)
	{
		for (auto it = pParticipants->begin(); it != pParticipants->end(); ++it)
		{
			ExecuteEffects(pParticipants, it->first);
		}
	}

	if (Next.isset())
	{
		Next.Get()->HandleEvent(pParticipants);
	}
}

bool EventHandlerTypeClass::CheckFilters(std::map<EventActorType, AbstractClass*>* pParticipants, EventActorType actorType) const
{
	for (auto const& handlerComp : this->HandlerComps)
	{
		if (handlerComp.get()->ActorType == actorType)
		{
			if (!handlerComp.get()->CheckFilters(pParticipants))
			{
				return false;
			}
		}
	}

	return true;
}

void EventHandlerTypeClass::ExecuteEffects(std::map<EventActorType, AbstractClass*>* pParticipants, EventActorType actorType) const
{
	for (auto const& handlerComp : this->HandlerComps)
	{
		if (handlerComp.get()->ActorType == actorType)
		{
			handlerComp.get()->ExecuteEffects(pParticipants);
		}
	}
}
