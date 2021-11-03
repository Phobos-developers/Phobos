#include <TriggerClass.h>

#include <Helpers/Macro.h>

#include <Ext/TEvent/Body.h>

DEFINE_HOOK(0x727064, TriggerTypeClass_HasLocalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return 
		nIndex >= (int)PhobosTriggerEvent::LocalVariableGreaterThan && nIndex <= (int)PhobosTriggerEvent::LocalVariableAndIsTrue ?
		0x72706E : 
		0x72707E;
}

DEFINE_HOOK(0x727024, TriggerTypeClass_HasGlobalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return 
		nIndex >= (int)PhobosTriggerEvent::GlobalVariableGreaterThan && nIndex <= (int)PhobosTriggerEvent::GlobalVariableAndIsTrue ?
		0x72702E : 
		0x72703E;
}