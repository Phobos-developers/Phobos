#include "Body.h"


// Bugfix: TAction 7,80,107. Is rewrite needed?
DEFINE_HOOK(0x65DF81, TeamTypeClass_CreateMembers_LeMeGetInMyGoddamnIFV, 0x7)
{
	GET(FootClass* const, pPayload, EAX);
	GET(FootClass* const, pTransport, ESI);
	GET(TeamClass* const, pTeam, EBP);

	const bool isTransportOpenTopped = pTransport->GetTechnoType()->OpenTopped;

	for (auto pNext = pPayload; pNext; pNext = abstract_cast<FootClass*>(pNext->NextObject))
	{
		if (pNext && pNext != pTransport && pNext->Team == pTeam)
		{
			pNext->Transporter = pTransport;
			if (isTransportOpenTopped)
				pTransport->EnteredOpenTopped(pNext);
		}
	}

	pPayload->SetLocation(pTransport->Location);
	pTransport->AddPassenger(pPayload); // ReceiveGunner is done inside FootClass::AddPassenger
	// Ares' CreateInitialPayload doesn't work here
	return 0x65DF8D;
}

DEFINE_HOOK(0x4DE652, FootClass_AddPassenger_NumPassengerGeq0, 0x7)
{
	return R->ESI<FootClass*>()->Passengers.NumPassengers > 0 ? 0x4DE65B : 0x4DE666;
}
