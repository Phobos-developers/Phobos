#include "EventInvokerTypeClass.h"
#include <nameof/nameof.h>
#include <InfantryClass.h>
#include <Ext/Techno/Body.h>

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
	LoadForActor(exINI, pSection, EventActorType::Me, "Invoker");
	LoadForActor(exINI, pSection, EventActorType::They, "Target");
	LoadForActor(exINI, pSection, EventActorType::Scoper, "Scoper");
	LoadForActor(exINI, pSection, EventActorType::Enchanter, "Enchanter");
	EventTypeClass::LoadTypeListFromINI(exINI, pSection, "EventType", &this->EventTypes);
	EventHandlerTypeClass::LoadTypeListFromINI(exINI, pSection, "ExtraEventHandler", &this->ExtraEventHandlers);
	this->PassDown_Passengers.Read(exINI, pSection, "PassDown.Passengers");
	this->PassDown_MindControlled.Read(exINI, pSection, "PassDown.MindControlled");
}

void EventInvokerTypeClass::LoadForActor(INI_EX& exINI, const char* pSection, const EventActorType actorType, const char* actorName)
{
	auto comp = HandlerCompClass::Parse(exINI, pSection, actorType, actorName, false);
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

void EventInvokerTypeClass::LoadForExtendedActor(INI_EX& exINI, const char* pSection, const EventActorType actorType, const EventExtendedActorType extendedActorType, const char* actorName, const char* extendedActorName)
{
	auto comp = HandlerCompClass::Parse(exINI, pSection, actorType, extendedActorType, actorName, extendedActorName, false);
	if (comp)
	{
		this->HandlerComps.push_back(std::move(comp));
	}
}

bool EventInvokerTypeClass::CheckFilters(HouseClass* pHouse, EventActorType actorType, AbstractClass* pInvoker) const
{
	for (auto const& handlerComp : this->HandlerComps)
	{
		if (handlerComp.get()->ActorType == actorType)
		{
			if (!handlerComp.get()->CheckFilters(pHouse, pInvoker))
			{
				return false;
			}
		}
	}

	return true;
}

// This function is invoked from the external source.
// The "Me" actor can shift multiple times through the passing down.
// We have to record the initial "Me" actor and give it back,
// because multiple invokers may be invoked at a same time,
// and the same participants map will be reused.
void EventInvokerTypeClass::TryExecute(HouseClass* pHouse, PhobosMap<EventActorType, AbstractClass*>* pParticipants)
{
	if (auto pTarget = abstract_cast<TechnoClass*>(pParticipants->get_or_default(EventActorType::Me, nullptr)))
	{
		auto pInvoker = pParticipants->get_or_default(EventActorType::They, nullptr);
		auto pScoper = pParticipants->get_or_default(EventActorType::Scoper, nullptr);
		auto pEnchanter = pParticipants->get_or_default(EventActorType::Enchanter, nullptr);
		if (CheckFilters(pHouse, EventActorType::They, pInvoker)
			&& CheckFilters(pHouse, EventActorType::Scoper, pScoper)
			&& CheckFilters(pHouse, EventActorType::Enchanter, pEnchanter))
		{
			TryExecuteOnTarget(pHouse, pParticipants, pTarget);
			pParticipants->operator[](EventActorType::Me) = pTarget;
		}
	}
}

// This function is invoked internally in this invoker class.
// This function checks for a single target, and invoke the events on it if appropriate.
// It also tries to pass down the target to its passengers, and every appropriate additional targets will go back to this function.
void EventInvokerTypeClass::TryExecuteOnTarget(HouseClass* pHouse, PhobosMap<EventActorType, AbstractClass*>* pParticipants, TechnoClass* pTarget)
{
	if (CheckFilters(pHouse, EventActorType::Me, pTarget))
	{
		auto pTargetExt = TechnoExt::ExtMap.Find(pTarget);

		for (auto pEventHandlerTypeClass : ExtraEventHandlers)
		{
			pEventHandlerTypeClass->HandleEvent(pParticipants);
		}

		for (auto pEventType : EventTypes)
		{
			pTargetExt->InvokeEvent(pEventType, pParticipants);
		}
	}

	TryPassDown(pHouse, pParticipants, pTarget);
}

void EventInvokerTypeClass::TryPassDown(HouseClass* pHouse, PhobosMap<EventActorType, AbstractClass*>* pParticipants, TechnoClass* pRoot)
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
