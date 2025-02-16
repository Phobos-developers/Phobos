#include "Body.h"
#include <HoverLocomotionClass.h>

#include <Ext/Scenario/Body.h>

DEFINE_HOOK_AGAIN(0x6FA33C, TechnoClass_ThreatEvals_OpenToppedOwner, 0x6) // TechnoClass::AI
DEFINE_HOOK_AGAIN(0x6F89F4, TechnoClass_ThreatEvals_OpenToppedOwner, 0x6) // TechnoClass::EvaluateCell
DEFINE_HOOK_AGAIN(0x6F7EC2, TechnoClass_ThreatEvals_OpenToppedOwner, 0x6) // TechnoClass::EvaluateObject
DEFINE_HOOK(0x6F8FD7, TechnoClass_ThreatEvals_OpenToppedOwner, 0x5)       // TechnoClass::Greatest_Threat
{
	enum { SkipCheckOne = 0x6F8FDC, SkipCheckTwo = 0x6F7EDA, SkipCheckThree = 0x6F8A0F, SkipCheckFour = 0x6FA37A };

	TechnoClass* pThis = nullptr;
	auto returnAddress = SkipCheckOne;

	switch (R->Origin())
	{
	case 0x6F8FD7:
		pThis = R->ESI<TechnoClass*>();
		break;
	case 0x6F7EC2:
		pThis = R->EDI<TechnoClass*>();
		returnAddress = SkipCheckTwo;
		break;
	case 0x6F89F4:
		pThis = R->ESI<TechnoClass*>();
		returnAddress = SkipCheckThree;
	case 0x6FA33C:
		pThis = R->ESI<TechnoClass*>();
		returnAddress = SkipCheckFour;
	default:
		return 0;
	}

	if (auto pTransport = pThis->Transporter)
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			if (pTypeExt->Passengers_SyncOwner)
				return returnAddress;
		}
	}

	return 0;
}

DEFINE_HOOK(0x701881, TechnoClass_ChangeHouse_Passenger_SyncOwner, 0x5)
{
	GET(TechnoClass*, pThis, ESI);

	if (FootClass* pPassenger = pThis->Passengers.GetFirstPassenger())
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			if (pTypeExt->Passengers_SyncOwner && pThis->Passengers.NumPassengers > 0)
			{
				pPassenger->SetOwningHouse(pThis->Owner, false);

				while (pPassenger->NextObject)
				{
					pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);

					if (pPassenger)
						pPassenger->SetOwningHouse(pThis->Owner, false);
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x71067B, TechnoClass_EnterTransport, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(FootClass*, pPassenger, EDI);

	if (pThis && pPassenger)
	{
		auto const pType = pPassenger->GetTechnoType();
		auto const pExt = TechnoExt::ExtMap.Find(pPassenger);
		auto const pTransTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pTransTypeExt->Passengers_SyncOwner && pTransTypeExt->Passengers_SyncOwner_RevertOnExit)
			pExt->OriginalPassengerOwner = pPassenger->Owner;

		if (pPassenger->WhatAmI() != AbstractType::Aircraft && pPassenger->WhatAmI() != AbstractType::Building
			&& pType->Ammo > 0 && pExt->TypeExtData->ReloadInTransport)
		{
			ScenarioExt::Global()->TransportReloaders.push_back(pExt);
		}
	}

	return 0;
}

DEFINE_HOOK(0x4DE722, FootClass_LeaveTransport, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(FootClass*, pPassenger, EAX);

	if (pThis && pPassenger)
	{
		auto const pType = pPassenger->GetTechnoType();
		auto const pExt = TechnoExt::ExtMap.Find(pPassenger);
		auto const pTransTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		// Remove from transport reloader list before switching house
		if (pPassenger->WhatAmI() != AbstractType::Aircraft && pPassenger->WhatAmI() != AbstractType::Building
			&& pType->Ammo > 0 && pExt->TypeExtData->ReloadInTransport)
		{
			auto& vec = ScenarioExt::Global()->TransportReloaders;
			vec.erase(std::remove(vec.begin(), vec.end(), pExt), vec.end());
		}

		if (pTransTypeExt->Passengers_SyncOwner && pTransTypeExt->Passengers_SyncOwner_RevertOnExit &&
			pExt->OriginalPassengerOwner)
		{
			pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);
		}
	}

	return 0;
}

// Has to be done here, before Ares survivor hook to take effect.
DEFINE_HOOK(0x737F80, TechnoClass_ReceiveDamage_Cargo_SyncOwner, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (pThis && pThis->Passengers.NumPassengers > 0)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pTypeExt->Passengers_SyncOwner && pTypeExt->Passengers_SyncOwner_RevertOnExit)
		{
			auto pPassenger = pThis->Passengers.GetFirstPassenger();
			auto pExt = TechnoExt::ExtMap.Find(pPassenger);

			if (pExt->OriginalPassengerOwner)
				pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);

			while (pPassenger->NextObject)
			{
				pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
				pExt = TechnoExt::ExtMap.Find(pPassenger);

				if (pExt->OriginalPassengerOwner)
					pPassenger->SetOwningHouse(pExt->OriginalPassengerOwner, false);
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x51DF82, InfantryClass_FireAt_ReloadInTransport, 0x6)
{
	GET(InfantryClass* const, pThis, ESI);

	if (pThis->Transporter)
	{
		auto const pType = pThis->GetTechnoType();
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (pTypeExt->ReloadInTransport && pType->Ammo > 0 && pThis->Ammo < pType->Ammo)
			pThis->StartReloading();
	}

	return 0;
}

