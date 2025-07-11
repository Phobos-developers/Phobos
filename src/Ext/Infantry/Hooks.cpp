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
	bool IgnoreForce = false;
	bool Fire = false;
	bool Move = false;
}

DEFINE_HOOK(0x51E462, InfantryClass_WhatAction_ObjectClass_SkipBomb, 0x6)
{
	GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pTarget, ESI);
	GET_STACK(const bool, ignoreForce, STACK_OFFSET(0x38, 0x8));
	enum { Skip = 0x51E668, SkipBomb = 0x51E49E, CanBomb = 0x51E48F };

	WhatActionObjectTemp::IgnoreForce = ignoreForce;
	WhatActionObjectTemp::Fire = InputManagerClass::Instance->IsForceFireKeyPressed();
	WhatActionObjectTemp::Move = InputManagerClass::Instance->IsForceMoveKeyPressed();

	if (!pThis->Type->Engineer)
		return Skip;

	if (pThis->Owner->IsControlledByCurrentPlayer()
		&& pTarget->AttachedBomb && pTarget->BombVisible)
	{
		if (!ignoreForce && WhatActionObjectTemp::Move)
			return SkipBomb;

		const int index = pThis->SelectWeapon(pTarget);
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

	const bool ignoreForce = WhatActionObjectTemp::IgnoreForce;

	if (!ignoreForce && WhatActionObjectTemp::Fire)
		return Skip;

	const bool bridgeRepairHut = pBuildingType->BridgeRepairHut;

	if (!bridgeRepairHut && pThis->Owner->IsAlliedWith(pBuilding->Owner))
	{
		if ((!ignoreForce && WhatActionObjectTemp::Move)
			|| pBuilding->Health >= pBuildingType->Strength)
		{
			return Skip;
		}
	}

	R->CL(bridgeRepairHut);
	return Continue;
}

DEFINE_HOOK(0x51EE6B, InfantryClass_WhatAction_ObjectClass_InfiltrateForceAttack, 0x6)
{
	return (!WhatActionObjectTemp::IgnoreForce && WhatActionObjectTemp::Fire) ? 0x51F05E : 0;
}

#pragma endregion
