#include "Body.h"

#include <EventClass.h>
#include <IsometricTileTypeClass.h>

#include <Ext/House/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Unit/Body.h>
#include <Ext/TerrainType/Body.h>

// AIConstructionYard Hook #1 -> sub_740810 - Check number of construction yard before deploy.
DEFINE_HOOK(0x740A11, UnitClass_Mission_Guard_AIAutoDeployMCV, 0x6)
{
	enum { SkipGameCode = 0x740A50 };

	GET(UnitClass*, pMCV, ESI);

	return (!RulesExt::Global()->AIAutoDeployMCV && pMCV->Owner->NumConYards > 0) ? SkipGameCode : 0;
}

// AIConstructionYard Hook #2 -> sub_7393C0 - Skip useless base center setting.
DEFINE_HOOK(0x739889, UnitClass_TryToDeploy_AISetBaseCenter, 0x6)
{
	enum { SkipGameCode = 0x73992B };

	GET(UnitClass*, pMCV, EBP);

	return (!RulesExt::Global()->AISetBaseCenter && pMCV->Owner->NumConYards > 1) ? SkipGameCode : 0;
}

// AIConstructionYard Hook #3 -> sub_4FD500 - Update better base center.
DEFINE_HOOK(0x4FD538, HouseClass_AIHouseUpdate_CheckAIBaseCenter, 0x7)
{
	if (RulesExt::Global()->AIBiasSpawnCell && !SessionClass::IsCampaign())
	{
		GET(HouseClass*, pAI, EBX);

		if (pAI->ConYards.Count > 1)
		{
			const int wayPoint = pAI->GetSpawnPosition();

			if (wayPoint != -1)
			{
				const auto center = ScenarioClass::Instance->GetWaypointCoords(wayPoint);
				auto newCenter = center;
				double distanceSquared = 131072.0;

				for (int i = 0; i < pAI->ConYards.Count; ++i)
				{
					if (const auto pBuilding = pAI->ConYards.GetItem(i))
					{
						if (pBuilding->IsAlive && pBuilding->Health > 0 && !pBuilding->InLimbo)
						{
							const double newDistanceSquared = pBuilding->GetMapCoords().DistanceFromSquared(center);

							if (newDistanceSquared < distanceSquared)
							{
								distanceSquared = newDistanceSquared;
								newCenter = pBuilding->GetMapCoords();
							}
						}
					}
				}

				if (newCenter != center)
				{
					pAI->BaseSpawnCell = newCenter;
					pAI->Base.Center = newCenter;
				}
			}
		}
	}

	return 0;
}

// AIConstructionYard Hook #4-2 -> sub_42EB50 - Prohibit AI from building construction yard.
DEFINE_HOOK(0x42EB8E, BaseClass_GetBaseNodeIndex_CheckValidBaseNode, 0x6)
{
	enum { Valid = 0x42EBC3, Invalid = 0x42EBAE };

	GET(BaseClass* const, pBase, ESI);
	GET(BaseNodeClass* const, pBaseNode, EAX);

	if (RulesExt::Global()->AIForbidConYard && pBaseNode->Placed)
	{
		const auto index = pBaseNode->BuildingTypeIndex;

		if (index >= 0 && index < BuildingTypeClass::Array.Count && BuildingTypeClass::Array.Items[index]->ConstructionYard)
			return Invalid;
	}

	return reinterpret_cast<bool(__thiscall*)(HouseClass*, BaseNodeClass*)>(0x50CAD0)(pBase->Owner, pBaseNode) ? Valid : Invalid;
}

// AIConstructionYard Hook #5-1 -> sub_588570 - Only expand walls on nodes.
DEFINE_HOOK(0x5885D1, MapClass_BuildingToFirestormWall_SkipExtraWalls, 0x6)
{
	enum { NextDirection = 0x588730 };

	GET_STACK(const HouseClass* const, pHouse, STACK_OFFSET(0x38, 0x8));
	GET_STACK(const int, count, STACK_OFFSET(0x38, -0x24));

	if (pHouse->IsControlledByHuman() || !RulesExt::Global()->AINodeWallsOnly || count)
		return 0;

	GET(const CellStruct, cell, EBX);
	GET(const BuildingTypeClass* const, pType, EBP);

	const auto index = pType->ArrayIndex;
	const auto& nodes = pHouse->Base.BaseNodes;

	for (const auto& pNode : nodes)
	{
		if (pNode.MapCoords == cell && pNode.BuildingTypeIndex == index)
			return 0;
	}

	return NextDirection;
}

// AIConstructionYard Hook #5-2 -> sub_588750 - Only expand walls on nodes.
DEFINE_HOOK(0x5887C1, MapClass_BuildingToWall_SkipExtraWalls, 0x6)
{
	enum { NextDirection = 0x588935 };

	GET_STACK(const HouseClass* const, pHouse, STACK_OFFSET(0x3C, 0x8));
	GET_STACK(const int, count, STACK_OFFSET(0x3C, -0x2C));

	if (pHouse->IsControlledByHuman() || !RulesExt::Global()->AINodeWallsOnly || count)
		return 0;

	GET(const CellStruct, cell, EDX);
	GET(const BuildingTypeClass* const, pType, EDI);

	const auto index = pType->ArrayIndex;
	const auto& nodes = pHouse->Base.BaseNodes;

	for (const auto& pNode : nodes)
	{
		if (pNode.MapCoords == cell && pNode.BuildingTypeIndex == index)
			return 0;
	}

	return NextDirection;
}

// Let the game do the proximity and shroud check when the cell which mouse is pointing at has not changed
DEFINE_JUMP(LJMP, 0x4AACD9, 0x4AACF5);
DEFINE_JUMP(LJMP, 0x4A9361, 0x4A9371);

static inline bool IsSameFenceType(const BuildingTypeClass* const pPostType, const BuildingTypeClass* const pFenceType)
{
	const auto count = BuildingTypeClass::Array.Count;

	for (int i = 0; i < count; ++i)
	{
		const auto pSearchType = BuildingTypeClass::Array.Items[i];

		if (pSearchType->LaserFence)
		{
			if (pSearchType != pFenceType)
				return false;

			break;
		}
	}

	return true;
}

