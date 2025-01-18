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
	LoadForScope(exINI, pSection, EventActorType::Me, "Me");
	LoadForScope(exINI, pSection, EventActorType::They, "They");
}

void EventHandlerTypeClass::LoadForScope(INI_EX& exINI, const char* pSection, const EventActorType scopeType, const char* scopeName)
{
	auto comp = HandlerCompClass::Parse(exINI, pSection, scopeType, scopeName);
	if (comp)
	{
		this->HandlerComps.push_back(std::move(comp));
	}

	LoadForExtendedScope(exINI, pSection, scopeType, EventExtendedActorType::Owner, scopeName, "Owner");
	LoadForExtendedScope(exINI, pSection, scopeType, EventExtendedActorType::Transport, scopeName, "Transport");
	LoadForExtendedScope(exINI, pSection, scopeType, EventExtendedActorType::Bunker, scopeName, "Bunker");
	LoadForExtendedScope(exINI, pSection, scopeType, EventExtendedActorType::MindController, scopeName, "MindController");
	LoadForExtendedScope(exINI, pSection, scopeType, EventExtendedActorType::Parasite, scopeName, "Parasite");
	LoadForExtendedScope(exINI, pSection, scopeType, EventExtendedActorType::Host, scopeName, "Host");
}

void EventHandlerTypeClass::LoadForExtendedScope(INI_EX& exINI, const char* pSection, const EventActorType scopeType, const EventExtendedActorType extendedScopeType, const char* scopeName, const char* extendedScopeName)
{
	auto comp = HandlerCompClass::Parse(exINI, pSection, scopeType, extendedScopeType, scopeName, extendedScopeName);
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
	for (auto it = pParticipants->begin(); it != pParticipants->end(); ++it)
	{
		if (!CheckFilters(pParticipants, it->first))
			return;
	}

	for (auto it = pParticipants->begin(); it != pParticipants->end(); ++it)
	{
		ExecuteEffects(pParticipants, it->first);
	}
}

bool EventHandlerTypeClass::CheckFilters(std::map<EventActorType, AbstractClass*>* pParticipants, EventActorType scopeType) const
{
	for (auto const& handlerComp : this->HandlerComps)
	{
		if (handlerComp.get()->ScopeType == scopeType)
		{
			if (!handlerComp.get()->CheckFilters(pParticipants))
			{
				return false;
			}
		}
	}

	return true;
}

void EventHandlerTypeClass::ExecuteEffects(std::map<EventActorType, AbstractClass*>* pParticipants, EventActorType scopeType) const
{
	for (auto const& handlerComp : this->HandlerComps)
	{
		if (handlerComp.get()->ScopeType == scopeType)
		{
			handlerComp.get()->ExecuteEffects(pParticipants);
		}
	}
}
