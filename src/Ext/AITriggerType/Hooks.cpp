#include <Utilities/Macro.h>
#include <AITriggerTypeClass.h>
#include <Ext/AITriggerType/Body.h>
#include <Phobos.h>
#include "Body.h"

DEFINE_HOOK_AGAIN(0x41E8DD, Phobos_AITrigger_Handler, 0x8)
DEFINE_HOOK(0x41E8F0, Phobos_AITrigger_Handler, 0x8)
{
	GET(AITriggerTypeClass*, pAITriggerType, ESI);
	GET(HouseClass*, pHouse, EDI);

	//get Condition String
	char ConditionString[68];
	int idx = 0;
	char* condStr = ConditionString;
	auto buf = reinterpret_cast<const byte*>(&pAITriggerType->Conditions);
	do
	{
		sprintf_s(condStr, 4, "%02x", *buf);
		++buf;
		++idx;
		condStr += 2;
	}
	while (idx < 0x20);
	*condStr = '\0';

	//this is when Phobos strats to work
	if ((pAITriggerType->ConditionType == AITriggerCondition::Pool || pAITriggerType->ConditionType == AITriggerCondition::AIOwns)
		&& pAITriggerType->ConditionObject == nullptr)
	{
		std::string ConditionString2 = ConditionString;
		int PhobosAIConditionType = atoi(ConditionString2.substr(56, 4).c_str());		
		int PhobosAIConditionList = atoi(ConditionString2.substr(60, 4).c_str());

		if (PhobosAIConditionType > 0)
		{
			AITriggerTypeExt::ProcessCondition(pAITriggerType, pHouse, PhobosAIConditionType, PhobosAIConditionList);
		}
	}
		
	return 0;
}


