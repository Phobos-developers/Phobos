#include "Body.h"
#include <Ext/House/Body.h>

// Issue #503
// Author : Otamaa
DEFINE_HOOK(0x4AE670, DisplayClass_GetToolTip_EnemyUIName, 0x8)
{
	enum { SetUIName = 0x4AE678 };

	GET(ObjectClass*, pObject, ECX);

	auto pDecidedUIName = pObject->GetUIName();
	auto pFoot = generic_cast<FootClass*>(pObject);
	auto pTechnoType = pObject->GetTechnoType();

	if (pFoot && pTechnoType && !pObject->IsDisguised())
	{
		bool IsAlly = true;
		bool IsCivilian = false;
		bool IsObserver = HouseClass::Observer || HouseClass::IsCurrentPlayerObserver();

		if (auto pOwnerHouse = pFoot->GetOwningHouse())
		{
			IsAlly = pOwnerHouse->IsAlliedWith(HouseClass::CurrentPlayer);
			IsCivilian = (pOwnerHouse == HouseClass::FindCivilianSide()) || pOwnerHouse->IsNeutral();
		}

		if (!IsAlly && !IsCivilian && !IsObserver)
		{
			auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

			if (auto pEnemyUIName = pTechnoTypeExt->EnemyUIName.Get().Text)
			{
				pDecidedUIName = pEnemyUIName;
			}
		}
	}

	R->EAX(pDecidedUIName);
	return SetUIName;
}

DEFINE_HOOK(0x711F39, TechnoTypeClass_CostOf_FactoryPlant, 0x8)
{
	GET(TechnoTypeClass*, pThis, ESI);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(float, mult, STACK_OFFSET(0x10, -0x8));

	auto const pHouseExt = HouseExt::ExtMap.Find(pHouse);

	if (pHouseExt->RestrictedFactoryPlants.size() > 0)
		mult *= pHouseExt->GetRestrictedFactoryPlantMult(pThis);

	return 0;
}

DEFINE_HOOK(0x711FDF, TechnoTypeClass_RefundAmount_FactoryPlant, 0x8)
{
	GET(TechnoTypeClass*, pThis, ESI);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(float, mult, STACK_OFFSET(0x10, -0x4));

	auto const pHouseExt = HouseExt::ExtMap.Find(pHouse);

	if (pHouseExt->RestrictedFactoryPlants.size() > 0)
		mult *= pHouseExt->GetRestrictedFactoryPlantMult(pThis);

	return 0;
}
