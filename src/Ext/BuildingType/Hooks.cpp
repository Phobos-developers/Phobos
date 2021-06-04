#include "Body.h"

#include <BuildingClass.h>
#include <HouseClass.h>

int BuildingTypeExt::GetEnchancedPower(BuildingClass* pBuilding, HouseClass* pHouse)
{
	int nPower = pBuilding->GetPowerOutput();

	for (const auto pBld : pHouse->Buildings)
	{
		const auto pSrcType = abstract_cast<BuildingTypeClass*>(pBuilding->GetType());
		const auto pType = abstract_cast<BuildingTypeClass*>(pBld->GetType());
		const auto pExt = BuildingTypeExt::ExtMap.Find(pType);

		if (pExt->PowerPlantEnchancer_Buildings.Contains(pSrcType))
			if (pExt->PowerPlantEnchancer_Amount > 0)
				nPower += pExt->PowerPlantEnchancer_Amount;
	}

	for (const auto pBld : pHouse->Buildings)
	{
		const auto pSrcType = abstract_cast<BuildingTypeClass*>(pBuilding->GetType());
		const auto pType = abstract_cast<BuildingTypeClass*>(pBld->GetType());
		const auto pExt = BuildingTypeExt::ExtMap.Find(pType);

		if (pExt->PowerPlantEnchancer_Buildings.Contains(pSrcType))
			if (pExt->PowerPlantEnchancer_Factor > 0)
				nPower = static_cast<int>(nPower * pExt->PowerPlantEnchancer_Factor);
	}
	
	return nPower;
}

// Power Plant Enhancer #131
DEFINE_HOOK(508CF2, HouseClass_Recalc_508C30_Power_Output, 7)
{
	GET(HouseClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);
	
	pThis->PowerOutput += BuildingTypeExt::GetEnchancedPower(pBld, pThis);

	return 0x508D07;
}