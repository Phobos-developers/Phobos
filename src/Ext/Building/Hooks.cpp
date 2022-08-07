#include "Body.h"

#include <BulletClass.h>
#include <UnitClass.h>

#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>


DEFINE_HOOK(0x43FB29, BuildingClass_AI, 0x8)
{
	GET(BuildingClass*, pThis, ESI);

	// Do not search this up again in any functions called here because it is costly for performance - Starkku
	auto pExt = BuildingExt::ExtMap.Find(pThis);
	auto pType = pThis->Type;

	// Set only if unset or type has changed
	if (!pExt->TypeExtData || pExt->TypeExtData->OwnerObject() != pType)
		pExt->TypeExtData = BuildingTypeExt::ExtMap.Find(pType);

	pExt->DisplayGrinderRefund();

	return 0;
}

DEFINE_HOOK(0x7396D2, UnitClass_TryToDeploy_Transfer, 0x5)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	if (pUnit->Type->DeployToFire && pUnit->Target)
		pStructure->LastTarget = pUnit->Target;

	if (auto pStructureExt = BuildingExt::ExtMap.Find(pStructure))
		pStructureExt->DeployedTechno = true;

	return 0;
}

DEFINE_HOOK(0x449ADA, BuildingClass_MissionConstruction_DeployToFireFix, 0x0)
{
	GET(BuildingClass*, pThis, ESI);

	auto pExt = BuildingExt::ExtMap.Find(pThis);
	if (pExt && pExt->DeployedTechno && pThis->LastTarget)
	{
		pThis->Target = pThis->LastTarget;
		pThis->QueueMission(Mission::Attack, false);
	}
	else
	{
		pThis->QueueMission(Mission::Guard, false);
	}

	return 0x449AE8;
}

DEFINE_HOOK(0x4401BB, Factory_AI_PickWithFreeDocks, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	if (Phobos::Config::AllowParallelAIQueues)
		return 0;

	if (!pBuilding)
		return 0;

	HouseClass* pOwner = pBuilding->Owner;

	if (!pOwner)
		return 0;

	if (pOwner->Type->MultiplayPassive
		|| pOwner->IsPlayer()
		|| pOwner->IsNeutral())
		return 0;

	if (pBuilding->Type->Factory == AbstractType::AircraftType
		&& Phobos::Config::ForbidParallelAIQueues_Aircraft)
	{
		if (pBuilding->Factory
			&& !BuildingExt::HasFreeDocks(pBuilding))
		{
			auto BuildingExt = BuildingExt::ExtMap.Find(pBuilding);
			if (!BuildingExt)
				return 0;

			BuildingExt::UpdatePrimaryFactoryAI(pBuilding);
		}
	}

	return 0;
}

DEFINE_HOOK(0x44D455, BuildingClass_Mission_Missile_EMPPulseBulletWeapon, 0x8)
{
	GET(WeaponTypeClass*, pWeapon, EBP);
	GET_STACK(BulletClass*, pBullet, STACK_OFFS(0xF0, 0xA4));

	pBullet->SetWeaponType(pWeapon);

	return 0;
}

DEFINE_HOOK(0x44224F, BuildingClass_ReceiveDamage_DamageSelf, 0x5)
{
	enum { SkipCheck = 0x442268 };

	REF_STACK(args_ReceiveDamage const, receiveDamageArgs, STACK_OFFS(0x9C, -0x4));

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(receiveDamageArgs.WH))
	{
		if (pWHExt->AllowDamageOnSelf)
			return SkipCheck;
	}

	return 0;
}

