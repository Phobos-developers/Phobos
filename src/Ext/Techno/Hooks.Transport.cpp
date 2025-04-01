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

static inline bool IsCloseEnoughToEnter(UnitClass* pTransport, FootClass* pPassenger)
{
	return (abs(pPassenger->Location.X - pTransport->Location.X) <= 384 && abs(pPassenger->Location.Y - pTransport->Location.Y) <= 384);
}

// Rewrite from 0x73758A, replace send RadioCommand::QueryCanEnter
bool __fastcall CanEnterNow(UnitClass* pTransport, FootClass* pPassenger)
{
	if (!pTransport->Owner->IsAlliedWith(pPassenger) || pTransport->IsBeingWarpedOut())
		return false;

	if (pPassenger->IsMindControlled() || pPassenger->ParasiteEatingMe)
		return false;

	const auto pManager = pPassenger->CaptureManager;

	if (pManager && pManager->IsControllingSomething())
		return false;

	const auto pTransportType = pTransport->Type;
	const auto bySize = TechnoTypeExt::ExtMap.Find(pTransportType)->Passengers_BySize;
	const auto passengerSize = bySize ? Game::F2I(pPassenger->GetTechnoType()->Size) : 1;

	if (passengerSize > Game::F2I(pTransportType->SizeLimit))
		return false;

	const auto maxSize = pTransportType->Passengers;
	const auto predictSize = (bySize ? pTransport->Passengers.GetTotalSize() : pTransport->Passengers.NumPassengers) + passengerSize;
	const auto pLink = abstract_cast<FootClass*>(pTransport->GetNthLink());
	const auto needCalculate = pLink && pLink != pPassenger && pLink->Destination == pTransport;

	// When the most important passenger is close, need to prevent overlap
	if (needCalculate && IsCloseEnoughToEnter(pTransport, pLink))
		return (predictSize <= (maxSize - (bySize ? Game::F2I(pLink->GetTechnoType()->Size) : 1)));

	return predictSize < maxSize;
}

// Rewrite from 0x51A21B
void __fastcall InfantryEnterNow(UnitClass* pTransport, InfantryClass* pPassenger)
{
	if (const auto pTag = pTransport->AttachedTag)
		pTag->RaiseEvent(TriggerEvent::EnteredBy, pPassenger, CellStruct::Empty);

	pPassenger->ArchiveTarget = nullptr;
	pPassenger->OnBridge = false;
	pPassenger->MissionAccumulateTime = 0;
	pPassenger->GattlingValue = 0;
	pPassenger->CurrentGattlingStage = 0;

	/* Have checked in CanEnterNow
	if (const auto pMind = pPassenger->MindControlledBy)
	{
		if (const auto pManager = pMind->CaptureManager)
			pManager->FreeUnit(pPassenger);
	}
	*/

	pPassenger->Limbo();

	if (pTransport->Type->OpenTopped)
		pTransport->EnteredOpenTopped(pPassenger);

	pPassenger->Transporter = pTransport;
	pTransport->AddPassenger(pPassenger);
	pPassenger->Undiscover();

	// Added, to prevent passengers from wanting to get on after getting off
	pPassenger->QueueUpToEnter = nullptr;

	// Added, to stop the passengers and let OpenTopped work normally
	pPassenger->SetSpeedPercentage(0.0);

	// Added, to stop hover unit's meaningless behavior
	if (const auto pHover = locomotion_cast<HoverLocomotionClass*>(pPassenger->Locomotor))
		pHover->MaxSpeed = 0;
}

// Rewrite from 0x73A6D1
void __fastcall UnitEnterNow(UnitClass* pTransport, UnitClass* pPassenger)
{
	// I don't know why units have no trigger

	pPassenger->ArchiveTarget = nullptr;
	pPassenger->OnBridge = false;
	pPassenger->MissionAccumulateTime = 0;
	pPassenger->GattlingValue = 0;
	pPassenger->CurrentGattlingStage = 0;

	/* Have checked in CanEnterNow
	if (const auto pMind = pPassenger->MindControlledBy)
	{
		if (const auto pManager = pMind->CaptureManager)
			pManager->FreeUnit(pPassenger);
	}
	*/

	pPassenger->Limbo();
	pTransport->AddPassenger(pPassenger);

	if (pTransport->Type->OpenTopped)
		pTransport->EnteredOpenTopped(pPassenger);

	pPassenger->Transporter = pTransport;

	if (pPassenger->Type->OpenTopped)
		pPassenger->SetTargetForPassengers(nullptr);

	pPassenger->Undiscover();

	// Added, to prevent passengers from wanting to get on after getting off
	pPassenger->QueueUpToEnter = nullptr;

	// Added, to stop the passengers and let OpenTopped work normally
	pPassenger->SetSpeedPercentage(0.0);

	// Added, to stop hover unit's meaningless behavior
	if (const auto pHover = locomotion_cast<HoverLocomotionClass*>(pPassenger->Locomotor))
		pHover->MaxSpeed = 0;
}

