#include "Body.h"

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

			if (CaptureManager::CaptureUnit(Manager, pTechnoTo, false))
			{
				if (auto pBld = abstract_cast<BuildingClass*>(pTechnoTo))
				{
					pBld->BeginMode(BStateType::Construction);
					pBld->QueueMission(Mission::Construction, false);
				}
			}
			else
			{
				int nSound = pTechnoTo->GetTechnoType()->MindClearedSound;
				if (nSound == -1)
					nSound = RulesClass::Instance->MindClearedSound;
				if (nSound != -1)
					VocClass::PlayIndexAtPos(nSound, pTechnoTo->Location);
			}

			MindControlFixTemp::isMindControlBeingTransferred = false;
		}
	}
	else if (auto MCHouse = pTechnoFrom->MindControlledByHouse)
	{
		pTechnoTo->MindControlledByHouse = MCHouse;
		pTechnoFrom->MindControlledByHouse = nullptr;
	}

	if (auto fromAnim = pTechnoFrom->MindControlRingAnim)
	{
		auto& toAnim = pTechnoTo->MindControlRingAnim;

		if (toAnim)
			toAnim->TimeToDie = 1;

		toAnim = fromAnim;
		fromAnim->SetOwnerObject(pTechnoTo);
	}
}

DEFINE_HOOK(0x739956, UnitClass_Deploy_Transfer, 0x6)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	TechnoExt::TransferMindControlOnDeploy(pUnit, pStructure);
	ShieldClass::SyncShieldToAnother(pUnit, pStructure);
	TechnoExt::SyncIronCurtainStatus(pUnit, pStructure);

	return 0;
}

DEFINE_HOOK(0x44A03C, BuildingClass_Mi_Selling_Transfer, 0x6)
{
	GET(BuildingClass*, pStructure, EBP);
	GET(UnitClass*, pUnit, EBX);

	TechnoExt::TransferMindControlOnDeploy(pStructure, pUnit);
	ShieldClass::SyncShieldToAnother(pStructure, pUnit);
	TechnoExt::SyncIronCurtainStatus(pStructure, pUnit);

	pUnit->QueueMission(Mission::Hunt, true);
	//Why?
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
