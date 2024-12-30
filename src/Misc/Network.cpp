#include "Network.h"

#include "../Ext/Building/Body.h"
#include "../Ext/House/Body.h"

#include <BuildingClass.h>
#include <HouseClass.h>
#include <EventClass.h>

// We have to do it earlier than Ares does, because Ares unresponsibly took over the handling of event codes beyond 0x2D.
DEFINE_HOOK(0x4C6CBD, Networking_RespondToEvent, 0x6)
{
	enum { Terminate = 0x4C8109 };

	GET(EventClass*, Event, ESI);

	auto kind = static_cast<PhobosNetEvent::Events>(Event->Type);

	if (kind >= PhobosNetEvent::Events::First)
	{
		switch (kind)
		{
		case PhobosNetEvent::Events::ToggleAggressiveStance:
			PhobosNetEvent::Handlers::RespondToToggleAggressiveStance(Event);
			break;
		}
	}

	// Simply return 0 because YR can't handle the custom event and won't do anything, who needs a jump
	return 0;
}

void PhobosNetEvent::Handlers::RaiseToggleAggressiveStance(TechnoClass* pTechno)
{
	auto target = TargetClass(pTechno);
	auto eventType = static_cast<EventType>(PhobosNetEvent::Events::ToggleAggressiveStance);
	EventClass Event = EventClass(pTechno->Owner->ArrayIndex, target.m_ID);
	Event.Type = eventType;
	Event.Idle.Whom.m_ID = target.m_ID;
	Event.Idle.Whom.m_RTTI = target.m_RTTI;
	EventClass::AddEvent(Event);
}

void PhobosNetEvent::Handlers::RespondToToggleAggressiveStance(EventClass* pEvent)
{
	auto pTarget = pEvent->Idle.Whom;
	if (auto pTechno = pTarget.As_Techno())
	{
		if (auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno))
		{
			pTechnoExt->ToggleAggressiveStance(pTechno);
		}
	}
}
