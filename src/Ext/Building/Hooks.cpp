#include "Body.h"

#include <BulletClass.h>
#include <UnitClass.h>
#include <SuperClass.h>
#include <GameOptionsClass.h>
#include <Ext/Anim/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>
#include <TacticalClass.h>

//After TechnoClass_AI?
DEFINE_HOOK(0x43FE69, BuildingClass_AI, 0xA)
{
	GET(BuildingClass*, pThis, ESI);

	// Do not search this up again in any functions called here because it is costly for performance - Starkku
	auto pExt = BuildingExt::ExtMap.Find(pThis);

	/*
	// Set only if unset or type has changed - Not currently useful as building type does not change.
	auto pType = pThis->Type;

	if (!pExt->TypeExtData || pExt->TypeExtData->OwnerObject() != pType)
		pExt->TypeExtData = BuildingTypeExt::ExtMap.Find(pType);
	*/

	pExt->DisplayIncomeString();
	pExt->ApplyPoweredKillSpawns();

	return 0;
}

DEFINE_HOOK(0x4403D4, BuildingClass_AI_ChronoSparkle, 0x6)
{
	enum { SkipGameCode = 0x44055D };

	GET(BuildingClass*, pThis, ESI);

	if (RulesClass::Instance->ChronoSparkle1)
	{
		auto const displayPositions = RulesExt::Global()->ChronoSparkleBuildingDisplayPositions;
		auto const pType = pThis->Type;
		bool displayOnBuilding = (displayPositions & ChronoSparkleDisplayPosition::Building) != ChronoSparkleDisplayPosition::None;
		bool displayOnSlots = (displayPositions & ChronoSparkleDisplayPosition::OccupantSlots) != ChronoSparkleDisplayPosition::None;
		bool displayOnOccupants = (displayPositions & ChronoSparkleDisplayPosition::Occupants) != ChronoSparkleDisplayPosition::None;
		int occupantCount = displayOnSlots ? pType->MaxNumberOccupants : pThis->GetOccupantCount();
		bool showOccupy = occupantCount && (displayOnOccupants || displayOnSlots);

		if (showOccupy)
		{
			for (int i = 0; i < occupantCount; i++)
			{
				if (!((Unsorted::CurrentFrame + i) % RulesExt::Global()->ChronoSparkleDisplayDelay))
				{
					auto muzzleOffset = pType->MaxNumberOccupants <= 10 ? pType->MuzzleFlash[i] : BuildingTypeExt::ExtMap.Find(pType)->OccupierMuzzleFlashes.at(i);
					auto coords = CoordStruct::Empty;
					auto const renderCoords = pThis->GetRenderCoords();
					auto offset = TacticalClass::Instance->ApplyMatrix_Pixel(muzzleOffset);
					coords.X += offset.X;
					coords.Y += offset.Y;
					coords += renderCoords;

					if (auto const pAnim = GameCreate<AnimClass>(RulesClass::Instance->ChronoSparkle1, coords))
						pAnim->ZAdjust = -200;
				}
			}
		}

		if ((!showOccupy || displayOnBuilding) && !(Unsorted::CurrentFrame % RulesExt::Global()->ChronoSparkleDisplayDelay))
		{
			GameCreate<AnimClass>(RulesClass::Instance->ChronoSparkle1, pThis->GetCenterCoords());
		}

	}

	return SkipGameCode;
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

DEFINE_HOOK(0x4401BB, BuildingClass_AI_PickWithFreeDocks, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	auto pRulesExt = RulesExt::Global();
	HouseClass* pOwner = pBuilding->Owner;
	int index = pOwner->ProducingAircraftTypeIndex;
	auto const pType = index >= 0 ? AircraftTypeClass::Array()->GetItem(index) : nullptr;

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

DEFINE_HOOK(0x44D455, BuildingClass_Mission_Missile_EMPPulseBulletWeapon, 0x8)
{
	GET(WeaponTypeClass*, pWeapon, EBP);
	GET_STACK(BulletClass*, pBullet, STACK_OFFSET(0xF0, -0xA4));

	pBullet->SetWeaponType(pWeapon);

	return 0;
}

DEFINE_HOOK(0x44224F, BuildingClass_ReceiveDamage_DamageSelf, 0x5)
{
	enum { SkipCheck = 0x442268 };

	REF_STACK(args_ReceiveDamage const, receiveDamageArgs, STACK_OFFSET(0x9C, 0x4));

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(receiveDamageArgs.WH))
	{
		if (pWHExt->AllowDamageOnSelf)
			return SkipCheck;
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
				pType = index >= 0 ? BuildingTypeClass::Array()->GetItem(index) : nullptr;
				break;
			case AbstractType::InfantryType:
				if (pRulesExt->ForbidParallelAIQueues_Infantry)
					return Skip;

				index = pOwner->ProducingInfantryTypeIndex;
				pType = index >= 0 ? InfantryTypeClass::Array()->GetItem(index) : nullptr;
				break;
			case AbstractType::AircraftType:
				if (pRulesExt->ForbidParallelAIQueues_Aircraft)
					return Skip;

				index = pOwner->ProducingAircraftTypeIndex;
				pType = index >= 0 ? AircraftTypeClass::Array()->GetItem(index) : nullptr;
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

				pType = index >= 0 ? UnitTypeClass::Array()->GetItem(index) : nullptr;

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
	if(calledby<0x7F0000)
	// Replace the old log with this to figure out where keeps flushing the stream
	Debug::LogGame("(%08x) : %s is abandoning production of %s[%s]\n",
		calledby-5, pFactory->Owner->PlainName, pTechno->GetType()->Name, pTechno->get_ID());

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

// Ares didn't have something like 0x7397E4 in its UnitDelivery code
DEFINE_HOOK(0x44FBBF, CreateBuildingFromINIFile_AfterCTOR_BeforeUnlimbo, 0x8)
{
	GET(BuildingClass* const, pBld, ESI);

	if (auto pExt = BuildingExt::ExtMap.Find(pBld))
		pExt->IsCreatedFromMapFile = true;

	return 0;
}

DEFINE_HOOK(0x440B4F, BuildingClass_Unlimbo_SetShouldRebuild, 0x5)
{
	enum { ContinueCheck = 0x440B58, SkipSetShouldRebuild = 0x440B81 };
	GET(BuildingClass* const, pThis, ESI);

	if (SessionClass::IsCampaign())
	{
		// Preplaced structures are already managed before
		if (BuildingExt::ExtMap.Find(pThis)->IsCreatedFromMapFile)
			return SkipSetShouldRebuild;

		// Per-house dehardcoding: BaseNodes + SW-Delivery
		if (!HouseExt::ExtMap.Find(pThis->Owner)->RepairBaseNodes[GameOptionsClass::Instance->Difficulty])
			return SkipSetShouldRebuild;
	}
	// Vanilla instruction: always repairable in other game modes
	return ContinueCheck;
}

DEFINE_HOOK(0x440EBB, BuildingClass_Unlimbo_NaturalParticleSystem_CampaignSkip, 0x5)
{
	enum { DoNotCreateParticle = 0x440F61 };
	GET(BuildingClass* const, pThis, ESI);
	return BuildingExt::ExtMap.Find(pThis)->IsCreatedFromMapFile ? DoNotCreateParticle : 0;
}

DEFINE_HOOK(0x4519A2, BuildingClass_UpdateAnim_SetParentBuilding, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EBP);

	auto const pAnimExt = AnimExt::ExtMap.Find(pAnim);
	pAnimExt->ParentBuilding = pThis;

	return 0;
}


DEFINE_HOOK(0x43D6E5, BuildingClass_Draw_ZShapePointMove, 0x5)
{
	enum { Apply = 0x43D6EF, Skip = 0x43D712 };

	GET(BuildingClass*, pThis, ESI);
	GET(Mission, mission, EAX);

	if ((mission != Mission::Selling && mission != Mission::Construction) || BuildingTypeExt::ExtMap.Find(pThis->Type)->ZShapePointMove_OnBuildup)
		return Apply;

	return Skip;
}

DEFINE_HOOK(0x4511D6, BuildingClass_AnimationAI_SellBuildup, 0x7)
{
	enum { Skip = 0x4511E6, Continue = 0x4511DF };

	GET(BuildingClass*, pThis, ESI);

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	return pTypeExt->SellBuildupLength == pThis->Animation.Value ? Continue : Skip;
}

#pragma region FactoryPlant

DEFINE_HOOK(0x441501, BuildingClass_Unlimbo_FactoryPlant, 0x6)
{
	enum { Skip = 0x441553 };

	GET(BuildingClass*, pThis, ESI);

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->FactoryPlant_AllowTypes.size() > 0 || pTypeExt->FactoryPlant_DisallowTypes.size() > 0)
	{
		auto const pHouseExt = HouseExt::ExtMap.Find(pThis->Owner);
		pHouseExt->RestrictedFactoryPlants.push_back(pThis);

		return Skip;
	}

	return 0;
}

DEFINE_HOOK(0x448A31, BuildingClass_Captured_FactoryPlant1, 0x6)
{
	enum { Skip = 0x448A78 };

	GET(BuildingClass*, pThis, ESI);

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->FactoryPlant_AllowTypes.size() > 0 || pTypeExt->FactoryPlant_DisallowTypes.size() > 0)
	{
		auto const pHouseExt = HouseExt::ExtMap.Find(pThis->Owner);

		if (!pHouseExt->RestrictedFactoryPlants.empty())
		{
			auto& vec = pHouseExt->RestrictedFactoryPlants;
			vec.erase(std::remove(vec.begin(), vec.end(), pThis), vec.end());
		}

		return Skip;
	}

	return 0;
}

DEFINE_HOOK(0x449149, BuildingClass_Captured_FactoryPlant2, 0x6)
{
	enum { Skip = 0x449197 };

	GET(BuildingClass*, pThis, ESI);
	GET(HouseClass*, pNewOwner, EBP);

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->FactoryPlant_AllowTypes.size() > 0 || pTypeExt->FactoryPlant_DisallowTypes.size() > 0)
	{
		auto const pHouseExt = HouseExt::ExtMap.Find(pNewOwner);
		pHouseExt->RestrictedFactoryPlants.push_back(pThis);

		return Skip;
	}

	return 0;
}

#pragma endregion
