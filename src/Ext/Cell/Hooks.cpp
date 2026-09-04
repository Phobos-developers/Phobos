#include "Body.h"

#include <Ext/Rules/Body.h>

DEFINE_HOOK(0x480EA8, CellClass_DamageWall_AdjacentWallDamage, 0x7)
{
	enum{ SkipGameCode = 0x480EB4 };
	GET(CellClass*, pThis, EAX);
	pThis->DamageWall(RulesExt::Global()->AdjacentWallDamage);
	return SkipGameCode;
}

#pragma region LightSource Optimize

namespace LightSourceTemp
{
	std::vector<TerrainClass*> RedrawTerrains {};
}

DEFINE_HOOK(0x5547C8, LightSourceClass_CTOR_SetInCells, 0x5)
{
	GET(LightSourceClass*, pThis, ESI);

	for (CellRangeEnumerator cell(CellClass::Coord2Cell(pThis->Location), pThis->LightVisibility / Unsorted::LeptonsPerCell + 0.5); cell; ++cell)
	{
		const auto pCell = MapClass::Instance.TryGetCellAt(*cell);

		if (!pCell)
			continue;

		const auto pCellExt = CellExt::Fetch(pCell);
		pCellExt->CoveringLights.emplace_back(pThis);
	}

	return 0;
}

DEFINE_HOOK(0x555176, LightSourceClass_DTOR_ResetInCells, 0x6)
{
	GET(LightSourceClass*, pThis, ESI);

	for (CellRangeEnumerator cell(CellClass::Coord2Cell(pThis->Location), pThis->LightVisibility / Unsorted::LeptonsPerCell + 0.5); cell; ++cell)
	{
		const auto pCell = MapClass::Instance.TryGetCellAt(*cell);

		if (!pCell)
			continue;

		const auto pCellExt = CellExt::Fetch(pCell);
		const auto it = std::ranges::find(pCellExt->CoveringLights, pThis);

		if (it != pCellExt->CoveringLights.cend())
			pCellExt->CoveringLights.erase(it);
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x48444C, CellClass_ProcessColourComponents_LightSourceCount, 0x6)
DEFINE_HOOK(0x48427D, CellClass_ProcessColourComponents_LightSourceCount, 0x6)
{
	GET(CellClass*, pThis, EDI);
	const auto pExt = CellExt::Fetch(pThis);

	R->ECX(static_cast<int>(pExt->CoveringLights.size()));
	return R->Origin() + 0x6;
}

DEFINE_HOOK(0x48428B, CellClass_ProcessColourComponents_LightSourceItem, 0x6)
{
	enum { ApplyItem = 0x484294 };

	GET(CellClass*, pThis, EDI);
	GET(int, idx, EAX);
	const auto pExt = CellExt::Fetch(pThis);

	R->ESI(pExt->CoveringLights[idx]);
	return ApplyItem;
}

DEFINE_JUMP(LJMP, 0x4842DC, 0x4842E2) // Skip useless code

DEFINE_HOOK(0x554BF6, LightSourceClass_554AF0_Distance_Optimize, 0x5)
{
	enum { InRange = 0x554C4B, OutOfRange = 0x554CE4 };

	GET(LightSourceClass*, pThis, EDI);
	REF_STACK(CellStruct, cell, STACK_OFFSET(0x30, -0x24));
	const auto cellCoords = CellClass::Cell2Coord(cell);
	const int diffX = pThis->Location.X - cellCoords.X;
	const int diffY = pThis->Location.Y - cellCoords.Y;
	const double distanceSqr = static_cast<double>(diffX) * diffX + static_cast<double>(diffY) * diffY;

	if (static_cast<double>(distanceSqr) > static_cast<double>(pThis->LightVisibility) * pThis->LightVisibility)
		return OutOfRange;

	if (const auto pCell = MapClass::Instance.TryGetCellAt(cell))
	{
		pCell->MarkForRedraw();

		if (const auto pBuilding = pCell->GetBuilding())
			pBuilding->MarkForRedraw();

		auto& redrawTerrains = LightSourceTemp::RedrawTerrains;
		const auto pCellExt = CellExt::Fetch(pCell);

		for (const auto pCoveringTerrain : pCellExt->CoveringTerrains)
		{
			if (std::ranges::find(redrawTerrains, pCoveringTerrain) != redrawTerrains.cend())
				continue;

			redrawTerrains.emplace_back(pCoveringTerrain);
		}
	}

	return InRange;
}

DEFINE_HOOK(0x554D0A, LightSourceClass_554AF0_CellRedraw, 0x7)
{
	enum { SkipGScreenRedraw = 0x554D16 };

	auto& redrawTerrains = LightSourceTemp::RedrawTerrains;

	if (!redrawTerrains.empty())
	{
		for (const auto pTerrain : redrawTerrains)
		{
			RectangleStruct rect;
			pTerrain->GetRenderDimensions(&rect);
			TacticalClass::Instance->RegisterDirtyArea(rect, false);
		}

		redrawTerrains.clear();
	}

	return SkipGScreenRedraw;
}

#pragma endregion
