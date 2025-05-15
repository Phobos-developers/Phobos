#include "Body.h"

#include <Ext/House/Body.h>

DEFINE_HOOK(0x4401BB, BuildingClass_AI_PickWithFreeDocks, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	auto pRulesExt = RulesExt::Global();
	HouseClass* pOwner = pBuilding->Owner;
	int index = pOwner->ProducingAircraftTypeIndex;
	auto const pType = index >= 0 ? AircraftTypeClass::Array.GetItem(index) : nullptr;

	if (pRulesExt->AllowParallelAIQueues && !pRulesExt->ForbidParallelAIQueues_Aircraft && (!pType || !TechnoTypeExt::ExtMap.Find(pType)->ForbidParallelAIQueues))
		return 0;

	if (pOwner->Type->MultiplayPassive
		|| pOwner->IsCurrentPlayer()
		|| pOwner->IsNeutral())
		return 0;

	if (pBuilding->Type->Factory == AbstractType::AircraftType)
	{
		if (pBuilding->Factory
			&& !BuildingExt::HasFreeDocks(pBuilding))
		{
			if (auto pBldExt = BuildingExt::ExtMap.Find(pBuilding))
				pBldExt->UpdatePrimaryFactoryAI();
		}
	}

	return 0;
}

DEFINE_HOOK(0x4502F4, BuildingClass_Update_Factory_Phobos, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	HouseClass* pOwner = pThis->Owner;

	auto pRulesExt = RulesExt::Global();

	if (pOwner->Production && pRulesExt->AllowParallelAIQueues)
	{
		auto pOwnerExt = HouseExt::ExtMap.Find(pOwner);
		BuildingClass** currFactory = nullptr;

		switch (pThis->Type->Factory)
		{
		case AbstractType::BuildingType:
			currFactory = &pOwnerExt->Factory_BuildingType;
			break;
		case AbstractType::UnitType:
			currFactory = pThis->Type->Naval ? &pOwnerExt->Factory_NavyType : &pOwnerExt->Factory_VehicleType;
			break;
		case AbstractType::InfantryType:
			currFactory = &pOwnerExt->Factory_InfantryType;
			break;
		case AbstractType::AircraftType:
			currFactory = &pOwnerExt->Factory_AircraftType;
			break;
		default:
			break;
		}

		if (!*currFactory)
		{
			*currFactory = pThis;
			return 0;
		}
		else if (*currFactory != pThis)
		{
			enum { Skip = 0x4503CA };


			TechnoTypeClass* pType = nullptr;
			int index = -1;

			switch (pThis->Type->Factory)
			{
			case AbstractType::BuildingType:
				if (pRulesExt->ForbidParallelAIQueues_Building)
					return Skip;

				index = pOwner->ProducingBuildingTypeIndex;
				pType = index >= 0 ? BuildingTypeClass::Array.GetItem(index) : nullptr;
				break;
			case AbstractType::InfantryType:
				if (pRulesExt->ForbidParallelAIQueues_Infantry)
					return Skip;

				index = pOwner->ProducingInfantryTypeIndex;
				pType = index >= 0 ? InfantryTypeClass::Array.GetItem(index) : nullptr;
				break;
			case AbstractType::AircraftType:
				if (pRulesExt->ForbidParallelAIQueues_Aircraft)
					return Skip;

				index = pOwner->ProducingAircraftTypeIndex;
				pType = index >= 0 ? AircraftTypeClass::Array.GetItem(index) : nullptr;
				break;
			case AbstractType::UnitType:
				if (pThis->Type->Naval ? pRulesExt->ForbidParallelAIQueues_Navy : pRulesExt->ForbidParallelAIQueues_Vehicle)
					return Skip;

				if (pThis->Type->Naval)
				{
					auto const pExt = HouseExt::ExtMap.Find(pOwner);
					index = pExt->ProducingNavalUnitTypeIndex;
				}
				else
				{
					index = pOwner->ProducingUnitTypeIndex;
				}

				pType = index >= 0 ? UnitTypeClass::Array.GetItem(index) : nullptr;

				break;
			default:
				break;
			}

			if (pType && TechnoTypeExt::ExtMap.Find(pType)->ForbidParallelAIQueues)
				return Skip;
		}
	}

	return 0;
}

