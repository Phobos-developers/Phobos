#include "Body.h"

#include <InfantryTypeClass.h>
#include <HouseClass.h>

DEFINE_HOOK(0x4430BC, BuildingClass_RegisterDestruction_EjectCountryCrew, 0x6)
{
	GET(InfantryTypeClass*, pInfType, EAX);

	GET(BuildingClass*, pThis, EDI);
	HouseTypeClass* pHouseType = pThis->Owner->Type;
	auto pExt = HouseTypeExt::ExtMap.Find(pHouseType);

	if (pExt->CountryCrew)
	{
		pInfType = pExt->CountryCrew_Type;
		R->EAX(pInfType);
	}

	return 0;
}