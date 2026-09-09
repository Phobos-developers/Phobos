#include "Body.h"

#include <Ext/Foot/Body.h>
#include <Ext/House/Body.h>
#include <Ext/TechnoType/Body.h>
#include <AircraftClass.h>
#include <AnimClass.h>
#include <CellClass.h>
#include <FlyLocomotionClass.h>
#include <JumpjetLocomotionClass.h>
#include <Unsorted.h>
#include <Utilities/Debug.h>

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
	auto const pTypeExt = TechnoExt::Fetch(pTechno)->TypeExtData;
	auto const pType = pTypeExt->OwnerObject();
	const bool forbid = pTypeExt->ForbidParallelAIQueues;

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

bool BuildingExt::TrySpawnFlyingProduction(BuildingClass* pFactory, TechnoClass* pProduction)
{
	if (!pFactory || !pProduction)
		return false;

	if (pFactory->GetCurrentMission() == Mission::Construction)
		return false;

	auto const pType = pProduction->GetTechnoType();
	if (!pType)
		return false;

	auto const pTypeExt = TechnoTypeExt::Fetch(pType);
	if (!pTypeExt || !pTypeExt->FlyingProduction)
		return false;

	BuildingClass* pSpawnBuilding = nullptr;

	if (!pTypeExt->FlyingProduction_SpawnAt.empty())
	{
		for (auto const pTargetType : pTypeExt->FlyingProduction_SpawnAt)
		{
			if (!pTargetType)
				continue;

			if (pFactory->Type == pTargetType)
			{
				pSpawnBuilding = pFactory;
				break;
			}

			BuildingClass* pPrimaryCandidate = nullptr;
			BuildingClass* pClosestCandidate = nullptr;
			int minDistance = (std::numeric_limits<int>::max)();

			for (auto const pBld : pFactory->Owner->Buildings)
			{
				if (!pBld || !pBld->IsAlive || pBld->InLimbo || pBld->Health <= 0 || pBld->GetCurrentMission() == Mission::Selling)
					continue;

				if (pBld->Type == pTargetType)
				{
					if (pBld->IsPrimaryFactory && !pPrimaryCandidate)
						pPrimaryCandidate = pBld;

					const int dist = pFactory->DistanceFrom(pBld);
					if (dist < minDistance)
					{
						minDistance = dist;
						pClosestCandidate = pBld;
					}
				}
			}

			if (auto const pCandidate = pPrimaryCandidate ? pPrimaryCandidate : pClosestCandidate)
			{
				pSpawnBuilding = pCandidate;
				break;
			}
		}
	}

	if (!pSpawnBuilding)
		pSpawnBuilding = pFactory;

	auto const pSpawnBldTypeExt = BuildingTypeExt::Fetch(pSpawnBuilding->Type);

	int height = pTypeExt->FlyingProduction_SpawnHeight.Get();
	if (pSpawnBldTypeExt && pSpawnBldTypeExt->FlyingProduction_SpawnHeight.isset())
		height = pSpawnBldTypeExt->FlyingProduction_SpawnHeight.Get();

	if (height <= 0)
		return false;

	CoordStruct spawnCoords = pSpawnBuilding->GetCoords();
	if (pSpawnBldTypeExt && pSpawnBldTypeExt->FlyingProduction_SpawnOffset.isset())
	{
		const auto& offset = pSpawnBldTypeExt->FlyingProduction_SpawnOffset.Get();
		spawnCoords.X += offset.X;
		spawnCoords.Y += offset.Y;
	}
	spawnCoords.Z += height;

	DirStruct facing = pSpawnBuilding->PrimaryFacing.Current();
	if (pSpawnBldTypeExt && pSpawnBldTypeExt->FlyingProduction_SpawnFacing.isset())
		facing = DirStruct(pSpawnBldTypeExt->FlyingProduction_SpawnFacing.Get());

	pProduction->SetOwningHouse(pFactory->Owner, true);

	bool unlimboSuccess = false;
	{
		struct ScenarioInitGuard
		{
			ScenarioInitGuard() { ++Unsorted::ScenarioInit; }
			~ScenarioInitGuard() { --Unsorted::ScenarioInit; }
		} guard;

		unlimboSuccess = pProduction->Unlimbo(spawnCoords, facing.GetDir());
	}

	if (!unlimboSuccess)
		return false;

	pProduction->SetLocation(spawnCoords);
	pProduction->Location = spawnCoords;
	pProduction->InAir = true;
	pProduction->IsALoaner = false;
	pProduction->UnmarkAllOccupationBits(spawnCoords);
	pProduction->PrimaryFacing.SetCurrent(facing);
	pProduction->PrimaryFacing.SetDesired(facing);

	if (auto const pFoot = abstract_cast<FootClass*>(pProduction))
	{
		if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pFoot->Locomotor))
		{
			pJJLoco->CurrentHeight = height;
			pJJLoco->LocomotionFacing.SetCurrent(facing);
			pJJLoco->LocomotionFacing.SetDesired(facing);
			pFoot->Jumpjet_OccupyCell(CellClass::Coord2Cell(spawnCoords));

			if (height < pJJLoco->Height)
			{
				pJJLoco->State = JumpjetLocomotionClass::State::Ascending;
				FootExt::Fetch(pFoot)->JumpjetStraightAscend = true;
			}
			else
			{
				pJJLoco->State = JumpjetLocomotionClass::State::Hovering;
			}
		}
		else if (auto const pFlyLoco = locomotion_cast<FlyLocomotionClass*>(pFoot->Locomotor))
		{
			const auto pFootType = pFoot->GetTechnoType();
			const int cruiseFlightLevel = pFootType->GetFlightLevel();
			const int targetFlightLevel = cruiseFlightLevel > 0 ? cruiseFlightLevel : (RulesClass::Instance ? RulesClass::Instance->FlightLevel : height);
			pFlyLoco->FlightLevel = targetFlightLevel;
			pFlyLoco->CurrentSpeed = 0.0;
			pFlyLoco->TargetSpeed = 0.0;
			pFlyLoco->IsTakingOff = height < targetFlightLevel ? 1 : 0;
			pFlyLoco->IsLanding = false;
		}
	}

	if (auto const pAircraft = abstract_cast<AircraftClass*>(pProduction))
		pAircraft->SetHeight(height);

	if (auto const pAnimType = pTypeExt->FlyingProduction_SpawnAnim.Get())
	{
		if (auto const pAnim = GameCreate<AnimClass>(pAnimType, spawnCoords))
		{
			if (pTypeExt->FlyingProduction_SpawnAnim_AttachedToObject.Get())
				pAnim->SetOwnerObject(pProduction);
		}
	}

	AbstractClass* pRallyTarget = nullptr;
	if (pTypeExt->FlyingProduction_RallyPointFromSpawnBuilding.Get() && pSpawnBuilding->ArchiveTarget)
		pRallyTarget = pSpawnBuilding->ArchiveTarget;
	else
		pRallyTarget = pFactory->ArchiveTarget;

	if (pRallyTarget && pRallyTarget != pSpawnBuilding)
	{
		pProduction->SetDestination(pRallyTarget, true);
		pProduction->QueueMission(Mission::Move, true);
	}
	else
	{
		// No rally point set: give an exit nudge away from the spawn building in the facing direction
		// to prevent units from remaining frozen directly over the building or blocking subsequent spawns.
		const auto pBldType = pSpawnBuilding->Type;
		const int nudgeDistance = std::max({ static_cast<int>(pBldType->GetFoundationWidth()), static_cast<int>(pBldType->GetFoundationHeight(true)), 2 }) / 2 + 1;
		auto pDestCell = MapClass::Instance.GetCellAt(spawnCoords);
		const auto facingType = static_cast<FacingType>(facing.GetValue<3>());

		for (int i = 0; i < nudgeDistance; ++i)
		{
			if (auto const pNext = pDestCell->GetNeighbourCell(facingType))
				pDestCell = pNext;
		}

		if (pDestCell)
		{
			pProduction->SetDestination(pDestCell, true);
			pProduction->QueueMission(Mission::Move, true);
		}
		else
		{
			pProduction->Scatter(CoordStruct::Empty, true, false);
			pProduction->QueueMission(Mission::Guard, true);
		}
	}

	if (pTypeExt->FlyingProduction_PlayFactoryAnim.Get())
		pFactory->QueueMission(Mission::Unload, false);

	if (auto const pOwner = pFactory->Owner)
	{
		auto const pHouseExt = HouseExt::Fetch(pOwner);
		auto const abs = pProduction->WhatAmI();

		if (abs == AbstractType::Unit)
		{
			if (auto const pUnit = abstract_cast<UnitClass*>(pProduction))
			{
				if (pUnit->Type->Naval)
				{
					if (pHouseExt)
						pHouseExt->ProducingNavalUnitTypeIndex = -1;
				}
				else
				{
					pOwner->ProducingUnitTypeIndex = -1;
				}
			}
		}
		else if (abs == AbstractType::Aircraft)
		{
			pOwner->ProducingAircraftTypeIndex = -1;
		}
		else if (abs == AbstractType::Infantry)
		{
			pOwner->ProducingInfantryTypeIndex = -1;
		}
		else if (abs == AbstractType::Building)
		{
			pOwner->ProducingBuildingTypeIndex = -1;
		}

		if (pHouseExt)
		{
			if (pHouseExt->Factory_VehicleType == pFactory)
				pHouseExt->Factory_VehicleType = nullptr;
			if (pHouseExt->Factory_NavyType == pFactory)
				pHouseExt->Factory_NavyType = nullptr;
			if (pHouseExt->Factory_InfantryType == pFactory)
				pHouseExt->Factory_InfantryType = nullptr;
			if (pHouseExt->Factory_AircraftType == pFactory)
				pHouseExt->Factory_AircraftType = nullptr;
			if (pHouseExt->Factory_BuildingType == pFactory)
				pHouseExt->Factory_BuildingType = nullptr;
		}

		if (!pFactory->Type->Cloning)
		{
			auto info = std::make_pair(pType, pOwner);
			for (const auto pVat : pOwner->CloningVats)
				BuildingExt::KickOutClone(info, 0, pVat);
		}
	}

	return true;
}

DEFINE_HOOK(0x443C60, BuildingClass_KickOutUnit_FlyingProduction, 0x6)
{
	enum { DoneSucceeded = 0x4456A2 };

	GET(BuildingClass*, pFactory, ECX);
	GET_STACK(TechnoClass*, pProduction, 0x4);

	if (BuildingExt::TrySpawnFlyingProduction(pFactory, pProduction))
	{
		R->EAX(static_cast<int>(KickOutResult::Succeeded));
		return DoneSucceeded;
	}

	return 0;
}

DEFINE_HOOK(0x455DA0, BuildingClass_IsUnitFactory_FlyingProduction, 0x6)
{
	enum { ReturnTrue = 0x455DCD };

	GET(BuildingClass*, pThis, ECX);

	if (auto const pTypeExt = BuildingTypeExt::Fetch(pThis->Type))
	{
		if (pTypeExt->FlyingProduction_RallyPoint.Get())
			return ReturnTrue;
	}

	return 0;
}



