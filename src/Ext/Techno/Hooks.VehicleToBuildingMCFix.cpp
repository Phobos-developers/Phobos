#include "Body.h"

#include <Phobos.h>
#include <JumpjetLocomotionClass.h>
#include <HouseClass.h>
#include <AnimClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <Misc/CaptureManager.h>

namespace MindControlFixTemp
{
	bool isMindControlBeingTransferred = false;
}

void TechnoExt::TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo)
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

DEFINE_HOOK(0x739956, UnitClass_Deploy_TransferMindControl, 0x6)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	TechnoExt::TransferMindControlOnDeploy(pUnit, pStructure);

	return 0;
}

DEFINE_HOOK(0x44A03C, BuildingClass_Mi_Selling_TransferMindControl, 0x6)
{
	GET(BuildingClass*, pStructure, EBP);
	GET(UnitClass*, pUnit, EBX);

	TechnoExt::TransferMindControlOnDeploy(pStructure, pUnit);

	pUnit->QueueMission(Mission::Hunt, true);

	return 0;
}

DEFINE_HOOK(0x449E2E, BuildingClass_Mi_Selling_CreateUnit, 0x6)
{
	GET(BuildingClass*, pStructure, EBP);
	R->ECX<HouseClass*>(pStructure->GetOriginalOwner());

	return 0x449E34;
}

DEFINE_HOOK(0x7396AD, UnitClass_Deploy_CreateBuilding, 0x6)
{
	GET(UnitClass*, pUnit, EBP);
	R->EDX<HouseClass*>(pUnit->GetOriginalOwner());

	return 0x7396B3;
}

DEFINE_HOOK(0x448460, BuildingClass_Captured_MuteSound, 0x6)
{
	return MindControlFixTemp::isMindControlBeingTransferred ?
		0x44848F : 0;
}

DEFINE_HOOK(0x54AEC0, JumpjetLocomotion_Process_TurnToTarget, 0x8)
{
	GET_STACK(ILocomotion*, iLoco, 0x4);
	const auto pLoco = static_cast<JumpjetLocomotionClass*>(iLoco);
	const auto pThis = pLoco->Owner;
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt && pTypeExt->JumpjetTurnToTarget.Get(RulesExt::Global()->JumpjetTurnToTarget) &&
		pThis->WhatAmI() == AbstractType::Unit && pThis->IsInAir() && !pType->TurretSpins && pLoco)
	{
		if (const auto pTarget = pThis->Target)
		{
			const CoordStruct source = pThis->Location;
			const CoordStruct target = pTarget->GetCoords();
			const DirStruct tgtDir = DirStruct(Math::arctanfoo(source.Y - target.Y, target.X - source.X));

			if (pThis->GetRealFacing().value32() != tgtDir.value32())
				pLoco->LocomotionFacing.turn(tgtDir);
		}
	}

	return 0;
}
