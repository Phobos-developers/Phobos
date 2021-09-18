#include "Body.h"

#include <InfantryTypeClass.h>
#include <HouseClass.h>

DEFINE_HOOK(0x4430BC, BuildingClass_RegisterDestruction_EjectCountryCrew, 0x6)
{
	GET(InfantryTypeClass*, pInfType, EAX);
	GET(HouseClass*, pHouse, EDX);
	auto pExt = HouseTypeExt::ExtMap.Find(pHouse->Type);

	if (pExt->CountryCrew)
		R->EAX(pExt->CountryCrew_Type.Get(pInfType));

	return 0;
}