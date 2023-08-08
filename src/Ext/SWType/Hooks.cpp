#include "Body.h"

#include <SuperClass.h>

//Ares hooked at 0x6CC390 and jumped to 0x6CDE40
// If a super is not handled by Ares however, we do it at the original entry point
DEFINE_HOOK_AGAIN(0x6CC390, SuperClass_Place_FireExt, 0x6)
DEFINE_HOOK(0x6CDE40, SuperClass_Place_FireExt, 0x4)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct const* const, pCell, 0x4);
	// GET_STACK(bool const, isPlayer, 0x8);

	// Check if the SuperClass pointer is valid and not corrupted.
	if (pSuper && VTable::Get(pSuper) == SuperClass::AbsVTable)
		SWTypeExt::FireSuperWeaponExt(pSuper, *pCell);
	else
		Debug::Log("SuperClass_Place_FireExt: Hook entered with an invalid or corrupt SuperClass pointer.");

	return 0;
}

DEFINE_HOOK(0x6CB5EB, SuperClass_Grant_ShowTimer, 0x5)
{
	GET(SuperClass*, pThis, ESI);

	if (SuperClass::ShowTimers->AddItem(pThis))
		std::sort(SuperClass::ShowTimers->begin(), SuperClass::ShowTimers->end(),
			[](SuperClass* a, SuperClass* b){
				auto aExt = SWTypeExt::ExtMap.Find(a->Type);
				auto bExt = SWTypeExt::ExtMap.Find(b->Type);
				return aExt->ShowTimer_Priority.Get() > bExt->ShowTimer_Priority.Get();
			});

	return 0x6CB63E;
}

DEFINE_HOOK(0x6DC2C5, Tactical_SuperLinesCircles_ShowDesignatorRange, 0x5)
{
	GET(const SuperWeaponTypeClass*, pSuperType, EDI);

	if (!Phobos::Config::ShowDesignatorRange)
		return 0;

	const auto pExt = SWTypeExt::ExtMap.Find(pSuperType);
	if (!pExt)
		return 0;

	for (const auto pType : pExt->SW_Designators)
	{
		for (const auto pCurrentTechno : *TechnoClass::Array)
		{
			const auto pCurrentTechnoType = pCurrentTechno->GetTechnoType();
			const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pCurrentTechnoType);

			if (!pTechnoTypeExt
				|| !pCurrentTechno->IsAlive
				|| pCurrentTechno->InLimbo
				|| !pExt->SW_Designators.Contains(pCurrentTechnoType)
				|| (pCurrentTechno->Owner != HouseClass::CurrentPlayer))
			{
				continue;
			}

			const CoordStruct coords = pCurrentTechno->GetCenterCoords();
			const auto color = pCurrentTechno->Owner->Color;
			const float radius = (float)(pTechnoTypeExt->DesignatorRange.Get(pCurrentTechnoType->Sight));
			Game::DrawRadialIndicator(false, true, coords, color, radius, false, true);
		}
	}

	return 0;
}
