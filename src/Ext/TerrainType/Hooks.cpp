#include "Body.h"

#include <ScenarioClass.h>
#include <TiberiumClass.h>
#include <OverlayTypeClass.h>
#include <TerrainClass.h>
#include <SpecificStructures.h>
#include <AnimClass.h>

#include <Utilities/GeneralUtils.h>

namespace TerrainTypeTemp
{
	TerrainTypeClass* pCurrentType = nullptr;
	TerrainTypeExt::ExtData* pCurrentExt = nullptr;
}

DEFINE_HOOK(0x71C853, TerrainTypeClass_Context_Set, 0x6)
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

DEFINE_HOOK(0x483811, CellClass_SpreadTiberium_TiberiumType, 0x8)
{
	if (TerrainTypeTemp::pCurrentExt)
	{
		LEA_STACK(int*, pTibType, STACK_OFFS(0x1C, -0x4));

		*pTibType = TerrainTypeTemp::pCurrentExt->SpawnsTiberium_Type;

		return 0x483819;
	}

	return 0;
}

DEFINE_HOOK(0x48381D, CellClass_SpreadTiberium_CellSpread, 0x6)
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

DEFINE_HOOK(0x71C8D7, TerrainTypeClass_Context_Unset, 0x5)
{
	TerrainTypeTemp::pCurrentType = nullptr;
	TerrainTypeTemp::pCurrentExt = nullptr;

	return 0;
}

//This one on Very end of it , let everything play first
DEFINE_HOOK(0x71BB2C, TerrainClass_TakeDamage_NowDead_Add, 0x6)
{
	GET(TerrainClass*, pThis, ESI);
	//saved for later usage !
	//REF_STACK(args_ReceiveDamage const, ReceiveDamageArgs, STACK_OFFS(0x3C, -0x4));

	if (auto const pTerrainExt = TerrainTypeExt::ExtMap.Find(pThis->Type))
	{
		auto const nCoords = pThis->GetCoords();
		VocClass::PlayIndexAtPos(pTerrainExt->DestroySound.Get(-1), nCoords);

		if (auto const pAnimType = pTerrainExt->DestroyAnim.Get(nullptr))
			GameCreate<AnimClass>(pAnimType, nCoords);
	}

	return 0;
}