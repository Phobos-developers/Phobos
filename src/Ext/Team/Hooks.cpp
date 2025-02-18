#include "Body.h"
#include <Utilities/AresHelper.h>

// Bugfix: TAction 7,80,107.
DEFINE_HOOK(0x65DF67, TeamTypeClass_CreateMembers_LoadOntoTransport, 0x6)
{
	if(!AresHelper::CanUseAres) // If you're not using Ares you don't deserve a fix
		return 0;

	GET(FootClass* const, pPayload, EAX);
	GET(FootClass* const, pTransport, ESI);
	GET(TeamClass* const, pTeam, EBP);
	GET(TeamTypeClass const*, pThis, EBX);

	auto unmarkPayloadCreated = [](FootClass* member){reinterpret_cast<char*>(member->align_154)[0x9E] = false;};

	if (!pTransport)
	{
		for (auto pNext = pPayload;
		pNext && pNext != pTransport && pNext->Team == pTeam;
		pNext = abstract_cast<FootClass*>(pNext->NextObject))
			unmarkPayloadCreated(pNext);

		return 0x65DFE8;
	}

	unmarkPayloadCreated(pTransport);

	if (!pPayload || !pThis->Full)
		return 0x65E004;

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

	return 0x65DF8D;
}
