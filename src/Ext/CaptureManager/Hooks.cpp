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

DEFINE_HOOK(0x4DBF23, FootClass_ChangeOwner_IAmNowHuman, 0x6)
{
	GET(FootClass* const, pThis, ESI);
	// This is not limited to mind control, could possibly affect many map triggers
	// This is still not even correct, but let's see how far this can help us

	pThis->ShouldScanForTarget = false;
	pThis->ShouldEnterAbsorber = false;
	pThis->ShouldEnterOccupiable = false;
	pThis->ShouldGarrisonStructure = false;

	if (pThis->HasAnyLink() || pThis->GetTechnoType()->ResourceGatherer) // Don't want miners to stop
		return 0;

	switch (pThis->GetCurrentMission())
	{
	case Mission::Harvest:
	case Mission::Sleep:
	case Mission::Harmless:
	case Mission::Repair:
		return 0;
	}

	pThis->vt_entry_1F4(Mission::Guard, nullptr, nullptr); // I don't even know what this is, just clear the target and destination for me
	pThis->ShouldLoseTargetNow = TRUE;
	pThis->QueueMission(pThis->GetTechnoType()->DefaultToGuardArea ? Mission::Area_Guard : Mission::Guard, true);

	return 0;
}

