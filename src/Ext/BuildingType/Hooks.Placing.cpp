#include "Body.h"

#include <HouseClass.h>

#include "Ext/Rules/Body.h"

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

		if (const auto count = pAI->ConYards.Count)
		{
			const auto wayPoint = pAI->GetSpawnPosition();

			if (wayPoint != -1)
			{
				const auto center = ScenarioClass::Instance->GetWaypointCoords(wayPoint);
				auto newCenter = center;
				double distanceSquared = 131072.0;

				for (int i = 0; i < count; ++i)
				{
					if (const auto pBuilding = pAI->ConYards.GetItem(i))
					{
						if (pBuilding->IsAlive && pBuilding->Health > 0 && !pBuilding->InLimbo)
						{
							const auto newDistanceSquared = pBuilding->GetMapCoords().DistanceFromSquared(center);

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

// AIConstructionYard Hook #4-1 -> sub_443C60 - Prohibit AI from building construction yard and clean up invalid walls nodes.
DEFINE_HOOK(0x4451F8, BuildingClass_KickOutUnit_CleanUpAIBuildingSpace, 0x6)
{
	enum { CanNotBuild = 0x4454E6, BuildFailed = 0x445696 };

	GET(BaseNodeClass* const, pBaseNode, EBX);
	GET(BuildingClass* const, pBuilding, EDI);
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

// AIConstructionYard Hook #4-3 -> sub_7393C0 - Prohibit AI from building construction yard.
DEFINE_HOOK(0x7397F4, UnitClass_TryToDeploy_SkipSetShouldRebuild, 0x7)
{
	enum { SkipRebuildFlag = 0x7397FB };

	GET(BuildingClass* const, pBuilding, EBX);

	return (pBuilding->Type->ConstructionYard && RulesExt::Global()->AIForbidConYard) ? SkipRebuildFlag : 0;
}

// AIConstructionYard Hook #4-4 -> sub_440580 - Prohibit AI from building construction yard.
DEFINE_HOOK(0x440B7A, BuildingClass_Unlimbo_SkipSetShouldRebuild, 0x7)
{
	enum { SkipRebuildFlag = 0x440B81 };

	GET(BuildingClass* const, pBuilding, ESI);

	return (pBuilding->Type->ConstructionYard && RulesExt::Global()->AIForbidConYard) ? SkipRebuildFlag : 0;
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