//const byte old_empty_log[] = { 0xC3 };
DEFINE_JUMP(CALL, 0x4CA016, 0x4CA19F); // randomly chosen 0xC3

DEFINE_HOOK(0x4CA07A, FactoryClass_AbandonProduction_Phobos, 0x8)
{
	GET(FactoryClass*, pFactory, ESI);
	GET_STACK(DWORD const, calledby, 0x18);

	TechnoClass* pTechno = pFactory->Object;
	if (calledby < 0x7F0000) // Replace the old log with this to figure out where keeps flushing the stream
	{
		Debug::LogGame("(%08x) : %s is abandoning production of %s[%s]\n"
			, calledby - 5
			, pFactory->Owner->PlainName
			, pTechno->GetType()->Name
			, pTechno->get_ID());
	}

	auto pRulesExt = RulesExt::Global();

	if (!pRulesExt->AllowParallelAIQueues)
		return 0;

	auto const pOwnerExt = HouseExt::ExtMap.Find(pFactory->Owner);
	bool forbid = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType())->ForbidParallelAIQueues;

	switch (pTechno->WhatAmI())
	{
	case AbstractType::Building:
		if (pRulesExt->ForbidParallelAIQueues_Building || forbid)
			pOwnerExt->Factory_BuildingType = nullptr;
		break;
	case AbstractType::Unit:
		if (!pTechno->GetTechnoType()->Naval)
		{
			if (pRulesExt->ForbidParallelAIQueues_Vehicle || forbid)
				pOwnerExt->Factory_VehicleType = nullptr;
		}
		else
		{
			if (pRulesExt->ForbidParallelAIQueues_Navy || forbid)
				pOwnerExt->Factory_NavyType = nullptr;
		}
		break;
	case AbstractType::Infantry:
		if (pRulesExt->ForbidParallelAIQueues_Infantry || forbid)
			pOwnerExt->Factory_InfantryType = nullptr;
		break;
	case AbstractType::Aircraft:
		if (pRulesExt->ForbidParallelAIQueues_Aircraft || forbid)
			pOwnerExt->Factory_AircraftType = nullptr;
		break;
	default:
		break;
	}

	return 0;
}

DEFINE_HOOK(0x444119, BuildingClass_KickOutUnit_UnitType_Phobos, 0x6)
{
	GET(UnitClass*, pUnit, EDI);
	GET(BuildingClass*, pFactory, ESI);

	auto const pHouseExt = HouseExt::ExtMap.Find(pFactory->Owner);

	if (pUnit->Type->Naval && pHouseExt->Factory_NavyType == pFactory)
		pHouseExt->Factory_NavyType = nullptr;
	else if (pHouseExt->Factory_VehicleType == pFactory)
		pHouseExt->Factory_VehicleType = nullptr;

	return 0;
}

DEFINE_HOOK(0x444131, BuildingClass_KickOutUnit_InfantryType_Phobos, 0x6)
{
	GET(BuildingClass*, pFactory, ESI);

	auto const pHouseExt = HouseExt::ExtMap.Find(pFactory->Owner);

	if (pHouseExt->Factory_InfantryType == pFactory)
		pHouseExt->Factory_InfantryType = nullptr;

	return 0;
}

DEFINE_HOOK(0x44531F, BuildingClass_KickOutUnit_BuildingType_Phobos, 0xA)
{
	GET(BuildingClass*, pFactory, ESI);

	auto const pHouseExt = HouseExt::ExtMap.Find(pFactory->Owner);

	if (pHouseExt->Factory_BuildingType == pFactory)
		pHouseExt->Factory_BuildingType = nullptr;

	return 0;
}