DEFINE_HOOK(0x51A0D4, InfantryClass_UpdatePosition_NoQueueUpToEnter, 0x6)
{
	enum { EnteredThenReturn = 0x51A47E };

	GET(InfantryClass* const, pThis, ESI);

	if (const auto pDest = abstract_cast<UnitClass*>(pThis->CurrentMission == Mission::Enter ? pThis->Destination : pThis->QueueUpToEnter))
	{
		const auto pType = pDest->Type;

		if (pType->Passengers > 0 && TechnoTypeExt::ExtMap.Find(pType)->NoQueueUpToEnter.Get(RulesExt::Global()->NoQueueUpToEnter))
		{
			if (IsCloseEnoughToEnter(pDest, pThis) && CanEnterNow(pDest, pThis))
			{
				InfantryEnterNow(pDest, pThis);
				return EnteredThenReturn;
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
		const auto pType = pDest->Type;

		if (pType->Passengers > 0 && TechnoTypeExt::ExtMap.Find(pType)->NoQueueUpToEnter.Get(RulesExt::Global()->NoQueueUpToEnter))
		{
			if (IsCloseEnoughToEnter(pDest, pThis) && CanEnterNow(pDest, pThis))
			{
				UnitEnterNow(pDest, pThis);
				return EnteredThenReturn;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x70D965, FootClass_QueueEnter_ForceEnter, 0x7)
{
	GET(FootClass* const, pThis, ESI);

	const auto pDest = abstract_cast<UnitClass*>(pThis->QueueUpToEnter);

	if (pDest && !pThis->Deactivated && !pThis->IsUnderEMP() && !pThis->Locomotor->Is_Moving() // Entering while moving can cause many problems
		&& TechnoTypeExt::ExtMap.Find(pDest->Type)->NoQueueUpToEnter.Get(RulesExt::Global()->NoQueueUpToEnter))
	{
		if (IsCloseEnoughToEnter(pDest, pThis))
		{
			const auto absType = pThis->WhatAmI();

			if (absType == AbstractType::Infantry)
			{
				if (CanEnterNow(pDest, pThis))
					InfantryEnterNow(pDest, static_cast<InfantryClass*>(pThis));
			}
			else if (absType == AbstractType::Unit)
			{
				if (CanEnterNow(pDest, pThis))
					UnitEnterNow(pDest, static_cast<UnitClass*>(pThis));
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x70D894, FootClass_UpdateEnter_UpdateEnterPosition, 0x7)
{
	GET(FootClass* const, pThis, ESI);
	GET(UnitClass* const, pDest, EDI); // Is techno not unit, only for convenience

	if (pDest->WhatAmI() == AbstractType::Unit && !pThis->Deactivated && !pThis->IsUnderEMP() && !pDest->Locomotor->Is_Moving())
	{
		if (IsCloseEnoughToEnter(pDest, pThis))
		{
			if (TechnoTypeExt::ExtMap.Find(pDest->Type)->NoQueueUpToEnter.Get(RulesExt::Global()->NoQueueUpToEnter))
			{
				if (!pThis->Locomotor->Is_Moving()) // Entering while moving can cause many problems
				{
					const auto absType = pThis->WhatAmI();

					if (absType == AbstractType::Infantry)
					{
						if (CanEnterNow(pDest, pThis))
							InfantryEnterNow(pDest, static_cast<InfantryClass*>(pThis));
					}
					else if (absType == AbstractType::Unit)
					{
						if (CanEnterNow(pDest, pThis))
							UnitEnterNow(pDest, static_cast<UnitClass*>(pThis));
					}
				}
			}
		}
		else if (!pThis->Destination) // Move to enter position, prevent other passengers from waiting for call and not moving early
		{
			if (TechnoTypeExt::ExtMap.Find(pDest->Type)->NoQueueUpToEnter.Get(RulesExt::Global()->NoQueueUpToEnter))
			{
				auto cell = CellStruct::Empty;
				reinterpret_cast<CellStruct*(__thiscall*)(FootClass*, CellStruct*, AbstractClass*)>(0x703590)(pThis, &cell, pDest);

				if (cell != CellStruct::Empty)
				{
					pThis->SetDestination(MapClass::Instance.GetCellAt(cell), true);
					pThis->QueueMission(Mission::Move, false);
					pThis->NextMission();
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x737945, UnitClass_ReceiveCommand_MoveTransporter, 0x7)
{
	enum { SkipGameCode = 0x737952 };

	GET(UnitClass* const, pThis, ESI);
	GET(FootClass* const, pPassenger, EDI);

	// Move to the vicinity of the passenger
	auto cell = CellStruct::Empty;
	reinterpret_cast<CellStruct*(__thiscall*)(FootClass*, CellStruct*, AbstractClass*)>(0x703590)(pThis, &cell, pPassenger);
	pThis->SetDestination((cell != CellStruct::Empty ? static_cast<AbstractClass*>(MapClass::Instance.GetCellAt(cell)) : pPassenger), true);

	return SkipGameCode;
}

// Rewrite from 0x4835D5/0x74004B, replace check pThis->GetCell()->LandType != LandType::Water
static inline bool CanUnloadNow(UnitClass* pTransport, FootClass* pPassenger)
{
	if (TechnoTypeExt::ExtMap.Find(pTransport->Type)->AmphibiousUnload.Get(RulesExt::Global()->AmphibiousUnload))
		return GroundType::Array[static_cast<int>(pTransport->GetCell()->LandType)].Cost[static_cast<int>(pPassenger->GetTechnoType()->SpeedType)] != 0.0;

	return pTransport->GetCell()->LandType != LandType::Water;
}

DEFINE_HOOK(0x73DC9C, UnitClass_Mission_Unload_NoQueueUpToUnloadBreak, 0xA)
{
	enum { SkipGameCode = 0x73E289 };

	GET(UnitClass* const, pThis, ESI);
	GET(FootClass* const, pPassenger, EDI);

	// Restore vanilla function
	pPassenger->Undiscover();

	// Play the sound when interrupted for some reason
	if (TechnoTypeExt::ExtMap.Find(pThis->Type)->NoQueueUpToUnload.Get(RulesExt::Global()->NoQueueUpToUnload))
		VoxClass::PlayAtPos(pThis->Type->LeaveTransportSound, &pThis->Location);

	return SkipGameCode;
}

DEFINE_HOOK(0x73DC1E, UnitClass_Mission_Unload_NoQueueUpToUnloadLoop, 0xA)
{
	enum { UnloadLoop = 0x73D8CB, UnloadReturn = 0x73E289, NoUnloadReturn = 0x73D8AA };

	GET(UnitClass* const, pThis, ESI);

	const auto pPassenger = pThis->Passengers.GetFirstPassenger();

	if (TechnoTypeExt::ExtMap.Find(pThis->Type)->NoQueueUpToUnload.Get(RulesExt::Global()->NoQueueUpToUnload))
	{
		if (!pPassenger || pThis->Passengers.NumPassengers <= pThis->NonPassengerCount)
		{
			// If unloading is required within one frame, the sound will only be played when the last passenger leaves
			VoxClass::PlayAtPos(pThis->Type->LeaveTransportSound, &pThis->Location);
			return UnloadReturn;
		}
		else if (!CanUnloadNow(pThis, pPassenger))
		{
			VoxClass::PlayAtPos(pThis->Type->LeaveTransportSound, &pThis->Location);
			pThis->MissionStatus = 0; // Then retry
			return NoUnloadReturn;
		}

		R->EBX(0); // Reset
		return UnloadLoop;
	}
	else if (pPassenger && !CanUnloadNow(pThis, pPassenger))
	{
		VoxClass::PlayAtPos(pThis->Type->LeaveTransportSound, &pThis->Location);
		pThis->MissionStatus = 0; // Then retry
		return NoUnloadReturn;
	}

	// PlayAtPos has already handled the situation where Sound is less than 0 internally, so unnecessary checks will be skipped
	VoxClass::PlayAtPos(pThis->Type->LeaveTransportSound, &pThis->Location);
	return UnloadReturn;
}

#pragma endregion

#pragma region AmphibiousEnterAndUnload

// Enter building
DEFINE_JUMP(LJMP, 0x43C38D, 0x43C3FF); // Skip amphibious and naval check if no Ares

// Enter unit
DEFINE_HOOK(0x73796B, UnitClass_ReceiveCommand_AmphibiousEnter, 0x7)
{
	enum { ContinueCheck = 0x737990, MoveToPassenger = 0x737974 };

	GET(CellClass* const, pCell, EBP);

	if (pCell->LandType != LandType::Water)
		return ContinueCheck;

	GET(UnitClass* const, pThis, ESI);

	return TechnoTypeExt::ExtMap.Find(pThis->Type)->AmphibiousEnter.Get(RulesExt::Global()->AmphibiousEnter) ? ContinueCheck : MoveToPassenger;
}

// Unit unload
static inline bool CanUnloadLater(UnitClass* pTransport)
{
	return pTransport->Passengers.GetFirstPassenger()
		&& (TechnoTypeExt::ExtMap.Find(pTransport->Type)->AmphibiousUnload.Get(RulesExt::Global()->AmphibiousUnload)
			|| pTransport->GetCell()->LandType != LandType::Water);
}

DEFINE_HOOK(0x74004B, UnitClass_MouseOverObject_AmphibiousUnload, 0x6)
{
	enum { ContinueCheck = 0x7400C6, CannotUnload = 0x7400BE };

	GET(UnitClass* const, pThis, ESI);

	return CanUnloadLater(pThis) ? ContinueCheck : CannotUnload;
}

DEFINE_HOOK(0x70106A, TechnoClass_CanDeploySlashUnload_AmphibiousUnload, 0x6)
{
	enum { ContinueCheck = 0x701087, CannotUnload = 0x700DCE };

	GET(UnitClass* const, pThis, ESI);

	return !pThis->Owner->IsHumanPlayer || CanUnloadLater(pThis) ? ContinueCheck : CannotUnload;
}

DEFINE_HOOK(0x73D769, UnitClass_Mission_Unload_ReplaceLandTypeCheck, 0x7)
{
	enum { MoveToLand = 0x73D772, UnloadCheck = 0x73D7E4 };

	GET(UnitClass* const, pThis, ESI);

	const auto pPassenger = pThis->Passengers.GetFirstPassenger();

	return (!pPassenger || CanUnloadNow(pThis, pPassenger)) ? UnloadCheck : MoveToLand;
}

DEFINE_HOOK(0x73D7B7, UnitClass_Mission_Unload_CheckInvalidCell, 0x6)
{
	enum { CannotUnload = 0x73D87F };

	GET(const CellStruct, cell, EAX);

	return cell != CellStruct::Empty ? 0 : CannotUnload;
}

DEFINE_HOOK(0x740C9C, UnitClass_vt_entry_304_ReplaceSpeedType, 0x7)
{
	GET(UnitClass* const, pThis, EDI);

	if (TechnoTypeExt::ExtMap.Find(pThis->Type)->AmphibiousUnload.Get(RulesExt::Global()->AmphibiousUnload))
	{
		if (const auto pPassenger = pThis->Passengers.GetFirstPassenger())
		{
			GET(const int, speedType, EDX);
			R->EDX(speedType + static_cast<int>(pPassenger->GetTechnoType()->SpeedType)); // For GroundType.Cost
		}
	}

	return 0;
}

DEFINE_HOOK(0x73DAB9, UnitClass_Mission_Unload_ReplaceMovementZone, 0x5)
{
	GET(UnitClass* const, pThis, ESI);

	if (TechnoTypeExt::ExtMap.Find(pThis->Type)->AmphibiousUnload.Get(RulesExt::Global()->AmphibiousUnload))
	{
		GET(FootClass* const, pFoot, EDI);
		R->EDX(pFoot->GetTechnoType()->MovementZone); // Replace hard code
	}

	return 0;
}

#pragma endregion
