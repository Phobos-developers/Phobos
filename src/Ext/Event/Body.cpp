#include "Body.h"

#include <Helpers/Macro.h>
#include <EventClass.h>
#include <HouseClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>

bool EventExt::AddEvent()
{
	return EventClass::AddEvent(*reinterpret_cast<EventClass*>(this));
}

void EventExt::RespondEvent()
{
	switch (this->Type)
	{
	case EventTypeExt::ManualReload:
		this->RespondToManualReloadEvent();
		break;
	}
}

void EventExt::RaiseManualReloadEvent(TechnoClass* pTechno)
{
	EventExt eventExt {};
	eventExt.Type = EventTypeExt::ManualReload;
	eventExt.HouseIndex = static_cast<char>(pTechno->Owner->ArrayIndex);
	eventExt.Frame = Unsorted::CurrentFrame;
	eventExt.ManualReloadEvent.Who = TargetClass(pTechno);
	eventExt.AddEvent();
	Debug::LogGame("Adding event MANUAL_RELOAD\n");
}

void EventExt::RespondToManualReloadEvent()
{
	if (const auto pTechno = this->ManualReloadEvent.Who.As_Techno())
	{
		if (pTechno->Ammo > 0 && pTechno->IsAlive && !pTechno->Berzerk)
		{
			const auto pType = pTechno->GetTechnoType();
			const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

			if (pType && pTechno->Ammo != pType->Ammo && pTypeExt->CanManualReload)
			{
				if (pTypeExt->CanManualReload_DetonateWarhead && pTypeExt->CanManualReload_DetonateConsume <= pTechno->Ammo)
					WarheadTypeExt::DetonateAt(pTypeExt->CanManualReload_DetonateWarhead.Get(), pTechno->GetCoords(), pTechno, 1, pTechno->Owner, pTechno->Target);

				if (pTypeExt->CanManualReload_ResetROF)
					pTechno->RearmTimer.Stop();

				pTechno->Ammo = 0;

				if (pTechno->WhatAmI() != AbstractType::Aircraft)
					pTechno->StartReloading();
			}
		}
	}
}

size_t EventExt::GetDataSize(EventTypeExt type)
{
	switch (type)
	{
	case EventTypeExt::ManualReload:
		return sizeof(EventExt::ManualReloadEvent);
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
		pEvent->RespondEvent();

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
