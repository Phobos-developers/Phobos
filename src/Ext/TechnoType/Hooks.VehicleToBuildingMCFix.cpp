#include "Body.h"

#include <AnimClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include "../Phobos.h"
#include "../../Misc/CaptureManager.h"

namespace MindControlFixTemp
{
	bool isMindControlBeingTransferred = false;
}

void TechnoTypeExt::TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo)
{
	if (auto Controller = pTechnoFrom->MindControlledBy)
	{
		if (auto Manager = Controller->CaptureManager)
		{
			MindControlFixTemp::isMindControlBeingTransferred = true;

			CaptureManager::FreeUnit(Manager, pTechnoFrom, true);
			CaptureManager::CaptureUnit(Manager, pTechnoTo, true);

			if (pTechnoTo->WhatAmI() == AbstractType::Building)
			{
				pTechnoTo->QueueMission(Mission::Construction, 0);
				pTechnoTo->Mission_Construction();
			}

			MindControlFixTemp::isMindControlBeingTransferred = false;
		}
	}
	else if (auto MCHouse = pTechnoFrom->MindControlledByHouse)
	{
		pTechnoTo->MindControlledByHouse = MCHouse;
		pTechnoFrom->MindControlledByHouse = NULL;
	}

	if (auto Anim = pTechnoFrom->MindControlRingAnim)
	{
		auto ToAnim = &pTechnoTo->MindControlRingAnim;
		
		if (*ToAnim)
			(*ToAnim)->TimeToDie = 1;

		*ToAnim = Anim;
		Anim->SetOwnerObject(pTechnoTo);
	}
}

DEFINE_HOOK(739956, UnitClass_Deploy_TransferMindControl, 6)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	TechnoTypeExt::TransferMindControlOnDeploy(pUnit, pStructure);

	return 0;
}

DEFINE_HOOK(44A03C, BuildingClass_Mi_Selling_TransferMindControl, 6)
{
	GET(BuildingClass*, pStructure, EBP);
	GET(UnitClass*, pUnit, EBX);

	TechnoTypeExt::TransferMindControlOnDeploy(pStructure, pUnit);

	pUnit->QueueMission(Mission::Hunt, true);

	return 0;
}

DEFINE_HOOK(449E2E, BuildingClass_Mi_Selling_CreateUnit, 6)
{
	GET(BuildingClass*, pStructure, EBP);
	R->ECX<HouseClass*>(pStructure->GetOriginalOwner());

	return 0x449E34;
}

DEFINE_HOOK(7396AD, UnitClass_Deploy_CreateBuilding, 6)
{
	GET(UnitClass*, pUnit, EBP);
	R->EDX<HouseClass*>(pUnit->GetOriginalOwner());

	return 0x7396B3;
}

DEFINE_HOOK(448460, BuildingClass_Captured_MuteSound, 6)
{
	return MindControlFixTemp::isMindControlBeingTransferred ?
		0x44848F : 0;
}
