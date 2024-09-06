#include "Body.h"
#include <Ext/Techno/Body.h>

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
	if (!pThis || !pThis->GetTechnoType())
		return;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (!pTypeExt ||
		pTypeExt->InitialPayload_Types.empty() ||
		pThis->Transporter ||
		pThis->GetTechnoType()->Passengers <= 0 ||
		pThis->Passengers.NumPassengers > 0)
		return;

	for (size_t idx = 0; idx < pTypeExt->InitialPayload_Types.size(); idx++)
	{
		auto pType = pTypeExt->InitialPayload_Types.at(idx);

		if (pType->WhatAmI() == AbstractType::AircraftType ||
			pType->WhatAmI() == AbstractType::BuildingType)
			continue;

		int pNum = pTypeExt->InitialPayload_Nums.size() > idx ?
			pNum = pTypeExt->InitialPayload_Nums.at(idx) : 1;

		if (pNum <= 0)
			continue;

		for (int i = 0; i < pNum; i++)
		{
			TechnoClass* pTechno = abstract_cast<TechnoClass*>(pType->CreateObject(pThis->Owner));
			FootClass* pFoot = abstract_cast<FootClass*>(pTechno);

			pTechno->OnBridge = pThis->OnBridge;
			Unsorted::IKnowWhatImDoing++;
			pTechno->Unlimbo(pThis->GetCoords(), DirType::North);
			Unsorted::IKnowWhatImDoing--;

			pTechno->SetLocation(pThis->GetCoords());
			pTechno->Limbo();
			pTechno->Transporter = pThis;

			const auto old = VocClass::VoicesEnabled ? true : false;
			VocClass::VoicesEnabled = false;
			pThis->AddPassenger(pFoot);
			VocClass::VoicesEnabled = old;

			if (pThis->GetTechnoType()->OpenTopped)
				pThis->EnteredOpenTopped(pTechno);

			if (pThis->GetTechnoType()->Gunner &&
				pThis->Passengers.NumPassengers == 1)
				pThis->ReceiveGunner(pFoot);
		}
	}
}

DEFINE_HOOK_AGAIN(0x65D9DA, TeamTypeClass_CreateTeam_InitialPayload, 0x6)
DEFINE_HOOK(0x65ED10, TeamTypeClass_CreateTeam_InitialPayload, 0x6)
{
	FootClass* pThis = nullptr;

	if (R->Origin() == 0x65D9DA)
	{
		GET(FootClass*, pTeam, EBP);
		pThis = pTeam;
	}
	else
	{
		GET(FootClass*, pTeam, EDI);
		pThis = pTeam;
	}

	for (FootClass* pFoot = pThis; pFoot; pFoot = abstract_cast<FootClass*>(pFoot->NextObject))
	{
		PayloadFix(pFoot);
	}

	return 0;
}
