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

	if (dumpAmount > 0)
		dumpAmount = std::min(dumpAmount, totalAmount);
	else
		dumpAmount = totalAmount;

	__asm fld dumpAmount;

	return SkipGameCode;
}
