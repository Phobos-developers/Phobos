#include <IsometricTileTypeClass.h>

#include "Body.h"

#include <Ext/TerrainType/Body.h>
#include <Ext/Building/Body.h>

#pragma region AllowDeployControlledMCV

DEFINE_HOOK_AGAIN(0x443770, TechnoClass_AllowDeployControlledMCV, 0x6)// BuildingClass::CellClickedAction
DEFINE_HOOK_AGAIN(0x443AB0, TechnoClass_AllowDeployControlledMCV, 0x6)// BuildingClass::SetRallyPoint
DEFINE_HOOK_AGAIN(0x44F614, TechnoClass_AllowDeployControlledMCV, 0x6)// BuildingClass::IsControllable
DEFINE_HOOK(0x700ED0, TechnoClass_AllowDeployControlledMCV, 0x6)// UnitClass::CanDeploySlashUnload
{
	return RulesExt::Global()->AllowDeployControlledMCV ? R->Origin() + 0xE : 0;
}

#pragma endregion

DEFINE_HOOK(0x739956, UnitClass_Deploy_Transfer, 0x6)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	TechnoExt::TransferStatus(pUnit, pStructure);

	return 0;
}

DEFINE_HOOK(0x44A03C, BuildingClass_Mi_Selling_Transfer, 0x6)
{
	GET(BuildingClass*, pStructure, EBP);
	GET(UnitClass*, pUnit, EBX);

	TechnoExt::TransferStatus(pStructure, pUnit);

	// This line will break the bahavior of UnDeploysInto buildings. However, it might serve a purpose that no one knows yet
	// Comment out the line instead of removing it for now, so we can turn to it if something related goes wrong in the future
	// pUnit->QueueMission(Mission::Hunt, true);
	return 0;
}

DEFINE_HOOK(0x449E2E, BuildingClass_Mi_Selling_CreateUnit, 0x6)
{
	GET(BuildingClass*, pStructure, EBP);
	R->ECX<HouseClass*>(pStructure->GetOriginalOwner());

	// Remember MC ring animation.
	if (pStructure->IsMindControlled())
	{
		auto const pTechnoExt = TechnoExt::Fetch(pStructure);
		pTechnoExt->UpdateMindControlAnim();
	}

	return 0x449E34;
}

DEFINE_HOOK(0x7000DF, TechnoClass_WhatAction_Deploy, 0x5)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto pInfantry = abstract_cast<InfantryClass*, true>(pThis))
	{
		if (!FootExt::CanDeployIntoBuilding(pInfantry, true))
		{
			R->EAX(Action::NoDeploy);

			return 0x7000E4;
		}
	}

	return 0;
}

DEFINE_HOOK(0x7396AD, UnitClass_Deploy_CreateBuilding, 0x6)
{
	GET(UnitClass*, pUnit, EBP);
	R->EDX<HouseClass*>(pUnit->GetOriginalOwner());

	return 0x7396B3;
}

// Game removes deploying vehicles from map temporarily to check if there's enough
// space to deploy into a building when displaying allow/disallow deploy cursor.
// This can cause desyncs if there are certain types of units around the deploying
// unit because the OccupationFlags may be accidentally cleared, or the order of
// the objects linked list may be scrambled.
#pragma region DeploysIntoDesyncFix

DEFINE_HOOK(0x73FEC1, UnitClass_WhatAction_DeploysIntoDesyncFix, 0x6)
{
	enum { SkipGameCode = 0x73FFDF };

	GET(UnitClass* const, pThis, ESI);
	REF_STACK(Action, action, STACK_OFFSET(0x20, 0x8));

	if (!FootExt::CanDeployIntoBuilding(pThis))
		action = Action::NoDeploy;

	return SkipGameCode;
}

