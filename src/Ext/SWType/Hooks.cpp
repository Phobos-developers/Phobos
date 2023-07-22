#include "Body.h"

#include <SuperClass.h>

//Ares hooked at 0x6CC390 and jumped to 0x6CDE40
// If a super is not handled by Ares however, we do it at the original entry point
DEFINE_HOOK_AGAIN(0x6CC390, SuperClass_Place_FireExt, 0x6)
DEFINE_HOOK(0x6CDE40, SuperClass_Place_FireExt, 0x4)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct const* const, pCell, 0x4);
	// GET_STACK(bool const, isPlayer, 0x8);

	// Check if the SuperClass pointer is valid and not corrupted.
	if (pSuper && VTable::Get(pSuper) == SuperClass::AbsVTable)
		SWTypeExt::FireSuperWeaponExt(pSuper, *pCell);
	else
		Debug::Log("SuperClass_Place_FireExt: Hook entered with an invalid or corrupt SuperClass pointer.");

	return 0;
}

DEFINE_HOOK(0x6CB5EB, SuperClass_Grant_ShowTimer, 0x5)
{
	GET(SuperClass*, pThis, ESI);

	if (SuperClass::ShowTimers->AddItem(pThis))
		std::sort(SuperClass::ShowTimers->begin(), SuperClass::ShowTimers->end(),
			[](SuperClass* a, SuperClass* b){
				auto aExt = SWTypeExt::ExtMap.Find(a->Type);
				auto bExt = SWTypeExt::ExtMap.Find(b->Type);
				return aExt->ShowTimer_Priority.Get() > bExt->ShowTimer_Priority.Get();
			});

	return 0x6CB63E;
}
