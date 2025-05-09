#include "Body.h"

#include <CellClass.h>
#include <OverlayClass.h>

#include <Utilities/GeneralUtils.h>

DEFINE_HOOK(0x47C210, CellClass_CellColor_TiberiumRadarColor, 0x6)
{
	enum { ReturnFromFunction = 0x47C23F };

	GET(CellClass*, pThis, ESI);

	int tiberiumType = OverlayClass::GetTiberiumType(pThis->OverlayTypeIndex);

	if (tiberiumType < 0)
		return 0;

	auto pTiberium = TiberiumClass::Array.GetItem(tiberiumType);

	if (const auto pTiberiumExt = TiberiumExt::ExtMap.Find(pTiberium))
	{
		if (pTiberiumExt->MinimapColor.isset())
		{
			GET_STACK(ColorStruct*, arg0, STACK_OFFSET(0x14, 0x4));
			GET_STACK(ColorStruct*, arg4, STACK_OFFSET(0x14, 0x8));
			auto& color = pTiberiumExt->MinimapColor.Get();

			arg0->R = color.R;
			arg0->G = color.G;
			arg0->B = color.B;

			arg4->R = color.R;
			arg4->G = color.G;
			arg4->B = color.B;

			R->ECX(arg4);
			R->AL(color.B);

			return ReturnFromFunction;
		}
	}

	return 0;
}
