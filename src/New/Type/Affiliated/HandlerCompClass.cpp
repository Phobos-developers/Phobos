#include "HandlerCompClass.h"
#include <Ext/Techno/Body.h>

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

TechnoClass* HandlerCompClass::GetTrueTarget(TechnoClass* pTarget, Nullable<EventExtendedScopeType> ExtendedScopeType)
{
	if (pTarget && ExtendedScopeType.isset())
	{
		switch (ExtendedScopeType.Get())
		{
		case EventExtendedScopeType::Transport:
			return GetTransportingTechno(pTarget);
		case EventExtendedScopeType::Bunker:
			return pTarget->BunkerLinkedItem;
		case EventExtendedScopeType::MindController:
			return pTarget->MindControlledBy;
		case EventExtendedScopeType::Parasite:
			return GetParasiteTechno(pTarget);
		case EventExtendedScopeType::Host:
			return GetHostTechno(pTarget);
		}
	}
	return pTarget;
}

TechnoClass* HandlerCompClass::GetTransportingTechno(TechnoClass* pTarget)
{
	if (pTarget->Transporter)
	{
		return pTarget->Transporter;
	}
	auto pTargetExt = TechnoExt::ExtMap.Find(pTarget);
	if (pTargetExt->HousingMe)
	{
		return pTargetExt->HousingMe;
	}
	return nullptr;
}

TechnoClass* HandlerCompClass::GetParasiteTechno(TechnoClass* pTarget)
{
	return reinterpret_cast<FootClass*>(pTarget)->ParasiteEatingMe;
}

TechnoClass* HandlerCompClass::GetHostTechno(TechnoClass* pTarget)
{
	auto pFoot = reinterpret_cast<FootClass*>(pTarget);
	if (pFoot->ParasiteImUsing)
	{
		return pFoot->ParasiteImUsing->Victim;
	}
	return nullptr;
}

bool HandlerCompClass::CheckFilters(std::map<EventScopeType, TechnoClass*>* pParticipants) const
{
	auto pOwner = pParticipants->at(EventScopeType::Me);
	auto pTarget = pParticipants->at(EventScopeType::They);
	auto const pTrueTarget = HandlerCompClass::GetTrueTarget(pTarget, this->ExtendedScopeType);

	if (this->Filter)
	{
		if (!(pTrueTarget && this->Filter.get()->Check(pOwner->Owner, pTrueTarget, false)))
		{
			return false;
		}
	}

	if (this->NegFilter)
	{
		if (!(pTrueTarget && this->NegFilter.get()->Check(pOwner->Owner, pTrueTarget, true)))
		{
			return false;
		}
	}

	return true;
}

void HandlerCompClass::ExecuteEffects(std::map<EventScopeType, TechnoClass*>* pParticipants) const
{
	auto pTarget = pParticipants->at(EventScopeType::They);
	auto const pTrueTarget = HandlerCompClass::GetTrueTarget(pTarget, this->ExtendedScopeType);

	if (pTrueTarget)
	{
		if (this->Effect)
		{
			this->Effect.get()->Execute(pParticipants, pTrueTarget);
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