// Customizable OpenTopped Properties
// Author: Otamaa
DEFINE_HOOK(0x6F72D2, TechnoClass_IsCloseEnoughToTarget_OpenTopped_RangeBonus, 0xC)
{
	GET(TechnoClass* const, pThis, ESI);

	if (auto pTransport = pThis->Transporter)
	{
		if (auto pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			R->EAX(pExt->OpenTopped_RangeBonus.Get(RulesClass::Instance->OpenToppedRangeBonus));
			return 0x6F72DE;
		}
	}

	return 0;
}

DEFINE_HOOK(0x71A82C, TemporalClass_AI_Opentopped_WarpDistance, 0xC)
{
	GET(TemporalClass* const, pThis, ESI);

	if (auto pTransport = pThis->Owner->Transporter)
	{
		if (auto pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			R->EDX(pExt->OpenTopped_WarpDistance.Get(RulesClass::Instance->OpenToppedWarpDistance));
			return 0x71A838;
		}
	}

	return 0;
}

DEFINE_HOOK(0x710552, TechnoClass_SetOpenTransportCargoTarget_ShareTarget, 0x6)
{
	enum { ReturnFromFunction = 0x71057F };

	GET(TechnoClass* const, pThis, ECX);
	GET_STACK(AbstractClass* const, pTarget, STACK_OFFSET(0x8, 0x4));

	if (pTarget)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (!pTypeExt->OpenTopped_ShareTransportTarget)
			return ReturnFromFunction;
	}

	return 0;
}

#pragma region NoQueueUpToEnterAndUnload

bool __fastcall CanEnterNow(UnitClass* pTransport, FootClass* pPassenger)
{
	const auto pOwner = pTransport->Owner;

	if (!pOwner || !pOwner->IsAlliedWith(pPassenger) || pTransport->IsBeingWarpedOut())
		return false;

	if (pPassenger->IsMindControlled() || pPassenger->ParasiteEatingMe)
		return false;

	const auto pManager = pPassenger->CaptureManager;

	if (pManager && pManager->IsControllingSomething())
		return false;

	const auto passengerSize = pPassenger->GetTechnoType()->Size;
	const auto pTransportType = pTransport->Type;

	if (passengerSize > pTransportType->SizeLimit)
		return false;

	const auto maxSize = pTransportType->Passengers;
	const auto predictSize = pTransport->Passengers.GetTotalSize() + static_cast<int>(passengerSize);
	const auto pLink = pTransport->GetNthLink();
	const auto needCalculate = pLink && pLink != pPassenger;

	if (needCalculate)
	{
		const auto linkCell = pLink->GetCoords();
		const auto tranCell = pTransport->GetCoords();

		// When the most important passenger is close, need to prevent overlap
		if (abs(linkCell.X - tranCell.X) <= 384 && abs(linkCell.Y - tranCell.Y) <= 384)
			return (predictSize <= (maxSize - pLink->GetTechnoType()->Size));
	}

	const auto remain = maxSize - predictSize;

	if (remain < 0)
		return false;

	if (needCalculate && remain < static_cast<int>(pLink->GetTechnoType()->Size))
	{
		// Avoid passenger moving forward, resulting in overlap with transport and create invisible barrier
		pLink->SendToFirstLink(RadioCommand::NotifyUnlink);
		pLink->EnterIdleMode(false, true);
	}

	return true;
}

