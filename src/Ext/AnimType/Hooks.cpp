#include "Body.h"

#include <Ext/Anim/Body.h>
/*
DEFINE_HOOK(0x422BE0, AnimClass_CenterCoord_XDrawOffset, 0x5)
{
	GET(AnimClass* const, pThis, ECX);
	GET_BASE(CoordStruct*, pCoord, 0x4);
	auto pThisTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	int offset = pThisTypeExt->XDrawOffset;
	CoordStruct Result = pThis->Location;

	if (pThis->OwnerObject)
	{
		CoordStruct OwnerCoord;
		pThis->OwnerObject->GetCenterCoord(&OwnerCoord);
		Result += OwnerCoord;
	}
	if (offset != 0)
		Result.X += offset;

	//R->EDI(&Result);
	pCoord = &Result;
	return 0x422C36;
}*/

DEFINE_HOOK(0x422CAB, AnimClass_DrawIt_XDrawOffset, 0x5)
{
	GET(AnimClass* const, pThis, ECX);
	GET_STACK(Point2D*, pCoord, STACK_OFFS(0x100, -0x4));
	auto pThisTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	int offset = pThisTypeExt->XDrawOffset;

	if (offset != 0)
	{
		pCoord->X += offset;
	}

	return 0;
}
