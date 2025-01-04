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

void EventInvokerTypeClass::TryExecute(HouseClass* pHouse, std::map<EventScopeType, TechnoClass*>* pParticipants)
{
	auto pTarget = pParticipants->at(EventScopeType::Me);
	auto pThey = pParticipants->at(EventScopeType::They);

	if (CheckFilters(pHouse, pTarget))
	{
		auto pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType());

		for (auto pEventType : EventTypes)
		{
			pTargetTypeExt->InvokeEvent(pEventType, pParticipants);
		}
	}

	// Target Pass Down: Passengers
	if (Target_PassDown_Passengers.Get())
	{
		if (pTarget->Passengers.NumPassengers > 0)
		{
			TechnoClass* pPassenger = nullptr;
			for (NextObject obj(pTarget->Passengers.FirstPassenger->NextObject); obj; ++obj)
			{
				pPassenger = abstract_cast<TechnoClass*>(*obj);
				if (CheckFilters(pHouse, pPassenger))
				{
					auto pPassengerTypeExt = TechnoTypeExt::ExtMap.Find(pPassenger->GetTechnoType());
					pParticipants->operator[](EventScopeType::Me) = pPassenger;
					for (auto pEventType : EventTypes)
					{
						pPassengerTypeExt->InvokeEvent(pEventType, pParticipants);
					}
				}
			}
		}
		else if (pTarget->GetOccupantCount() > 0)
		{
			auto pTargetBld = reinterpret_cast<BuildingClass*>(pTarget);
			for (auto pPassenger : pTargetBld->Occupants)
			{
				if (CheckFilters(pHouse, pPassenger))
				{
					auto pPassengerTypeExt = TechnoTypeExt::ExtMap.Find(pPassenger->GetTechnoType());
					pParticipants->operator[](EventScopeType::Me) = pPassenger;
					for (auto pEventType : EventTypes)
					{
						pPassengerTypeExt->InvokeEvent(pEventType, pParticipants);
					}
				}
			}
		}
	}

	// Target Pass Down: Mind Controlled
	if (Target_PassDown_MindControlled.Get())
	{
		if (pTarget->CaptureManager && pTarget->CaptureManager->IsControllingSomething())
		{
			for (auto controlNode : pTarget->CaptureManager->ControlNodes)
			{
				auto pMCedTechno = controlNode->Unit;
				if (CheckFilters(pHouse, pMCedTechno))
				{
					auto pMCedTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pMCedTechno->GetTechnoType());
					pParticipants->operator[](EventScopeType::Me) = pMCedTechno;
					for (auto pEventType : EventTypes)
					{
						pMCedTechnoTypeExt->InvokeEvent(pEventType, pParticipants);
					}
				}
			}
		}
	}

	pParticipants->operator[](EventScopeType::Me) = pTarget;
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
