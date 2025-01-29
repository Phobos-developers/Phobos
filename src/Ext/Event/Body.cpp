#include "Body.h"

#include <Helpers/Macro.h>
#include <EventClass.h>
#include <HouseClass.h>
#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>

bool EventExt::AddEvent()
{
	return EventClass::AddEvent(*reinterpret_cast<EventClass*>(this));
}

void EventExt::RespondEvent()
{
	switch (this->Type)
	{
	case EventTypeExt::TriggerCustomHotkey:
		RespondToTriggerCustomHotkey();
		break;
	}
}

void EventExt::RaiseTriggerCustomHotkey(AbstractClass* pTarget, HouseClass* pHouse, int numeralSequence)
{
	EventExt eventExt {};
	eventExt.Type = EventTypeExt::TriggerCustomHotkey;
	eventExt.HouseIndex = static_cast<char>(pHouse->ArrayIndex);
	eventExt.Frame = Unsorted::CurrentFrame;
	eventExt.TriggerCustomHotkey.Who = TargetClass(pTarget);
	eventExt.TriggerCustomHotkey.NumeralSequence = numeralSequence;
	eventExt.AddEvent();
}

void EventExt::RespondToTriggerCustomHotkey()
{
	auto numeralSequence = this->TriggerCustomHotkey.NumeralSequence;

	static PhobosMap<int, EventTypeClass*> CachedEventTypeMap;

	if (!CachedEventTypeMap.contains(numeralSequence))
	{
		static char tempBuffer[32];
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "CustomHotkey_%i", numeralSequence);
		// Since if this event type is not in use, there is no need to fire it in the first place.
		// Thus, we only find here, we don't allocate.
		auto pEventType = EventTypeClass::Find(tempBuffer);
		CachedEventTypeMap.insert(numeralSequence, pEventType);
	}

	auto pEventType = CachedEventTypeMap.get_or_default(numeralSequence, nullptr);

	if (pEventType)
	{
		static PhobosMap<EventActorType, AbstractClass*> participants;
		participants.clear();

		if (auto const pTechno = this->TriggerCustomHotkey.Who.As_Techno())
		{
			if (auto const pTechnoExt = TechnoExt::ExtMap.Find(pTechno))
			{
				participants.insert(EventActorType::Me, pTechno);
				pTechnoExt->InvokeEvent(pEventType, &participants);
			}
		}
		else if (auto const pHouse = this->TriggerCustomHotkey.Who.As_House())
		{
			if (auto const pHouseExt = HouseExt::ExtMap.Find(pHouse))
			{
				participants.insert(EventActorType::Me, pHouse);
				pHouseExt->InvokeEvent(pEventType, &participants);
			}
		}
	}
}

size_t EventExt::GetDataSize(EventTypeExt type)
{
	switch (type)
	{
	case EventTypeExt::TriggerCustomHotkey:
		return sizeof(EventExt::TriggerCustomHotkey);
	}

	return 0;
}

bool EventExt::IsValidType(EventTypeExt type)
{
	return (type >= EventTypeExt::FIRST && type <= EventTypeExt::LAST);
}

// hooks

DEFINE_HOOK(0x4C6CC8, Networking_RespondToEvent, 0x5)
{
	GET(EventExt*, pEvent, ESI);

	if (EventExt::IsValidType(pEvent->Type))
	{
		pEvent->RespondEvent();
	}

	return 0;
}

DEFINE_HOOK(0x64B6FE, sub_64B660_GetEventSize, 0x6)
{
	const auto eventType = static_cast<EventTypeExt>(R->EDI() & 0xFF);

	if (EventExt::IsValidType(eventType))
	{
		const size_t eventSize = EventExt::GetDataSize(eventType);

		R->EDX(eventSize);
		R->EBP(eventSize);
		return 0x64B71D;
	}

	return 0;
}

DEFINE_HOOK(0x64BE7D, sub_64BDD0_GetEventSize1, 0x6)
{
	const auto eventType = static_cast<EventTypeExt>(R->EDI() & 0xFF);

	if (EventExt::IsValidType(eventType))
	{
		const size_t eventSize = EventExt::GetDataSize(eventType);

		REF_STACK(size_t, eventSizeInStack, STACK_OFFSET(0xAC, -0x8C));
		eventSizeInStack = eventSize;
		R->ECX(eventSize);
		R->EBP(eventSize);
		return 0x64BE97;
	}

	return 0;
}

DEFINE_HOOK(0x64C30E, sub_64BDD0_GetEventSize2, 0x6)
{
	const auto eventType = static_cast<EventTypeExt>(R->ESI() & 0xFF);

	if (EventExt::IsValidType(eventType))
	{
		const size_t eventSize = EventExt::GetDataSize(eventType);

		R->ECX(eventSize);
		R->EBP(eventSize);
		return 0x64C321;
	}

	return 0;
}
