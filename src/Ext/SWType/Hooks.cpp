#include "Body.h"

#include <SuperClass.h>

//Ares hooked from 0x6CC390 and jumped to this offset
DEFINE_HOOK(0x6CDE40, SuperClass_Launch_finale, 0x3)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct const* const, pCell, 0x4);
	// GET_STACK(bool const, isPlayer, 0x8);

	if (auto const pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type))
		pSWExt->FireSuperWeapon(pSuper, *pCell);

	return 0;
}
