#include "Body.h"

#include <Ext/House/Body.h>
#include <FactoryClass.h>
#include <New/Type/ResourceTypeClass.h>

DEFINE_HOOK(0x4C9BD5, FactoryClass_AI_ProcessProductionStep, 0x31)
{
	GET(FactoryClass*, pFactory, ESI);
	GET(int, moneyCost, EDI);

	const auto pOwner = pFactory->Owner;
	if (pOwner)
	{
		const auto pHouseExt = HouseExt::TryFetch(pOwner);
		const int availableMoney = pOwner->Available_Money();
		bool canAfford = (moneyCost <= availableMoney);

		for (size_t resIdx = 0; resIdx < ResourceTypeClass::Array.size(); ++resIdx)
		{
			const auto pResource = ResourceTypeClass::Array[resIdx].get();
			if (pResource && pResource->IsMoneyResource())
			{
				if (pHouseExt && !pHouseExt->IsResourceEnabled(static_cast<int>(resIdx)))
				{
					canAfford = false;
				}
				break;
			}
		}

		TechnoTypeClass* pType = pFactory->Object ? pFactory->Object->GetTechnoType() : (pFactory->QueuedObjects.Count > 0 ? pFactory->QueuedObjects[0] : nullptr);
		const auto pTypeExt = TechnoTypeExt::TryFetch(pType);

		std::vector<int> resDues;
		if (pTypeExt && pHouseExt && !pTypeExt->ResourceCosts.empty())
		{
			const int stage = pFactory->Production.Value;
			const int stepsRemaining = 54 - stage;
			resDues.resize(pTypeExt->ResourceCosts.size(), 0);

			for (size_t i = 0; i < pTypeExt->ResourceCosts.size(); ++i)
			{
				const int totalCost = pTypeExt->ResourceCosts[i];
				if (totalCost > 0)
				{
					const int spent = pHouseExt->GetFactoryResourceSpent(pFactory, i);
					const int remaining = totalCost - spent;
					int resCost = (stage >= 54) ? remaining : (remaining / (stepsRemaining > 0 ? stepsRemaining : 1));
					resCost = std::clamp(resCost, 0, remaining);

					if (resCost > 0)
					{
						resDues[i] = resCost;
						if (!pHouseExt->CanAffordResource(static_cast<int>(i), resCost))
						{
							canAfford = false;
						}
					}
				}
			}
		}

		if (canAfford)
		{
			// Deduct money
			if (moneyCost > 0)
			{
				pOwner->TakeMoney(moneyCost);
			}
			pFactory->Balance -= moneyCost;
			pFactory->OnHold = false;

			// Deduct custom resources
			if (pHouseExt && !resDues.empty())
			{
				for (size_t i = 0; i < resDues.size(); ++i)
				{
					if (resDues[i] > 0)
					{
						pHouseExt->UpdateResourceAmount(static_cast<int>(i), -resDues[i]);
						pHouseExt->AddFactoryResourceSpent(pFactory, i, resDues[i]);
					}
				}
			}
		}
		else
		{
			// Insufficient money or resources: put on hold and revert step
			pFactory->OnHold = true;
			pFactory->Production.Value = std::max(0, pFactory->Production.Value - 1);
		}
	}

	return 0x4C9C06;
}

DEFINE_HOOK(0x4C9C28, FactoryClass_AI_FinishProduction, 0x13)
{
	GET(FactoryClass*, pFactory, ESI);

	if (pFactory->Owner)
	{
		const int remainingMoney = pFactory->Balance;
		if (remainingMoney > 0)
		{
			pFactory->Owner->TakeMoney(remainingMoney);
		}
		pFactory->Balance = 0;

		if (const auto pHouseExt = HouseExt::TryFetch(pFactory->Owner))
		{
			pHouseExt->ClearFactoryResourceState(pFactory);
		}
	}

	return 0x4C9C3B;
}

DEFINE_HOOK(0x4401BB, BuildingClass_AI_PickWithFreeDocks, 0x6)
{
	enum { SkipGameCode = 0x4401D2 };

	GET(BuildingClass*, pBuilding, ESI);

	if (pBuilding->IsUnderEMP())
		return SkipGameCode;

	auto const pOwner = pBuilding->Owner;
	const int index = pOwner->ProducingAircraftTypeIndex;
	auto const pType = index >= 0 ? AircraftTypeClass::Array.GetItem(index) : nullptr;

	if (RulesExt::Global()->AllowParallelAIQueues && !RulesExt::Global()->ForbidParallelAIQueues_Aircraft && (!pType || !TechnoTypeExt::Fetch(pType)->ForbidParallelAIQueues))
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
			if (auto const pBldExt = BuildingExt::TryFetch(pBuilding))
				pBldExt->UpdatePrimaryFactoryAI();
		}
	}

	return 0;
}

