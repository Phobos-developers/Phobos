#include "Body.h"

#include "../Techno/Body.h"
#include "../Building/Body.h"
#include <unordered_map>
#include <Ext/Building/Body.h>
#include <Ext/TechnoType/Body.h>
#include <UnitClass.h>
#include <InfantryClass.h>
#include <ScenarioClass.h>

DEFINE_HOOK(0x508C30, HouseClass_UpdatePower_UpdateCounter, 0x5)
{
	GET(HouseClass*, pThis, ECX);
	auto pHouseExt = HouseExt::ExtMap.Find(pThis);

	pHouseExt->BuildingCounter.clear();

	// This pre-iterating ensure our process to be done in O(NM) instead of O(N^2),
	// as M should be much less than N, this will be a great improvement. - secsome
	for (auto& pBld : pThis->Buildings)
	{
		if (pBld && !pBld->InLimbo && pBld->IsOnMap)
		{
			const auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
			if (pExt->PowerPlantEnhancer_Buildings.size() &&
				(pExt->PowerPlantEnhancer_Amount != 0 || pExt->PowerPlantEnhancer_Factor != 1.0f))
			{
				++pHouseExt->BuildingCounter[pExt];
			}
		}
	}

	return 0;
}

// Power Plant Enhancer #131
DEFINE_HOOK(0x508CF2, HouseClass_UpdatePower_PowerOutput, 0x7)
{
	GET(HouseClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);

	pThis->PowerOutput += BuildingTypeExt::GetEnhancedPower(pBld, pThis);

	return 0x508D07;
}

DEFINE_HOOK(0x73E474, UnitClass_Unload_Storage, 0x6)
{
	GET(BuildingClass* const, pBuilding, EDI);
	GET(int const, idxTiberium, EBP);
	REF_STACK(float, amount, 0x1C);

	auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
	if (!pTypeExt)
		return 0;

	if (!pBuilding->Owner)
		return 0;

	auto storageTiberiumIndex = RulesExt::Global()->Storage_TiberiumIndex;

	if (pTypeExt->Refinery_UseStorage && storageTiberiumIndex >= 0)
	{
		BuildingExt::StoreTiberium(pBuilding, amount, idxTiberium, storageTiberiumIndex);
		amount = 0.0f;
	}
	
	return 0;
}

DEFINE_HOOK(0x444119, BuildingClass_KickOutUnit_UnitType, 0x6)
{
	GET(UnitClass*, pUnit, EDI);
	GET(BuildingClass*, pFactory, ESI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pUnit->GetTechnoType());
	if (!pTypeExt->RandomProduct.empty())
	{
		int iPos = ScenarioClass::Instance->Random(0, int(pTypeExt->RandomProduct.size()) - 1);
		TechnoTypeClass* pType = TechnoTypeClass::Array->GetItem(pTypeExt->RandomProduct[iPos]);
		UnitClass* pNewUnit = static_cast<UnitClass*>(pType->CreateObject(pUnit->GetOwningHouse()));
		pNewUnit->Limbo();
		pNewUnit->Unlimbo(pUnit->Location, Direction::SouthEast);
		pUnit->Limbo();
		pUnit->UnInit();
		R->EDI(pNewUnit);
		pUnit = pNewUnit;
	}

	return 0;
}


DEFINE_HOOK(0x444131, BuildingClass_KickOutUnit_InfantryType, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	GET(InfantryClass*, pInf, EDI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pInf->GetTechnoType());
	if (!pTypeExt->RandomProduct.empty())
	{
		int iPos = ScenarioClass::Instance->Random(0, int(pTypeExt->RandomProduct.size()) - 1);
		TechnoTypeClass* pType = TechnoTypeClass::Array->GetItem(pTypeExt->RandomProduct[iPos]);
		InfantryClass* pNewInf = static_cast<InfantryClass*>(pType->CreateObject(pHouse));
		pInf->Limbo();
		pInf->UnInit();
		R->EDI(pNewInf);
		pInf = pNewInf;
	}

	return 0;
}