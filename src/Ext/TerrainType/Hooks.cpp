#include "Body.h"

#include <ScenarioClass.h>
#include <TiberiumClass.h>
#include <OverlayTypeClass.h>

#include "../../Utilities/GeneralUtils.h"

namespace TerrainTypeTemp {
	TerrainTypeClass* pCurrentType = nullptr;
	TerrainTypeExt::ExtData* pCurrentExt = nullptr;
}

DEFINE_HOOK(71C853, TerrainTypeClass_Context_Set, 6)
{
	TerrainTypeTemp::pCurrentType = R->ECX<TerrainTypeClass*>();
	TerrainTypeTemp::pCurrentExt = TerrainTypeExt::ExtMap.Find(TerrainTypeTemp::pCurrentType);

	return 0;
}

DEFINE_HOOK(483811, CellClass_SpreadTiberium_TiberiumType, 8)
{
	LEA_STACK(int*, pTibType, STACK_OFFS(0x1C, -0x4));

	if (TerrainTypeTemp::pCurrentExt) {
		*pTibType = TerrainTypeTemp::pCurrentExt->SpawnsTiberium_Type;

		return 0x483819;
	}

	return 0;
}

DEFINE_HOOK(48381D, CellClass_SpreadTiberium_CellSpread, 6)
{
	if (TerrainTypeTemp::pCurrentExt)
	{
		GET(CellClass*, pThis, EDI);
		GET(int, tibIndex, EAX);

		TiberiumClass* pTib = TiberiumClass::Array->GetItem(tibIndex);

		std::vector<CellStruct> adjacentCells = GeneralUtils::AdjacentCellsInRange(TerrainTypeTemp::pCurrentExt->SpawnsTiberium_Range);
		size_t size = adjacentCells.size();
		int rand = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);

		for (unsigned int i = 0; i < size; i++) {
			unsigned int cellIndex = (i + rand) % size;
			CellStruct tgtPos = pThis->MapCoords + adjacentCells[cellIndex];
			CellClass* tgtCell = MapClass::Instance->GetCellAt(tgtPos);

			if (tgtCell && tgtCell->CanTiberiumGerminate(pTib))
			{
				// return with call to spread tib
				R->EAX<bool>(tgtCell->IncreaseTiberium(tibIndex, 3));

				return 0x4838CA;
			}
		}

		// return without spreading tib
		return 0x4838B0;
	}
	
	return 0;
}

DEFINE_HOOK(71C8D7, TerrainTypeClass_Context_Unset, 5)
{
	TerrainTypeTemp::pCurrentType = nullptr;
	TerrainTypeTemp::pCurrentExt = nullptr;

	return 0;
}