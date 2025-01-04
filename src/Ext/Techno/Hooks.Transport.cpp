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

// This hook point doesn't work with Ares abduction. All codes migrated into the hook "CargoClass_Attach_Hook_BeforeLoad". -- Aephiex
/*
DEFINE_HOOK(0x71067B, TechnoClass_EnterTransport, 0x7)
{
	GET(TechnoClass*, pTransport, ESI);
	GET(FootClass*, pPassenger, EDI);

	if (pTransport && pPassenger)
	{
		auto const pPassengerType = pPassenger->GetTechnoType();
		auto const pPassExt = TechnoExt::ExtMap.Find(pPassenger);
		auto const pTransTypeExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType());

		if (pTransTypeExt->Passengers_SyncOwner && pTransTypeExt->Passengers_SyncOwner_RevertOnExit)
			pPassExt->OriginalPassengerOwner = pPassenger->Owner;

		if (pPassenger->WhatAmI() != AbstractType::Aircraft && pPassenger->WhatAmI() != AbstractType::Building
			&& pPassengerType->Ammo > 0 && pPassExt->TypeExtData->ReloadInTransport)
		{
			ScenarioExt::Global()->TransportReloaders.push_back(pPassExt);
		}
	}

	return 0;
}
*/

// At this point, the passenger is not yet loaded into the transport.
DEFINE_HOOK(0x4733B0, CargoClass_Attach_Hook_BeforeLoad, 0x6)
{
	GET(PassengersClass*, pThis, EDI);
	GET(FootClass*, pPassenger, ESI);

	if (pPassenger)
	{
		// A "PassengersClass" pointer is equal to the transport's pointer plus 0x114.
		// Therefore, we can get the transport's pointer by this manner.
		// This is confirmed to support both transports and Bio Reactors.
		auto dword = reinterpret_cast<DWORD>(pThis);
		auto pTransport = reinterpret_cast<TechnoClass*>(dword - 0x114);

		auto const pPassengerType = pPassenger->GetTechnoType();
		auto const pPassExt = TechnoExt::ExtMap.Find(pPassenger);
		auto const pTransTypeExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType());

		if (pTransTypeExt->Passengers_SyncOwner && pTransTypeExt->Passengers_SyncOwner_RevertOnExit)
			pPassExt->OriginalPassengerOwner = pPassenger->Owner;

		if (pPassenger->WhatAmI() != AbstractType::Aircraft && pPassenger->WhatAmI() != AbstractType::Building
			&& pPassengerType->Ammo > 0 && pPassExt->TypeExtData->ReloadInTransport)
		{
			ScenarioExt::Global()->TransportReloaders.push_back(pPassExt);
		}
	}

	return 0;
}

// At this point, the passenger is already loaded into the transport.
DEFINE_HOOK(0x47342A, CargoClass_Attach_Hook_AfterLoad, 0x5)
{
	GET(PassengersClass*, pThis, EDI);
	GET(FootClass*, pPassenger, ESI);

	if (pPassenger)
	{
		// A "PassengersClass" pointer is equal to the transport's pointer plus 0x114.
		// Therefore, we can get the transport's pointer by this manner.
		// This is confirmed to support both transports and Bio Reactors.
		auto dword = reinterpret_cast<DWORD>(pThis);
		auto pTransport = reinterpret_cast<TechnoClass*>(dword - 0x114);

		auto const pTransTypeExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType());
		auto const pPassExt = TechnoExt::ExtMap.Find(pPassenger);
		auto const pPassTypeExt = pPassExt->TypeExtData;

		// Set a pointer to the Bio Reactor.
		if (auto pTransportBld = abstract_cast<BuildingClass*>(pTransport))
		{
			pPassExt->HousingMe = pTransportBld;
		}

		pTransTypeExt->InvokeEvent(EventTypeClass::WhenLoad, pTransport, pPassenger);
		pPassTypeExt->InvokeEvent(EventTypeClass::WhenBoard, pPassenger, pTransport);
	}

	return 0;
}

