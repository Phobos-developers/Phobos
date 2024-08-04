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

	if (auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pThis->Type))
	{
		if (pTypeExt->IsPassable)
			return Skip;
	}

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

	if (pTarget->WhatAmI() == AbstractType::Terrain)
	{
		if (auto const pTypeExt = TerrainTypeExt::ExtMap.Find((abstract_cast<TerrainClass*>(pTarget))->Type))
		{
			if (pTypeExt->IsPassable && !isForceFire)
			{
				R->EBP(Action::Move);
				return ReturnAction;
			}
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

	if (auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type))
	{
		if (pTypeExt->IsPassable)
		{
			pThis->Passability = PassabilityType::Passable;
			return ReturnFromFunction;
		}
	}

	return 0;
}

// Passable TerrainTypes Hook #4 - Make passable for vehicles.
DEFINE_HOOK(0x73FB71, UnitClass_CanEnterCell_PassableTerrain, 0x6)
{
	enum { ReturnPassable = 0x73FD37, SkipTerrainChecks = 0x73FA7C };

	GET(AbstractClass*, pTarget, ESI);

	if (auto const pTerrain = abstract_cast<TerrainClass*>(pTarget))
	{
		auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

		if (pTypeExt->IsPassable)
			return SkipTerrainChecks;
	}

	return 0;
}
