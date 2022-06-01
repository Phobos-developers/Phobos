#include <TriggerClass.h>

#include <Helpers/Macro.h>

#include <Utilities/PointerMapper.h>

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

DEFINE_HOOK(0x7268D0, TriggerClass_Save, 0x8)
{
	GET_STACK(TriggerClass*, pThis, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	PhobosByteStream TmpByteStream(0x4);
	PhobosStreamWriter Stm(TmpByteStream);
	Stm.Save(pThis);
	TmpByteStream.WriteToStream(pStm);
	return 0;
}

DEFINE_HOOK(0x726860, TriggerClass_Load, 0x5)
{
	GET_STACK(TriggerClass*, pThis, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	PhobosByteStream TmpByteStream(0x4);
	TmpByteStream.ReadFromStream(pStm, 0x4);
	PhobosStreamReader Stm(TmpByteStream);
	TriggerClass* pOldThis = nullptr;
	Stm.Load(pOldThis);
	PointerMapper::AddMapping(pOldThis, pThis);
	return 0;
}