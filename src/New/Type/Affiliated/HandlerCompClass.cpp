#include "HandlerCompClass.h"
#include <Ext/Techno/Body.h>

HandlerCompClass::HandlerCompClass()
	: ActorType {}
	, ExtendedActorType {}
	, Filter {}
	, NegFilter {}
	, Effect {}
{ }

std::unique_ptr<HandlerCompClass> HandlerCompClass::Parse(INI_EX& exINI, const char* pSection, EventActorType ActorType, const char* actorName, bool includeEffects)
{
	auto handlerUnit = std::make_unique<HandlerCompClass>();
	handlerUnit.get()->ActorType = ActorType;
	handlerUnit.get()->LoadFromINI(exINI, pSection, actorName, nullptr, includeEffects);
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

std::unique_ptr<HandlerCompClass> HandlerCompClass::Parse(INI_EX& exINI, const char* pSection, EventActorType ActorType, EventExtendedActorType ExtendedActorType, const char* actorName, const char* extendedActorName, bool includeEffects)
{
	auto handlerUnit = std::make_unique<HandlerCompClass>();
	handlerUnit.get()->ActorType = ActorType;
	handlerUnit.get()->ExtendedActorType = ExtendedActorType;
	handlerUnit.get()->LoadFromINI(exINI, pSection, actorName, extendedActorName, includeEffects);
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

void HandlerCompClass::LoadFromINI(INI_EX& exINI, const char* pSection, const char* actorName, const char* extendedActorName, bool includeEffects)
{
	auto localActorName = actorName;
	if (extendedActorName)
	{
		char tempBuffer[32];
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.%s", actorName, extendedActorName);
		localActorName = tempBuffer;
	}

	this->Filter = HandlerFilterClass::Parse(exINI, pSection, localActorName, "Filter");
	this->NegFilter = HandlerFilterClass::Parse(exINI, pSection, localActorName, "NegFilter");
	if (includeEffects)
		this->Effect = HandlerEffectClass::Parse(exINI, pSection, localActorName, "Effect");
}

bool HandlerCompClass::IsDefined() const
{
	return Filter != nullptr
		|| NegFilter != nullptr
		|| Effect != nullptr;
}

AbstractClass* HandlerCompClass::GetTrueTarget(AbstractClass* pTarget, Nullable<EventExtendedActorType> ExtendedActorType)
{
	if (pTarget && ExtendedActorType.isset())
	{
		if (auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget))
		{
			switch (ExtendedActorType.Get())
			{
			case EventExtendedActorType::Owner:
				return pTargetTechno->Owner;
			case EventExtendedActorType::Transport:
				return GetTransportingTechno(pTargetTechno);
			case EventExtendedActorType::Bunker:
				return pTargetTechno->BunkerLinkedItem;
			case EventExtendedActorType::MindController:
				return pTargetTechno->MindControlledBy;
			case EventExtendedActorType::Parasite:
				return GetParasiteTechno(pTargetTechno);
			case EventExtendedActorType::Host:
				return GetHostTechno(pTargetTechno);
			default:
				return nullptr;
			}
		}
		else
		{
			switch (ExtendedActorType.Get())
			{
			case EventExtendedActorType::Owner:
				return GetOwningHouseOfActor(pTarget);
			default:
				return nullptr;
			}
		}
	}
	return pTarget;
}

HouseClass* HandlerCompClass::GetOwningHouseOfActor(AbstractClass* pTarget)
{
	if (pTarget->WhatAmI() == AbstractType::House)
	{
		return static_cast<HouseClass*>(pTarget);
	}
	/*else if (auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget))
	{
		return pTargetTechno->Owner;
	}*/
	else
	{
		return pTarget->GetOwningHouse();
	}
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

bool HandlerCompClass::CheckFilters(HouseClass* pHouse, AbstractClass* pTarget) const
{
	auto const pTrueTarget = HandlerCompClass::GetTrueTarget(pTarget, this->ExtendedActorType);

	if (this->Filter)
	{
		if (!(pTrueTarget && this->Filter.get()->Check(pHouse, pTrueTarget, false)))
		{
			return false;
		}
	}

	if (this->NegFilter)
	{
		if (!(pTrueTarget && this->NegFilter.get()->Check(pHouse, pTrueTarget, true)))
		{
			return false;
		}
	}

	return true;
}

bool HandlerCompClass::CheckFilters(std::map<EventActorType, AbstractClass*>* pParticipants) const
{
	auto pOwner = pParticipants->at(EventActorType::Me);
	auto pHouse = GetOwningHouseOfActor(pOwner);
	auto pTarget = pParticipants->at(this->ActorType);
	return CheckFilters(pHouse, pTarget);
}

void HandlerCompClass::ExecuteEffects(std::map<EventActorType, AbstractClass*>* pParticipants) const
{
	auto pTarget = pParticipants->at(this->ActorType);
	auto const pTrueTarget = HandlerCompClass::GetTrueTarget(pTarget, this->ExtendedActorType);

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
		.Process(this->ActorType)
		.Process(this->ExtendedActorType)
		.Process(this->Filter)
		.Process(this->NegFilter)
		.Process(this->Effect)
		.Success();
}