DEFINE_HOOK(0x51A0D4, InfantryClass_UpdatePosition_NoQueueUpToEnter, 0x6)
{
	enum { EnteredThenReturn = 0x51A47E };

	GET(InfantryClass* const, pThis, ESI);

	if (const auto pDest = abstract_cast<UnitClass*>(pThis->CurrentMission == Mission::Enter ? pThis->Destination : pThis->QueueUpToEnter))
	{
		if (pDest->Type->Passengers > 0 && TechnoTypeExt::ExtMap.Find(pDest->Type)->NoQueueUpToEnter.Get(RulesExt::Global()->NoQueueUpToEnter))
		{
			const auto thisCell = pThis->GetCoords();
			const auto destCell = pDest->GetCoords();

			if (abs(thisCell.X - destCell.X) <= 384 && abs(thisCell.Y - destCell.Y) <= 384)
			{
				if (CanEnterNow(pDest, pThis)) // Replace send radio command: QueryCanEnter
				{
					if (const auto pTag = pDest->AttachedTag)
						pTag->RaiseEvent(TriggerEvent::EnteredBy, pThis, CellStruct::Empty);

					pThis->ArchiveTarget = nullptr;
					pThis->OnBridge = false;
					pThis->MissionAccumulateTime = 0;
					pThis->GattlingValue = 0;
					pThis->CurrentGattlingStage = 0;

					if (const auto pMind = pThis->MindControlledBy)
					{
						if (const auto pManager = pMind->CaptureManager)
							pManager->FreeUnit(pThis);
					}

					pThis->Limbo();

					if (pDest->Type->OpenTopped)
						pDest->EnteredOpenTopped(pThis);

					pThis->Transporter = pDest;
					pDest->AddPassenger(pThis);
					pThis->Undiscover();

					// Added, to prevent passengers from wanting to get on after getting off
					pThis->QueueUpToEnter = nullptr;

					// Added, to stop the passengers and let OpenTopped work normally
					pThis->SetSpeedPercentage(0.0);

					// Added, to stop hover unit's meaningless behavior
					if (const auto pHover = locomotion_cast<HoverLocomotionClass*>(pThis->Locomotor))
						pHover->MaxSpeed = 0;

					return EnteredThenReturn;
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x73A5EA, UnitClass_UpdatePosition_NoQueueUpToEnter, 0x5)
{
	enum { EnteredThenReturn = 0x73A78C };

	GET(UnitClass* const, pThis, EBP);

	if (const auto pDest = abstract_cast<UnitClass*>(pThis->CurrentMission == Mission::Enter ? pThis->Destination : pThis->QueueUpToEnter))
	{
		if (pDest->Type->Passengers > 0 && TechnoTypeExt::ExtMap.Find(pDest->Type)->NoQueueUpToEnter.Get(RulesExt::Global()->NoQueueUpToEnter))
		{
			const auto thisCell = pThis->GetCoords();
			const auto destCell = pDest->GetCoords();

			if (abs(thisCell.X - destCell.X) <= 384 && abs(thisCell.Y - destCell.Y) <= 384)
			{
				if (CanEnterNow(pDest, pThis)) // Replace send radio command: QueryCanEnter
				{
					// I don't know why units have no trigger

					pThis->ArchiveTarget = nullptr;
					pThis->OnBridge = false;
					pThis->MissionAccumulateTime = 0;
					pThis->GattlingValue = 0;
					pThis->CurrentGattlingStage = 0;

					if (const auto pMind = pThis->MindControlledBy)
					{
						if (const auto pManager = pMind->CaptureManager)
							pManager->FreeUnit(pThis);
					}

					pThis->Limbo();
					pDest->AddPassenger(pThis);

					if (pDest->Type->OpenTopped)
						pDest->EnteredOpenTopped(pThis);

					pThis->Transporter = pDest;

					if (pThis->Type->OpenTopped)
						pThis->SetTargetForPassengers(nullptr);

					pThis->Undiscover();

					// Added, to prevent passengers from wanting to get on after getting off
					pThis->QueueUpToEnter = nullptr;

					// Added, to stop the passengers and let OpenTopped work normally
					pThis->SetSpeedPercentage(0.0);

					// Added, to stop hover unit's meaningless behavior
					if (const auto pHover = locomotion_cast<HoverLocomotionClass*>(pThis->Locomotor))
						pHover->MaxSpeed = 0;

					return EnteredThenReturn;
				}
			}
		}
	}

	return 0;
}

static inline void PlayUnitLeaveTransportSound(UnitClass* pThis)
{
	const int sound = pThis->Type->LeaveTransportSound;

	if (sound != -1)
		VoxClass::PlayAtPos(sound, &pThis->Location);
}

DEFINE_HOOK(0x73DC9C, UnitClass_Mission_Unload_NoQueueUpToUnloadBreak, 0xA)
{
	enum { SkipGameCode = 0x73E289 };

	GET(UnitClass* const, pThis, ESI);
	GET(FootClass* const, pPassenger, EDI);

	pPassenger->Undiscover();

	// Play the sound when interrupted for some reason
	if (TechnoTypeExt::ExtMap.Find(pThis->Type)->NoQueueUpToUnload.Get(RulesExt::Global()->NoQueueUpToUnload))
		PlayUnitLeaveTransportSound(pThis);

	return SkipGameCode;
}

DEFINE_HOOK(0x73DC1E, UnitClass_Mission_Unload_NoQueueUpToUnloadLoop, 0xA)
{
	enum { UnloadLoop = 0x73D8CB, UnloadReturn = 0x73E289 };

	GET(UnitClass* const, pThis, ESI);

	if (TechnoTypeExt::ExtMap.Find(pThis->Type)->NoQueueUpToUnload.Get(RulesExt::Global()->NoQueueUpToUnload))
	{
		if (pThis->Passengers.NumPassengers <= pThis->NonPassengerCount)
		{
			// If unloading is required within one frame, the sound will only be played when the last passenger leaves
			PlayUnitLeaveTransportSound(pThis);
			pThis->MissionStatus = 4;
			return UnloadReturn;
		}

		R->EBX(0); // Reset
		return UnloadLoop;
	}

	PlayUnitLeaveTransportSound(pThis);
	return UnloadReturn;
}

#pragma endregion
