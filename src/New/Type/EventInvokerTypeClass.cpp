#include "EventInvokerTypeClass.h"
#include <nameof/nameof.h>
#include <Ext/TechnoType/Body.h>
#include <InfantryClass.h>

template<>

const char* Enumerable<EventInvokerTypeClass>::GetMainSection()
{
	return "EventInvokerTypes";
}

void EventInvokerTypeClass::LoadFromINI(CCINIClass* pINI)
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

void EventInvokerTypeClass::LoadFromINI(INI_EX& exINI)
{
	if (this->loaded.Get())
		return;
	this->loaded = true;

	const char* pSection = this->Name;
	if (strcmp(pSection, NONE_STR) == 0)
		return;

	LoadFromINIPrivate(exINI, pSection);
}

void EventInvokerTypeClass::LoadFromINIPrivate(INI_EX& exINI, const char* pSection)
{
	LoadForScope(exINI, pSection, EventActorType::Me, "Invoker");
	LoadForScope(exINI, pSection, EventActorType::They, "Target");
	EventTypeClass::LoadTypeListFromINI(exINI, pSection, "EventType", &this->EventTypes);
	EventHandlerTypeClass::LoadTypeListFromINI(exINI, pSection, "ExtraEventHandler", &this->ExtraEventHandlers);
	this->PassDown_Passengers.Read(exINI, pSection, "PassDown.Passengers");
	this->PassDown_MindControlled.Read(exINI, pSection, "PassDown.MindControlled");
}

void EventInvokerTypeClass::LoadForScope(INI_EX& exINI, const char* pSection, const EventActorType scopeType, const char* scopeName)
{
	auto comp = HandlerCompClass::Parse(exINI, pSection, scopeType, scopeName, false);
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

void EventInvokerTypeClass::LoadForExtendedScope(INI_EX& exINI, const char* pSection, const EventActorType scopeType, const EventExtendedActorType extendedScopeType, const char* scopeName, const char* extendedScopeName)
{
	auto comp = HandlerCompClass::Parse(exINI, pSection, scopeType, extendedScopeType, scopeName, extendedScopeName, false);
	if (comp)
	{
		this->HandlerComps.push_back(std::move(comp));
	}
}

bool EventInvokerTypeClass::CheckInvokerFilters(HouseClass* pHouse, AbstractClass* pInvoker) const
{
	for (auto const& handlerComp : this->HandlerComps)
	{
		if (handlerComp.get()->ScopeType == EventActorType::Me)
		{
			if (!handlerComp.get()->CheckFilters(pHouse, pInvoker))
			{
				return false;
			}
		}
	}

	return true;
}

bool EventInvokerTypeClass::CheckTargetFilters(HouseClass* pHouse, AbstractClass* pTarget) const
{
	for (auto const& handlerComp : this->HandlerComps)
	{
		if (handlerComp.get()->ScopeType == EventActorType::They)
		{
			if (!handlerComp.get()->CheckFilters(pHouse, pTarget))
			{
				return false;
			}
		}
	}

	return true;
}

// This function is invoked from the external source.
// The "Me" scope can shift multiple times through the passing down.
// We have to record the initial "Me" scope and give it back,
// because multiple invokers may be invoked at a same time,
// and the same participants map will be reused.
void EventInvokerTypeClass::TryExecute(HouseClass* pHouse, std::map<EventActorType, AbstractClass*>* pParticipants)
{
	auto pTarget = abstract_cast<TechnoClass*>(pParticipants->at(EventActorType::Me));
	if (pTarget)
	{
		auto pInvoker = pParticipants->at(EventActorType::They);
		if (CheckInvokerFilters(pHouse, pInvoker))
		{
			TryExecuteOnTarget(pHouse, pParticipants, pTarget);
			pParticipants->operator[](EventActorType::Me) = pTarget;
		}
	}
}

// This function is invoked internally in this invoker class.
// This function checks for a single target, and invoke the events on it if appropriate.
// It also tries to pass down the target to its passengers, and every appropriate additional targets will go back to this function.
void EventInvokerTypeClass::TryExecuteOnTarget(HouseClass* pHouse, std::map<EventActorType, AbstractClass*>* pParticipants, TechnoClass* pTarget)
{
	if (CheckTargetFilters(pHouse, pTarget))
	{
		auto pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType());

		for (auto pEventHandlerTypeClass : ExtraEventHandlers)
		{
			pEventHandlerTypeClass->HandleEvent(pParticipants);
		}

		for (auto pEventType : EventTypes)
		{
			pTargetTypeExt->InvokeEvent(pEventType, pParticipants);
		}
	}

	TryPassDown(pHouse, pParticipants, pTarget);
}

void EventInvokerTypeClass::TryPassDown(HouseClass* pHouse, std::map<EventActorType, AbstractClass*>* pParticipants, TechnoClass* pRoot)
{
	if (PassDown_Passengers.Get())
	{
		if (pRoot->Passengers.NumPassengers > 0)
		{
			TechnoClass* pPassenger = nullptr;
			for (NextObject obj(pRoot->Passengers.FirstPassenger); obj; ++obj)
			{
				pPassenger = static_cast<TechnoClass*>(*obj);
				pParticipants->operator[](EventActorType::Me) = pPassenger;
				TryExecuteOnTarget(pHouse, pParticipants, pPassenger);
			}
		}
		else if (pRoot->GetOccupantCount() > 0)
		{
			auto pBld = reinterpret_cast<BuildingClass*>(pRoot);
			for (auto pPassenger : pBld->Occupants)
			{
				pParticipants->operator[](EventActorType::Me) = pPassenger;
				TryExecuteOnTarget(pHouse, pParticipants, pPassenger);
			}
		}
	}

	if (PassDown_MindControlled.Get())
	{
		if (pRoot->CaptureManager && pRoot->CaptureManager->IsControllingSomething())
		{
			for (auto controlNode : pRoot->CaptureManager->ControlNodes)
			{
				auto pMCedTechno = controlNode->Unit;
				pParticipants->operator[](EventActorType::Me) = pMCedTechno;
				TryExecuteOnTarget(pHouse, pParticipants, pMCedTechno);
			}
		}
	}
}

template <typename T>
void EventInvokerTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->loaded)
		.Process(this->HandlerComps)
		.Process(this->EventTypes)
		.Process(this->ExtraEventHandlers)
		.Process(this->PassDown_Passengers)
		.Process(this->PassDown_MindControlled)
		;
}

void EventInvokerTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void EventInvokerTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

void EventInvokerTypeClass::LoadTypeListFromINI(INI_EX& exINI, const char* pSection, const char* pHeader, ValueableVector<EventInvokerTypeClass*>* vec)
{
	char tempBuffer[64];

	// read event invokers
	Nullable<EventInvokerTypeClass*> eventInvokerNullable;
	for (size_t i = 0; ; ++i)
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s%d", pHeader, i);
		eventInvokerNullable.Reset();
		eventInvokerNullable.Read<true>(exINI, pSection, tempBuffer);
		if (eventInvokerNullable.isset())
		{
			eventInvokerNullable.Get()->LoadFromINI(exINI);
			vec->push_back(eventInvokerNullable.Get());
		}
		else
		{
			break;
		}
	}

	// read single event invokers
	if (vec->empty())
	{
		eventInvokerNullable.Reset();
		eventInvokerNullable.Read<true>(exINI, pSection, pHeader);
		if (eventInvokerNullable.isset())
		{
			eventInvokerNullable.Get()->LoadFromINI(exINI);
			vec->push_back(eventInvokerNullable.Get());
		}
	}
}
