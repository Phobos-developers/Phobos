#include <Ext/AITriggerType/Body.h>
#include <Helpers/Macro.h>

DEFINE_HOOK(0x41E8FF, AITriggerTypeClass_NewTeam_CheckConditions, 0x9) // ConditionMet() in YRpp
{
	enum { ContinueGameChecks = 0x41E908, ReturnFromFunction = 0x41E9E1, Success = 0x41E9EA };

	GET(AITriggerTypeClass*, pThis, ESI);
	GET(HouseClass*, pOwner, EDI);
	GET(HouseClass*, pEnemy, EBX);

	int result = AITriggerTypeExt::CheckConditions(pThis, pOwner, pEnemy);

	switch (result)
	{
	case 0:
		return ReturnFromFunction;
		break;
	case 1:
		return Success;
		break;
	default:
		return ContinueGameChecks;
		break;
	}
}
