#include "Body.h"

#include <SidebarClass.h>
#include <BuildingTypeClass.h>
#include <TechnoClass.h>

// Hook into HouseClass::RegisterObjectGain_FromFactory (0x4FB6B0).
// Fires when a unit or building is delivered from a factory.
// At this point: EDI = HouseClass*, EBP = TechnoClass* (the object being delivered).
// We track the delivered type per sidebar tab so the "Build Last" commands can re-queue it.

DEFINE_HOOK(0x4FB6C2, HouseClass_RegisterObjectGain_TrackLastBuiltTab, 0x7)
{
	GET(HouseClass* const, pThis, EDI);
	GET(TechnoClass* const, pTechno, EBP);

	if (pThis != HouseClass::CurrentPlayer)
		return 0;

	auto const pType = pTechno->GetTechnoType();
	if (!pType)
		return 0;

	auto const absType = pType->WhatAmI();
	auto const isNaval = pType->Naval;

	auto buildCat = BuildCat::DontCare;
	if (absType == AbstractType::BuildingType)
		buildCat = static_cast<BuildingTypeClass const*>(pType)->BuildCat;

	int const tabIdx = SidebarClass::GetObjectTabIdx(absType, buildCat, isNaval);

	if (tabIdx >= 0 && tabIdx < 4)
	{
		auto const pExt = HouseExt::ExtMap.Find(pThis);
		pExt->LastBuiltPerTab[tabIdx] = pType->GetArrayIndex();
		pExt->LastBuiltRTTIPerTab[tabIdx] = absType;
		pExt->LastBuiltIsNavalPerTab[tabIdx] = isNaval;

		// Track per-subtype within tab 3 for dedicated rebuild commands.
		if (tabIdx == 3)
		{
			if (absType == AbstractType::AircraftType)
			{
				pExt->LastBuiltAircraftTypeIndex = pType->GetArrayIndex();
				pExt->LastBuiltAircraftRTTI = absType;
			}
			else if (isNaval)
			{
				pExt->LastBuiltNavalTypeIndex = pType->GetArrayIndex();
				pExt->LastBuiltNavalRTTI = absType;
			}
			else
			{
				pExt->LastBuiltVehicleTypeIndex = pType->GetArrayIndex();
				pExt->LastBuiltVehicleRTTI = absType;
			}
		}
	}

	return 0;
}
