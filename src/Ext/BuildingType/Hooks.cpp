#include "Body.h"

#include <BuildingClass.h>
#include <HouseClass.h>

int BuildingTypeExt::GetEnhancedPower(BuildingClass* pBuilding, HouseClass* pHouse)
{
	int nAmount = 0;
	float fFactor = 1.0f;

	// Please don't use this logic to neutral house, I'm not sure why it will lead to lag - secsome
	if (!pHouse->IsNeutral())
	{
		for (const auto pBld : pHouse->Buildings)
		{
			const auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
			if (pExt->PowerPlantEnhancer_Buildings.Contains(pBuilding->Type))
			{
				fFactor *= pExt->PowerPlantEnhancer_Factor.Get(1.0f);
				nAmount += pExt->PowerPlantEnhancer_Amount.Get(0);
			}
		}
	}

	return static_cast<int>(pBuilding->GetPowerOutput() * fFactor) + nAmount;
}

// Power Plant Enhancer #131
DEFINE_HOOK(0x508CF2, HouseClass_Recalc_508C30_PowerOutput_Add, 0x7)
{
	GET(HouseClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);
	
	pThis->PowerOutput += BuildingTypeExt::GetEnhancedPower(pBld, pThis);

	return 0x508D07;
}

DEFINE_HOOK(0x508D45, HouseClass_Recalc_508C30_PowerOutput_Check, 0x5)
{
	GET(HouseClass*, pThis, ESI);

	if (pThis->PowerOutput < 0)
		pThis->PowerOutput = 0;

	return 0;
}