#include <Utilities/Macro.h>

#include <InfantryClass.h>
#include <HouseClass.h>
#include <InputManagerClass.h>
#include <WarheadTypeClass.h>

#include <Ext/TechnoType/Body.h>

DEFINE_HOOK(0x51B2BD, InfantryClass_UpdateTarget_IsControlledByHuman, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EDI);

	return (!pTarget || pThis->Owner->IsControlledByHuman()) ? 0x51B33F : 0;
}

#pragma region WhatActionObjectFix

namespace WhatActionObjectTemp
{
	bool Fire = false;
	bool Move = false;
}

DEFINE_HOOK(0x51E462, InfantryClass_WhatAction_ObjectClass_SkipBomb, 0x6)
{
	enum { Skip = 0x51E668, SkipBomb = 0x51E49E, CanBomb = 0x51E48F };

	GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pTarget, ESI);
	GET_STACK(const bool, ignoreForce, STACK_OFFSET(0x38, 0x8));

	WhatActionObjectTemp::Fire = !ignoreForce && InputManagerClass::Instance->IsForceFireKeyPressed();
	WhatActionObjectTemp::Move = !ignoreForce && InputManagerClass::Instance->IsForceMoveKeyPressed();

	if (!pThis->Type->Engineer)
		return Skip;

	if (pThis->Owner->IsControlledByCurrentPlayer()
		&& pTarget->AttachedBomb && pTarget->BombVisible)
	{
		if (WhatActionObjectTemp::Move)
			return SkipBomb;

		const int index = pThis->SelectWeapon(pTarget);
		const auto pWeaponType = pThis->GetWeapon(index)->WeaponType;

		return pWeaponType && pWeaponType->Warhead->BombDisarm ? CanBomb : SkipBomb;
	}

	return SkipBomb;
}

DEFINE_HOOK(0x51E4FB, InfantryClass_WhatAction_ObjectClass_EnigneerEnterBuilding, 0x6)
{
	enum { Skip = 0x51E668, Continue = 0x51E501 };

	GET(InfantryClass*, pThis, EDI);
	GET(BuildingClass*, pBuilding, ESI);
	GET(BuildingTypeClass*, pBuildingType, EAX);

	if (WhatActionObjectTemp::Fire)
		return Skip;

	const bool bridgeRepairHut = pBuildingType->BridgeRepairHut;

	if (!bridgeRepairHut && pThis->Owner->IsAlliedWith(pBuilding->Owner))
	{
		if (WhatActionObjectTemp::Move || pBuilding->Health >= pBuildingType->Strength)
			return Skip;
	}

	R->CL(bridgeRepairHut);
	return Continue;
}

DEFINE_HOOK(0x51EE6B, InfantryClass_WhatAction_ObjectClass_InfiltrateForceAttack, 0x6)
{
	return WhatActionObjectTemp::Fire ? 0x51F05E : 0;
}

DEFINE_HOOK(0x51ECC0, InfantryClass_WhatAction_ObjectClass_IsAreaFire, 0xA)
{
	enum { IsAreaFire = 0x51ECE5, NotAreaFire = 0x51ECEC };

	GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pObject, ESI);
	const int deployWeaponIdx = pThis->Type->DeployFireWeapon;
	const auto deployWeapon = pThis->GetWeapon(deployWeaponIdx >= 0 ? deployWeaponIdx : pThis->SelectWeapon(pObject))->WeaponType;

	return deployWeapon && deployWeapon->AreaFire ? IsAreaFire : NotAreaFire;
}

#pragma endregion

DEFINE_HOOK(0x7093F8, TechnoClass_709290_DeployWeapon, 0x5)
{
	enum { ReturnTrue = 0x70944F, ReturnFalse = 0x709449 };

	GET(TechnoClass*, pThis, ESI);

	if (const auto pInfantry = abstract_cast<InfantryClass*, true>(pThis))
	{
		const int deployWeaponIdx = pInfantry->Type->DeployFireWeapon;

		if (deployWeaponIdx >= 0)
		{
			const auto pWeaponStruct = pThis->GetWeapon(deployWeaponIdx);

			if (pWeaponStruct && pWeaponStruct->WeaponType && pWeaponStruct->WeaponType->AreaFire && deployWeaponIdx == pThis->SelectWeapon(pThis->Target))
				return ReturnFalse;
		}
		else
		{
			const auto pWeaponStruct = pThis->GetWeapon(pThis->SelectWeapon(pThis->Target));

			if (pWeaponStruct && pWeaponStruct->WeaponType && pWeaponStruct->WeaponType->AreaFire)
				return ReturnFalse;
		}
	}
	else
	{
		const int weaponIdx = pThis->IsNotSprayAttack();
		const auto pWeaponStruct = pThis->GetWeapon(weaponIdx);

		if (pWeaponStruct && pWeaponStruct->WeaponType && pWeaponStruct->WeaponType->AreaFire && weaponIdx == pThis->SelectWeapon(pThis->Target))
			return ReturnFalse;
	}

	return ReturnTrue;
}

// Skip incorrect retn to restore the auto deploy behavior of infantry
DEFINE_HOOK(0x522373, InfantryClass_ApproachTarget_InfantryAutoDeploy, 0x5)
{
	enum { Deploy = 0x522378 };
	GET(InfantryClass*, pThis, ESI);
	return TechnoTypeExt::ExtMap.Find(pThis->Type)->InfantryAutoDeploy.Get(RulesExt::Global()->InfantryAutoDeploy) ? Deploy : 0;
}
