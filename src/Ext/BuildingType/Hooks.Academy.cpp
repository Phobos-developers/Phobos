#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/Body.h>

static void UpdateAcademy(HouseExt* pHouseExt, BuildingClass* pAcademy, bool added)
{
	auto it = std::find(pHouseExt->Academies.cbegin(), pHouseExt->Academies.cend(), pAcademy);

	if(added == (it != pHouseExt->Academies.cend()))
		return;

	if(added)
	{
		pHouseExt->Academies.push_back(pAcademy);
	} 
	else 
	{
		pHouseExt->Academies.erase(it);
	}
}
static void ApplyAcademy(TechnoClass* const pTechno, AbstractType const considerAs)
{
	if(Unsorted::ScenarioInit) {
		return;
	}
	
	const auto pType = pTechno->GetTechnoType();
	const auto pHouseExt = HouseExt::Fetch(pTechno->Owner);
	const auto pHouseTypeExt = HouseTypeExt::Fetch(pTechno->Owner->Type);
	const int countryIdx = pTechno->Owner->Type->ArrayIndex;

	if(!pHouseTypeExt->Academy_AllowCountryFilter)
		return;

	double finalBonus = 0.0;

	for(auto const& pBuilding : pHouseExt->Academies) 
	{
		const auto pBuildingType = pBuilding->Type;
		const auto pBuildingTypeExt = BuildingTypeExt::Fetch(pBuildingType);
		
		double veterancyBonus = 0.0;

		switch(considerAs)
		{
			case AbstractType::Infantry:
			{
				veterancyBonus = pBuildingTypeExt->Academy_Infantry_Veterancy;
				break;
			}
			case AbstractType::Unit:
			{
				veterancyBonus = pBuildingTypeExt->Academy_Vehicle_Veterancy;
				break;
			}
			case AbstractType::Aircraft:
			{
				veterancyBonus = pBuildingTypeExt->Academy_Aircraft_Veterancy;
				break;
			}
			case AbstractType::Building:
			{
				veterancyBonus = pBuildingTypeExt->Academy_Building_Veterancy;
				break;
			}
			default:
				break;
		}

		const auto& academyWhiteList = pBuildingTypeExt->Academy_Country_Types[countryIdx];

		const auto& academyBlackList = pBuildingTypeExt->Academy_Country_Ignore[countryIdx];

		const bool canAffect = (academyWhiteList.empty() || academyWhiteList.Contains(pType))
								&& (!academyBlackList.Contains(pType));

		if(canAffect)
			finalBonus = std::max(finalBonus, veterancyBonus);
	}

	if(pType->Trainable)
	{
		auto& value = pTechno->Veterancy.Veterancy;
		
		const auto pTechnoExt = TechnoExt::Fetch(pTechno);
		value = static_cast<float>(pTechnoExt->OriginVeterancy);

		if(finalBonus > value)
			value = static_cast<float>(std::min(finalBonus, RulesClass::Instance->VeteranCap));
	}
}

static void SetOriginVeterancy(TechnoClass* pTechno)
{
	const auto pHouse = pTechno->Owner;
	auto pTechnoExt = TechnoExt::Fetch(pTechno);

	if(pTechno->WhatAmI() == AbstractType::Infantry && pHouse->BarracksInfiltrated
		|| pTechno->WhatAmI() == AbstractType::Unit && pHouse->WarFactoryInfiltrated)
	{
		pTechnoExt->OriginVeterancy = 1.0;
	}
}

DEFINE_HOOK(0x446366, BuildingClass_Place_Academy, 0x6)
{
	GET(BuildingClass*, pThis, EBP);

	auto pBuildingExt = BuildingTypeExt::Fetch(pThis->Type);
	auto pHouseExt = HouseExt::Fetch(pThis->Owner);

	if(pBuildingExt->Academy)
		UpdateAcademy(pHouseExt, pThis, true);

	return 0;
}

DEFINE_HOOK(0x445905, BuildingClass_Remove_Academy, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	auto pBuildingExt = BuildingTypeExt::Fetch(pThis->Type);
	auto pHouseExt = HouseExt::Fetch(pThis->Owner);

	if(pThis->IsOnMap && pBuildingExt->Academy)
		UpdateAcademy(pHouseExt, pThis, false);

	return 0;
}

DEFINE_HOOK(0x448AB2, BuildingClass_ChangeOwnership_Remove_Academy, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	auto pBuildingExt = BuildingTypeExt::Fetch(pThis->Type);
	auto pHouseExt = HouseExt::Fetch(pThis->Owner);

	if(pThis->IsOnMap && pBuildingExt->Academy) 
		UpdateAcademy(pHouseExt, pThis, false);

	return 0;
}

DEFINE_HOOK(0x4491D5, BuildingClass_ChangeOwnership_Add_Academy, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	auto pBuildingExt = BuildingTypeExt::Fetch(pThis->Type);
	auto pHouseExt = HouseExt::Fetch(pThis->Owner);

	if(pThis->IsOnMap && pBuildingExt->Academy) 
		UpdateAcademy(pHouseExt, pThis, true);

	return 0;
}

DEFINE_HOOK(0x517D57, InfantryClass_Init_Academy, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	SetOriginVeterancy(pThis);
	ApplyAcademy(pThis, AbstractType::Infantry);
	return 0;
}

DEFINE_HOOK_AGAIN(0x735678, UnitClass_Init_Academy, 0x6) // in CTOR
DEFINE_HOOK(0x74689B, UnitClass_Init_Academy, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	SetOriginVeterancy(pThis);

	if(pThis->Type->ConsideredAircraft) 
	{
		ApplyAcademy(pThis, AbstractType::Aircraft);
	} 
	else if(pThis->Type->Organic) 
	{
		ApplyAcademy(pThis, AbstractType::Infantry);
	} 
	else 
	{
		ApplyAcademy(pThis, AbstractType::Unit);
	}

	return 0;
}

DEFINE_HOOK(0x413FD2, AircraftClass_Init_Academy, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	SetOriginVeterancy(pThis);
	ApplyAcademy(pThis, AbstractType::Aircraft);
	return 0;
}

DEFINE_HOOK(0x442D34, BuildingClass_Init_Academy, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	SetOriginVeterancy(pThis);
	ApplyAcademy(pThis, AbstractType::Building);
	return 0;
}