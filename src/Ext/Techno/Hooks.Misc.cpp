#include "Body.h"

#include <SpawnManagerClass.h>

DEFINE_HOOK(0x6B0B9C, SlaveManagerClass_Killed_DecideOwner, 0x6)
{
	enum { KillTheSlave = 0x6B0BDF, ChangeSlaveOwner = 0x6B0BB4 };

	GET(InfantryClass*, pSlave, ESI);

	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pSlave->Type))
	{
		switch (pTypeExt->Slaved_OwnerWhenMasterKilled.Get())
		{
		case SlaveChangeOwnerType::Suicide:
			return KillTheSlave;

		case SlaveChangeOwnerType::Master:
			R->EAX(pSlave->Owner);
			return ChangeSlaveOwner;

		case SlaveChangeOwnerType::Neutral:
			if (auto pNeutral = HouseClass::FindNeutral())
			{
				R->EAX(pNeutral);
				return ChangeSlaveOwner;
			}

		default: // SlaveChangeOwnerType::Killer
			return 0x0;
		}
	}

	return 0x0;
}

// Fix slaves cannot always suicide due to armor multiplier or something
DEFINE_PATCH(0x6B0BF7,
	0x6A, 0x01  // push 1       // ignoreDefense=false->true
);

DEFINE_HOOK(0x6B7265, SpawnManagerClass_AI_UpdateTimer, 0x6)
{
	GET(SpawnManagerClass* const, pThis, ESI);

	if (pThis->Owner && pThis->Status == SpawnManagerStatus::Launching && pThis->CountDockedSpawns() != 0)
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType()))
		{
			if (pTypeExt->Spawner_DelayFrames.isset())
				R->EAX(std::min(pTypeExt->Spawner_DelayFrames.Get(), 10));
		}
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6B73BE, SpawnManagerClass_AI_SpawnTimer, 0x6)
DEFINE_HOOK(0x6B73AD, SpawnManagerClass_AI_SpawnTimer, 0x5)
{
	GET(SpawnManagerClass* const, pThis, ESI);

	if (pThis->Owner)
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType()))
		{
			if (pTypeExt->Spawner_DelayFrames.isset())
				R->ECX(pTypeExt->Spawner_DelayFrames.Get());
		}
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6B769F, SpawnManagerClass_AI_InitDestination, 0x7)
DEFINE_HOOK(0x6B7600, SpawnManagerClass_AI_InitDestination, 0x6)
{
	enum { SkipGameCode1 = 0x6B760E, SkipGameCode2 = 0x6B76DE };

	GET(SpawnManagerClass* const, pThis, ESI);
	GET(AircraftClass* const, pSpawnee, EDI);

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());

	if (pTypeExt->Spawner_AttackImmediately)
	{
		pSpawnee->SetTarget(pThis->Target);
		pSpawnee->QueueMission(Mission::Attack, true);
		pSpawnee->IsReturningFromAttackRun = false;
	}
	else
	{
		auto const mapCoords = pThis->Owner->GetMapCoords();
		auto const pCell = MapClass::Instance->GetCellAt(mapCoords);
		pSpawnee->SetDestination(pCell->GetNeighbourCell(FacingType::North), true);
		pSpawnee->QueueMission(Mission::Move, false);
	}

	return R->Origin() == 0x6B7600 ? SkipGameCode1 : SkipGameCode2;
}

DEFINE_HOOK(0x6B77B4, SpawnManagerClass_Update_RecycleSpawned, 0x7)
{
	//enum { RecycleIsOk = 0x6B77FF, RecycleIsNotOk = 0x6B7838 };

	GET(SpawnManagerClass* const, pThis, ESI);
	GET(AircraftClass* const, pSpawned, EDI);
	GET(CellStruct* const, pSpawnerMapCrd, EBP);

	if (!pThis)
	{
		return 0;
	}
	auto pSpawner = pThis->Owner;
	if (!pSpawner || !pSpawned)
	{
		return 0;
	}
	auto const pSpawnerType = pSpawner->GetTechnoType();
	if (!pSpawnerType)
	{
		return 0;
	}
	auto const pSpawnerExt = TechnoTypeExt::ExtMap.Find(pSpawnerType);
	if (!pSpawnerExt)
	{
		return 0;
	}
	auto SpawnerCrd = pSpawner->GetCoords();
	auto SpawnedCrd = pSpawned->GetCoords();
	auto RecycleCrd = SpawnerCrd;
	auto DeltaCrd = SpawnedCrd - RecycleCrd;
	bool bShouldRecycleSpawned = false;
	int RecycleRange = pSpawnerExt->Spawner_RecycleRange;
	if (RecycleRange == -1)
	{
		if (DeltaCrd.X <= 182 && DeltaCrd.Y <= 182 && DeltaCrd.Z < 20)
		{
			bShouldRecycleSpawned = true;
		}
	}
	else
	{
		if (Math::sqrt(DeltaCrd.X * DeltaCrd.X + DeltaCrd.Y * DeltaCrd.Y + DeltaCrd.Z * DeltaCrd.Z) <= RecycleRange)
		{
			bShouldRecycleSpawned = true;
		}
	}
	if (!bShouldRecycleSpawned)
	{
		return 0;
	}
	else
	{
		if (pSpawnerExt->Spawner_RecycleAnim)
		{
			GameCreate<AnimClass>(pSpawnerExt->Spawner_RecycleAnim, SpawnedCrd);
		}
		pSpawned->SetLocation(SpawnerCrd);
		R->EAX(pSpawnerMapCrd);
		return 0;
	}
}
