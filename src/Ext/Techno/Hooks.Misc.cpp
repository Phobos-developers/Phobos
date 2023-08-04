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

DEFINE_HOOK(0x730D1F, DeployCommandClass_Execute_VoiceDeploy, 0x5)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(int, unitsToDeploy, STACK_OFFSET(0x18, -0x4));
	REF_STACK(char, endDeploy, STACK_OFFSET(0x18, -0x5));

	if ((pThis->WhatAmI() == AbstractType::Infantry) && unitsToDeploy == 1)
	{
		auto pInfantry = abstract_cast<InfantryClass*>(pThis);
		if(pInfantry->GetTechnoType()->VoiceDeploy && pInfantry->IsOwnedByCurrentPlayer && pInfantry->CanDeployNow())
		{
			pInfantry->QueueVoice(pInfantry->GetTechnoType()->VoiceDeploy);
		}
	}
	else if ((pThis->WhatAmI() == AbstractType::Unit) && unitsToDeploy == 1)
	{
		auto pUnit = abstract_cast<UnitClass*>(pThis);
		if(pUnit->GetTechnoType()->VoiceDeploy && pUnit->IsOwnedByCurrentPlayer && (pUnit->TryToDeploy() || pUnit->Type->IsSimpleDeployer))
		{
			pUnit->QueueVoice(pUnit->GetTechnoType()->VoiceDeploy);
		}
	}
	endDeploy = 1;
	return 0x730D24;
}
