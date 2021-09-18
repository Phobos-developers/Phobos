#include "Body.h"

#include <InfantryTypeClass.h>
#include <HouseClass.h>

DEFINE_HOOK(0x4430BC, BuildingClass_RegisterDestruction_EjectCountryCrew, 0x6)
{
	GET(InfantryTypeClass*, pInfType, EAX);
	GET(BuildingClass*, pThis, EDI);
	auto pExt = HouseTypeExt::ExtMap.Find(pThis->Owner->Type);

	if (pExt->CountryCrew)
		R->EAX(pExt->CountryCrew_Type.Get(pInfType));

	return 0;
}

DEFINE_HOOK(0x44A645, BuildingClass_MissionDeconstruction_EjectCountryCrew, 0x6)
{
	GET(InfantryTypeClass*, pInfType, ESI);
	GET(BuildingClass*, pThis, EBP);
	auto pExt = HouseTypeExt::ExtMap.Find(pThis->Owner->Type);

	if (pExt->CountryCrew)
		R->ESI(pExt->CountryCrew_Type.Get(pInfType));

	return 0;
}

