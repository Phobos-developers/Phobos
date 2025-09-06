// Issue #5 Permanently stationary units
// Author: Starkku

#include "UnitClass.h"

#include <Utilities/GeneralUtils.h>
#include <Ext/Techno/Body.h>

DEFINE_HOOK(0x740A93, UnitClass_Mission_Move_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return TechnoExt::CannotMove(pThis) ? 0x740AEF : 0;
}

DEFINE_HOOK(0x741AA7, UnitClass_Assign_Destination_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, EBP);

	return TechnoExt::CannotMove(pThis) ? 0x743173 : 0;
}

DEFINE_HOOK(0x743B4B, UnitClass_Scatter_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, EBP);

	return TechnoExt::CannotMove(pThis) ? 0x74408E : 0;
}

DEFINE_HOOK(0x74038F, UnitClass_What_Action_ObjectClass_DisallowMoving_1, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return TechnoExt::CannotMove(pThis) ? 0x7403A3 : 0;
}

DEFINE_HOOK(0x7403B7, UnitClass_What_Action_ObjectClass_DisallowMoving_2, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return TechnoExt::CannotMove(pThis) ? 0x7403C1 : 0;
}

DEFINE_HOOK(0x740709, UnitClass_What_Action_DisallowMoving_1, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return TechnoExt::CannotMove(pThis) ? 0x740727 : 0;
}

DEFINE_HOOK(0x740744, UnitClass_What_Action_DisallowMoving_2, 0x6)
{
	enum { AllowAttack = 0x74078E, ReturnNoMove = 0x740769, ReturnResult = 0x740801 };

	GET(UnitClass*, pThis, ESI);
	GET_STACK(Action, result, 0x30);

	if (TechnoExt::CannotMove(pThis))
	{
		if (result == Action::Move)
			return ReturnNoMove;
		if (result != Action::Attack)
			return ReturnResult;

		return AllowAttack;
	}

	return 0;
}

DEFINE_HOOK(0x736B60, UnitClass_Rotation_AI_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return !TechnoTypeExt::ExtMap.Find(pThis->Type)->TurretResponse.Get(!TechnoExt::CannotMove(pThis)) ? 0x736AFB : 0;
}

DEFINE_HOOK(0x73891D, UnitClass_Active_Click_With_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return TechnoExt::CannotMove(pThis) ? 0x738927 : 0;
}

DEFINE_HOOK(0x73EFC4, UnitClass_Mission_Hunt_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	if (TechnoExt::CannotMove(pThis))
	{
		pThis->QueueMission(Mission::Guard, false);
		pThis->NextMission();

		R->EAX(pThis->Mission_Guard());
		return 0x73F091;
	}

	return 0;
}

// 3 Sep, 2025 - Starkku: Separated from above, do not change to guard mission
// and only handle the target acquisition part of area guard for immobile units.
DEFINE_HOOK(0x744103, UnitClass_Mission_AreaGuard_DisallowMoving, 0x6) 
{
	GET(UnitClass*, pThis, ESI);

	if (TechnoExt::CannotMove(pThis))
	{
		if (pThis->CanPassiveAcquireTargets() && pThis->TargetingTimer.Completed()) 
			pThis->TargetAndEstimateDamage(pThis->Location, ThreatType::Range);

		int delay = 1;

		if (!pThis->Target)
		{
			pThis->UpdateIdleAction();
			auto const control = &MissionControlClass::Array[(int)Mission::Area_Guard];
			delay = static_cast<int>(control->Rate * 900) + ScenarioClass::Instance->Random(1, 5);
		}

		R->EAX(delay);
		return 0x744173;
	}

	return 0;
}

DEFINE_HOOK(0x74132B, UnitClass_GetFireError_DisallowMoving, 0x7)
{
	GET(UnitClass*, pThis, ESI);
	GET(FireError, result, EAX);

	if (result == FireError::RANGE && TechnoExt::CannotMove(pThis))
		R->EAX(FireError::ILLEGAL);

	return 0;
}

namespace UnitApproachTargetTemp
{
	int WeaponIndex;
}

DEFINE_HOOK(0x7414E0, UnitClass_ApproachTarget_DisallowMoving, 0xA)
{
	GET(UnitClass*, pThis, ECX);

	int weaponIndex = -1;

	if (TechnoExt::CannotMove(pThis))
	{
		const auto pTarget = pThis->Target;
		weaponIndex = pThis->SelectWeapon(pTarget);

		if (!pThis->IsCloseEnough(pTarget, weaponIndex))
		{
			pThis->SetTarget(nullptr);
			return 0x741690;
		}
	}

	UnitApproachTargetTemp::WeaponIndex = weaponIndex;
	return 0;
}

DEFINE_HOOK(0x7415A9, UnitClass_ApproachTarget_SetWeaponIndex, 0x6)
{
	if (UnitApproachTargetTemp::WeaponIndex != -1)
	{
		GET(UnitClass*, pThis, ESI);

		R->EDI(VTable::Get(pThis));
		R->EAX(UnitApproachTargetTemp::WeaponIndex);
		UnitApproachTargetTemp::WeaponIndex = -1;

		return 0x7415BA;
	}

	return 0;
}

DEFINE_HOOK(0x6F7CE2, TechnoClass_CanAutoTargetObject_DisallowMoving, 0x6)
{
	GET(TechnoClass* const, pThis, EDI);
	GET(AbstractClass* const, pTarget, ESI);
	GET(const int, weaponIndex, EBX);

	if (const auto pUnit = abstract_cast<UnitClass*, true>(pThis))
	{
		if (TechnoExt::CannotMove(pUnit))
		{
			R->EAX(pUnit->GetFireError(pTarget, weaponIndex, true));
			return 0x6F7CEE;
		}
	}

	return 0;
}

DEFINE_HOOK(0x7088E3, TechnoClass_ShouldRetaliate_DisallowMoving, 0x6)
{
	GET(TechnoClass* const, pThis, EDI);
	GET(AbstractClass* const, pTarget, EBP);
	GET(const int, weaponIndex, EBX);

	if (const auto pUnit = abstract_cast<UnitClass*, true>(pThis))
	{
		if (TechnoExt::CannotMove(pUnit))
		{
			R->Stack(STACK_OFFSET(0x18, 0x4), weaponIndex);
			R->EAX(pUnit->GetFireError(pTarget, weaponIndex, true));
			return 0x7088F3;
		}
	}

	return 0;
}
