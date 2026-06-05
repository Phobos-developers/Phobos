#include "Body.h"


// Passable TerrainTypes Hook #1 - Do not set occupy bits.
DEFINE_HOOK(0x71C110, TerrainClass_SetOccupyBit_PassableTerrain, 0x6)
{
	enum { Skip = 0x71C1A0 };

	GET(TerrainClass*, pThis, ECX);

	auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->IsPassable)
		return Skip;

	return 0;
}

// Passable TerrainTypes Hook #2 - Do not display attack cursor unless force-firing.
DEFINE_HOOK(0x7002E9, TechnoClass_WhatAction_PassableTerrain, 0x5)
{
	enum { ReturnAction = 0x70020E };

	GET(TechnoClass*, pThis, ESI);
	GET(ObjectClass*, pTarget, EDI);
	GET_STACK(const bool, isForceFire, STACK_OFFSET(0x1C, 0x8));

	if (!pThis->Owner->IsControlledByCurrentPlayer() || !pThis->IsControllable())
		return 0;

	if (const auto pTerrain = abstract_cast<TerrainClass*, true>(pTarget))
	{
		if (!isForceFire && TerrainTypeExt::ExtMap.Find(pTerrain->Type)->IsPassable)
		{
			R->EBP(Action::Move);
			return ReturnAction;
		}
	}

	return 0;
}

// Passable TerrainTypes Hook #3 - Count passable TerrainTypes as completely passable.
DEFINE_HOOK(0x483DDF, CellClass_CheckPassability_PassableTerrain, 0x6)
{
	enum { ReturnFromFunction = 0x483E25 };

	GET(CellClass*, pThis, EDI);
	GET(TerrainClass*, pTerrain, ESI);

	auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

	if (pTypeExt->IsPassable)
	{
		pThis->Passability = PassabilityType::Passable;
		return ReturnFromFunction;
	}

	return 0;
}

// Passable TerrainTypes Hook #4 - Make passable for vehicles.
DEFINE_HOOK(0x73FB71, UnitClass_CanEnterCell_PassableTerrain, 0x6)
{
	enum { SkipTerrainChecks = 0x73FA7C };

	GET(AbstractClass*, pTarget, ESI);

	if (auto const pTerrain = abstract_cast<TerrainClass*>(pTarget))
	{
		auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

		if (pTypeExt->IsPassable)
			return SkipTerrainChecks;
	}

	return 0;
}

// Buildable-upon TerrainTypes Hook #1 - Allow placing buildings on top of them.
// DEFINE_HOOK(0x73FEC1, UnitClass_WhatAction_DeploysIntoDesyncFix, 0x6) in Hooks.DeploysInto.cpp

// Buildable-upon TerrainTypes Hook #2 - Draw laser fence placement even if they are on the way.
DEFINE_HOOK(0x6D57C1, TacticalClass_DrawLaserFencePlacement_BuildableTerrain, 0x9)
{
	enum { ContinueChecks = 0x6D57D2, DontDraw = 0x6D59A6 };

	GET(CellClass*, pCell, ESI);

	if (auto const pTerrain = pCell->GetTerrain(false))
		return TerrainTypeExt::ExtMap.Find(pTerrain->Type)->CanBeBuiltOn ? ContinueChecks : DontDraw;

	return ContinueChecks;
}

// Buildable-upon TerrainTypes Hook #3 - Remove them when buildings are placed on them.
DEFINE_HOOK(0x5684B1, MapClass_PlaceDown_BuildableTerrain, 0x6)
{
	GET(ObjectClass*, pObject, EDI);
	GET(CellClass*, pCell, EAX);

	if (pObject->WhatAmI() == AbstractType::Building)
	{
		if (auto const pTerrain = pCell->GetTerrain(false))
		{
			if (TerrainTypeExt::ExtMap.Find(pTerrain->Type)->CanBeBuiltOn)
			{
				pCell->RemoveContent(pTerrain, false);
				TerrainTypeExt::Remove(pTerrain);
			}
		}
	}

	return 0;
}

