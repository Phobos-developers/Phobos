#include "Body.h"

#include <Ext/Anim/Body.h>

DEFINE_HOOK(0x422CAB, AnimClass_DrawIt_XDrawOffset, 0x5)
{
	GET(AnimClass* const, pThis, ECX);
	GET_STACK(Point2D*, pCoord, STACK_OFFS(0x100, -0x4));
	auto pThisTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	pCoord->X += pThisTypeExt->XDrawOffset;

	return 0;
}
