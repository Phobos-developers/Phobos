/*
#include "Body.h"

#include <Helpers/Macro.h>
#include <EventClass.h>

bool EventExt::AddEvent()
{
	return EventClass::AddEvent(*reinterpret_cast<EventClass*>(this));
}

void EventExt::RespondEvent()
{
	switch (this->Type)
	{
	case EventTypeExt::Sample:
		// Place the handler here
		break;
	}
}

size_t EventExt::GetDataSize(EventTypeExt type)
{
	switch (type)
	{
	case EventTypeExt::Sample:
		return sizeof(EventExt::Sample);
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
*/