DEFINE_HOOK(0x443CCA, BuildingClass_KickOutUnit_AircraftType_Phobos, 0xA)
{
	GET(BuildingClass*, pFactory, ESI);

	auto const pHouseExt = HouseExt::ExtMap.Find(pFactory->Owner);

	if (pHouseExt->Factory_AircraftType == pFactory)
		pHouseExt->Factory_AircraftType = nullptr;

	return 0;
}

inline InfantryClass* CreateInfantryFromFactory(TechnoTypeClass* pType, HouseClass* pOwner)
{
	// BuildLimit check goes before creation
	if (pType->BuildLimit > 0)
	{
		int sum = pOwner->CountOwnedNow(pType);

		// copy Ares' deployable units x build limit fix
		if (auto const pUndeploy = pType->UndeploysInto)
			sum += pOwner->CountOwnedNow(pUndeploy);

		if (sum >= pType->BuildLimit)
			return nullptr;
	}

	if (auto const pInfantry = static_cast<InfantryClass*>(pType->CreateObject(pOwner)))
	{
		return pInfantry;
	}

	return nullptr;
}

DEFINE_HOOK(0x444DDF, BuildingClass_KickOutUnit_InfantrySquad, 0x5)
{
	Debug::Log("InfantryFactory Step 0\n");

	GET(BuildingClass*, pFactory, ESI);
	GET(TechnoClass*, pTechno, EDI);

	const auto pExtType = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());
	const auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
	bool isInitAsTeam = pExtType->Squad_IsInitAsTeam;
	SquadManager* pSquadManager;

	if (isInitAsTeam)
	{
		pSquadManager = new SquadManager;
		pSquadManager->AddTechno(pTechno);
		pTechnoExt->SquadManager = pSquadManager;
		pTechnoExt->HasSquad = true;
	}

	if (pExtType->Squad_Members.size() > 0)
	{
		for (int i = 0; i < pExtType->Squad_Members.size(); i++)
		{
			auto pType = pExtType->Squad_Members[i];
			auto pInfantry = CreateInfantryFromFactory(pType, pTechno->GetOwningHouse());
			if (pInfantry != nullptr)
			{
				++Unsorted::ScenarioInit;
				pInfantry->Unlimbo(pTechno->GetCoords(), DirType::North);
				if (pFactory->ArchiveTarget)
				{
					pInfantry->QueueMission(Mission::Move, 0);
					pInfantry->SetDestination(pFactory->ArchiveTarget, 1);
				}
				--Unsorted::ScenarioInit;
				if (isInitAsTeam)
				{
					auto tempTechnoExt = TechnoExt::ExtMap.Find(pInfantry);
					pSquadManager->AddTechno(pInfantry);
					tempTechnoExt->SquadManager = pSquadManager;
					tempTechnoExt->HasSquad = true;
				}

			}
		}
	}

	return 0x444971;
}

DEFINE_HOOK(0x444971, BuildingClass_KickOutUnit_PassengerSquad, 0x5)
{
	GET(BuildingClass*, pFactory, ESI);
	GET(TechnoClass*, pTechno, EDI);

	if ( pTechno->WhatAmI() == AbstractType::Unit)
	{
		const auto pExtType = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

		if (pExtType->Squad_IsInitAsTeam)
		{
			if (pTechno->Passengers.NumPassengers > 0)
			{
				SquadManager* pSquadManager = new SquadManager;
				FootClass* pOldPassenger = pTechno->Passengers.FirstPassenger;
				int PassengersNum = pTechno->Passengers.NumPassengers;

				//DynamicVectorClass<FootClass*> passengersList;

				while (pOldPassenger)
				{
					auto tempTechnoExt = TechnoExt::ExtMap.Find(pOldPassenger);
					pSquadManager->AddTechno(pOldPassenger);
					tempTechnoExt->SquadManager = pSquadManager;
					tempTechnoExt->HasSquad = true;
					pOldPassenger = static_cast<FootClass*>(pOldPassenger->NextObject);
				}
			}
		}
	}
	
	return 0;
}
