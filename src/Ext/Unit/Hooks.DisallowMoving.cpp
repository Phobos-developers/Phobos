// Issue #5 Permanently stationary units
// Author: Starkku

#include "UnitClass.h"
#include "InfantryClass.h"

#include <Utilities/GeneralUtils.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

DEFINE_HOOK(0x740A93, UnitClass_Mission_Move_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return pThis->Type->Speed == 0 ? 0x740AEF : 0;
}

DEFINE_HOOK(0x741AA7, UnitClass_Assign_Destination_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, EBP);

	return pThis->Type->Speed == 0 ? 0x743173 : 0;
}

DEFINE_HOOK(0x743B4B, UnitClass_Scatter_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, EBP);

	return pThis->Type->Speed == 0 ? 0x74408E : 0;
}

DEFINE_HOOK(0x74038F, UnitClass_What_Action_ObjectClass_DisallowMoving_1, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return pThis->Type->Speed == 0 ? 0x7403A3 : 0;
}

DEFINE_HOOK(0x7403B7, UnitClass_What_Action_ObjectClass_DisallowMoving_2, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return pThis->Type->Speed == 0 ? 0x7403C1 : 0;
}

DEFINE_HOOK(0x740709, UnitClass_What_Action_DisallowMoving_1, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return pThis->Type->Speed == 0 ? 0x740727 : 0;
}

DEFINE_HOOK(0x740744, UnitClass_What_Action_DisallowMoving_2, 0x6)
{
	enum { AllowAttack = 0x74078E, ReturnNoMove = 0x740769, ReturnResult = 0x740801 };

	GET(UnitClass*, pThis, ESI);
	GET_STACK(Action, result, 0x30);

	if (pThis->Type->Speed == 0)
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

	return pThis->Type->Speed == 0 ? 0x736AFB : 0;
}

DEFINE_HOOK(0x73891D, UnitClass_Active_Click_With_DisallowMoving, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	return pThis->Type->Speed == 0 ? 0x738927 : 0;
}

DEFINE_HOOK(0x51AA49, InfantryClass_Assign_Destination_DisallowMoving, 0x6)
{
	GET(InfantryClass*, pThis, ECX);

	if (pThis->ParalysisTimer.HasTimeLeft())
		return 0x51B1D7;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->WebbyDurationCountDown > 0)
	{
		if (pThis->Target)
		{
			pThis->SetTarget(nullptr);
			pThis->SetDestination(nullptr, false);
			pThis->QueueMission(Mission::Sleep, false);
		}
	}

	return 0;
}
