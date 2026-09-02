#include <Ext/Infantry/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

#include <InputManagerClass.h>

DEFINE_HOOK(0x51DF42, InfantryClass_Limbo_Cyborg, 0x7)
{
	enum { SkipReset = 0x51DF53 };

	GET(InfantryClass*, pThis, ESI);

	if (pThis->Type->Cyborg && pThis->Crawling)
	{
		if (pThis->Transporter)
		{
			// Note: When infantry enters into a transport this Limbo will be executed 2 times, in the second run of this hook infantry will contain information of the transport unit (Transporter variable)
			auto const pTypeExt = TechnoExt::Fetch(pThis->Transporter)->TypeExtData;

			if (pTypeExt->FixEnteringCyborgLegs)
				return 0;
		}

		return SkipReset;
	}

	return 0;
}

// When infantry enters into structures (not executed in Ares Tunnel logic)
DEFINE_HOOK(0x52291A, InfantryClass_InfantryEnteredThing_Cyborg, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	REF_STACK(TechnoClass*, pBuilding, 0x1C);

	if (pThis->Type->Cyborg && pThis->Crawling)
	{
		auto const pTypeExt = TechnoExt::Fetch(pBuilding)->TypeExtData;

		if (pTypeExt->FixEnteringCyborgLegs)
			pThis->Crawling = false;
	}

	return 0;
}

// When infantry enters into a structure with "tunnel logic" created by Ares
DEFINE_HOOK(0x51A27F, InfantryClass_PerCellProcess_AresTunnel_Cyborg, 0xA)
{
	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EDI);

	if (pThis->Type->Cyborg && pThis->Crawling)
	{
		auto const pTypeExt = TechnoTypeExt::Fetch(pBuilding->Type);

		if (pTypeExt->FixEnteringCyborgLegs)
			pThis->Crawling = false;
	}

	return 0;
}

DEFINE_HOOK(0x51B2BD, InfantryClass_UpdateTarget_IsControlledByHuman, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EDI);

	return (!pTarget || pThis->Owner->IsControlledByHuman()) ? 0x51B33F : 0;
}

// Deploy case: DoAction(Deployed)
DEFINE_HOOK(0x520B3E, InfantryClass_DoingAI_DeployConvert_Deploy, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	auto const pExt = InfantryExt::Fetch(pThis);
	auto const pTypeExt = pExt->TypeExtData;

	if (pTypeExt->Convert_Deploy && !pExt->HasDeployConverted)
	{
		pExt->HasDeployConverted = true;
		pExt->HasUndeployConverted = false;
		TechnoExt::ConvertToType(pThis, pTypeExt->Convert_Deploy);
	}

	return 0;
}

// Undeploy case: DoAction(Ready)
DEFINE_HOOK(0x520B99, InfantryClass_DoingAI_DeployConvert_Undeploy, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	auto const pExt = InfantryExt::Fetch(pThis);
	auto const pTypeExt = pExt->TypeExtData;

	if (pTypeExt->Convert_Undeploy && !pExt->HasUndeployConverted)
	{
		pExt->HasUndeployConverted = true;
		pExt->HasDeployConverted = false;
		TechnoExt::ConvertToType(pThis, pTypeExt->Convert_Undeploy);
	}

	return 0;
}

// Reset mark when Deploy/Undeploy
DEFINE_HOOK(0x520E75, InfantryClass_DoingAI_DeployConvert_ResetFlags, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	const auto curSeq = pThis->SequenceAnim;

	if (curSeq != Sequence::Deploy && curSeq != Sequence::Undeploy)
	{
		auto const pExt = InfantryExt::Fetch(pThis);
		pExt->HasDeployConverted = false;
		pExt->HasUndeployConverted = false;
	}

	return 0;
}

#pragma region WhatActionObjectFix

namespace WhatActionObjectTemp
{
	bool Fire = false;
	bool Move = false;
}

DEFINE_HOOK(0x51E462, InfantryClass_WhatAction_ObjectClass_SkipBomb, 0x6)
{
	enum { Skip = 0x51E668, SkipBomb = 0x51E49E, GoToAres = 0x51E488 };

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

		return pWeaponType && pWeaponType->Warhead->BombDisarm ? GoToAres : SkipBomb;
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
		if (WhatActionObjectTemp::Move)
			return Skip;

		if (pBuilding->Health >= pBuildingType->Strength)
		{
			const auto pTypeExt = BuildingTypeExt::Fetch(pBuildingType);

			if (!pTypeExt->RubbleIntact && !pTypeExt->RubbleIntactRemove)
				return Skip;
		}
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
	return InfantryTypeExt::Fetch(pThis->Type)->InfantryAutoDeploy.Get(RulesExt::Global()->InfantryAutoDeploy) ? Deploy : 0;
}

DEFINE_HOOK(0x51A002, InfantryClass_UpdatePosition_InfiltrateBuilding, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EDI);

	if (const auto pTag = pBuilding->AttachedTag)
		pTag->RaiseEvent(TriggerEvent::SpiedBy, pThis, CellStruct::Empty);

	if (const auto pTag = pBuilding->AttachedTag)
		pTag->RaiseEvent(TriggerEvent::SpyAsHouse, pThis, CellStruct::Empty);

	if (const auto pTag = pBuilding->AttachedTag)
		pTag->RaiseEvent(TriggerEvent::SpyAsInfantry, pThis, CellStruct::Empty);

	return 0;
}
