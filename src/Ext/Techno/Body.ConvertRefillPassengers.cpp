#include "Body.h"

void TechnoExt::ConvertRefillWithPassengers(TechnoClass* pThis)
{
	if (pThis->WhatAmI() == AbstractType::Infantry || pThis->WhatAmI() == AbstractType::Building)
		return;

	FootClass* pTransport = abstract_cast<FootClass*>(pThis);
	if (!pTransport)
		return;

	auto const pType = pTransport->GetTechnoType();

	//if (pType->Passengers)
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->Convert_RefillWithPassengers.size() > 0 && pTransport->Passengers.NumPassengers < pType->Passengers)
	{
		for (auto const pPayloadType : pTypeExt->Convert_RefillWithPassengers)
		{
			auto const pPayload = static_cast<FootClass*>(pPayloadType->CreateObject(pTransport->Owner));

			const bool isTransportOpenTopped = pTransport->GetTechnoType()->OpenTopped;
			FootClass* pGunner = nullptr;

			for (auto pNext = pPayload;
				pNext && pNext != pTransport;
				pNext = abstract_cast<FootClass*>(pNext->NextObject))
			{
				pPayload->Transporter = pTransport;
				pGunner = pNext;

				if (isTransportOpenTopped)
					pTransport->EnteredOpenTopped(pNext);
			}

			// Add to transport - this will load the payload object and everything linked to it (rest of the team) in reverse order
			pThis->AddPassenger(pPayload);

			// Handle gunner change - this is the 'last' passenger because of reverse order
			if (pThis->GetTechnoType()->Gunner && pGunner)
				pTransport->ReceiveGunner(pGunner);

			/*if (pPayload->SpawnManager->SpawnCount > 0)
			{
				if (pThis->Target && pThis->CurrentMission == Mission::Attack)
				{
					pPayload->Target = pThis->Target;
					pPayload->QueueMission(Mission::Attack, true);

					pPayload->SpawnManager->UpdateTimer.Start(pPayload->GetTechnoType()->SpawnRegenRate);
					pPayload->SpawnManager->Target = nullptr;
					pPayload->SpawnManager->NewTarget = pThis->Target;

					for (auto newSpawnControl : pPayload->SpawnManager->SpawnedNodes)
					{
						newSpawnControl->Unit->SetTarget(pThis->Target);
						newSpawnControl->Unit->QueueMission(Mission::Attack, true);
						newSpawnControl->Unit->IsReturningFromAttackRun = false;
					}
				}
			}*/

			if (pThis->Passengers.NumPassengers >= pType->Passengers)
				break;
		}
	}
}
