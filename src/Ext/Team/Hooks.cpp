#include "Body.h"

// Ares InitialPayload fix: Man, what can I say
DEFINE_HOOK(0x65DE21, TeamTypeClass_CreateMembers_MutexOut, 0x6)
{
	GET(TeamClass*, pTeam, EBP);
	GET(TechnoTypeClass*, pType, EDI);
	R->ESI(pType->CreateObject(pTeam->Owner));
	return 0x65DE53;
}

// Bugfix: TAction 7,80,107.
DEFINE_HOOK(0x65DF81, TeamTypeClass_CreateMembers_LoadOntoTransport, 0x7)
{
	GET(FootClass* const, pPayload, EAX);
	GET(FootClass* const, pTransport, ESI);
	GET(TeamClass* const, pTeam, EBP);

	const bool isTransportOpenTopped = pTransport->GetTechnoType()->OpenTopped;
	FootClass* pGunner = nullptr;

	for (auto pNext = pPayload;
		pNext && pNext != pTransport && pNext->Team == pTeam;
		pNext = abstract_cast<FootClass*>(pNext->NextObject))
	{
		pPayload->Transporter = pTransport;
		pGunner = pNext;

		if (isTransportOpenTopped)
			pTransport->EnteredOpenTopped(pNext);
	}

	// Add to transport - this will load the payload object and everything linked to it (rest of the team) in reverse order
	pTransport->AddPassenger(pPayload);

	// Handle gunner change - this is the 'last' passenger because of reverse order
	if (pTransport->GetTechnoType()->Gunner && pGunner)
		pTransport->ReceiveGunner(pGunner);

	// Ares' CreateInitialPayload doesn't work here
	return 0x65DF8D;
}
