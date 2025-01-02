#include "HandlerCompClass.h"

HandlerCompClass::HandlerCompClass()
	: ScopeType {}
	, ExtendedScopeType {}
	, Filter {}
	, NegFilter {}
	, Effect {}
{ }

std::unique_ptr<HandlerCompClass> HandlerCompClass::Parse(INI_EX& exINI, const char* pSection, EventScopeType ScopeType, const char* scopeName)
{
	auto handlerUnit = std::make_unique<HandlerCompClass>();
	handlerUnit.get()->ScopeType = ScopeType;
	handlerUnit.get()->LoadFromINI(exINI, pSection, scopeName, nullptr);
	if (handlerUnit.get()->IsDefined())
	{
		return handlerUnit;
	}
	else
	{
		handlerUnit.reset();
		return nullptr;
	}
}

std::unique_ptr<HandlerCompClass> HandlerCompClass::Parse(INI_EX& exINI, const char* pSection, EventScopeType ScopeType, EventExtendedScopeType ExtendedScopeType, const char* scopeName, const char* extendedScopeName)
{
	auto handlerUnit = std::make_unique<HandlerCompClass>();
	handlerUnit.get()->ScopeType = ScopeType;
	handlerUnit.get()->ExtendedScopeType = ExtendedScopeType;
	handlerUnit.get()->LoadFromINI(exINI, pSection, scopeName, extendedScopeName);
	if (handlerUnit.get()->IsDefined())
	{
		return handlerUnit;
	}
	else
	{
		handlerUnit.reset();
		return nullptr;
	}
}

void HandlerCompClass::LoadFromINI(INI_EX& exINI, const char* pSection, const char* scopeName, const char* extendedScopeName)
{
	auto localScopeName = scopeName;
	if (extendedScopeName)
	{
		char tempBuffer[32];
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s", scopeName, extendedScopeName);
		localScopeName = tempBuffer;
	}

	this->Filter = HandlerFilterClass::Parse(exINI, pSection, localScopeName, "Filter");
	this->NegFilter = HandlerFilterClass::Parse(exINI, pSection, localScopeName, "NegFilter");
	this->Effect = HandlerEffectClass::Parse(exINI, pSection, localScopeName, "Effect");
}

bool HandlerCompClass::IsDefined() const
{
	return Filter != nullptr
		|| NegFilter != nullptr
		|| Effect != nullptr;
}

TechnoClass* HandlerCompClass::GetTrueTarget(TechnoClass* pTarget) const
{
	if (pTarget && this->ExtendedScopeType.isset())
	{
		switch (this->ExtendedScopeType.Get())
		{
		case EventExtendedScopeType::Transport:
			return pTarget->Transporter;
		case EventExtendedScopeType::Bunker:
			return pTarget->BunkerLinkedItem;
		case EventExtendedScopeType::MindController:
			return pTarget->MindControlledBy;
		}
	}
	return pTarget;
}

bool HandlerCompClass::CheckFilters(TechnoClass* pOwner, TechnoClass* pTarget) const
{
	auto const pTrueTarget = this->GetTrueTarget(pTarget);

	if (this->Filter)
	{
		if (!(pTrueTarget && this->Filter.get()->Check(pOwner, pTarget, false)))
		{
			return false;
		}
	}

	if (this->NegFilter)
	{
		if (!(pTrueTarget && this->NegFilter.get()->Check(pOwner, pTarget, true)))
		{
			return false;
		}
	}

	return true;
}

void HandlerCompClass::ExecuteEffects(TechnoClass* pOwner, TechnoClass* pTarget) const
{
	auto const pTrueTarget = this->GetTrueTarget(pTarget);
	if (pTrueTarget)
	{
		if (this->Effect)
		{
			this->Effect.get()->Execute(pOwner, pTrueTarget);
		}
	}
}

bool HandlerCompClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool HandlerCompClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<HandlerCompClass*>(this)->Serialize(stm);
}

template<typename T>
bool HandlerCompClass::Serialize(T& stm)
{
	return stm
		.Process(this->ScopeType)
		.Process(this->ExtendedScopeType)
		.Process(this->Filter)
		.Process(this->NegFilter)
		.Process(this->Effect)
		.Success();
}
