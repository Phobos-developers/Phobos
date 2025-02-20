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
DEFINE_HOOK_AGAIN(0x47C80E, CellClass_IsClearTo_Build_BuildableTerrain, 0x5)
DEFINE_HOOK(0x47C745, CellClass_IsClearTo_Build_BuildableTerrain, 0x5)
{
	enum { Skip = 0x47C85F, SkipFlags = 0x47C6A0 };

	GET(CellClass*, pThis, EDI);

	auto pTerrain = pThis->GetTerrain(false);

	if (pTerrain)
	{
		if (auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type))
		{
			if (pTypeExt->CanBeBuiltOn)
			{
				if (IS_CELL_OCCUPIED(pThis))
					return Skip;
				else
					return SkipFlags;
			}
		}
	}

	return 0;
}

// Buildable-upon TerrainTypes Hook #2 - Allow placing laser fences on top of them.
DEFINE_HOOK(0x47C657, CellClass_IsClearTo_Build_BuildableTerrain_LF, 0x6)
{
	enum { Skip = 0x47C6A0, Return = 0x47C6D1 };

	GET(CellClass*, pThis, EDI);

	auto pObj = pThis->FirstObject;

	if (pObj)
	{
		bool isEligible = true;

		while (true)
		{
			isEligible = pObj->WhatAmI() != AbstractType::Building;

			if (auto const pTerrain = abstract_cast<TerrainClass*>(pObj))
			{
				isEligible = false;
				auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

				if (pTypeExt->CanBeBuiltOn)
					isEligible = true;
			}

			if (!isEligible)
				break;

			pObj = pObj->NextObject;

			if (!pObj)
				return Skip;
		}

		return Return;
	}

	return Skip;
}


// Buildable-upon TerrainTypes Hook #3 - Draw laser fence placement even if they are on the way.
DEFINE_HOOK(0x6D57C1, TacticalClass_DrawLaserFencePlacement_BuildableTerrain, 0x9)
{
	enum { ContinueChecks = 0x6D57D2, DontDraw = 0x6D59A6 };

	GET(CellClass*, pCell, ESI);

	if (auto const pTerrain = pCell->GetTerrain(false))
	{
		auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

		if (pTypeExt->CanBeBuiltOn)
			return ContinueChecks;

		return DontDraw;
	}

	return ContinueChecks;
}

// Buildable-upon TerrainTypes Hook #4 - Remove them when buildings are placed on them.
DEFINE_HOOK(0x5684B1, MapClass_PlaceDown_BuildableTerrain, 0x6)
{
	GET(ObjectClass*, pObject, EDI);
	GET(CellClass*, pCell, EAX);

	if (pObject->WhatAmI() == AbstractType::Building)
	{
		if (auto const pTerrain = pCell->GetTerrain(false))
		{
			auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

			if (pTypeExt->CanBeBuiltOn)
			{
				pCell->RemoveContent(pTerrain, false);
				TerrainTypeExt::Remove(pTerrain);
			}
		}
	}

	return 0;
}