DEFINE_HOOK(0x4502F4, BuildingClass_Update_Factory_Phobos, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	const HouseClass* pOwner = pThis->Owner;

	if (pOwner->Production && RulesExt::Global()->AllowParallelAIQueues)
	{
		auto const pOwnerExt = HouseExt::Fetch(pOwner);
		auto const pFactory = pThis->Type->Factory;
		const bool naval = pThis->Type->Naval;
		BuildingClass** currFactory = nullptr;

		switch (pFactory)
		{
		case AbstractType::BuildingType:
			currFactory = &pOwnerExt->Factory_BuildingType;
			break;
		case AbstractType::UnitType:
			currFactory = naval ? &pOwnerExt->Factory_NavyType : &pOwnerExt->Factory_VehicleType;
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

			switch (pFactory)
			{
			case AbstractType::BuildingType:
				if (RulesExt::Global()->ForbidParallelAIQueues_Building)
					return Skip;

				index = pOwner->ProducingBuildingTypeIndex;
				pType = index >= 0 ? BuildingTypeClass::Array.GetItem(index) : nullptr;
				break;
			case AbstractType::InfantryType:
				if (RulesExt::Global()->ForbidParallelAIQueues_Infantry)
					return Skip;

				index = pOwner->ProducingInfantryTypeIndex;
				pType = index >= 0 ? InfantryTypeClass::Array.GetItem(index) : nullptr;
				break;
			case AbstractType::AircraftType:
				if (RulesExt::Global()->ForbidParallelAIQueues_Aircraft)
					return Skip;

				index = pOwner->ProducingAircraftTypeIndex;
				pType = index >= 0 ? AircraftTypeClass::Array.GetItem(index) : nullptr;
				break;
			case AbstractType::UnitType:
				if (naval ? RulesExt::Global()->ForbidParallelAIQueues_Navy : RulesExt::Global()->ForbidParallelAIQueues_Vehicle)
					return Skip;

				index = naval ? HouseExt::Fetch(pOwner)->ProducingNavalUnitTypeIndex : pOwner->ProducingUnitTypeIndex;
				pType = index >= 0 ? UnitTypeClass::Array.GetItem(index) : nullptr;
				break;
			default:
				break;
			}

			if (pType && TechnoTypeExt::Fetch(pType)->ForbidParallelAIQueues)
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

	if (pFactory && pFactory->Owner)
	{
		if (const auto pHouseExt = HouseExt::TryFetch(pFactory->Owner))
		{
			for (size_t i = 0; i < ResourceTypeClass::Array.size(); ++i)
			{
				const int spent = pHouseExt->GetFactoryResourceSpent(pFactory, i);
				if (spent > 0)
				{
					pHouseExt->UpdateResourceAmount(static_cast<int>(i), spent);
				}
			}
			pHouseExt->ClearFactoryResourceState(pFactory);
		}
	}

	auto const pTechno = pFactory->Object;

	if (calledby < 0x7F0000) // Replace the old log with this to figure out where keeps flushing the stream
	{
		Debug::LogGame("(%08x) : %s is abandoning production of %s[%s]\n"
			, calledby - 5
			, pFactory->Owner->PlainName
			, pTechno->GetType()->Name
			, pTechno->get_ID());
	}

	if (!RulesExt::Global()->AllowParallelAIQueues)
		return 0;

	auto const pOwnerExt = HouseExt::Fetch(pFactory->Owner);
	auto const pType = pTechno->GetTechnoType();
	const bool forbid = TechnoTypeExt::Fetch(pType)->ForbidParallelAIQueues;

	switch (pTechno->WhatAmI())
	{
	case AbstractType::Building:
		if (RulesExt::Global()->ForbidParallelAIQueues_Building || forbid)
			pOwnerExt->Factory_BuildingType = nullptr;
		break;
	case AbstractType::Unit:
		if (!pType->Naval)
		{
			if (RulesExt::Global()->ForbidParallelAIQueues_Vehicle || forbid)
				pOwnerExt->Factory_VehicleType = nullptr;
		}
		else
		{
			if (RulesExt::Global()->ForbidParallelAIQueues_Navy || forbid)
				pOwnerExt->Factory_NavyType = nullptr;
		}
		break;
	case AbstractType::Infantry:
		if (RulesExt::Global()->ForbidParallelAIQueues_Infantry || forbid)
			pOwnerExt->Factory_InfantryType = nullptr;
		break;
	case AbstractType::Aircraft:
		if (RulesExt::Global()->ForbidParallelAIQueues_Aircraft || forbid)
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

	auto const pHouseExt = HouseExt::Fetch(pFactory->Owner);

	if (pUnit->Type->Naval && pHouseExt->Factory_NavyType == pFactory)
		pHouseExt->Factory_NavyType = nullptr;
	else if (pHouseExt->Factory_VehicleType == pFactory)
		pHouseExt->Factory_VehicleType = nullptr;

	return 0;
}

DEFINE_HOOK(0x444131, BuildingClass_KickOutUnit_InfantryType_Phobos, 0x6)
{
	GET(BuildingClass*, pFactory, ESI);

	auto const pHouseExt = HouseExt::Fetch(pFactory->Owner);

	if (pHouseExt->Factory_InfantryType == pFactory)
		pHouseExt->Factory_InfantryType = nullptr;

	return 0;
}

DEFINE_HOOK(0x44531F, BuildingClass_KickOutUnit_BuildingType_Phobos, 0xA)
{
	GET(BuildingClass*, pFactory, ESI);

	auto const pHouseExt = HouseExt::Fetch(pFactory->Owner);

	if (pHouseExt->Factory_BuildingType == pFactory)
		pHouseExt->Factory_BuildingType = nullptr;

	return 0;
}

DEFINE_HOOK(0x443CCA, BuildingClass_KickOutUnit_AircraftType_Phobos, 0xA)
{
	GET(BuildingClass*, pFactory, ESI);

	auto const pHouseExt = HouseExt::Fetch(pFactory->Owner);

	if (pHouseExt->Factory_AircraftType == pFactory)
		pHouseExt->Factory_AircraftType = nullptr;

	return 0;
}

DEFINE_HOOK(0x4449FB, BuildingClass_KickOutUnit_CloningVats, 0x8)
{
	enum { SkipGameCode = 0x444A53 };

	GET(BuildingClass*, pFactory, ESI);
	GET(TechnoTypeClass*, pProductionType, EAX);
	const auto pOwner = pFactory->Owner;
	auto info = std::make_pair(pProductionType, pOwner);

	for (const auto pVat : pOwner->CloningVats)
		BuildingExt::KickOutClone(info, 0, pVat);

	return SkipGameCode;
}
