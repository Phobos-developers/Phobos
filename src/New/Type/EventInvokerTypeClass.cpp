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
	EventTypeClass::LoadTypeListFromINI(exINI, pSection, "EventType", &this->EventTypes);
	this->Invoker_Filter = HandlerFilterClass::Parse(exINI, pSection, "Invoker", "Filter");
	this->Invoker_NegFilter = HandlerFilterClass::Parse(exINI, pSection, "Invoker", "NegFilter");
	this->Target_Filter = HandlerFilterClass::Parse(exINI, pSection, "Target", "Filter");
	this->Target_NegFilter = HandlerFilterClass::Parse(exINI, pSection, "Target", "NegFilter");
	EventHandlerTypeClass::LoadTypeListFromINI(exINI, pSection, "Target.ExtraEventHandler", &this->Target_ExtraEventHandlers);
	this->Target_PassDown_Passengers.Read(exINI, pSection, "Target.PassDown.Passengers");
	this->Target_PassDown_MindControlled.Read(exINI, pSection, "Target.PassDown.MindControlled");
}

bool EventInvokerTypeClass::CheckInvokerFilters(HouseClass* pHouse, TechnoClass* pInvoker, bool fromSuperWeapon) const
{
	if (!pInvoker && fromSuperWeapon)
	{
		if (this->Invoker_Filter)
		{
			if (this->Invoker_Filter->IsDefinedAnyTechnoCheck()
				|| !this->Invoker_Filter.get()->CheckForHouse(pHouse, pHouse, false))
			{
				return false;
			}
		}

		if (this->Invoker_NegFilter)
		{
			if (this->Invoker_NegFilter->IsDefinedAnyTechnoCheck()
				|| !this->Invoker_NegFilter.get()->CheckForHouse(pHouse, pHouse, true))
			{
				return false;
			}
		}
	}
	else
	{
		if (this->Invoker_Filter)
		{
			if (!(pInvoker && this->Invoker_Filter.get()->Check(pHouse, pInvoker, false)))
			{
				return false;
			}
		}

		if (this->Invoker_NegFilter)
		{
			if (!(pInvoker && this->Invoker_NegFilter.get()->Check(pHouse, pInvoker, true)))
			{
				return false;
			}
		}
	}

	return true;
}

bool EventInvokerTypeClass::CheckTargetFilters(HouseClass* pHouse, TechnoClass* pTarget) const
{
	if (this->Target_Filter)
	{
		if (!(pTarget && this->Target_Filter.get()->Check(pHouse, pTarget, false)))
		{
			return false;
		}
	}

	if (this->Target_NegFilter)
	{
		if (!(pTarget && this->Target_NegFilter.get()->Check(pHouse, pTarget, true)))
		{
			return false;
		}
	}

	return true;
}

// This function is invoked from the external source.
// The "Me" scope can shift multiple times through the passing down.
// We have to record the initial "Me" scope and give it back,
// because multiple invokers may be invoked at a same time,
// and the same participants map will be reused.
void EventInvokerTypeClass::TryExecute(HouseClass* pHouse, std::map<EventScopeType, TechnoClass*>* pParticipants, bool fromSuperWeapon)
{
	auto pTarget = pParticipants->at(EventScopeType::Me);
	auto pInvoker = pParticipants->at(EventScopeType::They);
	if (CheckInvokerFilters(pHouse, pInvoker, fromSuperWeapon))
	{
		TryExecuteOnTarget(pHouse, pParticipants, pTarget);
		pParticipants->operator[](EventScopeType::Me) = pTarget;
	}
}

// This function is invoked internally in this invoker class.
// This function checks for a single target, and invoke the events on it if appropriate.
// It also tries to pass down the target to its passengers, and every appropriate additional targets will go back to this function.
void EventInvokerTypeClass::TryExecuteOnTarget(HouseClass* pHouse, std::map<EventScopeType, TechnoClass*>* pParticipants, TechnoClass* pTarget)
{
	if (CheckTargetFilters(pHouse, pTarget))
	{
		auto pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType());

		for (auto pEventHandlerTypeClass : Target_ExtraEventHandlers)
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

void EventInvokerTypeClass::TryPassDown(HouseClass* pHouse, std::map<EventScopeType, TechnoClass*>* pParticipants, TechnoClass* pRoot)
{
	if (Target_PassDown_Passengers.Get())
	{
		if (pRoot->Passengers.NumPassengers > 0)
		{
			TechnoClass* pPassenger = nullptr;
			for (NextObject obj(pRoot->Passengers.FirstPassenger); obj; ++obj)
			{
				pPassenger = static_cast<TechnoClass*>(*obj);
				pParticipants->operator[](EventScopeType::Me) = pPassenger;
				TryExecuteOnTarget(pHouse, pParticipants, pPassenger);
			}
		}
		else if (pRoot->GetOccupantCount() > 0)
		{
			auto pBld = reinterpret_cast<BuildingClass*>(pRoot);
			for (auto pPassenger : pBld->Occupants)
			{
				pParticipants->operator[](EventScopeType::Me) = pPassenger;
				TryExecuteOnTarget(pHouse, pParticipants, pPassenger);
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
				pParticipants->operator[](EventScopeType::Me) = pMCedTechno;
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
		.Process(this->EventTypes)
		.Process(this->Target_Filter)
		.Process(this->Target_NegFilter)
		.Process(this->Target_ExtraEventHandlers)
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
