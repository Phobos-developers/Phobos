#include "Body.h"

#include <Ext/Techno/Body.h>


// because 🦅💣 takes over, we have to do reimpl even more of the func and replicate Ares code

void __fastcall UnitClass_SetOccupyBit_Reimpl(UnitClass* pThis, discard_t, CoordStruct* pCrd)
{
	if (TechnoExt::DoesntOccupyCellAsChild(pThis))
		return;

	CellClass* pCell = MapClass::Instance->GetCellAt(*pCrd);
	auto pCellExt = CellExt::ExtMap.Find(pCell);
	int height = MapClass::Instance->GetCellFloorHeight(*pCrd) + CellClass::BridgeHeight;
	bool alt = (pCrd->Z >= height && pCell->ContainsBridge());

	// remember which occupation bit we set
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->AltOccupation = alt;

	if (alt)
	{
		pCell->AltOccupationFlags |= 0x20;
		// Phobos addition: set incoming unit tracker
		pCellExt->IncomingUnitAlt = pThis;
	}
	else
	{
		pCell->OccupationFlags |= 0x20;
		// Phobos addition: set incoming unit tracker
		pCellExt->IncomingUnit = pThis;
	}
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5D60, UnitClass_SetOccupyBit_Reimpl);

void __fastcall UnitClass_ClearOccupyBit_Reimpl(UnitClass* pThis, discard_t, CoordStruct* pCrd)
{
	if (TechnoExt::DoesntOccupyCellAsChild(pThis))
		return;

	enum { obNormal = 1, obAlt = 2 };

	CellClass* pCell = MapClass::Instance->GetCellAt(*pCrd);
	auto pCellExt = CellExt::ExtMap.Find(pCell);
	int height = MapClass::Instance->GetCellFloorHeight(*pCrd) + CellClass::BridgeHeight;
	int alt = (pCrd->Z >= height) ? obAlt : obNormal;

	// also clear the last occupation bit, if set
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	if(pExt->AltOccupation.has_value())
	{
		int lastAlt = pExt->AltOccupation.value() ? obAlt : obNormal;
		alt |= lastAlt;
		pExt->AltOccupation.reset();
	}

	if (alt & obAlt)
	{
		pCell->AltOccupationFlags &= ~0x20;
		// Phobos addition: clear incoming unit tracker
		pCellExt->IncomingUnitAlt = nullptr;
	}

	if (alt & obNormal)
	{
		pCell->OccupationFlags &= ~0x20;
		// Phobos addition: clear incoming unit tracker
		pCellExt->IncomingUnit = nullptr;
	}
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5D64, UnitClass_ClearOccupyBit_Reimpl);

// TODO ^ same for TA for non-UnitClass, not needed so cba for now