static inline bool CheckCanNotExistHere(FootClass* const pTechno, HouseClass* const pOwner, bool expand, bool& skipFlag, bool& landFootOnly)
{
	auto const pParent = pTechno ? TechnoExt::GetTopLevelParent(pTechno) : nullptr;
	if (pTechno == UnitExt::Deployer
		|| (pTechno && UnitExt::Deployer && TechnoExt::IsChildOf(pTechno, UnitExt::Deployer))
		|| (pTechno && TechnoExt::DoesntOccupyCellAsChild(pTechno))
		|| (pParent && UnitExt::Deployer && pParent == UnitExt::Deployer))
	{
		skipFlag = true;
		return false;
	}

	if (!expand || pTechno->GetTechnoType()->Speed <= 0 || !BuildingTypeExt::CheckOccupierCanLeave(pOwner, pTechno->Owner))
		return true;
	else
		landFootOnly = true;

	return false;
}

// Buildable-upon TechnoTypes Helper
namespace ProximityTemp
{
	bool Exist = false;
	bool Mouse = false;
}

// Buildable-upon TerrainTypes Hook #1 -> sub_47C620 - Allow placing buildings on top of them
// Buildable-upon TechnoTypes Hook #1 -> sub_47C620 - Rewrite and check whether allow placing buildings on top of them
DEFINE_HOOK(0x47C640, CellClass_CanThisExistHere_IgnoreSomething, 0x6)
{
	enum { CanNotExistHere = 0x47C6D1, CanExistHere = 0x47C6A0 };

	GET(const CellClass* const, pCell, EDI);
	GET(const BuildingTypeClass* const, pBuildingType, EAX);
	GET_STACK(HouseClass* const, pOwner, STACK_OFFSET(0x18, 0xC));

	ProximityTemp::Exist = false;

	if (!Game::IsActive)
		return CanExistHere;

	const bool expand = RulesExt::Global()->ExtendedBuildingPlacing.Get();
	bool landFootOnly = false;

	if (pBuildingType->LaserFence)
	{
		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			switch (pObject->WhatAmI())
			{
			case AbstractType::Building:
			{
				return CanNotExistHere;
			}
			case AbstractType::Terrain:
			{
				auto const pTerrain = abstract_cast<TerrainClass*>(pObject);
				auto const pType = pTerrain->Type;
				auto const pTypeExt = TerrainTypeExt::Fetch(pType);
				bool const canBuild = pTypeExt->CanBeBuiltOn.Get(pType->SpawnsTiberium ? RulesExt::Global()->Tibtree_CanBeBuiltOn : RulesExt::Global()->Terrain_CanBeBuiltOn);

				if (!canBuild)
					return CanNotExistHere;

				break;
			}
			default:
			{
				break;
			}
			}
		}
	}
	else if (pBuildingType->LaserFencePost || pBuildingType->Gate)
	{
		bool skipFlag = UnitExt::Deployer ? UnitExt::Deployer->CurrentMapCoords == pCell->MapCoords : false;
		bool builtOnCanBeBuiltOn = false;

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			switch (pObject->WhatAmI())
			{
			case AbstractType::Aircraft:
			{
				return CanNotExistHere;
			}
			case AbstractType::Building:
			{
				const auto pBuilding = static_cast<BuildingClass*>(pObject);
				const auto pType = pBuilding->Type;

				if (pOwner != pBuilding->Owner || !pType->LaserFence)
					return CanNotExistHere;
				else if (pBuildingType->LaserFencePost && !IsSameFenceType(pBuildingType, pType))
					return CanNotExistHere;

				break;
			}
			case AbstractType::Infantry:
			case AbstractType::Unit:
			{
				if (CheckCanNotExistHere(static_cast<FootClass*>(pObject), pOwner, expand, skipFlag, landFootOnly))
					return CanNotExistHere;

				break;
			}
			case AbstractType::Terrain:
			{
				auto const pTerrain = abstract_cast<TerrainClass*>(pObject);
				auto const pType = pTerrain->Type;
				auto const pTypeExt = TerrainTypeExt::Fetch(pType);
				bool const canBuild = pTypeExt->CanBeBuiltOn.Get(pType->SpawnsTiberium ? RulesExt::Global()->Tibtree_CanBeBuiltOn : RulesExt::Global()->Terrain_CanBeBuiltOn);

				if (!canBuild)
					return CanNotExistHere;

				builtOnCanBeBuiltOn = true;
				break;
			}
			default:
			{
				break;
			}
			}
		}

		if (!landFootOnly && !builtOnCanBeBuiltOn && (pCell->OccupationFlags & (skipFlag ? 0x1F : 0x3F)))
		{
			if (expand)
				landFootOnly = true;
			else
				return CanNotExistHere;
		}
	}
	else if (pBuildingType->ToTile)
	{
		const auto isoTileTypeIndex = pCell->IsoTileTypeIndex;

		if (isoTileTypeIndex >= 0 && isoTileTypeIndex < IsometricTileTypeClass::Array.Count && !IsometricTileTypeClass::Array.Items[isoTileTypeIndex]->Morphable)
			return CanNotExistHere;

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			if (const auto pBuilding = abstract_cast<BuildingClass*, true>(pObject))
				return CanNotExistHere;
		}
	}
	else
	{
		bool skipFlag = UnitExt::Deployer ? UnitExt::Deployer->CurrentMapCoords == pCell->MapCoords : false;
		bool builtOnCanBeBuiltOn = false;

		for (auto pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			switch (pObject->WhatAmI())
			{
			case AbstractType::Aircraft:
			case AbstractType::Building:
			{
				return CanNotExistHere;
			}

			case AbstractType::Infantry:
			case AbstractType::Unit:
			{
				if (CheckCanNotExistHere(static_cast<FootClass*>(pObject), pOwner, expand, skipFlag, landFootOnly))
					return CanNotExistHere;

				break;
			}

			case AbstractType::Terrain:
			{
				auto const pTerrain = abstract_cast<TerrainClass*>(pObject);
				auto const pType = pTerrain->Type;
				auto const pTypeExt = TerrainTypeExt::Fetch(pType);
				bool const canBuild = pTypeExt->CanBeBuiltOn.Get(pType->SpawnsTiberium ? RulesExt::Global()->Tibtree_CanBeBuiltOn : RulesExt::Global()->Terrain_CanBeBuiltOn);

				if (!canBuild)
					return CanNotExistHere;

				builtOnCanBeBuiltOn = true;
				break;
			}

			default:
			{
				break;
			}
			}
		}

		if (!landFootOnly && !builtOnCanBeBuiltOn && (pCell->OccupationFlags & (skipFlag ? 0x1F : 0x3F)))
		{
			if (expand)
				landFootOnly = true;
			else
				return CanNotExistHere;
		}
	}

	if (landFootOnly)
		ProximityTemp::Exist = true;

	return CanExistHere; // Continue check the overlays .etc
}

