#include "Body.h"

DEFINE_HOOK(0x471D40, CaptureManagerClass_CaptureUnit, 0x7)
{
	GET(CaptureManagerClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, 0x4);

	R->AL(CaptureManagerExt::CaptureUnit(pThis, pTechno));

	return 0x471D5A;
}

DEFINE_HOOK(0x471FF0, CaptureManagerClass_FreeUnit, 0x8)
{
	GET(CaptureManagerClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, 0x4);

	R->AL(CaptureManagerExt::FreeUnit(pThis, pTechno));

	return 0x472006;
}

DEFINE_HOOK(0x6FCB34, TechnoClass_CanFire_CanCapture, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, EBP);

	R->AL(CaptureManagerExt::CanCapture(pThis->CaptureManager, pTarget));

	return 0x6FCB40;
}

DEFINE_HOOK(0x519F71, InfantryClass_UpdatePosition_BeforeBuildingChangeHouse, 0x6)
{
	GET(BuildingClass*, pBld, EDI);

	if (auto pBy = pBld->MindControlledBy)
		CaptureManagerExt::FreeUnit(pBy->CaptureManager, pBld);

	if (std::exchange(pBld->MindControlledByAUnit, false))
	{
		if (auto& pAnim = pBld->MindControlRingAnim)
		{
			pAnim->SetOwnerObject(nullptr);
			pAnim->UnInit();
			pAnim = nullptr;
		}
	}

	return 0;
}
