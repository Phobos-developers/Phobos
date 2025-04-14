#include "Body.h"

#include <FactoryClass.h>
#include <TEventClass.h>

// AI Naval queue bugfix hooks

namespace ExitObjectTemp
{
	int ProducingUnitIndex = -1;
}

DEFINE_HOOK(0x444113, BuildingClass_ExitObject_NavalProductionFix1, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	GET(FootClass* const, pObject, EDI);

	auto const pHouse = pThis->Owner;

	if (pObject->WhatAmI() == AbstractType::Unit && pObject->GetTechnoType()->Naval)
	{
		if (auto const pHouseExt = HouseExt::ExtMap.Find(pHouse))
			pHouseExt->ProducingNavalUnitTypeIndex = -1;

		ExitObjectTemp::ProducingUnitIndex = pHouse->ProducingUnitTypeIndex;
	}

	return 0;
}

DEFINE_HOOK(0x444137, BuildingClass_ExitObject_NavalProductionFix2, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	GET(FootClass* const, pObject, EDI);

	auto const pHouse = pThis->Owner;

	if (pObject->WhatAmI() == AbstractType::Unit && pObject->GetTechnoType()->Naval)
		pHouse->ProducingUnitTypeIndex = ExitObjectTemp::ProducingUnitIndex;

	return 0;
}

DEFINE_HOOK(0x450319, BuildingClass_AI_Factory_NavalProductionFix, 0x6)
{
	enum { SkipGameCode = 0x450332 };

	GET(BuildingClass* const, pThis, ESI);

	auto pHouse = pThis->Owner;
	TechnoTypeClass* pTechnoType = nullptr;
	int index = -1;

	switch (pThis->Type->Factory)
	{
	case AbstractType::Aircraft:
	case AbstractType::AircraftType:
		index = pHouse->ProducingAircraftTypeIndex;

		if (index >= 0)
			pTechnoType = AircraftTypeClass::Array.GetItem(index);

		break;

	case AbstractType::Building:
	case AbstractType::BuildingType:
		index = pHouse->ProducingBuildingTypeIndex;

		if (index >= 0)
			pTechnoType = BuildingTypeClass::Array.GetItem(index);

		break;

	case AbstractType::Infantry:
	case AbstractType::InfantryType:
		index = pHouse->ProducingInfantryTypeIndex;

		if (index >= 0)
			pTechnoType = InfantryTypeClass::Array.GetItem(index);

		break;

	case AbstractType::Unit:
	case AbstractType::UnitType:
		index = !pThis->Type->Naval ? pHouse->ProducingUnitTypeIndex : HouseExt::ExtMap.Find(pHouse)->ProducingNavalUnitTypeIndex;

		if (index >= 0)
			pTechnoType = UnitTypeClass::Array.GetItem(index);

		break;
	}

	R->EAX(pTechnoType);
	return SkipGameCode;
}

DEFINE_HOOK(0x4CA0A1, FactoryClass_Abandon_NavalProductionFix, 0x5)
{
	enum { SkipUnitTypeCheck = 0x4CA0B7 };

	GET(FactoryClass* const, pThis, ESI);

	if (pThis->Object->WhatAmI() == AbstractType::Unit && pThis->Object->GetTechnoType()->Naval)
	{
		if (auto const pHouseExt = HouseExt::ExtMap.Find(pThis->Owner))
		{
			pHouseExt->ProducingNavalUnitTypeIndex = -1;
			return SkipUnitTypeCheck;
		}
	}

	return 0;
}

DEFINE_HOOK(0x4F91A4, HouseClass_AI_BuildingProductionCheck, 0x6)
{
	enum { SkipGameCode = 0x4F9265, CheckBuildingProduction = 0x4F9240 };

	GET(HouseClass* const, pThis, ESI);

	auto const pExt = HouseExt::ExtMap.Find(pThis);

	bool cantBuild = pThis->ProducingUnitTypeIndex == -1 && pThis->ProducingInfantryTypeIndex == -1 &&
		pThis->ProducingAircraftTypeIndex == -1 && pExt->ProducingNavalUnitTypeIndex == -1;

	int index = pExt->ProducingNavalUnitTypeIndex;
	if (index != -1 && !UnitTypeClass::Array.GetItem(index)->FindFactory(true, true, true, pThis))
		cantBuild = true;

	index = pThis->ProducingUnitTypeIndex;
	if (index != -1 && !UnitTypeClass::Array.GetItem(index)->FindFactory(true, true, true, pThis))
		cantBuild = true;

	index = pThis->ProducingInfantryTypeIndex;
	if (index != -1 && !InfantryTypeClass::Array.GetItem(index)->FindFactory(true, true, true, pThis))
		cantBuild = true;

	index = pThis->ProducingAircraftTypeIndex;
	if ((index != -1 && !AircraftTypeClass::Array.GetItem(index)->FindFactory(true, true, true, pThis)) || cantBuild)
		return CheckBuildingProduction;

	return SkipGameCode;
}

DEFINE_HOOK(0x4FE0A3, HouseClass_AI_RaiseMoney_NavalProductionFix, 0x6)
{
	GET(HouseClass* const, pThis, ESI);

	if (auto const pExt = HouseExt::ExtMap.Find(pThis))
		pExt->ProducingNavalUnitTypeIndex = -1;

	return 0;
}

DEFINE_HOOK_AGAIN(0x4F90F0, HouseClass_AI_NavalProductionFix, 0x7)
DEFINE_HOOK(0x4F9250, HouseClass_AI_NavalProductionFix, 0x7)
{
	enum { SkipGameCodeOne = 0x4F9257, SkipGameCodeTwo = 0x4F90F7 };

	GET(HouseClass* const, pThis, ESI);

	HouseExt::ExtMap.Find(pThis)->UpdateVehicleProduction();

	return R->Origin() == 0x4F9250 ? SkipGameCodeOne : SkipGameCodeTwo;
}

DEFINE_HOOK(0x4FB6FC, HouseClass_JustBuilt_NavalProductionFix, 0x6)
{
	enum { SkipGameCode = 0x4FB702 };

	GET(HouseClass* const, pThis, EDI);
	GET(UnitTypeClass* const, pUnitType, EDX);
	GET(int const, ID, EAX);

	if (pUnitType->Naval)
	{
		HouseExt::ExtMap.Find(pThis)->LastBuiltNavalVehicleType = ID;
		return SkipGameCode;
	}

	return 0;
}

DEFINE_HOOK(0x71F003, TEventClass_Execute_NavalProductionFix, 0x6)
{
	enum { Execute = 0x71F014, Skip = 0x71F163 };

	GET(TEventClass* const, pThis, EBP);
	GET(HouseClass* const, pHouse, EAX);

	if (pHouse->LastBuiltVehicleType != pThis->Value &&
		HouseExt::ExtMap.Find(pHouse)->LastBuiltNavalVehicleType != pThis->Value)
	{
		return Skip;
	}

	return Execute;
}