DEFINE_HOOK(0x4502F4, BuildingClass_Update_Factory, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);
	HouseClass* pOwner = pBuilding->Owner;

	if (pOwner->Production && Phobos::Config::AllowParallelAIQueues)
	{
		HouseExt::ExtData* pData = HouseExt::ExtMap.Find(pOwner);
		BuildingClass** currFactory = nullptr;
		switch (pBuilding->Type->Factory)
		{
		case AbstractType::BuildingType:
			currFactory = &pData->Factory_BuildingType;
			break;
		case AbstractType::UnitType:
			if (!pBuilding->Type->Naval)
				currFactory = &pData->Factory_VehicleType;
			else
				currFactory = &pData->Factory_NavyType;
			break;
		case AbstractType::InfantryType:
			currFactory = &pData->Factory_InfantryType;
			break;
		case AbstractType::AircraftType:
			currFactory = &pData->Factory_AircraftType;
			break;
		}

		if (!currFactory)
		{
			Game::RaiseError(E_POINTER);
		}
		else if (!*currFactory)
		{
			*currFactory = pBuilding;
			return 0;
		}
		else if (*currFactory != pBuilding)
		{
			enum { Skip = 0x4503CA };
			if (pBuilding->Type->Factory == AbstractType::BuildingType
				&& Phobos::Config::ForbidParallelAIQueues_Building)
			{
				return Skip;
			}
			else if (pBuilding->Type->Factory == AbstractType::UnitType
				&& Phobos::Config::ForbidParallelAIQueues_Vehicle
				&& !pBuilding->Type->Naval)
			{
				return Skip;
			}
			else if (pBuilding->Type->Factory == AbstractType::UnitType
				&& Phobos::Config::ForbidParallelAIQueues_Navy
				&& pBuilding->Type->Naval)
			{
				return Skip;
			}
			else if (pBuilding->Type->Factory == AbstractType::InfantryType
				&& Phobos::Config::ForbidParallelAIQueues_Infantry)
			{
				return Skip;
			}
			else if (pBuilding->Type->Factory == AbstractType::AircraftType
				&& Phobos::Config::ForbidParallelAIQueues_Aircraft)
			{
				return Skip;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x4CA07A, FactoryClass_AbandonProduction, 0x8)
{
	GET(FactoryClass*, pFactory, ESI);

	if (!Phobos::Config::AllowParallelAIQueues)
		return 0;

	HouseClass* pOwner = pFactory->Owner;
	HouseExt::ExtData* pData = HouseExt::ExtMap.Find(pOwner);
	TechnoClass* pTechno = pFactory->Object;

	switch (pTechno->WhatAmI())
	{
	case AbstractType::Building:
		if (Phobos::Config::ForbidParallelAIQueues_Building)
			pData->Factory_BuildingType = nullptr;
		break;
	case AbstractType::Unit:
		if (!pTechno->GetTechnoType()->Naval)
		{
			if (Phobos::Config::ForbidParallelAIQueues_Vehicle)
				pData->Factory_VehicleType = nullptr;
		}
		else
		{
			if (Phobos::Config::ForbidParallelAIQueues_Navy)
				pData->Factory_NavyType = nullptr;
		}
		break;
	case AbstractType::Infantry:
		if (Phobos::Config::ForbidParallelAIQueues_Infantry)
			pData->Factory_InfantryType = nullptr;
		break;
	case AbstractType::Aircraft:
		if (Phobos::Config::ForbidParallelAIQueues_Aircraft)
			pData->Factory_AircraftType = nullptr;
		break;
	}

	return 0;
}

DEFINE_HOOK(0x444119, BuildingClass_KickOutUnit_UnitType, 0x6)
{
	GET(UnitClass*, pUnit, EDI);
	GET(BuildingClass*, pFactory, ESI);

	if (!Phobos::Config::AllowParallelAIQueues)
		return 0;

	HouseExt::ExtData* pData = HouseExt::ExtMap.Find(pFactory->Owner);

	if (!pUnit->Type->Naval)
	{
		if (Phobos::Config::ForbidParallelAIQueues_Vehicle)
			pData->Factory_VehicleType = nullptr;
	}
	else
	{
		if (Phobos::Config::ForbidParallelAIQueues_Navy)
			pData->Factory_NavyType = nullptr;
	}

	return 0;
}

DEFINE_HOOK(0x444131, BuildingClass_KickOutUnit_InfantryType, 0x6)
{
	GET(HouseClass*, pHouse, EAX);

	if (Phobos::Config::AllowParallelAIQueues || Phobos::Config::ForbidParallelAIQueues_Infantry)
		return 0;

	HouseExt::ExtMap.Find(pHouse)->Factory_InfantryType = nullptr;
	return 0;
}

DEFINE_HOOK(0x44531F, BuildingClass_KickOutUnit_BuildingType, 0xA)
{
	GET(HouseClass*, pHouse, EAX);

	if (Phobos::Config::AllowParallelAIQueues || Phobos::Config::ForbidParallelAIQueues_Building)
		return 0;

	HouseExt::ExtMap.Find(pHouse)->Factory_BuildingType = nullptr;
	return 0;
}

DEFINE_HOOK(0x443CCA, BuildingClass_KickOutUnit_AircraftType, 0xA)
{
	GET(HouseClass*, pHouse, EDX);

	if (Phobos::Config::AllowParallelAIQueues || Phobos::Config::ForbidParallelAIQueues_Aircraft)
		return 0;

	HouseExt::ExtMap.Find(pHouse)->Factory_AircraftType = nullptr;
	return 0;
}
