#include "Body.h"


TerrainTypeClass* currentType = nullptr;

DEFINE_HOOK(71C853, TerrainTypeClass_Context_Set, 6)
{
	currentType = R->ECX<TerrainTypeClass*>();

	return 0;
}

DEFINE_HOOK(483811, CellClass_SpreadTiberium_TiberiumType, 8)
{
	LEA_STACK(int*, pTibType, 0x20);

	if (auto pTypeExt = TerrainTypeExt::ExtMap.Find(currentType)) {
		*pTibType = pTypeExt->SpawnsTiberium_Type;

		return 0x483819;
	}

	return 0;
}

DEFINE_HOOK(71C8DB, TerrainTypeClass_Context_Unset, 1)
{
	currentType = nullptr;

	return 0;
}