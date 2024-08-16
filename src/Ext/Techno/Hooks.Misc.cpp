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

DEFINE_HOOK(0x41847E, AircraftClass_MissionAttack_ScatterCell1, 0x6)
{
	enum { SkipScatter = 0x4184C2, Scatter = 0 };
	return false ? Scatter : SkipScatter;
}

DEFINE_HOOK(0x4186DD, AircraftClass_MissionAttack_ScatterCell2, 0x6)
{
	enum { SkipScatter = 0x418720, Scatter = 0 };
	return false ? Scatter : SkipScatter;
}

DEFINE_HOOK(0x41882C, AircraftClass_MissionAttack_ScatterCell3, 0x6)
{
	enum { SkipScatter = 0x418870, Scatter = 0 };
	return false ? Scatter : SkipScatter;
}

DEFINE_HOOK(0x41893B, AircraftClass_MissionAttack_ScatterCell4, 0x6)
{
	enum { SkipScatter = 0x41897F, Scatter = 0 };
	return false ? Scatter : SkipScatter;
}

DEFINE_HOOK(0x418A4A, AircraftClass_MissionAttack_ScatterCell5, 0x6)
{
	enum { SkipScatter = 0x418A8E, Scatter = 0 };
	return false ? Scatter : SkipScatter;
}

DEFINE_HOOK(0x418B46, AircraftClass_MissionAttack_ScatterCell6, 0x6)
{
	enum { SkipScatter = 0x418B8A, Scatter = 0 };
	return false ? Scatter : SkipScatter;
}

// 航味麻酱: These are WW's bullshit checks.
// 
//if (  bHasAElite
//   || ignoreDestination
//   || RulesClass::Instance->PlayerScatter
//   || pTechnoToScatter && (FootClass::HasAbility(pTechnoToScatter, Ability::Scatter)
//   || pTechnoToScatter->Owner->IQLevel2 >= RulesClass::Instance->Scatter) )

// delete the first one 'bHasAElite'
DEFINE_HOOK(0x481778, CellClass_ScatterContent_Fix1, 0x6)
{
	R->AL(false);
	return 0;
}

// delete the second one 'ignoreDestination'
DEFINE_HOOK(0x481780, CellClass_ScatterContent_Fix2, 0x6)
{
	R->AL(false);
	return 0;
}

// fix the third one 'RulesClass::Instance->PlayerScatter'
DEFINE_HOOK(0x481788, CellClass_ScatterContent_Fix3, 0x5)
{
	enum { ret = 0x481793 };
	GET(ObjectClass*, pObject, ESI);

	auto pTechno = abstract_cast<TechnoClass*>(pObject);

	if (RulesClass::Instance()->PlayerScatter && pTechno && pTechno->Owner->IsHumanPlayer)
		R->CL(true);

	return ret;
}

// 航味麻酱: No idea about why these did not works. Not important though.
// 
//DEFINE_HOOK(0x418484, AircraftClass_MissionAttack_ScatterIgnoreMission1, 0x6)
//DEFINE_HOOK(0x4186E2, AircraftClass_MissionAttack_ScatterIgnoreMission2, 0xA)
//DEFINE_HOOK(0x418832, AircraftClass_MissionAttack_ScatterIgnoreMission3, 0xC)
//DEFINE_HOOK(0x418941, AircraftClass_MissionAttack_ScatterIgnoreMission4, 0x6)
//DEFINE_HOOK(0x418A50, AircraftClass_MissionAttack_ScatterIgnoreMission5, 0x6)
//DEFINE_HOOK(0x0418B4C, AircraftClass_MissionAttack_ScatterIgnoreMission6, 0xA)
//{
//	R->Stack8(false);
//	return 0;
//}
