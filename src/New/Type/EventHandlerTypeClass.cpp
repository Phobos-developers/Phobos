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

void EventHandlerTypeClass::HandleEvent(TechnoClass* pOwner, std::map<EventScopeType, TechnoClass*> participants)
{
	for (auto it = participants.begin(); it != participants.end(); ++it)
	{
		auto scopeType = it->first;
		auto pTarget = it->second;
		if (!CheckFilters(scopeType, pOwner, pTarget))
			return;
	}

	for (auto it = participants.begin(); it != participants.end(); ++it)
	{
		auto scopeType = it->first;
		auto pTarget = it->second;
		ExecuteEffects(scopeType, pOwner, pTarget);
	}
}

bool EventHandlerTypeClass::CheckFilters(EventScopeType scopeType, TechnoClass* pOwner, TechnoClass* pTarget) const
{
	for (auto const& handlerUnit : this->HandlerComps)
	{
		if (handlerUnit.get()->ScopeType == scopeType)
		{
			if (!handlerUnit.get()->CheckFilters(pOwner, pTarget))
			{
				return false;
			}
		}
	}

	return true;
}

void EventHandlerTypeClass::ExecuteEffects(EventScopeType scopeType, TechnoClass* pOwner, TechnoClass* pTarget) const
{
	for (auto const& handlerUnit : this->HandlerComps)
	{
		if (handlerUnit.get()->ScopeType == scopeType)
		{
			handlerUnit.get()->ExecuteEffects(pOwner, pTarget);
		}
	}
}
