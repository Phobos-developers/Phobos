#include <UnitClass.h>

#include <Ext/TechnoType/Body.h>

DEFINE_HOOK(0x73E411, UnitClass_Mission_Unload_DumpAmount, 0x7)
{
	enum { SkipGameCode = 0x73E41D };

	GET(UnitClass*, pThis, ESI);
	GET(int, tiberiumIdx, EBP);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	const float totalAmount = pThis->Tiberium.GetAmount(tiberiumIdx);
	float dumpAmount = pTypeExt->HarvesterDumpAmount.Get(RulesExt::Global()->HarvesterDumpAmount);

	if (dumpAmount <= 0.0f || totalAmount < dumpAmount)
		dumpAmount = totalAmount;

	__asm fld dumpAmount;

	return SkipGameCode;
}

#pragma region HarvesterScanAfterUnload

DEFINE_HOOK(0x73E730, UnitClass_MissionHarvest_HarvesterScanAfterUnload, 0x5)
{
	GET(UnitClass* const, pThis, EBP);
	GET(AbstractClass* const, pFocus, EAX);

	// Focus is set when the harvester is fully loaded and go home.
	if (pFocus && TechnoTypeExt::ExtMap.Find(pThis->Type)->HarvesterScanAfterUnload.Get(RulesExt::Global()->HarvesterScanAfterUnload))
	{
		auto cellBuffer = CellStruct::Empty;
		const auto pCellStru = pThis->ScanForTiberium(&cellBuffer, RulesClass::Instance->TiberiumLongScan / Unsorted::LeptonsPerCell, 0);

		if (*pCellStru != CellStruct::Empty)
		{
			const auto pCell = MapClass::Instance.TryGetCellAt(*pCellStru);
			const auto distFromTiberium = pCell ? pThis->DistanceFrom(pCell) : -1;
			const auto distFromFocus = pThis->DistanceFrom(pFocus);

			// Check if pCell is better than focus.
			if (distFromTiberium > 0 && distFromTiberium < distFromFocus)
				R->EAX(pCell);
		}
	}

	return 0;
}

#pragma endregion
