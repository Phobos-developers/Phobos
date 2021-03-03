#include "Body.h"

#include <AnimClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>
#include <BuildingClass.h>
#include "../Phobos.h"

void TechnoTypeExt::TransferMindControl(TechnoClass* From, TechnoClass* To) {
	if (auto Controller = From->MindControlledBy) {
		if (auto Manager = Controller->CaptureManager) { // shouldn't be necessary, but WW uses it...
			bool FoundNode(false);
			for (int i = 0; i < Manager->ControlNodes.Count; ++i) {
				auto Node = Manager->ControlNodes[i];
				if (Node->Unit == From) {
					Node->Unit = To;
					To->Owner = From->GetOwningHouse();
					//To->SetOwningHouse(From->GetOwningHouse(), 0);
					To->MindControlledBy = Controller;
					From->MindControlledBy = NULL;
					FoundNode = true;
					break;
				}
			}
			if (!FoundNode) { // say the link was broken beforehand, by inconsistently ordered code...
				Manager->CaptureUnit(To);
			}
		}
	}
	else if (auto MCHouse = From->MindControlledByHouse) {
		To->MindControlledByHouse = MCHouse;
		From->MindControlledByHouse = NULL;
	}
	if (auto Anim = From->MindControlRingAnim) {
		auto ToAnim = &To->MindControlRingAnim;
		if (*ToAnim) {
			(*ToAnim)->TimeToDie = 1;
		}
		*ToAnim = Anim;
		Anim->SetOwnerObject(To);
	}
}

DEFINE_HOOK(739956, UnitClass_Deploy_TransferMindControl, 6)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	TechnoTypeExt::TransferMindControl(pUnit, pStructure);

	return 0;
}

DEFINE_HOOK(44A03C, BuildingClass_Mi_Selling_TransferMindControl, 6)
{
	GET(BuildingClass*, pStructure, EBP);
	GET(UnitClass*, pUnit, EBX);

	TechnoTypeExt::TransferMindControl(pStructure, pUnit);

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

