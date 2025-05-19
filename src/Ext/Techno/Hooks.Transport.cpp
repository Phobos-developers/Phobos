#include "Body.h"

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
		if (TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType())->Passengers_SyncOwner)
			return returnAddress;
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
	GET(FootClass*, pPassenger, EDI);

	if (pPassenger)
	{
		GET(TechnoClass*, pThis, ESI);

		if (!pThis)
			return 0;

		auto const pType = pPassenger->GetTechnoType();
		auto const pExt = TechnoExt::ExtMap.Find(pPassenger);
		auto const whatAmI = pPassenger->WhatAmI();
		auto const pTransTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pTransTypeExt->Passengers_SyncOwner && pTransTypeExt->Passengers_SyncOwner_RevertOnExit)
			pExt->OriginalPassengerOwner = pPassenger->Owner;

		if (whatAmI != AbstractType::Aircraft && whatAmI != AbstractType::Building
			&& pType->Ammo > 0 && pExt->TypeExtData->ReloadInTransport)
		{
			ScenarioExt::Global()->TransportReloaders.push_back(pExt);
		}
	}

	return 0;
}

DEFINE_HOOK(0x4DE722, FootClass_LeaveTransport, 0x6)
{
	GET(FootClass*, pPassenger, EAX);

	if (pPassenger)
	{
		GET(TechnoClass*, pThis, ESI);

		if (!pThis)
			return 0;

		auto const pType = pPassenger->GetTechnoType();
		auto const pExt = TechnoExt::ExtMap.Find(pPassenger);
		auto const whatAmI = pPassenger->WhatAmI();
		auto const pTransTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		// Remove from transport reloader list before switching house
		if (whatAmI != AbstractType::Aircraft && whatAmI != AbstractType::Building
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
		auto pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType());
		R->EAX(pExt->OpenTopped_RangeBonus.Get(RulesClass::Instance->OpenToppedRangeBonus));
		return 0x6F72DE;
	}

	return 0;
}

DEFINE_HOOK(0x71A82C, TemporalClass_AI_Opentopped_WarpDistance, 0xC)
{
	GET(TemporalClass* const, pThis, ESI);

	if (auto pTransport = pThis->Owner->Transporter)
	{
		auto pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType());
		R->EDX(pExt->OpenTopped_WarpDistance.Get(RulesClass::Instance->OpenToppedWarpDistance));
		return 0x71A838;
	}

	return 0;
}

DEFINE_HOOK(0x710552, TechnoClass_SetOpenTransportCargoTarget_ShareTarget, 0x6)
{
	enum { ReturnFromFunction = 0x71057F };

	GET_STACK(AbstractClass* const, pTarget, STACK_OFFSET(0x8, 0x4));

	if (pTarget)
	{
		GET(TechnoClass* const, pThis, ECX);
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (!pTypeExt->OpenTopped_ShareTransportTarget)
			return ReturnFromFunction;
	}

	return 0;
}

#pragma region TransportFix

DEFINE_HOOK(0x51D45B, InfantryClass_Scatter_NoProcess, 0x6)
{
	enum { SkipGameCode = 0x51D47B };

	REF_STACK(const int, addr, STACK_OFFSET(0x50, 0));
	// Skip process in InfantryClass::UpdatePosition which can create invisible barrier
	return (addr == 0x51A4B5) ? SkipGameCode : 0;
}

DEFINE_HOOK(0x4D92BF, FootClass_Mission_Enter_CheckLink, 0x5)
{
	enum { NextAction = 0x4D92ED, NotifyUnlink = 0x4D92CE, DoNothing = 0x4D946C };

	GET(UnitClass* const, pThis, ESI);
	// Restore vanilla check
	if (pThis->IsTether)
		return NextAction;

	GET(const RadioCommand, answer, EAX);

	if (answer == RadioCommand::AnswerPositive)
		return NextAction;
	// The link should not be disconnected while the transporter is in motion (passengers waiting to enter),
	// as this will result in the first passenger not getting on board
	return answer == RadioCommand::RequestLoading ? DoNothing : NotifyUnlink;
}

DEFINE_HOOK(0x73769E, UnitClass_ReceiveCommand_NoEnterOnBridge, 0x6)
{
	enum { NoEnter = 0x73780F };

	GET(UnitClass* const, pThis, ESI);
	GET(TechnoClass* const, pCall, EDI);
	// If both the transport vehicle and passengers are on the bridge, they should not board
	return pThis->OnBridge && pCall->OnBridge ? NoEnter : 0;
}

DEFINE_HOOK(0x70D842, FootClass_UpdateEnter_NoMoveToBridge, 0x5)
{
	enum { NoMove = 0x70D84F };

	GET(TechnoClass* const, pEnter, EDI);
	// If the transport vehicle is on the bridge, passengers should wait in place for the transport vehicle to arrive
	return pEnter->OnBridge && (pEnter->WhatAmI() == AbstractType::Unit && static_cast<UnitClass*>(pEnter)->Type->Passengers > 0) ? NoMove : 0;
}

DEFINE_HOOK(0x70D910, FootClass_QueueEnter_NoMoveToBridge, 0x5)
{
	enum { NoMove = 0x70D977 };

	GET(TechnoClass* const, pEnter, EAX);
	// If the transport vehicle is on the bridge, passengers should wait in place for the transport vehicle to arrive
	return pEnter->OnBridge && (pEnter->WhatAmI() == AbstractType::Unit && static_cast<UnitClass*>(pEnter)->Type->Passengers > 0) ? NoMove : 0;
}

DEFINE_HOOK(0x7196BB, TeleportLocomotionClass_Process_MarkDown, 0xA)
{
	enum { SkipGameCode = 0x7196C5 };

	GET(FootClass*, pLinkedTo, ECX);
	// When Teleport units board transport vehicles on the bridge, the lack of this repair can lead to numerous problems
	// An impassable invisible barrier will be generated on the bridge (the object linked list of the cell will leave it)
	// And the transport vehicle will board on the vehicle itself (BFRT Passenger:..., BFRT)
	// If any infantry attempts to pass through this position on the bridge later, it will cause the game to freeze
	auto shouldMarkDown = [pLinkedTo]()
	{
		if (pLinkedTo->GetCurrentMission() != Mission::Enter)
			return true;

		const auto pEnter = pLinkedTo->GetNthLink();

		return (!pEnter || pEnter->GetTechnoType()->Passengers <= 0);
	};

	if (shouldMarkDown())
		pLinkedTo->Mark(MarkType::Down);

	return SkipGameCode;
}

#pragma endregion

#pragma region AmphibiousEnterAndUnload

// Enter building
DEFINE_JUMP(LJMP, 0x43C38D, 0x43C3FF); // Skip amphibious and naval check if no Ares

// TODO Enter unit


DEFINE_HOOK(0x73796B, UnitClass_ReceiveCommand_AmphibiousEnter, 0x7)
{
	enum { ContinueCheck = 0x737990, MoveToPassenger = 0x737974 };

	GET(UnitClass* const, pThis, ESI);

	if (pThis->OnBridge)
		return MoveToPassenger;

	GET(CellClass* const, pCell, EBP);

	return (pCell->LandType != LandType::Water) ? ContinueCheck : MoveToPassenger;
}

#pragma endregion