// Exclude the specific unit who want to deploy
// Allow placing buildings on top of TerrainType with CanBeBuiltOn
DEFINE_HOOK(0x47C640, CellClass_CanThisExistHere_IgnoreSomething, 0x6)
{
	enum { CanNotExistHere = 0x47C6D1, CanExistHere = 0x47C6A0 };

	GET(const CellClass* const, pCell, EDI);
	GET(const BuildingTypeClass* const, pBuildingType, EAX);
	GET_STACK(HouseClass* const, pOwner, STACK_OFFSET(0x18, 0xC));

	if (!Game::IsActive)
		return CanExistHere;

	auto isTerrainBuildable = [](TerrainClass* pTerrain) -> bool
	{
		auto const pType = pTerrain->Type;
		auto const pTypeExt = TerrainTypeExt::Fetch(pType);
		return pType->SpawnsTiberium
			? pTypeExt->CanBeBuiltOn.Get(RulesExt::Global()->Tibtree_CanBeBuiltOn)
			: pTypeExt->CanBeBuiltOn.Get(RulesExt::Global()->Terrain_CanBeBuiltOn);
	};

	if (pBuildingType->LaserFence)
	{
		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (pObject->WhatAmI() == AbstractType::Building)
			{
				return CanNotExistHere;
			}
			else if (const auto pTerrain = abstract_cast<TerrainClass*, true>(pObject))
			{
				if (!isTerrainBuildable(pTerrain))
					return CanNotExistHere;
			}
		}
	}
	else if (pBuildingType->LaserFencePost || pBuildingType->Gate)
	{
		bool skipFlag = FootExt::Deployer ? FootExt::Deployer->CurrentMapCoords == pCell->MapCoords : false;
		bool builtOnCanBeBuiltOn = false;

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (const auto pTerrain = abstract_cast<TerrainClass*, true>(pObject))
			{
				if (!isTerrainBuildable(pTerrain))
					return CanNotExistHere;

				builtOnCanBeBuiltOn = true;
			}
			else if (pObject->AbstractFlags & AbstractFlags::Techno)
			{
				if (pObject == FootExt::Deployer)
				{
					skipFlag = true;
				}
				else
				{
					const auto pBuilding = abstract_cast<BuildingClass*, true>(pObject);

					if (!pBuilding || pOwner != pBuilding->Owner || !pBuilding->Type->LaserFence)
						return CanNotExistHere;
				}
			}
		}

		if (!builtOnCanBeBuiltOn && (pCell->OccupationFlags & (skipFlag ? 0x1F : 0x3F)))
			return CanNotExistHere;
	}
	else if (pBuildingType->ToTile)
	{
		const auto isoTileTypeIndex = pCell->IsoTileTypeIndex;

		if (isoTileTypeIndex >= 0 && isoTileTypeIndex < IsometricTileTypeClass::Array.Count
			&& !IsometricTileTypeClass::Array.Items[isoTileTypeIndex]->Morphable)
		{
			return CanNotExistHere;
		}

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (pObject->WhatAmI() == AbstractType::Building)
				return CanNotExistHere;
		}
	}
	else
	{
		bool skipFlag = FootExt::Deployer ? FootExt::Deployer->CurrentMapCoords == pCell->MapCoords : false;
		bool builtOnCanBeBuiltOn = false;

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (pObject->AbstractFlags & AbstractFlags::Techno)
			{
				if (pObject == FootExt::Deployer)
					skipFlag = true;
				else
					return CanNotExistHere;
			}
			else if (const auto pTerrain = abstract_cast<TerrainClass*, true>(pObject))
			{
				if (!isTerrainBuildable(pTerrain))
					return CanNotExistHere;

				builtOnCanBeBuiltOn = true;
			}
		}

		if (!builtOnCanBeBuiltOn && (pCell->OccupationFlags & (skipFlag ? 0x02 : 0x3F)))
			return CanNotExistHere;
	}

	return CanExistHere; // Continue check the overlays .etc
}


DEFINE_HOOK(0x7396D2, UnitClass_TryToDeploy_Transfer, 0x5)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	if (pUnit->Type->DeployToFire && pUnit->Target)
		pStructure->LastTarget = pUnit->Target;

	const auto pStructureExt = BuildingExt::Fetch(pStructure);
	pStructureExt->DeployedTechno = true;

	return 0;
}

#pragma endregion
