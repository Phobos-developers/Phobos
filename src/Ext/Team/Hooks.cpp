#include "Body.h"
#include <Ext/Techno/Body.h>
#include <Ext/Techno/AresExtData.h>
#include <Utilities/AresFunctions.h>

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

void _fastcall PayloadFix(FootClass* pThis)
{
	if (!pThis ||
		pThis->WhatAmI() == AbstractType::Infantry ||
		pThis->Transporter ||
		pThis->GetTechnoType()->Passengers <= 0 ||
		pThis->Passengers.NumPassengers > 0)
		return;

	const auto pAresTechnoExt = AresTechnoExt::FindExtData(static_cast<TechnoClass*>(pThis));

	if (pAresTechnoExt && pAresTechnoExt->PayloadCreated)
	{
		pAresTechnoExt->PayloadCreated = false;
	}
	else
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pTypeExt && !pTypeExt->InitialPayload_Types.empty())
			TechnoExt::CreateInitialPayload(pThis, &pTypeExt->InitialPayload_Types, &pTypeExt->InitialPayload_Nums);
	}
}

// FlyStar : I hope it doesn't trigger twice, but I don't know if that's right.
DEFINE_HOOK(0x65D995, TeamTypeClass_CreateInstance_InitialPayload, 0x6)
{
	GET(FootClass*, pThis, EBP);
	enum { SkipGameCode = 0x65DD1B, Continue = 0x65D9A9 };

	for (FootClass* pFoot = pThis; pFoot; pFoot = abstract_cast<FootClass*>(pFoot->NextObject))
	{
		PayloadFix(pFoot);
	}

	if (pThis && pThis->Team)
		pThis->Team->IsTransient = false;

	R->Stack(STACK_OFFSET(0x30, -0x18), pThis);
	return !pThis ? SkipGameCode : Continue;
}

// FlyStar : I hope it doesn't trigger twice, but I don't know if that's right.
DEFINE_HOOK(0x65ECD2, TeamTypeClass_CreateTeamChrono_Fix, 0x6)
{
	GET(FootClass*, pThis, EDI);
	enum { SkipGameCode = 0x65F301, Continue = 0x65ECE2 };

	for (FootClass* pFoot = pThis; pFoot; pFoot = abstract_cast<FootClass*>(pFoot->NextObject))
	{
		PayloadFix(pFoot);
	}

	if (pThis && pThis->Team)
		pThis->Team->IsTransient = false;

	return !pThis ? SkipGameCode : Continue;
}
