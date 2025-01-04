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
	char tempBuffer[32];

	// read event types
	Nullable<EventTypeClass*> eventTypeNullable;
	for (size_t i = 0; ; ++i)
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "EventType%d", i);
		eventTypeNullable.Reset();
		eventTypeNullable.Read<true>(exINI, pSection, tempBuffer);
		if (eventTypeNullable.isset())
		{
			this->EventTypes.push_back(eventTypeNullable.Get());
		}
		else
		{
			break;
		}
	}

	// read single event type
	if (this->EventTypes.empty())
	{
		eventTypeNullable.Reset();
		eventTypeNullable.Read<true>(exINI, pSection, "EventType");
		if (eventTypeNullable.isset())
		{
			this->EventTypes.push_back(eventTypeNullable.Get());
		}
	}

	this->Filter = HandlerFilterClass::Parse(exINI, pSection, "Target", "Filter");
	this->NegFilter = HandlerFilterClass::Parse(exINI, pSection, "Target", "NegFilter");
	this->Target_PassDown_Passengers.Read(exINI, pSection, "Target.PassDown.Passengers");
	this->Target_PassDown_MindControlled.Read(exINI, pSection, "Target.PassDown.MindControlled");
}

bool EventInvokerTypeClass::CheckFilters(HouseClass* pHouse, TechnoClass* pTarget) const
{
	if (this->Filter)
	{
		if (!this->Filter.get()->Check(pHouse, pTarget, false))
		{
			return false;
		}
	}

	if (this->NegFilter)
	{
		if (!this->NegFilter.get()->Check(pHouse, pTarget, true))
		{
			return false;
		}
	}

	return true;
}

// This function is invoked from the external source.
// - The "Me" scope can shift multiple times through the passing down.
//   We have to record the initial "Me" scope and give it back,
//   because multiple invokers may be invoked at a same time,
//   and the same participants map will be reused.
void EventInvokerTypeClass::TryExecute(HouseClass* pHouse, std::map<EventScopeType, TechnoClass*>* pParticipants)
{
	auto pTarget = pParticipants->at(EventScopeType::Me);
	TryExecuteSingle(pHouse, pParticipants, pTarget);
	pParticipants->operator[](EventScopeType::Me) = pTarget;
}

// This function is invoked internally in this invoker class.
// This function checks for a single target, and invoke the events on it if appropriate.
// It also tries to pass down the target to its passengers, and every appropriate additional targets will go back to this function.
void EventInvokerTypeClass::TryExecuteSingle(HouseClass* pHouse, std::map<EventScopeType, TechnoClass*>* pParticipants, TechnoClass* pTarget)
{
	if (CheckFilters(pHouse, pTarget))
	{
		auto pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType());

		for (auto pEventType : EventTypes)
		{
			pTargetTypeExt->InvokeEvent(pEventType, pParticipants);
		}
	}

	TryPassDown(pHouse, pParticipants, pTarget);
}

void EventInvokerTypeClass::TryPassDown(HouseClass* pHouse, std::map<EventScopeType, TechnoClass*>* pParticipants, TechnoClass* pRoot)
{
	if (Target_PassDown_Passengers.Get())
	{
		if (pRoot->Passengers.NumPassengers > 0)
		{
			TechnoClass* pPassenger = nullptr;
			for (NextObject obj(pRoot->Passengers.FirstPassenger->NextObject); obj; ++obj)
			{
				pPassenger = abstract_cast<TechnoClass*>(*obj);
				if (CheckFilters(pHouse, pPassenger))
				{
					pParticipants->operator[](EventScopeType::Me) = pPassenger;
					TryExecuteSingle(pHouse, pParticipants, pPassenger);
				}
			}
		}
		else if (pRoot->GetOccupantCount() > 0)
		{
			auto pBld = reinterpret_cast<BuildingClass*>(pRoot);
			for (auto pPassenger : pBld->Occupants)
			{
				if (CheckFilters(pHouse, pPassenger))
				{
					pParticipants->operator[](EventScopeType::Me) = pPassenger;
					TryExecuteSingle(pHouse, pParticipants, pPassenger);
				}
			}
		}
	}

	if (Target_PassDown_MindControlled.Get())
	{
		if (pRoot->CaptureManager && pRoot->CaptureManager->IsControllingSomething())
		{
			for (auto controlNode : pRoot->CaptureManager->ControlNodes)
			{
				auto pMCedTechno = controlNode->Unit;
				if (CheckFilters(pHouse, pMCedTechno))
				{
					pParticipants->operator[](EventScopeType::Me) = pMCedTechno;
					TryExecuteSingle(pHouse, pParticipants, pMCedTechno);
				}
			}
		}
	}
}

template <typename T>
void EventInvokerTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->loaded)
		.Process(this->EventTypes)
		.Process(this->Filter)
		.Process(this->NegFilter)
		.Process(this->Target_PassDown_Passengers)
		.Process(this->Target_PassDown_MindControlled)
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
