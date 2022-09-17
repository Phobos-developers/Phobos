#include "Body.h"

#include <Helpers\Macro.h>

#include <HouseClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <OverlayTypeClass.h>
#include <VocClass.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x71E940, TEventClass_Execute, 0x5)
{
	GET(TEventClass*, pThis, ECX);
	GET_STACK(int, iEvent, 0x4); // now trigger what?
	GET_STACK(HouseClass*, pHouse, 0x8);
	GET_STACK(ObjectClass*, pObject, 0xC);
	GET_STACK(CDTimerClass*, pTimer, 0x10);
	GET_STACK(bool*, isPersitant, 0x14);
	GET_STACK(TechnoClass*, pSource, 0x18);

	bool handled;

	R->AL(TEventExt::Execute(pThis, iEvent, pHouse, pObject, pTimer, isPersitant, pSource, handled));

	return handled ? 0x71EA2D : 0;
}

DEFINE_HOOK(0x7271F9, TEventClass_GetFlags, 0x5)
{
	GET(int, eAttach, EAX);
	GET(TEventClass*, pThis, ESI);

	int nEvent = static_cast<int>(pThis->EventKind);
	if (nEvent >= PhobosTriggerEvent::LocalVariableGreaterThan && nEvent < PhobosTriggerEvent::_DummyMaximum)
		eAttach |= 0x10; // LOGIC

	R->EAX(eAttach);

	return 0;
}

DEFINE_HOOK(0x71F3FE, TEventClass_BuildINIEntry, 0x5)
{
	GET(int, eNeedType, EAX);
	GET(TEventClass*, pThis, ECX);

	int nEvent = static_cast<int>(pThis->EventKind);
	if (nEvent >= PhobosTriggerEvent::LocalVariableGreaterThan && nEvent < PhobosTriggerEvent::_DummyMaximum)
		eNeedType = 43;

	R->EAX(eNeedType);

	return 0;
}

DEFINE_HOOK(0x726577, TEventClass_Persistable, 0x7)
{
	GET(TEventClass*, pThis, EDI);

	int nEvent = static_cast<int>(pThis->EventKind);
	if (nEvent >= PhobosTriggerEvent::LocalVariableGreaterThan && nEvent < PhobosTriggerEvent::_DummyMaximum)
		R->AL(true);
	else
		R->AL(pThis->GetStateB());

	return 0x72657E;
}
