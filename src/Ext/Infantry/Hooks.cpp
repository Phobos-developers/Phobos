#include <Helpers/Macro.h>

#include <InfantryClass.h>
#include <HouseClass.h>
#include <InputManagerClass.h>
#include <WarheadTypeClass.h>

#include <Utilities/Macro.h>

DEFINE_JUMP(LJMP,0x51B2CB, 0x51B2CF)

DEFINE_HOOK(0x51B2BD, InfantryClass_UpdateTarget_IsControlledByHuman, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0xC, 0x4));

	return (!pTarget || pThis->Owner->IsControlledByHuman()) ? 0x51B33F : 0;
}

#pragma region WhatActionObjectFix

namespace WhatActionObjectTemp
{
	bool ignoreForce = false;
	bool Fire = false;
	bool Move = false;
}

DEFINE_HOOK(0x51E462, InfantryClass_WhatAction_ObjectClass_SkipBomb, 0x6)
{
	GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pTarget, ESI);
	GET_STACK(bool, ignoreForce, STACK_OFFSET(0x38, 0x8));
	enum { Skip = 0x51E668, SkipBomb = 0x51E49E, CanBomb = 0x51E48F };

	WhatActionObjectTemp::ignoreForce = ignoreForce;

	const auto pInuptManager = InputManagerClass::Instance;
	WhatActionObjectTemp::Fire = pInuptManager->IsForceFireKeyPressed();
	WhatActionObjectTemp::Move = pInuptManager->IsForceMoveKeyPressed();

	if (!pThis->Type->Engineer)
		return Skip;

	if (pThis->Owner->IsControlledByCurrentPlayer()
		&& pTarget->AttachedBomb && pTarget->BombVisible)
	{
		if (!ignoreForce && WhatActionObjectTemp::Move)
			return SkipBomb;

		int index = pThis->SelectWeapon(pTarget);
		const auto pWeaponType = pThis->GetWeapon(index)->WeaponType;

		return pWeaponType && pWeaponType->Warhead->BombDisarm ? CanBomb : SkipBomb;
	}

	return SkipBomb;
}

DEFINE_HOOK(0x51E4FB, InfantryClass_WhatAction_ObjectClass_EnigneerEnterBuilding, 0x6)
{
	GET(InfantryClass*, pThis, EDI);
	GET(BuildingClass*, pBuilding, ESI);
	GET(BuildingTypeClass*, pBuildingType, EAX);
	enum { Skip = 0x51E668, Continue = 0x51E501 };

	bool ignoreForce = WhatActionObjectTemp::ignoreForce;

	if (!ignoreForce && WhatActionObjectTemp::Fire)
		return Skip;

	bool BridgeRepairHut = pBuildingType->BridgeRepairHut;

	if (!BridgeRepairHut && pThis->Owner->IsAlliedWith(pBuilding->Owner))
	{
		if ((!ignoreForce && WhatActionObjectTemp::Move)
			|| pBuilding->Health >= pBuildingType->Strength)
		{
			return Skip;
		}
	}

	R->CL(BridgeRepairHut);
	return Continue;
}

DEFINE_HOOK(0x51EE6B, InfantryClass_WhatAction_ObjectClass_InfiltrateForceAttack, 0x6)
{
	return (!WhatActionObjectTemp::ignoreForce && WhatActionObjectTemp::Fire) ? 0x51F05E : 0;
}

#pragma endregion
