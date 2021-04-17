#include "Body.h"

#include <ScenarioClass.h>
#include <TiberiumClass.h>
#include <OverlayTypeClass.h>

#include "../../Utilities/GeneralUtils.h"
#include "../../Utilities/Macro.h"

namespace TerrainTypeTemp
{
	TerrainTypeClass* pCurrentType = nullptr;
	TerrainTypeExt::ExtData* pCurrentExt = nullptr;
}

DEFINE_HOOK(71C853, TerrainTypeClass_Context_Set, 6)
{
	TerrainTypeTemp::pCurrentType = R->ECX<TerrainTypeClass*>();
	TerrainTypeTemp::pCurrentExt = TerrainTypeExt::ExtMap.Find(TerrainTypeTemp::pCurrentType);

	return 0;
}

// thiscall is being emulated here, ECX = pThis, EDX is discarded, second arg is passed thru stack - Kerbiter
void __fastcall TerrainClass_AI_CellsPerAnim(CellClass* pThis, void*, bool forced)
{
	int cellCount = 1;
	if (TerrainTypeTemp::pCurrentExt)
		cellCount = TerrainTypeTemp::pCurrentExt->GetCellsPerAnim();

	for (int i = 0; i < cellCount; i++)
		pThis->SpreadTiberium(forced);
}

DEFINE_POINTER_CALL(0x71C8D0, TerrainClass_AI_CellsPerAnim)

DEFINE_HOOK(483811, CellClass_SpreadTiberium_TiberiumType, 8)
{
	if (TerrainTypeTemp::pCurrentExt)
	{
		LEA_STACK(int*, pTibType, STACK_OFFS(0x1C, -0x4));
		
		*pTibType = TerrainTypeTemp::pCurrentExt->SpawnsTiberium_Type;

		return 0x483819;
	}

	return 0;
}

DEFINE_HOOK(48381D, CellClass_SpreadTiberium_CellSpread, 6)
{
	enum { SpreadReturn = 0x4838CA, NoSpreadReturn = 0x4838B0 };

	if (TerrainTypeTemp::pCurrentExt)
	{
		GET(CellClass*, pThis, EDI);
		GET(int, tibIndex, EAX);

		TiberiumClass* pTib = TiberiumClass::Array->GetItem(tibIndex);

		std::vector<CellStruct> adjacentCells = GeneralUtils::AdjacentCellsInRange(TerrainTypeTemp::pCurrentExt->SpawnsTiberium_Range);
		size_t size = adjacentCells.size();
		int rand = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);

		for (unsigned int i = 0; i < size; i++)
		{
			unsigned int cellIndex = (i + rand) % size;
			CellStruct tgtPos = pThis->MapCoords + adjacentCells[cellIndex];
			CellClass* tgtCell = MapClass::Instance->GetCellAt(tgtPos);

			if (tgtCell && tgtCell->CanTiberiumGerminate(pTib))
			{
				R->EAX<bool>(tgtCell->IncreaseTiberium(tibIndex,
					TerrainTypeTemp::pCurrentExt->GetTiberiumGrowthStage()));

				return SpreadReturn;
			}
		}

		return NoSpreadReturn;
	}

	return 0;
}

DEFINE_HOOK(71C8D7, TerrainTypeClass_Context_Unset, 5)
{
	TerrainTypeTemp::pCurrentType = nullptr;
	TerrainTypeTemp::pCurrentExt = nullptr;

	return 0;
}