// At this point, the passenger is already detached from the transport.
DEFINE_HOOK(0x4DE722, FootClass_RemoveFirstPassenger_Hook, 0x6)
{
	GET(TechnoClass*, pTransport, ESI);
	GET(FootClass*, pPassenger, EAX);

	if (pTransport && pPassenger)
	{
		auto const pPassType = pPassenger->GetTechnoType();
		auto const pPassExt = TechnoExt::ExtMap.Find(pPassenger);
		auto const pPassTypeExt = pPassExt->TypeExtData;
		auto const pTransTypeExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType());

		// Remove from transport reloader list before switching house
		if (pPassenger->WhatAmI() != AbstractType::Aircraft && pPassenger->WhatAmI() != AbstractType::Building
			&& pPassType->Ammo > 0 && pPassExt->TypeExtData->ReloadInTransport)
		{
			auto& vec = ScenarioExt::Global()->TransportReloaders;
			vec.erase(std::remove(vec.begin(), vec.end(), pPassExt), vec.end());
		}

		if (pTransTypeExt->Passengers_SyncOwner && pTransTypeExt->Passengers_SyncOwner_RevertOnExit &&
			pPassExt->OriginalPassengerOwner)
		{
			pPassenger->SetOwningHouse(pPassExt->OriginalPassengerOwner, false);
		}

		// Revoke the Bio Reactor pointer.
		pPassExt->HousingMe = nullptr;
		pTransTypeExt->InvokeEvent(EventTypeClass::WhenUnload, pTransport, pPassenger);
		pPassTypeExt->InvokeEvent(EventTypeClass::WhenUnboard, pPassenger, pTransport);
	}

	return 0;
}

// This method is invoked every time after an occupant is inserted into or removed from a building's occupant array,
// in order to evaluate the garrisonable structure's threat.
// It is an idea place to get the pointer to the building for its occupants.
DEFINE_HOOK(0x70F6EC, TechnoClass_UpdateThreatToCell_Hook, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto pBld = abstract_cast<BuildingClass*>(pThis))
	{
		if (pBld->GetOccupantCount() > 0)
		{
			for (auto pOccupant : pBld->Occupants)
			{
				auto const pBldTypeExt = TechnoTypeExt::ExtMap.Find(pBld->Type);
				auto pPassExt = TechnoExt::ExtMap.Find(pOccupant);
				if (!pPassExt->HousingMe)
				{
					pPassExt->HousingMe = pBld;
					pBldTypeExt->InvokeEvent(EventTypeClass::WhenLoad, pBld, pOccupant);
					pPassExt->TypeExtData->InvokeEvent(EventTypeClass::WhenBoard, pOccupant, pBld);
				}
			}
		}
	}

	return 0;
}

// At this point, the building is confirmed to release its occupants, but has not yet proceed.
// We record its passengers here, and get ready to invoke the "WhenUnload" event some time later.
DEFINE_HOOK(0x458060, BuildingClass_Remove_Occupants_BeforeHook, 0x5)
{
	GET(BuildingClass*, pBld, ESI);

	if (pBld->GetOccupantCount() > 0)
	{
		auto& vec = ScenarioExt::Global()->OccupantsCache;
		for (auto pOccupant : pBld->Occupants)
		{
			auto pPassExt = TechnoExt::ExtMap.Find(pOccupant);
			vec.push_back(pPassExt);
		}
	}

	return 0;
}

// At this point, the building has already released all of its occupants.
DEFINE_HOOK(0x4581CD, BuildingClass_Remove_Occupants_AfterHook, 0x6)
{
	GET(BuildingClass*, pBld, ESI);

	auto& vec = ScenarioExt::Global()->OccupantsCache;
	if (!vec.empty())
	{
		auto const pBldTypeExt = TechnoTypeExt::ExtMap.Find(pBld->Type);

		for (auto pPassExt : vec)
		{
			pPassExt->HousingMe = nullptr;
			pBldTypeExt->InvokeEvent(EventTypeClass::WhenUnload, pBld, pPassExt->OwnerObject());
			pPassExt->TypeExtData->InvokeEvent(EventTypeClass::WhenUnboard, pPassExt->OwnerObject(), pBld);
		}

		vec.clear();
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
