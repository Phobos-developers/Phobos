#include <Helpers/Macro.h>
#include <InfantryClass.h>
#include <HouseClass.h>
#include <InputManagerClass.h>
#include <Utilities/Macro.h>

DEFINE_JUMP(LJMP,0x51B2CB, 0x51B2CF)

DEFINE_HOOK(0x51B2BD, InfantryClass_UpdateTarget_InfiltrateSkip, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0xC, 0x4));

	return (!pTarget || pThis->Owner->IsControlledByHuman()) ? 0x51B33F : 0;
}

DEFINE_HOOK(0x51E4FB, InfantryClass_MouseOverObject_EnigneerEnterBuilding, 0x6)
{
	GET(InfantryClass*, pThis, EDI);
	GET(BuildingClass*, pBuilding, ESI);
	GET(BuildingTypeClass*, pBuildingType, EAX);
	enum { Skip = 0x51E668, Continue = 0x51E501 };

	if (InputManagerClass::Instance->IsForceFireKeyPressed())
		return Skip;

	bool BridgeRepairHut = pBuildingType->BridgeRepairHut;

	if (!BridgeRepairHut && pThis->Owner->IsAlliedWith(pBuilding->Owner))
	{
		if (InputManagerClass::Instance->IsForceMoveKeyPressed()
			|| pBuilding->Health >= pBuildingType->Strength)
		{
			return Skip;
		}
	}

	R->CL(BridgeRepairHut);
	return Continue;
}

DEFINE_HOOK(0x51EE6B, InfantryClass_MouseOverObject_AgentOrEngineerForceAttack, 0x6)
{
	return InputManagerClass::Instance->IsForceFireKeyPressed() ? 0x51F05E : 0;
}
