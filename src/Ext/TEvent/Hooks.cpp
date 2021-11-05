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
	GET_STACK(TimerStruct*, pTimer, 0x10);
	GET_STACK(bool*, isPersitant, 0x14);
	GET_STACK(TechnoClass*, pSource, 0x18);

	bool handled;

	R->AL(TEventExt::Execute(pThis, iEvent, pHouse, pObject, pTimer, isPersitant, pSource, handled));

	return handled ? 0x71EA2D : 0;
}

DEFINE_HOOK(0x71F683, TEventClass_GetFlags, 0x5)
{
	GET(int, eAttach, ESI);
	GET(int, nEvent, ECX);

	if (nEvent >= PhobosTriggerEvent::LocalVariableGreaterThan && nEvent <= PhobosTriggerEvent::GlobalVariableAndIsTrueGlobalVariable)
		eAttach |= 0x10; // LOGIC

	R->ESI(eAttach);

	return 0;
}