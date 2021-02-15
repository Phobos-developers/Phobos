#include <UnitClass.h>
#include "Body.h"

DEFINE_HOOK(6F64A9, HealthBar_Hide, 5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData->HealthBar_Hide) {
		return 0x6F6AB6;
	}
	return 0;
}
DEFINE_HOOK(6F9E50, TechnoClass_Update_2, 5) //TechnoClass_Update is in BuildingDeployerTargeting.cpp
{
	GET(TechnoClass*, pThis, ECX);
	auto pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	if (auto Capturer = pThis->MindControlledBy) {
		auto pCapturerExt = TechnoTypeExt::ExtMap.Find(Capturer->GetTechnoType());
		if (pCapturerExt->MindControlRangeLimit > 0 && pThis->DistanceFrom(Capturer) > pCapturerExt->MindControlRangeLimit * 256.0) {
			Capturer->CaptureManager->FreeUnit(pThis);
			if (!pThis->IsHumanControlled) {
				pThis->QueueMission(Mission::Hunt, 0);
			};
		}
	}
	return 0;
}
