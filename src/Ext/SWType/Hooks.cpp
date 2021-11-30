#include "Body.h"

#include <SuperClass.h>

DEFINE_HOOK(0x6CDE40, SuperClass_Place, 0x5)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CoordStruct const, coords, 0x230); // I think?

	if (auto const pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type))
		pSWExt->FireSuperWeapon(pSuper, pSuper->Owner, coords);

	return 0;
}
