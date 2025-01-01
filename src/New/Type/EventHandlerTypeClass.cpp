#include "EventHandlerTypeClass.h"
#include <nameof/nameof.h>

template<>

const char* Enumerable<EventHandlerTypeClass>::GetMainSection()
{
	return "EventHandlerTypes";
}

void EventHandlerTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;
	if (strcmp(pSection, NONE_STR) == 0)
		return;

	INI_EX exINI(pINI);

	this->EventType.Read<true>(exINI, pSection, "EventType");

	LoadForScope(exINI, pSection, EventScopeType::Me);
	LoadForScope(exINI, pSection, EventScopeType::They);
}

void EventHandlerTypeClass::LoadForScope(INI_EX& exINI, const char* pSection, const EventScopeType scopeType)
{
	auto comp = HandlerCompClass::Parse(exINI, pSection, scopeType);
	if (comp)
	{
		this->HandlerComps.push_back(std::move(comp));
	}

	LoadForExtendedScope(exINI, pSection, scopeType, EventExtendedScopeType::Transport);
}

void EventHandlerTypeClass::LoadForExtendedScope(INI_EX& exINI, const char* pSection, const EventScopeType scopeType, const EventExtendedScopeType extendedScopeType)
{
	auto comp = HandlerCompClass::Parse(exINI, pSection, scopeType, extendedScopeType);
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
