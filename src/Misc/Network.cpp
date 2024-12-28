#include "Network.h"

#include "../Ext/Building/Body.h"
#include "../Ext/House/Body.h"

#include <BuildingClass.h>
#include <HouseClass.h>
#include <NetworkEvents.h>

// We have to do it earlier than Ares does, because Ares unresponsibly took over the handling of event codes beyond 0x2D.
DEFINE_HOOK(0x4C6CBD, Networking_RespondToEvent, 0x6)
{
	enum { Terminate = 0x4C8109 };

	GET(NetworkEvent*, Event, ESI);

	auto kind = static_cast<PhobosNetEvent::Events>(Event->Kind);

	if (kind >= PhobosNetEvent::Events::First)
	{
		switch (kind)
		{
		case PhobosNetEvent::Events::ToggleAggressiveStance:
			PhobosNetEvent::Handlers::RespondToToggleAggressiveStance(Event);
			break;
		}
	}

	// Simply return 0 because YR can't handle the custom event and won't do anything, there is no need to jump
	return 0;
}

/*
 how to raise your own events
	NetworkEvent Event;
	Event.Kind = PhobosNetworkEvent::aev_blah;
	Event.HouseIndex = U->Owner->ArrayIndex;
	memcpy(Event.ExtraData, "Boom de yada", 0xkcd);
	Networking::AddEvent(&Event);
*/

void PhobosNetEvent::Handlers::RaiseToggleAggressiveStance(TechnoClass* pTechno)
{
	NetworkEvent Event;
	Event.Kind = static_cast<EventType>(PhobosNetEvent::Events::ToggleAggressiveStance);
	Event.HouseIndex = byte(pTechno->Owner->ArrayIndex);
	byte* ExtraData = Event.ExtraData;
	NetID SourceObject;

	SourceObject.Pack(pTechno);
	memcpy(ExtraData, &SourceObject, sizeof(SourceObject));
	ExtraData += sizeof(SourceObject);

	Networking::AddEvent(&Event);
}

void PhobosNetEvent::Handlers::RespondToToggleAggressiveStance(NetworkEvent * Event)
{
	NetID* ID = reinterpret_cast<NetID*>(Event->ExtraData);
	if (auto pTechno = ID->UnpackTechno())
	{
		if (auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno))
		{
			pTechnoExt->TogggleAutoTargetBuildings(pTechno);
		}
	}
}
