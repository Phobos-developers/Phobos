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

void EventHandlerTypeClass::LoadForScope(INI_EX& exINI, const char* pSection, const EventScopeType scope)
{
	auto const scopeName = nameof::customize::enum_name(scope).data();
	this->Filter.insert(scope, HandlerFilterClass::Parse(this, exINI, pSection, scopeName, "Filter"));
	this->Transport_Filter.insert(scope, HandlerFilterClass::Parse(this, exINI, pSection, scopeName, "Transport.Filter"));
	this->NegFilter.insert(scope, HandlerFilterClass::Parse(this, exINI, pSection, scopeName, "NegFilter"));
	this->Transport_NegFilter.insert(scope, HandlerFilterClass::Parse(this, exINI, pSection, scopeName, "Transport.NegFilter"));
	this->Effect.insert(scope, HandlerEffectClass::Parse(this, exINI, pSection, scopeName, "Effect"));
	this->Transport_Effect.insert(scope, HandlerEffectClass::Parse(this, exINI, pSection, scopeName, "Transport.Effect"));
}

template <typename T>
void EventHandlerTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->EventType)
		.Process(this->Filter)
		.Process(this->Transport_Filter)
		.Process(this->NegFilter)
		.Process(this->Transport_NegFilter)
		.Process(this->Effect)
		.Process(this->Transport_Effect)
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
		auto scope = it->first;
		auto pTarget = it->second;
		if (!CheckFilters(pOwner, scope, pTarget))
			return;
	}
}

bool EventHandlerTypeClass::CheckFilters(TechnoClass* pOwner, EventScopeType scope, TechnoClass* pTarget) const
{
	std::unique_ptr<HandlerFilterClass> filter;

	// check positive filter
	if (filter = this->Filter.get_or_default(scope, nullptr))
	{
		if (!pTarget || !filter.get()->Check(pOwner, pTarget, false))
			return false;
	}

	// check positive transport filter
	if (filter = this->Transport_Filter.get_or_default(scope, nullptr))
	{
		if (!pTarget || !pTarget->Transporter || !filter.get()->Check(pOwner, pTarget->Transporter, false))
			return false;
	}

	// check negative filter
	if (filter = this->NegFilter.get_or_default(scope, nullptr))
	{
		if (!pTarget || !filter.get()->Check(pOwner, pTarget, true))
			return false;
	}

	// check negative transport filter
	if (filter = this->Transport_NegFilter.get_or_default(scope, nullptr))
	{
		if (!pTarget || !pTarget->Transporter || !filter.get()->Check(pOwner, pTarget->Transporter, true))
			return false;
	}

	return true;
}

void EventHandlerTypeClass::ExecuteEffects(TechnoClass* pOwner, EventScopeType scope, TechnoClass* pTarget) const
{
	std::unique_ptr<HandlerEffectClass> effect;

	// execute effect
	if (effect = this->Effect.get_or_default(scope, nullptr))
	{
		if (pTarget)
		{
			effect.get()->Execute(pOwner, pTarget);
		}
	}

	// execute transport effect
	if (effect = this->Transport_Effect.get_or_default(scope, nullptr))
	{
		if (pTarget && pTarget->Transporter)
		{
			effect.get()->Execute(pOwner, pTarget->Transporter);
		}
	}
}
