#include <TriggerClass.h>

#include <Helpers/Macro.h>

#include <Ext/TEvent/Body.h>

DEFINE_HOOK(0x727064, TriggerTypeClass_HasLocalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return
		nIndex >= PhobosTriggerEvent::LocalVariableGreaterThan && nIndex <= PhobosTriggerEvent::LocalVariableAndIsTrue ||
		nIndex >= PhobosTriggerEvent::LocalVariableGreaterThanLocalVariable && nIndex >= PhobosTriggerEvent::LocalVariableAndIsTrueLocalVariable ||
		nIndex >= PhobosTriggerEvent::LocalVariableGreaterThanGlobalVariable && nIndex >= PhobosTriggerEvent::LocalVariableAndIsTrueGlobalVariable ||
		nIndex == static_cast<int>(TriggerEvent::LocalSet) ?
		0x72706E :
		0x727069;
}

DEFINE_HOOK(0x727024, TriggerTypeClass_HasGlobalSetOrClearedEvent, 0x5)
{
	GET(const int, nIndex, EDX);

	return
		nIndex >= PhobosTriggerEvent::GlobalVariableGreaterThan && nIndex <= PhobosTriggerEvent::GlobalVariableAndIsTrue ||
		nIndex >= PhobosTriggerEvent::GlobalVariableGreaterThanLocalVariable && nIndex >= PhobosTriggerEvent::GlobalVariableAndIsTrueLocalVariable ||
		nIndex >= PhobosTriggerEvent::GlobalVariableGreaterThanGlobalVariable && nIndex >= PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable ||
		nIndex == static_cast<int>(TriggerEvent::GlobalSet) ?
		0x72702E :
		0x727029;
}
