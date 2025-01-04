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
	this->EventType.Read<true>(exINI, pSection, "EventType");

	LoadForScope(exINI, pSection, EventScopeType::Me, "Me");
	LoadForScope(exINI, pSection, EventScopeType::They, "They");
}

void EventHandlerTypeClass::LoadForScope(INI_EX& exINI, const char* pSection, const EventScopeType scopeType, const char* scopeName)
{
	auto comp = HandlerCompClass::Parse(exINI, pSection, scopeType, scopeName);
	if (comp)
	{
		this->HandlerComps.push_back(std::move(comp));
	}

	LoadForExtendedScope(exINI, pSection, scopeType, EventExtendedScopeType::Transport, scopeName, "Transport");
	LoadForExtendedScope(exINI, pSection, scopeType, EventExtendedScopeType::Bunker, scopeName, "Bunker");
	LoadForExtendedScope(exINI, pSection, scopeType, EventExtendedScopeType::MindController, scopeName, "MindController");
}

void EventHandlerTypeClass::LoadForExtendedScope(INI_EX& exINI, const char* pSection, const EventScopeType scopeType, const EventExtendedScopeType extendedScopeType, const char* scopeName, const char* extendedScopeName)
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
		.Process(this->EventType)
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

void EventHandlerTypeClass::HandleEvent(std::map<EventScopeType, TechnoClass*>* pParticipants)
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

bool EventHandlerTypeClass::CheckFilters(std::map<EventScopeType, TechnoClass*>* pParticipants, EventScopeType scopeType) const
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

void EventHandlerTypeClass::ExecuteEffects(std::map<EventScopeType, TechnoClass*>* pParticipants, EventScopeType scopeType) const
{
	for (auto const& handlerComp : this->HandlerComps)
	{
		if (handlerComp.get()->ScopeType == scopeType)
		{
			handlerComp.get()->ExecuteEffects(pParticipants);
		}
	}
}