// Buildable-upon TerrainTypes Hook #2 - Draw laser fence placement even if they are on the way.
DEFINE_HOOK(0x6D57C1, TacticalClass_DrawLaserFencePlacement_BuildableTerrain, 0x9)
{
	enum { ContinueChecks = 0x6D57D2, DontDraw = 0x6D59A6 };

	GET(CellClass*, pCell, ESI);

	if (auto const pTerrain = pCell->GetTerrain(false))
	{
		auto const pType = pTerrain->Type;
		auto const pTypeExt = TerrainTypeExt::Fetch(pType);
		bool const canBuild = pTypeExt->CanBeBuiltOn.Get(pType->SpawnsTiberium ? RulesExt::Global()->Tibtree_CanBeBuiltOn : RulesExt::Global()->Terrain_CanBeBuiltOn);

		return canBuild ? ContinueChecks : DontDraw;
	}

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
			auto const pType = pTerrain->Type;
			auto const pTypeExt = TerrainTypeExt::Fetch(pType);
			bool const canBuild = pTypeExt->CanBeBuiltOn.Get(pType->SpawnsTiberium ? RulesExt::Global()->Tibtree_CanBeBuiltOn : RulesExt::Global()->Terrain_CanBeBuiltOn);

			if (canBuild)
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
		auto const pType = pTerrain->Type;
		auto const pTypeExt = TerrainTypeExt::Fetch(pType);
		bool const canBuild = pTypeExt->CanBeBuiltOn.Get(pType->SpawnsTiberium ? RulesExt::Global()->Tibtree_CanBeBuiltOn : RulesExt::Global()->Terrain_CanBeBuiltOn);

		if (!canBuild)
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
		auto const pType = pTerrain->Type;
		auto const pTypeExt = TerrainTypeExt::Fetch(pType);
		bool const canBuild = pTypeExt->CanBeBuiltOn.Get(pType->SpawnsTiberium ? RulesExt::Global()->Tibtree_CanBeBuiltOn : RulesExt::Global()->Terrain_CanBeBuiltOn);

		if (!canBuild)
			return Disallow;
	}

	return Continue;
}

static inline void ClearPlacingBuildingData(PlacingBuildingStruct* const pPlace)
{
	pPlace->Type = nullptr;
	pPlace->DrawType = nullptr;
	pPlace->Times = 0;
	pPlace->Timer.Stop();
	pPlace->TopLeft = CellStruct::Empty;
	pPlace->PlaceType = 0;
}

static inline void ClearCurrentBuildingData(DisplayClass* const pDisplay)
{
	pDisplay->SetActiveFoundation(nullptr);
	pDisplay->CurrentBuilding = nullptr;
	pDisplay->CurrentBuildingType = nullptr;
	pDisplay->CurrentBuildingOwnerArrayIndexCopy = -1;

	if (!Unsorted::ArmageddonMode)
	{
		reinterpret_cast<void(__thiscall*)(DisplayClass*, CellStruct*)>(0x4A8D50)(pDisplay, nullptr); // Clear CurrentFoundationCopy_Data
		pDisplay->CurrentBuildingCopy = nullptr;
		pDisplay->CurrentBuildingTypeCopy = nullptr;
	}
}

template <bool slam = false>
static inline void PlayConstructionYardAnim(BuildingClass* const pFactory)
{
	const auto pFactoryType = pFactory->Type;

	if (pFactoryType->ConstructionYard)
	{
		if constexpr (slam)
			VocClass::PlayGlobal(RulesClass::Instance->BuildingSlam, 0x2000, 1.0);

		pFactory->DestroyNthAnim(BuildingAnimSlot::PreProduction);
		pFactory->DestroyNthAnim(BuildingAnimSlot::Idle);

		const bool damaged = pFactory->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
		const auto pAnimName = damaged ? pFactoryType->BuildingAnim[8].Damaged : pFactoryType->BuildingAnim[8].Anim;

		if (pAnimName && *pAnimName)
			pFactory->PlayAnim(pAnimName, BuildingAnimSlot::Production, damaged, false);
	}
}

