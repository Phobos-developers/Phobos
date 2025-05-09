#include "Body.h"

#include <HouseClass.h>
#include <OverlayClass.h>
#include <TerrainClass.h>

#include <Utilities/GeneralUtils.h>

constexpr bool IS_CELL_OCCUPIED(CellClass* pCell)
{
	return pCell->OccupationFlags & 0x20 || pCell->OccupationFlags & 0x40 || pCell->OccupationFlags & 0x80 || pCell->GetInfantry(false);
}

// Passable TerrainTypes Hook #1 - Do not set occupy bits.
DEFINE_HOOK(0x71C110, TerrainClass_SetOccupyBit_PassableTerrain, 0x6)
{
	enum { Skip = 0x71C1A0 };

	GET(TerrainClass*, pThis, ECX);

	if (TerrainTypeExt::ExtMap.Find(pThis->Type)->IsPassable)
		return Skip;

	return 0;
}

// Passable TerrainTypes Hook #2 - Do not display attack cursor unless force-firing.
DEFINE_HOOK(0x7002E9, TechnoClass_WhatAction_PassableTerrain, 0x5)
{
	enum { ReturnAction = 0x70020E };

	GET(TechnoClass*, pThis, ESI);
	GET(ObjectClass*, pTarget, EDI);
	GET_STACK(bool, isForceFire, STACK_OFFSET(0x1C, 0x8));

	if (!pThis->Owner->IsControlledByCurrentPlayer() || !pThis->IsControllable())
		return 0;

	if (auto const pTerrain = abstract_cast<TerrainClass*>(pTarget))
	{
		if (TerrainTypeExt::ExtMap.Find(pTerrain->Type)->IsPassable && !isForceFire)
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

	if (TerrainTypeExt::ExtMap.Find(pTerrain->Type)->IsPassable)
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
		if (TerrainTypeExt::ExtMap.Find(pTerrain->Type)->IsPassable)
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

	if (pObject->WhatAmI() == AbstractType::Building)
	{
		GET(CellClass*, pCell, EAX);

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

	if (!Game::IsActive)
		return Unlimbo;

	GET(CellClass* const, pCell, EAX);

	if (auto const pTerrain = pCell->GetTerrain(false))
	{
		if (!TerrainTypeExt::ExtMap.Find(pTerrain->Type)->CanBeBuiltOn)
			return NoUnlimbo;

		pCell->RemoveContent(pTerrain, false);
		TerrainTypeExt::Remove(pTerrain);
	}

	return Unlimbo;
}
