#include "Body.h"

#include <BulletClass.h>
#include <UnitClass.h>
#include <Ext/House/Body.h>

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
		&& !Phobos::Config::ExtendParallelAIQueues[3])
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
			currFactory = pBuilding->Type->Naval
				? &pData->Factory_NavyType
				: &pData->Factory_VehicleType;
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
			if (pBuilding->Type->Factory == AbstractType::InfantryType
				&& !Phobos::Config::ExtendParallelAIQueues[0])
				return 0x4503CA;
			else if (pBuilding->Type->Factory == AbstractType::UnitType
				&& !Phobos::Config::ExtendParallelAIQueues[1]
				&& !pBuilding->Type->Naval)
				return 0x4503CA;
			else if (pBuilding->Type->Factory == AbstractType::UnitType
				&& !Phobos::Config::ExtendParallelAIQueues[2]
				&& pBuilding->Type->Naval)
				return 0x4503CA;
			else if (pBuilding->Type->Factory == AbstractType::AircraftType
				&& !Phobos::Config::ExtendParallelAIQueues[3])
				return 0x4503CA;
			else if (pBuilding->Type->Factory == AbstractType::BuildingType
				&& !Phobos::Config::ExtendParallelAIQueues[4])
				return 0x4503CA;
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