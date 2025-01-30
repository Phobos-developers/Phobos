#include "Body.h"

#include <Ext/Techno/Body.h>

namespace CellExtTemp
{
	UnitClass* pThis = nullptr;
}

DEFINE_HOOK(0x7441B0, UnitClass_SetOccupyBit_Prefix, 0x5)
{
	enum { SkipFuncAndReturn = 0x74420B };

	GET(UnitClass*, pThis, ECX);

	if (TechnoExt::DoesntOccupyCellAsChild(pThis))
		return SkipFuncAndReturn;

	CellExtTemp::pThis = pThis;

	return 0;
}

DEFINE_HOOK(0x744209, UnitClass_SetOccupyBit_IncomingUnit, 0x5)
{
	GET(CellClass*, pCell, EDI);

	CellExt::ExtMap.Find(pCell)->IncomingUnit = CellExtTemp::pThis;

	return 0;
}

DEFINE_HOOK(0x7441F6, UnitClass_SetOccupyBit_IncomingUnitAlt, 0x5)
{
	GET(CellClass*, pCell, EDI);

	CellExt::ExtMap.Find(pCell)->IncomingUnitAlt = CellExtTemp::pThis;

	return 0;
}

// currently the game, at least without other exts, doesn't do any checks if it's clearing a unit's occupy bit

DEFINE_HOOK(0x744210, UnitClass_ClearOccupyBit_Prefix, 0x5)
{
	enum { SkipFuncAndReturn = 0x74424D };

	GET(UnitClass*, pThis, ECX);

	if (TechnoExt::DoesntOccupyCellAsChild(pThis))
		return SkipFuncAndReturn;

	return 0;
}

DEFINE_HOOK(0x74425E, UnitClass_ClearOccupyBit_IncomingUnit, 0x5)
{
	GET(CellClass*, pCell, EDI);

	CellExt::ExtMap.Find(pCell)->IncomingUnit = nullptr;

	return 0;
}

DEFINE_HOOK(0x74424B, UnitClass_ClearOccupyBit_IncomingUnitAlt, 0x5)
{
	GET(CellClass*, pCell, EDI);

	CellExt::ExtMap.Find(pCell)->IncomingUnitAlt = nullptr;

	return 0;
}