static inline bool CheckBuildingFoundation(BuildingTypeClass* const pBuildingType, const CellStruct topLeftCell, HouseClass* const pHouse, bool& noOccupy)
{
	for (auto pFoundation = pBuildingType->GetFoundationData(true); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
	{
		if (const auto pCell = MapClass::Instance.TryGetCellAt(topLeftCell + *pFoundation))
		{
			if (!pCell->CanThisExistHere(pBuildingType->SpeedType, pBuildingType, pHouse))
				return false;
			else if (ProximityTemp::Exist)
				noOccupy = false;
		}
	}

	ProximityTemp::Exist = false;
	return true;
}

// Buildable-upon TechnoTypes Hook #2-1 -> sub_47EC90 - Record cell before draw it then skip vanilla AltFlags check
DEFINE_HOOK(0x47EEBC, CellClass_DrawPlaceGrid_RecordCell, 0x6)
{
	enum { DontDrawAlt = 0x47EF1A, DrawVanillaAlt = 0x47EED6 };

	GET(CellClass* const, pCell, ESI);
	GET(const bool, zero, EDX);

	const BlitterFlags flags = BlitterFlags::Centered | BlitterFlags::bf_400;

	if (!(pCell->AltFlags & AltCellFlags::ContainsBuilding))
	{
		if (!RulesExt::Global()->ExtendedBuildingPlacing)
		{
			R->EDX<BlitterFlags>(flags | (zero ? BlitterFlags::Zero : BlitterFlags::Nonzero));
			return DrawVanillaAlt;
		}
		else if (BuildingTypeClass* const pType = abstract_cast<BuildingTypeClass*>(DisplayClass::Instance.CurrentBuildingTypeCopy))
		{
			R->Stack<bool>(STACK_OFFSET(0x30, -0x1D), pCell->CanThisExistHere(pType->SpeedType, pType, HouseClass::CurrentPlayer));
			R->EDX<BlitterFlags>(flags | BlitterFlags::TransLucent75);
			return DontDrawAlt;
		}
	}

	R->EDX<BlitterFlags>(flags | (zero ? BlitterFlags::Zero : BlitterFlags::Nonzero));
	return DontDrawAlt;
}

// Buildable-upon TechnoTypes Hook #3 -> sub_4FB0E0 - Hang up place event if there is only infantries and units on the cell
DEFINE_HOOK(0x4FB1EA, HouseClass_UnitFromFactory_HangUpPlaceEvent, 0x5)
{
	enum { CanBuild = 0x4FB23C, TemporarilyCanNotBuild = 0x4FB5BA, CanNotBuild = 0x4FB35F, BuildSucceeded = 0x4FB649 };

	GET(HouseClass* const, pHouse, EBP);
	GET(TechnoClass* const, pTechno, ESI);
	GET(FactoryClass* const, pPrimary, EBX);
	GET(BuildingClass* const, pFactory, EDI);
	GET_STACK(const CellStruct, topLeftCell, STACK_OFFSET(0x3C, 0x10));

	if (pTechno->WhatAmI() != AbstractType::Building)
	{
		pFactory->SendCommand(RadioCommand::RequestLink, pTechno);

		if (pTechno->Unlimbo(CoordStruct { (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
			return CanBuild;

		ProximityTemp::Mouse = true;
		return CanNotBuild;
	}

	const auto pDisplay = &DisplayClass::Instance;
	auto pBuilding = static_cast<BuildingClass*>(pTechno);
	auto pBuildingType = pBuilding->Type;
	const auto pBufferBuilding = pBuilding;
	const auto pBufferType = pBuildingType;
	const bool upgrade = pBuildingType->PowersUpBuilding[0];
	const bool expand = RulesExt::Global()->ExtendedBuildingPlacing && !pBuildingType->PlaceAnywhere && !upgrade;
	bool revert = upgrade;

	if (expand)
	{
		bool noOccupy = true;
		bool canBuild = CheckBuildingFoundation(pBuildingType, topLeftCell, pHouse, noOccupy);
		const auto pHouseExt = HouseExt::Fetch(pHouse);
		auto& place = pBufferType->BuildCat != BuildCat::Combat ? pHouseExt->Common : pHouseExt->Combat;

		do
		{
			if (canBuild)
			{
				if (noOccupy)
					break; // Can Build

				do
				{
					if (topLeftCell != place.TopLeft || pBufferType != place.Type) // New command
					{
						place.Type = pBufferType;
						place.DrawType = pBuildingType;
						place.Times = 30;
						place.TopLeft = topLeftCell;
					}
					else if (place.Times <= 0)
					{
						break; // Time out
					}

					if (!(place.Times % 5) && BuildingTypeExt::CleanUpBuildingSpace(pBuildingType, topLeftCell, pHouse))
						break; // No place for cleaning

					if (pHouse == HouseClass::CurrentPlayer && place.Times == 30)
						ClearCurrentBuildingData(pDisplay);

					--place.Times;
					place.Timer.Start(8);

					return TemporarilyCanNotBuild;
				}
				while (false);
			}

			revert = place.Times == 30 || !place.Type;
			ClearPlacingBuildingData(&place);

			if (revert)
				ProximityTemp::Mouse = true;

			return CanNotBuild;
		}
		while (false);

		revert = !place.Type;
		ClearPlacingBuildingData(&place);
	}

	if (pBufferType != pBuildingType)
		pBuilding = static_cast<BuildingClass*>(pBuildingType->CreateObject(pHouse));

	pFactory->SendCommand(RadioCommand::RequestLink, pBuilding);

	if (pBuilding->Unlimbo(CoordStruct { (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
	{
		if (pBufferBuilding != pBuilding)
		{
			if (HouseClass::CurrentPlayer == pHouse)
			{
				if (pDisplay->CurrentBuildingType == pBufferType)
					pDisplay->CurrentBuildingType = pBuildingType;

				if (pDisplay->CurrentBuildingCopy == pBufferBuilding)
					pDisplay->CurrentBuildingCopy = pBuilding;

				if (pDisplay->CurrentBuildingTypeCopy == pBufferType)
					pDisplay->CurrentBuildingTypeCopy = pBuildingType;

				if (Make_Global<BuildingClass*>(0xB0FE5C) == pBufferBuilding) // Q
					Make_Global<BuildingClass*>(0xB0FE5C) = pBuilding;

				if (Make_Global<BuildingClass*>(0xB0FE60) == pBufferBuilding) // W
					Make_Global<BuildingClass*>(0xB0FE60) = pBuilding;
			}

			pBufferBuilding->UnInit();
			pPrimary->Object = pBuilding;
			R->ESI(pBuilding);
		}

		return CanBuild;
	}

	if (pBufferBuilding != pBuilding)
		pBuilding->UnInit();

	if (revert)
		ProximityTemp::Mouse = true;

	return CanNotBuild;
}

// Buildable-upon TechnoTypes Hook #4-1 -> sub_4FB0E0 - Check whether need to skip the replace command
DEFINE_HOOK(0x4FB395, HouseClass_UnitFromFactory_SkipMouseReturn, 0x6)
{
	enum { SkipGameCode = 0x4FB489, CheckMouseCoords = 0x4FB3E3 };

	if (!RulesExt::Global()->ExtendedBuildingPlacing)
		return 0;

	if (ProximityTemp::Mouse)
	{
		ProximityTemp::Mouse = false;
		return 0;
	}

	R->EBX(0);

	if (!DisplayClass::Instance.CurrentBuildingTypeCopy)
		return SkipGameCode;

	R->ECX(DisplayClass::Instance.CurrentBuildingTypeCopy);
	return CheckMouseCoords;
}

// Buildable-upon TechnoTypes Hook #4-2 -> sub_4FB0E0 - Check whether need to skip the clear command
DEFINE_HOOK(0x4FB339, HouseClass_UnitFromFactory_SkipMouseClear, 0x6)
{
	enum { SkipGameCode = 0x4FB4A0 };

	GET(TechnoClass* const, pTechno, ESI);

	if (RulesExt::Global()->ExtendedBuildingPlacing)
	{
		if (const auto pBuilding = abstract_cast<BuildingClass*, true>(pTechno))
		{
			if (const auto pCurrentType = abstract_cast<BuildingTypeClass*>(DisplayClass::Instance.CurrentBuildingType))
			{
				if (!BuildingTypeExt::IsSameBuildingType(pBuilding->Type, pCurrentType))
					return SkipGameCode;
			}
		}
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #4-3 -> sub_4FB0E0 - Check whether need to skip the clear command
DEFINE_HOOK(0x4FAB83, HouseClass_AbandonProductionOf_SkipMouseClear, 0x7)
{
	enum { SkipGameCode = 0x4FABA4 };

	GET(const int, index, EBX);

	if (RulesExt::Global()->ExtendedBuildingPlacing && index >= 0)
	{
		if (const auto pCurrentBuildingType = abstract_cast<BuildingTypeClass*>(DisplayClass::Instance.CurrentBuildingType))
		{
			if (!BuildingTypeExt::IsSameBuildingType(BuildingTypeClass::Array.Items[index], pCurrentBuildingType))
				return SkipGameCode;
		}
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #5 -> sub_4C9FF0 - Restart timer and clear buffer when abandon building production
DEFINE_HOOK(0x4CA05B, FactoryClass_AbandonProduction_AbandonCurrentBuilding, 0x5)
{
	GET(FactoryClass*, pFactory, ESI);

	if (RulesExt::Global()->ExtendedBuildingPlacing)
	{
		const auto pBuilding = abstract_cast<BuildingClass*>(pFactory->Object);

		if (!pBuilding)
			return 0;

		const auto pHouseExt = HouseExt::Fetch(pFactory->Owner);
		auto& place = pBuilding->Type->BuildCat != BuildCat::Combat ? pHouseExt->Common : pHouseExt->Combat;
		ClearPlacingBuildingData(&place);
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #6 -> sub_443C60 - Try to clean up the building space when AI is building
// AIConstructionYard Hook #4-1 -> sub_443C60 - Prohibit AI from building construction yard and clean up invalid walls nodes.
DEFINE_HOOK(0x4451F8, BuildingClass_KickOutUnit_CleanUpAIBuildingSpace, 0x6)
{
	enum { CanBuild = 0x4452F0, TemporarilyCanNotBuild = 0x445237, CanNotBuild = 0x4454E6, BuildSucceeded = 0x4454D4, BuildFailed = 0x445696 };

	GET(BaseNodeClass* const, pBaseNode, EBX);
	GET(BuildingClass* const, pBuilding, EDI);
	GET(BuildingClass* const, pFactory, ESI);
	GET(const CellStruct, topLeftCell, EDX);

	const auto pBuildingType = pBuilding->Type;

	// Prohibit AI from building construction yard
	if (RulesExt::Global()->AIForbidConYard && pBuildingType->ConstructionYard)
	{
		if (pBaseNode)
		{
			pBaseNode->Placed = true;
			pBaseNode->Attempts = 0;
		}

		return BuildFailed;
	}

	// Clean up invalid walls nodes
	if (RulesExt::Global()->AICleanWallNode && pBuildingType->Wall)
	{
		auto notValidWallNode = [topLeftCell]()
			{
				const auto pCell = MapClass::Instance.GetCellAt(topLeftCell);

				for (int i = 0; i < 8; ++i)
				{
					if (const auto pAdjBuilding = pCell->GetNeighbourCell(static_cast<FacingType>(i))->GetBuilding())
					{
						if (pAdjBuilding->Type->ProtectWithWall)
							return false;
					}
				}

				return true;
			};

		if (notValidWallNode())
			return CanNotBuild;
	}

	const auto pHouse = pFactory->Owner;

	if (!RulesExt::Global()->ExtendedBuildingPlacing)
		return 0;

	if (topLeftCell != CellStruct::Empty && !pBuildingType->PlaceAnywhere)
	{
		if (!pBuildingType->PowersUpBuilding[0])
		{
			bool noOccupy = true;
			bool canBuild = CheckBuildingFoundation(pBuildingType, topLeftCell, pHouse, noOccupy);
			const auto pHouseExt = HouseExt::Fetch(pHouse);
			auto& place = pBuildingType->BuildCat != BuildCat::Combat ? pHouseExt->Common : pHouseExt->Combat;

			do
			{
				if (canBuild)
				{
					if (noOccupy)
						break; // Can Build

					do
					{
						if (topLeftCell != place.TopLeft || pBuildingType != place.Type) // New command
						{
							place.Type = pBuildingType;
							place.DrawType = pBuildingType;
							place.TopLeft = topLeftCell;
						}

						if (!place.Timer.HasTimeLeft())
						{
							place.Timer.Start(40);

							if (BuildingTypeExt::CleanUpBuildingSpace(pBuildingType, topLeftCell, pHouse))
								break; // No place for cleaning
						}

						return TemporarilyCanNotBuild;
					}
					while (false);
				}

				ClearPlacingBuildingData(&place);
				return CanNotBuild;
			}
			while (false);

			ClearPlacingBuildingData(&place);
		}
		else
		{
			const auto pCell = MapClass::Instance.GetCellAt(topLeftCell);
			const auto pCellBuilding = pCell->GetBuilding();

			if (!pCellBuilding || !pCellBuilding->CanUpgradeBuilding(pBuildingType, pHouse))
				return CanNotBuild;
		}
	}

	if (pBuilding->Unlimbo(CoordStruct { (topLeftCell.X << 8) + 128, (topLeftCell.Y << 8) + 128, 0 }, DirType::North))
	{
		PlayConstructionYardAnim(pFactory);
		return CanBuild;
	}

	return CanNotBuild;
}

static inline bool CanDrawGrid(bool draw)
{
	if (ProximityTemp::Exist)
	{
		ProximityTemp::Exist = false;
		return false;
	}

	return draw;
}

// Laser fence use GetBuilding to check whether can build and draw, so no need to change

/*
	In sub_6D5C50

	- Buildable-upon TechnoTypes Hook #8-1 -> Don't draw overlay wall grid when have occupiers
*/
DEFINE_HOOK(0x6D5D38, TacticalClass_DrawOverlayWallGrid_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x6D5D40, Invalid = 0x6D5F0F };

	GET(const bool, valid, EAX);

	return CanDrawGrid(valid) ? Valid : Invalid;
}

/*
	In sub_6D59D0

	- Buildable-upon TechnoTypes Hook #8-2 -> Don't draw firestorm wall grid when have occupiers
*/
DEFINE_HOOK(0x6D5A9D, TacticalClass_DrawFirestormWallGrid_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x6D5AA5, Invalid = 0x6D5C2F };

	GET(const bool, valid, EAX);

	return CanDrawGrid(valid) ? Valid : Invalid;
}

/*
	In sub_588750

	- Buildable-upon TechnoTypes Hook #8-3 -> Don't place overlay wall when have occupiers
*/
DEFINE_HOOK(0x588873, MapClass_BuildingToWall_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x58887B, Invalid = 0x588935 };

	GET(const bool, valid, EAX);

	return CanDrawGrid(valid) ? Valid : Invalid;
}

/*
	In sub_588570

	- Buildable-upon TechnoTypes Hook #8-4 -> Don't place firestorm wall when have occupiers
*/
DEFINE_HOOK(0x588664, MapClass_BuildingToFirestormWall_DisableWhenHaveTechnos, 0x8)
{
	enum { Valid = 0x58866C, Invalid = 0x588730 };

	GET(const bool, valid, EAX);

	return CanDrawGrid(valid) ? Valid : Invalid;
}

// Buildable-upon TechnoTypes Hook #9-1 -> sub_7393C0 - Try to clean up the building space when is deploying
DEFINE_HOOK(0x73946C, UnitClass_TryToDeploy_CleanUpDeploySpace, 0x6)
{
	enum { CanDeploy = 0x73958A, TemporarilyCanNotDeploy = 0x73953B, CanNotDeploy = 0x7394E0 };

	GET(UnitClass* const, pThis, EBP);
	GET(CellStruct, topLeftCell, ESI);

	if (!RulesExt::Global()->ExtendedBuildingPlacing)
		return 0;

	const auto pUnitExt = UnitExt::Fetch(pThis);
	const auto pBuildingType = pThis->Type->DeploysInto;
	const auto pHouseExt = HouseExt::Fetch(pThis->Owner);
	auto& vec = pHouseExt->OwnedDeployingUnits;

	if (pBuildingType->GetFoundationWidth() > 2 || pBuildingType->GetFoundationHeight(false) > 2)
		topLeftCell -= CellStruct { 1, 1 };

	R->Stack<CellStruct>(STACK_OFFSET(0x28, -0x14), topLeftCell);

	if (!pBuildingType->PlaceAnywhere)
	{
		bool noOccupy = true;
		bool canBuild = CheckBuildingFoundation(pBuildingType, topLeftCell, pThis->Owner, noOccupy);

		do
		{
			if (canBuild)
			{
				if (noOccupy)
					break; // Can build

				do
				{
					if (!pUnitExt->UnitAutoDeployTimer.InProgress())
					{
						if (BuildingTypeExt::CleanUpBuildingSpace(pBuildingType, topLeftCell, pThis->Owner, pThis))
							break; // No place for cleaning

						if (vec.size() == 0 || std::find(vec.begin(), vec.end(), pThis) == vec.end())
							vec.push_back(pThis);

						pUnitExt->UnitAutoDeployTimer.Start(40);
					}

					if (pThis->PrimaryFacing.Current().GetFacing<256>() != static_cast<size_t>(pBuildingType->DeployFacing))
					{
						const auto pLoco = pThis->Locomotor;

						if (!pLoco->Is_Moving_Now())
							pLoco->Do_Turn(DirStruct(static_cast<DirType>(pBuildingType->DeployFacing)));
					}

					return TemporarilyCanNotDeploy;
				}
				while (false);
			}

			if (vec.size() > 0)
				vec.erase(std::remove(vec.begin(), vec.end(), pThis), vec.end());

			pUnitExt->UnitAutoDeployTimer.Stop();

			return CanNotDeploy;
		}
		while (false);
	}

	if (vec.size() > 0)
		vec.erase(std::remove(vec.begin(), vec.end(), pThis), vec.end());

	pUnitExt->UnitAutoDeployTimer.Stop();

	return CanDeploy;
}

// Buildable-upon TechnoTypes Hook #9-2 -> sub_73FD50 - Push the owner house into deploy check
DEFINE_HOOK(0x73FF8F, UnitClass_MouseOverObject_ShowDeployCursor, 0x6)
{
	if (RulesExt::Global()->ExtendedBuildingPlacing) // This IF check is not so necessary
	{
		GET(const UnitClass* const, pUnit, ESI);
		LEA_STACK(HouseClass**, pHousePtr, STACK_OFFSET(0x20, -0x20));
		*pHousePtr = pUnit->Owner;
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #10 -> sub_4C6CB0 - Stop deploy when get stop command
DEFINE_HOOK(0x4C7665, EventClass_RespondToEvent_StopDeployInIdleEvent, 0x6)
{
	if (RulesExt::Global()->ExtendedBuildingPlacing) // This IF check is not so necessary
	{
		GET(const UnitClass* const, pUnit, ESI);

		if (pUnit->Type->DeploysInto)
		{
			const auto mission = pUnit->CurrentMission;

			if (mission == Mission::Guard || mission == Mission::Unload)
			{
				const auto pHouseExt = HouseExt::Fetch(pUnit->Owner);
				auto& vec = pHouseExt->OwnedDeployingUnits;

				if (vec.size() > 0)
					vec.erase(std::remove(vec.begin(), vec.end(), pUnit), vec.end());
			}
		}
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #11 -> sub_4F8440 - Check whether can place again in each house
DEFINE_HOOK(0x4F8DB1, HouseClass_Update_CheckHangUpBuilding, 0x6)
{
	GET(HouseClass* const, pHouse, ESI);

	if (!pHouse->IsControlledByHuman())
		return 0;

	if (pHouse->RecheckTechTree)
	{
		if (const auto pFactory = pHouse->GetPrimaryFactory(AbstractType::BuildingType, false, BuildCat::DontCare))
		{
			if (pFactory->IsDone())
			{
				if (const auto pBuilding = abstract_cast<BuildingClass*>(pFactory->Object))
					BuildingTypeExt::AutoUpgradeBuilding(pBuilding);
			}
		}

		if (const auto pFactory = pHouse->GetPrimaryFactory(AbstractType::BuildingType, false, BuildCat::Combat))
		{
			if (pFactory->IsDone())
			{
				if (const auto pBuilding = abstract_cast<BuildingClass*>(pFactory->Object))
					BuildingTypeExt::AutoUpgradeBuilding(pBuilding);
			}
		}
	}

	if (!RulesExt::Global()->ExtendedBuildingPlacing)
		return 0;

	const auto pHouseExt = HouseExt::Fetch(pHouse);
	auto buildCurrent = [pHouse, pHouseExt](BuildingTypeClass* pType, CellStruct cell, size_t placeType)
		{
			if (!pType)
				return;

			auto currentCanBuild = [pHouse, pType]() -> const bool
				{
					auto const bitsOwners = pType->GetOwners();

					for (auto const& pConYard : pHouse->ConYards)
					{
						if (pConYard->InLimbo || !pConYard->HasPower || pConYard->Deactivated)
							continue;

						if (pConYard->CurrentMission == Mission::Construction || pConYard->QueuedMission == Mission::Selling)
							continue;

						const auto pConYardType = pConYard->Type;

						if (pConYardType->Factory != AbstractType::BuildingType || !pConYardType->InOwners(bitsOwners))
							continue;

						return true;
					}

					return false;
				};

			if (!currentCanBuild())
			{
				ClearPlacingBuildingData(pType->BuildCat != BuildCat::Combat ? &pHouseExt->Common : &pHouseExt->Combat);

				if (pHouse == HouseClass::CurrentPlayer)
					VoxClass::Play(GameStrings::EVA_CannotDeployHere);
			}
			else if (pHouse == HouseClass::CurrentPlayer) // Prevent unexpected wrong event
			{
				const int place = static_cast<int>(placeType);
				const auto arrayIndex = pType->GetArrayIndex();
				EventClass::OutList.Add(EventClass(pHouse->ArrayIndex, EventType::Place, AbstractType::Building, arrayIndex, place, cell));
			}
		};

	auto& commonPlace = pHouseExt->Common;

	if (commonPlace.Timer.Completed())
	{
		commonPlace.Timer.Stop();
		buildCurrent(commonPlace.Type, commonPlace.TopLeft, commonPlace.PlaceType);
	}

	auto& combatPlace = pHouseExt->Combat;

	if (combatPlace.Timer.Completed())
	{
		combatPlace.Timer.Stop();
		buildCurrent(combatPlace.Type, combatPlace.TopLeft, combatPlace.PlaceType);
	}

	if (pHouseExt->OwnedDeployingUnits.size() > 0)
	{
		auto& vec = pHouseExt->OwnedDeployingUnits;

		for (auto it = vec.begin(); it != vec.end(); )
		{
			const auto pUnit = *it;

			if (!pUnit->InLimbo
				&& pUnit->IsOnMap
				&& !pUnit->IsSinking
				&& pUnit->Owner == pHouse
				&& !pUnit->Destination
				&& !pUnit->ParasiteEatingMe
				&& !pUnit->TemporalTargetingMe
				&& pUnit->Type->DeploysInto)
			{
				const auto mission = pUnit->CurrentMission;

				if (mission == Mission::Unload)
				{
					++it;
					continue;
				}
				else if (mission == Mission::Guard)
				{
					if (const auto pExt = UnitExt::Fetch(pUnit))
					{
						if (!(pExt->UnitAutoDeployTimer.GetTimeLeft() % 8))
							pUnit->QueueMission(Mission::Unload, true);

						++it;
						continue;
					}
				}
			}

			it = vec.erase(it);
		}
	}

	return 0;
}

// Buildable-upon TechnoTypes Hook #12 -> sub_6D5030 - Draw the placing building preview
DEFINE_HOOK(0x6D504C, TacticalClass_DrawPlacement_DrawPlacingPreview, 0x6)
{
	if (!RulesExt::Global()->ExtendedBuildingPlacing)
		return 0;

	const auto pPlayer = HouseClass::CurrentPlayer;
	const auto pDisplay = &DisplayClass::Instance;

	auto drawImage = [&pDisplay](BuildingTypeClass* const pType, HouseClass* const pHouse, const CellStruct cell)
		{
			const auto pCell = pDisplay->TryGetCellAt(cell);

			if (!pCell || cell == CellStruct::Empty || pType->PowersUpBuilding[0])
				return;

			auto pImage = pType->LoadBuildup();
			int imageFrame = 0;

			if (pImage)
				imageFrame = ((pImage->Frames / 2) - 1);
			else
				pImage = pType->GetImage();

			if (!pImage)
				return;

			const auto blitFlags = BlitterFlags::TransLucent75 | BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass;
			auto rect = DSurface::Temp->GetRect();
			rect.Height -= 32;
			auto point = TacticalClass::Instance->CoordsToClient(CellClass::Cell2Coord(cell, (1 + pCell->GetFloorHeight(Point2D::Empty)))).first;
			point.Y -= 15;

			const auto ColorSchemeIndex = pHouse->ColorSchemeIndex;
			const auto Palettes = pType->Palette;
			const auto pColor = Palettes ? Palettes->Items[ColorSchemeIndex] : ColorScheme::Array.Items[ColorSchemeIndex];
			DSurface::Temp->DrawSHP(pColor->LightConvert, pImage, imageFrame, &point, &rect, blitFlags, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		};

	for (const auto& pHouse : HouseClass::Array)
	{
		if (pPlayer->IsObserver() || pHouse->IsAlliedWith(pPlayer))
		{
			const auto pHouseExt = HouseExt::Fetch(pHouse);

			if (const auto pType = abstract_cast<BuildingTypeClass*>(pDisplay->CurrentBuildingTypeCopy))
				drawImage(pType, pHouse, (pDisplay->CurrentFoundationCopy_TopLeftOffset + pDisplay->CurrentFoundationCopy_CenterCell));

			if (const auto pType = pHouseExt->Common.DrawType)
				drawImage(pType, pHouse, pHouseExt->Common.TopLeft);

			if (const auto pType = pHouseExt->Combat.DrawType)
				drawImage(pType, pHouse, pHouseExt->Combat.TopLeft);

			if (pHouseExt->OwnedDeployingUnits.size() <= 0)
				continue;

			for (const auto& pUnit : pHouseExt->OwnedDeployingUnits)
			{
				const auto pType = pUnit->Type->DeploysInto;

				if (!pType)
					continue;

				auto displayCell = CellClass::Coord2Cell(pUnit->GetCoords()); // pUnit->GetMapCoords();

				if (pType->GetFoundationWidth() > 2 || pType->GetFoundationHeight(false) > 2)
					displayCell -= CellStruct { 1, 1 };

				drawImage(pType, pHouse, displayCell);
			}
		}
	}

	return 0;
}

// Auto Build Hook -> sub_6A8B30 - Auto Build Buildings
DEFINE_HOOK(0x6A8E34, StripClass_Update_AutoBuildBuildings, 0x7)
{
	enum { SkipSetStripShortCut = 0x6A8E4D };

	GET(BuildingClass* const, pBuilding, ESI);

	return (RulesExt::Global()->ExtendedBuildingPlacing && BuildingTypeExt::AutoUpgradeBuilding(pBuilding)) ? SkipSetStripShortCut : 0;
}

/*
	In sub_45C300

	- Fix foundation outside Hook -> Fix incorrect building type's FoundationOutside
*/
DEFINE_HOOK(0x45DD04, BuildingTypeClass_InitFoundationOutsides, 0x5)
{
	auto& foundationOutsides = Make_Global<CellStruct[22][30]>(0x89D368);
	constexpr auto CellStruct_OutsideEnd = CellStruct { 0x7FFF, 0x7FFF };

	// 4x3 remove incorrect (3,2), (3,1), (3,0) and add missing (4,3), (4,2), (4,1), (4,0), (4,-1)
	{
		auto& outside4x3 = foundationOutsides[12];
		outside4x3[6] = CellStruct { 4, 2 };
		outside4x3[8] = CellStruct { 4, 1 };
		outside4x3[10] = CellStruct { 4, 0 };
		outside4x3[16] = CellStruct { 4, -1 };
		outside4x3[17] = CellStruct { 4, 3 };
		outside4x3[18] = CellStruct_OutsideEnd;
	}

	// 2x6 add missing (-1,4) and (2,4)
	{
		auto& outside2x6 = foundationOutsides[15];
		outside2x6[18] = CellStruct { -1, 4 };
		outside2x6[19] = CellStruct { 2, 4 };
		outside2x6[20] = CellStruct_OutsideEnd;
	}

	// 2x5 add missing (-1,4) and (2,4)
	{
		auto& outside2x5 = foundationOutsides[16];
		outside2x5[16] = CellStruct { -1, 4 };
		outside2x5[17] = CellStruct { 2, 4 };
		outside2x5[18] = CellStruct_OutsideEnd;
	}

	// 4x4 remove duplicate (-1,4) and (4,4)
	{
		auto& outside4x4 = foundationOutsides[18];
		outside4x4[16] = CellStruct { 4, 3 };
		outside4x4[20] = CellStruct_OutsideEnd;
		outside4x4[21] = CellStruct::Empty;
		outside4x4[22] = CellStruct::Empty;
	}

	// 3x4 add missing (-1,3) and (3,3)
	{
		auto& outside3x4 = foundationOutsides[19];
		outside3x4[16] = CellStruct { -1, 3 };
		outside3x4[17] = CellStruct { 3, 3 };
		outside3x4[18] = CellStruct_OutsideEnd;
	}

	// 6x4 fix the entire error without (2,-1)
	{
		auto& outside6x4 = foundationOutsides[20];
		outside6x4[1] = CellStruct { -1, -1 };
		outside6x4[2] = CellStruct { 0, -1 };
		outside6x4[3] = CellStruct { 1, -1 };
		outside6x4[4] = CellStruct { 3, -1 };
		outside6x4[5] = CellStruct { 4, -1 };
		outside6x4[6] = CellStruct { 5, -1 };
		outside6x4[7] = CellStruct { 6, -1 };
		outside6x4[8] = CellStruct { -1, 0 };
		outside6x4[9] = CellStruct { 6, 0 };
		outside6x4[10] = CellStruct { -1, 1 };
		outside6x4[11] = CellStruct { 6, 1 };
		outside6x4[12] = CellStruct { -1, 2 };
		outside6x4[13] = CellStruct { 6, 2 };
		outside6x4[14] = CellStruct { -1, 3 };
		outside6x4[15] = CellStruct { 6, 3 };
		outside6x4[16] = CellStruct { -1, 4 };
		outside6x4[17] = CellStruct { 0, 4 };
		outside6x4[18] = CellStruct { 1, 4 };
		outside6x4[19] = CellStruct { 2, 4 };
		outside6x4[20] = CellStruct { 3, 4 };
		outside6x4[21] = CellStruct { 4, 4 };
		outside6x4[22] = CellStruct { 5, 4 };
		outside6x4[23] = CellStruct { 6, 4 };
		outside6x4[24] = CellStruct_OutsideEnd;
	}

	return 0;
}