// Buildable-upon TerrainTypes Hook #4 -> Allow placing walls on top of terrain
DEFINE_HOOK(0x5FD2B6, OverlayClass_Unlimbo_SkipTerrainCheck, 0x9)
{
	enum { Unlimbo = 0x5FD2CA, NoUnlimbo = 0x5FD2C3 };

	GET(CellClass* const, pCell, EAX);

	if (!Game::IsActive)
		return Unlimbo;

	if (auto const pTerrain = pCell->GetTerrain(false))
	{
		if (!TerrainTypeExt::ExtMap.Find(pTerrain->Type)->CanBeBuiltOn)
			return NoUnlimbo;

		pCell->RemoveContent(pTerrain, false);
		TerrainTypeExt::Remove(pTerrain);
	}

	return Unlimbo;
}

// Buildable-upon TerrainTypes Hook #5 -> Ignore when flushing building foundations for placement.
DEFINE_HOOK(0x45EF3A, BuildingTypeClass_FlushForPlacement_BuildableTerrain, 0x7)
{
	enum { Disallow = 0x45F00B, Continue = 0x45EF4A };

	GET(ObjectClass* const, pObject, ESI);

	if (auto const pTerrain = abstract_cast<TerrainClass*>(pObject))
	{
		if (!TerrainTypeExt::ExtMap.Find(pTerrain->Type)->CanBeBuiltOn)
			return Disallow;
	}

	return Continue;
}

#pragma region FindBuildLocation

namespace FindBuildLocationTemp
{
	bool EvaluatingBuildLocation = false;
}

// Set the global flag when calling this from evaluating building locations for AI.
static bool __fastcall MapClass_IsAreaFree_Wrapper(MapClass* pThis, void* _, RectangleStruct* pRect, int houseID)
{
	FindBuildLocationTemp::EvaluatingBuildLocation = true;
	bool result = pThis->IsAreaFree(pRect, houseID);
	FindBuildLocationTemp::EvaluatingBuildLocation = false;
	return result;
}

DEFINE_FUNCTION_JUMP(CALL, 0x5069DB, MapClass_IsAreaFree_Wrapper);

// Ignore buildable terrain when evaluating building locations for AI. Replaces the vanilla function.
DEFINE_HOOK(0x586780, MapClass_IsAreaFree, 0x7)
{
	enum { ReturnFromFunction = 0x586887 };

	GET(MapClass*, pThis, ECX);
	GET_STACK(RectangleStruct*, pRect, 0x4);
	GET_STACK(int, houseID, 0x8);

	int mask = houseID >= 0 ? 1 << houseID : 0;

	for (int x = pRect->X; x < pRect->X + pRect->Width; x++)
	{
		for (int y = pRect->Y; y < pRect->Y + pRect->Height; y++)
		{
			CellClass* pCell = pThis->GetCellAt(CellStruct { static_cast<short>(x), static_cast<short>(y) });
			auto const pTerrain = pCell->GetTerrain(false);
			bool altPassability = false;

			if (pTerrain)
			{
				if (!FindBuildLocationTemp::EvaluatingBuildLocation || !TerrainTypeExt::ExtMap.Find(pTerrain->Type)->CanBeBuiltOn)
				{
					R->EAX(false);
					return ReturnFromFunction;
				}

				altPassability = true;
			}

			// If we're evaluating a cell with buildable TerrainType on it, passability check needs some alterations.
			const bool invalidPassability = altPassability
				? (pCell->Passability != PassabilityType::Passable && pCell->Passability != PassabilityType::HasFreeSpots)
				: (pCell->Passability != PassabilityType::Passable);

			if ((pCell->BaseSpacerOfHouses & mask) != 0
				|| pCell->OverlayTypeIndex != -1
				|| invalidPassability
				|| pCell->SlopeIndex
				|| pCell->GetBuilding())
			{
				R->EAX(false);
				return ReturnFromFunction;
			}
		}
	}

	R->EAX(pThis->InLocalRadar(pRect, true));
	return ReturnFromFunction;
}

#pragma endregion
