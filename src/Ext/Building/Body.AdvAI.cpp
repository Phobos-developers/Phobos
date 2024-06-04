#include "Body.h"
#include "Ext/House/Body.h"

#define	REFRESH_EOL         32767		// This number ends a refresh/occupy offset list.

#define TICKS_PER_SECOND    15
#define TICKS_PER_MINUTE    (TICKS_PER_SECOND * 60)
#define TICKS_PER_HOUR      (TICKS_PER_MINUTE * 60)

///
/// Advanced AI
///	Credits to Rampastring
///

HouseClass* BuildingExt::Find_Closest_Opponent(const HouseClass* pHouse)
{
	double nearestDistance = std::numeric_limits<double>::max();
	HouseClass* pNearestHouse = nullptr;

	for (const auto pOtherHouse : *HouseClass::Array)
	{
		if (pOtherHouse == pHouse)
		{
			continue;
		}

		if (pOtherHouse->Type->MultiplayPassive)
		{
			continue;
		}

		if (pHouse->IsAlliedWith(pOtherHouse))
		{
			continue;
		}

		const double distance = pHouse->Base_Center().DistanceFrom(pOtherHouse->Base_Center());

		if (distance < nearestDistance)
		{
			nearestDistance = distance;
			pNearestHouse = pOtherHouse;
		}
	}

	return pNearestHouse;
}

int BuildingExt::Get_Distance_To_Primary_Enemy(CellStruct cell, HouseClass* pHouse)
{
	HouseClass* enemy = nullptr;

	if (pHouse->EnemyHouseIndex != -1)
	{
		enemy = HouseClass::FindByCountryIndex(pHouse->EnemyHouseIndex);
	}

	if (enemy == nullptr)
	{
		enemy = Find_Closest_Opponent(pHouse);
	}

	int enemyDistance = 0;

	if (enemy != nullptr)
	{
		enemyDistance = static_cast<int>(cell.DistanceFrom(enemy->Base_Center()));
	}

	return enemyDistance;
}

/**
*  Stores a record of buildings owned by the house
*  that is currently placing down a building.
*  Allows performing lookups on them without
*  needing to go through the list of all buildings on
*  the entire map.
*/
BuildingClass* BuildingExt::ExtData::OurBuildings[1000];
size_t BuildingExt::ExtData::OurBuildingCount;

void BuildingExt::Mark_Expansion_As_Done(HouseClass* pHouse)
{
	const auto ext = HouseExt::ExtMap.Find(pHouse);

	if (ext->NextExpansionPointLocation.X == 0 || ext->NextExpansionPointLocation.Y == 0)
		return;

	ext->NextExpansionPointLocation = CellStruct(0, 0);
}

int BuildingExt::Try_Place(BuildingClass* pBuilding, CellStruct cell)
{
	HouseClass* owner = pBuilding->Owner;
	const auto houseExt = HouseExt::ExtMap.Find(owner);

	const int placementResult = pBuilding->Type->FlushForPlacement(cell, owner);

	if (placementResult == 1 || placementResult == 2)
		return 1;

	const CellStruct finalPlacementCell = cell;
	CoordStruct coord = GeneralUtils::CoordinatesFromCell(finalPlacementCell);

	if (pBuilding->Unlimbo(coord, DirType::North))
	{
		owner->ProducingBuildingTypeIndex = -1;

		// This is necessary or the building's build-up anim is played twice.
		// RA doesn't do this, must be a difference somewhere in the engine.
		pBuilding->QueueMission(Mission::Construction, true);
		pBuilding->NextMission();

		int closeEnough = 7;

		// If we just placed down our first barracks, then set our team timer to 0
		// so we can immediately start producing infantry.
		// Do not do this on Easy mode to avoid overwhelming the player.
		if (owner->AIDifficulty < AIDifficulty::Hard &&
			!houseExt->HasBuiltFirstBarracks &&
			pBuilding->Type->Factory == AbstractType::InfantryType)
		{
			houseExt->HasBuiltFirstBarracks = true;
			owner->TeamDelayTimer.Start(0);
		}

		// Check if we placed a refinery.
		// If yes, check if we were expanding. If yes, the expanding is done.
		// If not, but we're close to an expansion field, then flag us to build a refinery as our next building.
		if (pBuilding->Type->ResourceDestination)
		{
			if (houseExt->NextExpansionPointLocation.X != 0 && houseExt->NextExpansionPointLocation.Y != 0)
			{
				const auto buildingExt = ExtMap.Find(pBuilding);
				buildingExt->AssignedExpansionPoint = houseExt->NextExpansionPointLocation;
			}

			Mark_Expansion_As_Done(owner);
			houseExt->ShouldBuildRefinery = false;
		}
		else if (houseExt->NextExpansionPointLocation.X > 0 &&
			houseExt->NextExpansionPointLocation.Y > 0 &&
			GeneralUtils::CellFromCoordinates(pBuilding->GetCenterCoords()).DistanceFrom(houseExt->NextExpansionPointLocation) < closeEnough)
		{
			houseExt->ShouldBuildRefinery = true;
		}

		return 2;
	}

	return 0;
}

/**
 *  Fetches a house's base area as a rectangle.
 *  We can use this as a rough zone for placing new buildings.
 */
RectangleStruct BuildingExt::Get_Base_Rect(HouseClass* pHouse, int adjacency, int width, int height)
{
	int x = INT_MAX;
	int y = INT_MAX;
	int right = INT_MIN;
	int bottom = INT_MIN;

	for (const auto building : *BuildingClass::Array)
	{
		if (!building->IsAlive || building->InLimbo || building->Owner != pHouse)
		{
			continue;
		}

		const CellStruct buildingCell = building->GetMapCoords();
		if (buildingCell.X < x)
			x = buildingCell.X;

		if (buildingCell.Y < y)
			y = buildingCell.Y;

		const int buildingRight = buildingCell.X + building->Type->GetFoundationWidth() - 1;
		if (buildingRight > right)
			right = buildingRight;

		const int buildingBottom = buildingCell.Y + building->Type->GetFoundationHeight(false) - 1;
		if (buildingBottom > bottom)
			bottom = buildingBottom;
	}

	x -= adjacency;
	x -= width;
	y -= adjacency;
	y -= height;
	right += adjacency + width;
	bottom += adjacency + height;

	return RectangleStruct { x, y, right - x, bottom - y };
}

/**
 *  Checks whether a cell should be evaluated for AI building placement.
 */
bool BuildingExt::Should_Evaluate_Cell_For_Placement(CellStruct cell, BuildingClass* pBuilding, int adjacencyBonus)
{
	bool result = false;

	const int adjacency = pBuilding->Type->Adjacent + 1 + adjacencyBonus;

	for (const auto pOtherBuilding : *BuildingClass::Array)
	{
		if (!pOtherBuilding->IsAlive ||
			pOtherBuilding->InLimbo ||
			pOtherBuilding->Type->InvisibleInGame)
		{
			continue;
		}

		// We don't want the AI to get stuck.
		// Make sure that the building would leave at least 1 free cell to its neighbours.
		CellStruct origin = pOtherBuilding->GetMapCoords();
		CellStruct const* occupy = pOtherBuilding->GetFoundationData(true);
		bool allowance = true;
		while (occupy->X != REFRESH_EOL && occupy->Y != REFRESH_EOL)
		{
			const CellStruct sum = origin + *occupy;

			// The new building is close enough if even one 
			// of its cells would be close enough to the cells
			// of the current "other" building.
			CellStruct const* newOccupy = pBuilding->GetFoundationData(true);
			while (newOccupy->X != REFRESH_EOL && newOccupy->Y != REFRESH_EOL)
			{
				const CellStruct newSum = cell + *newOccupy;

				int xDiff = newSum.X - sum.X;
				int yDiff = newSum.Y - sum.Y;
				xDiff = std::abs(xDiff);
				yDiff = std::abs(yDiff);

				if (xDiff < 2 && yDiff < 2)
				{
					// This foundation cell is too close to the compared building
					allowance = false;
					break;
				}

				newOccupy++;
			}

			if (!allowance)
			{
				break;
			}

			occupy++;
		}

		// If the building was too close to an existing building, just bail out.
		// No need to check anything else.
		if (!allowance)
		{
			result = false;
			break;
		}

		// For the proximity check, check that this building
		// is owned by us and that it extends the adjacency range.
		if (pOtherBuilding->Owner != pBuilding->Owner ||
			!pOtherBuilding->Type->BaseNormal)
		{
			continue;
		}

		// Check that the cell would be close enough for the building placement 
		// to pass the proximity check if the building was on the cell.
		// This is only necessary to check if the building hasn't already
		// passed the proximity check.
		if (!result)
		{

			bool pass = false;

			origin = pOtherBuilding->GetMapCoords();
			occupy = pOtherBuilding->GetFoundationData(true);
			while (occupy->X != REFRESH_EOL && occupy->Y != REFRESH_EOL)
			{
				const CellStruct sum = origin + *occupy;

				// The new building is close enough if even one 
				// of its cells would be close enough to the cells
				// of the current "other" building.
				CellStruct const* newOccupy = pBuilding->GetFoundationData(true);
				while (newOccupy->X != REFRESH_EOL && newOccupy->Y != REFRESH_EOL)
				{
					const CellStruct newSum = cell + *newOccupy;

					int xDiff = newSum.X - sum.X;
					int yDiff = newSum.Y - sum.Y;
					xDiff = std::abs(xDiff);
					yDiff = std::abs(yDiff);

					if (xDiff <= adjacency && yDiff <= adjacency)
					{
						// This foundation cell is close enough to the compared building.
						pass = true;
						break;
					}

					newOccupy++;
				}

				if (pass)
				{
					break;
				}

				occupy++;
			}

			if (pass)
				result = true;
		}
	}

	return result;
}

/**
 *  Implements an extra check for terrain passability around the building.
 *  This is to make the AI less likely to get stuck.
 */
int inline BuildingExt::Modify_Rating_By_Terrain_Passability(CellStruct cell, BuildingClass* pBuilding, int originalValue)
{
	int value = originalValue;

	const CellStruct cellAboveCoords = cell + CellStruct(-1, -1);
	const CellStruct cellBelowCoords = cell + CellStruct(pBuilding->Type->GetFoundationWidth(), pBuilding->Type->GetFoundationHeight(false));
	bool passableAbove = true;
	bool passableBelow = true;

	const SpeedType speed = pBuilding->Type->SpeedType == SpeedType::Float ? SpeedType::Float : SpeedType::Foot;

	if (MapClass::Instance->CoordinatesLegal(cellAboveCoords))
	{
		CellClass& cellAbove = (*MapClass::Instance)[cellAboveCoords];
		if (!cellAbove.IsClearToMove(speed, true, true))
		{
			passableAbove = false;
		}
	}

	if (MapClass::Instance->CoordinatesLegal(cellBelowCoords))
	{
		CellClass& cellBelow = (*MapClass::Instance)[cellBelowCoords];
		if (!cellBelow.IsClearToMove(speed, true, true))
		{
			passableBelow = false;
		}
	}

	// If both above and below are impassable, consider this a very high-risk
	// position when it comes to the possibility of the AI getting stuck.
	if (!passableAbove && !passableBelow)
	{
		return INT_MAX;
	}

	// Do the same processing for left and right (east and west, considering in-game rendering iow. NOT logical in-game compass)
	const CellStruct cellEastCoords = cell + CellStruct(pBuilding->Type->GetFoundationWidth(), -1);
	const CellStruct cellWestCoords = cell + CellStruct(-1, pBuilding->Type->GetFoundationHeight(false));
	bool passableEast = true;
	bool passableWest = true;

	if (MapClass::Instance->CoordinatesLegal(cellEastCoords))
	{
		CellClass& cell_above = (*MapClass::Instance)[cellEastCoords];
		if (!cell_above.IsClearToMove(speed, true, true))
		{
			passableEast = false;
		}
	}

	if (MapClass::Instance->CoordinatesLegal(cellWestCoords))
	{
		CellClass& cell_below = (*MapClass::Instance)[cellWestCoords];
		if (!cell_below.IsClearToMove(speed, true, true))
		{
			passableWest = false;
		}
	}

	// If both east and west are impassable, consider this a very high-risk
	// position when it comes to the possibility of the AI getting stuck.
	if (!passableEast && !passableWest)
	{
		return INT_MAX;
	}

	// Individual stuck positions just result in a worse rating.
	if (!passableAbove) value = (value * 4) / 3;
	if (!passableBelow) value = (value * 4) / 3;
	if (!passableEast) value = (value * 4) / 3;
	if (!passableWest) value = (value * 4) / 3;

	return value;
}

/**
 *  Evaluates a rectangle from the map with a value generator
 *  function and finds the best cell for placing down a building.
 *  The best cell is one that has the LOWEST value and that allows legal
 *  building placement.
 */
CellStruct BuildingExt::Find_Best_Building_Placement_Cell(RectangleStruct baseArea, BuildingClass* pBuilding, int (*valueGenerator)(CellStruct, BuildingClass*), int adjacencyBonus)
{
	int lowestRating = INT_MAX;
	CellStruct bestCell = CellStruct(0, 0);

	// Check the resolution of the scan. If our base area is huge, we can't check as precisely
	// or we'll cause into performance issues.
	const int resCells = 2000;
	const int areaSize = baseArea.Width * baseArea.Height;
	const int resolution = 1 + (areaSize / resCells);

	for (int y = baseArea.Y; y < baseArea.Y + baseArea.Height; y += resolution)
	{
		for (int x = baseArea.X; x < baseArea.X + baseArea.Width; x += resolution)
		{
			CellStruct cell = CellStruct(x, y);

			// Skip cells that are outside of the visible map area.
			if (!MapClass::Instance->CoordinatesLegal(cell))
				continue;

			// Skip cells where we couldn't legally place the building on.
			// TODO: Manually check the cells? Currently our own units also block placement.
			if (!pBuilding->Type->CanPlaceHere(cell, pBuilding->Owner))
				continue;

			// Check whether this cell is fine by proximity rules.
			if (!Should_Evaluate_Cell_For_Placement(cell, pBuilding, adjacencyBonus))
				continue;

			// Get value for the cell.
			int value = valueGenerator(cell, pBuilding);

			// Adjust for terrain passability (lessen the chance for the dumb AI to get stuck).
			value = Modify_Rating_By_Terrain_Passability(cell, pBuilding, value);

			// Check whether this is the best placement cell so far.
			if (value < lowestRating)
			{
				lowestRating = value;
				bestCell = cell;
			}
		}
	}

	return bestCell;
}

int inline BuildingExt::Modify_Rating_By_Allied_Building_Proximity(CellStruct cell, BuildingClass* pBuilding, int originalValue)
{
	const int value = originalValue * 1000;

	const CellStruct centerCell = cell + CellStruct(pBuilding->Type->GetFoundationWidth() / 2, pBuilding->Type->GetFoundationHeight(false) / 2);

	double closest_distance = std::numeric_limits<double>::max();

	for (size_t i = 0; i < ExtData::OurBuildingCount; i++)
	{
		const BuildingClass* otherBuilding = ExtData::OurBuildings[i];

		CellStruct other_center_cell = GeneralUtils::CellFromCoordinates(otherBuilding->GetCenterCoords());
		const double dist = centerCell.DistanceFrom(other_center_cell);
		if (dist < closest_distance)
		{
			closest_distance = dist;
		}
	}

	// Extra check for terrain passability around the building.
	// If cells on both sides of the buildings are blocked, this is an unusually
	// bad place for placing the building and we should place it somewhere else.



	// The closer the building is to an existing building, the worse the placement position is.
	// In other words, being closer INCREASES the value (as lower is better).
	return value + (1000 - static_cast<int>(closest_distance));
}

int BuildingExt::Refinery_Placement_Cell_Value(CellStruct cell, BuildingClass* pBuilding)
{
	const HouseClass* pOwner = pBuilding->Owner;
	const auto houseExt = HouseExt::ExtMap.Find(pOwner);

	double value = 0;

	// If we have nowhere to expand, then just try placing it somewhere central, hopefully it's safe there.
	if (houseExt->NextExpansionPointLocation.X <= 0 || houseExt->NextExpansionPointLocation.Y <= 0)
	{
		const CellStruct center = pOwner->Base_Center();
		value = cell.DistanceFrom(center);
	}
	else
	{
		// For refinery placement, we can basically make the value equal to the distance
		// that the refinery has to our next expansion point.
		value = cell.DistanceFrom(houseExt->NextExpansionPointLocation);
	}

	// Take proximity into nearby buildings into account.
	// We do this to avoid traffic congestion in tight spaces in the AI's base.
	return Modify_Rating_By_Allied_Building_Proximity(cell, pBuilding, static_cast<int>(value));
}

/**
 *  Calculates the best refinery placement location.
 */
CellStruct BuildingExt::Get_Best_Refinery_Placement_Position(BuildingClass* pBuilding)
{
	const int adjacency = pBuilding->Type->Adjacent + 1 + 1;
	const RectangleStruct baseArea = Get_Base_Rect(pBuilding->Owner, adjacency, pBuilding->Type->GetFoundationWidth(), pBuilding->Type->GetFoundationHeight(false));
	return Find_Best_Building_Placement_Cell(baseArea, pBuilding, Refinery_Placement_Cell_Value, 1);
}

int BuildingExt::Near_Base_Center_Placement_Position_Value(CellStruct cell, BuildingClass* pBuilding)
{
	const HouseClass* pOwner = pBuilding->Owner;
	const CellStruct center = pOwner->Base_Center();
	return Modify_Rating_By_Allied_Building_Proximity(cell, pBuilding, cell.DistanceFrom(center));
}

int BuildingExt::Near_Base_Center_Defense_Placement_Position_Value(CellStruct cell, BuildingClass* pBuilding)
{
	const HouseClass* owner = pBuilding->Owner;
	const CellStruct center = owner->Base_Center();
	const int enemyDistance = Get_Distance_To_Primary_Enemy(cell, pBuilding->Owner);
	return Modify_Rating_By_Allied_Building_Proximity(cell, pBuilding, static_cast<int>(cell.DistanceFrom(center) * 100) + enemyDistance);
}

int BuildingExt::Near_Enemy_Placement_Position_Value(CellStruct cell, BuildingClass* pBuilding)
{
	const HouseClass* pOwner = pBuilding->Owner;
	const HouseClass* enemy = nullptr;

	if (pOwner->EnemyHouseIndex != -1)
	{
		enemy = HouseClass::FindByCountryIndex(pOwner->EnemyHouseIndex);
	}

	// If we have no enemy, then place it as close to the center of the map as possible.
	// Most commonly we are on the edge of a map, so if we place towards the center,
	// it doesn't go terribly wrong.
	if (enemy == nullptr)
	{
		const Point2D mapCenter = MapClass::Instance->VisibleRect.Center_Point();
		const CellStruct mapCenterCell = CellStruct(static_cast<short>(mapCenter.X), static_cast<short>(mapCenter.Y));
		return static_cast<int>(cell.DistanceFrom(mapCenterCell));
	}

	return Modify_Rating_By_Allied_Building_Proximity(cell, pBuilding, static_cast<int>(cell.DistanceFrom(enemy->Base_Center())));
}


int BuildingExt::Near_Refinery_Placement_Position_Value(CellStruct cell, BuildingClass* pBuilding)
{
	const BuildingClass* pRefinery = nullptr;
	for (int i = 0; i < ExtData::OurBuildingCount; i++)
	{
		if (ExtData::OurBuildings[i]->Type->Refinery)
		{
			pRefinery = ExtData::OurBuildings[i];
			break;
		}
	}

	CellStruct refineryCell;
	if (pRefinery != nullptr)
	{
		refineryCell = GeneralUtils::CellFromCoordinates(pRefinery->GetCenterCoords());
	}
	else
	{
		// Fallback
		const Point2D mapCenter = MapClass::Instance->VisibleRect.Center_Point();
		const CellStruct mapCenterCell = CellStruct(static_cast<short>(mapCenter.X), static_cast<short>(mapCenter.Y));
		refineryCell = mapCenterCell;
	}

	// Secondarily, consider distance to primary enemy.
	HouseClass* pOwner = pBuilding->Owner;
	const int enemyDistance = Get_Distance_To_Primary_Enemy(cell, pOwner);

	return Modify_Rating_By_Allied_Building_Proximity(cell, pBuilding, static_cast<int>(cell.DistanceFrom(refineryCell) * 100) + enemyDistance);
}

int BuildingExt::Near_ConYard_Placement_Position_Value(CellStruct cell, BuildingClass* pBuilding)
{
	CellStruct conyardCell;
	if (pBuilding->Owner->ConYards.Count > 0)
	{
		conyardCell = GeneralUtils::CellFromCoordinates(pBuilding->Owner->ConYards[0]->GetCenterCoords());
	}
	else
	{
		// Fallback
		const Point2D mapCenter = MapClass::Instance->VisibleRect.Center_Point();
		const CellStruct mapCenterCell = CellStruct(static_cast<short>(mapCenter.X), static_cast<short>(mapCenter.Y));
		conyardCell = mapCenterCell;
	}

	// Secondarily, consider distance to primary enemy.
	HouseClass* owner = pBuilding->Owner;
	const int enemyDistance = Get_Distance_To_Primary_Enemy(cell, owner);

	return Modify_Rating_By_Allied_Building_Proximity(cell, pBuilding, static_cast<int>(cell.DistanceFrom(conyardCell) * 100) + enemyDistance);
}

int BuildingExt::Far_From_Enemy_Placement_Position_Value(CellStruct cell, BuildingClass* pBuilding)
{
	const HouseClass* pOwner = pBuilding->Owner;
	const HouseClass* pEnemy = nullptr;

	if (pOwner->EnemyHouseIndex != -1)
	{
		pEnemy = HouseClass::FindByCountryIndex(pOwner->EnemyHouseIndex);
	}

	// If we have no enemy, then just place it near the base center.
	if (pEnemy == nullptr)
	{
		const CellStruct center = pOwner->Base_Center();
		return static_cast<int>(cell.DistanceFrom(center));
	}

	return SHRT_MAX - static_cast<int>(cell.DistanceFrom(pEnemy->Base_Center()));
}

CellStruct BuildingExt::Get_Best_SuperWeapon_Building_Placement_Position(BuildingClass* pBuilding)
{
	const int adjacency = pBuilding->Type->Adjacent + 1;
	const RectangleStruct baseArea = Get_Base_Rect(pBuilding->Owner, adjacency, pBuilding->Type->GetFoundationWidth(), pBuilding->Type->GetFoundationHeight(false));
	return Find_Best_Building_Placement_Cell(baseArea, pBuilding, Far_From_Enemy_Placement_Position_Value);
}

int BuildingExt::Towards_Expansion_Placement_Cell_Value(CellStruct cell, BuildingClass* pBuilding)
{
	const HouseClass* pOwner = pBuilding->Owner;
	const auto houseExt = HouseExt::ExtMap.Find(pOwner);

	// If we have nowhere to expand, then just try placing it somewhere that's far from our base.
	if (houseExt->NextExpansionPointLocation.X <= 0 || houseExt->NextExpansionPointLocation.Y <= 0)
	{
		const CellStruct center = pOwner->Base_Center();
		return SHRT_MAX - static_cast<int>(cell.DistanceFrom(center));
	}

	HouseClass* pEnemy = nullptr;

	if (pOwner->EnemyHouseIndex != -1)
	{
		pEnemy = HouseClass::FindByCountryIndex(pOwner->EnemyHouseIndex);
	}

	int enemyDistance = 0;
	if (pEnemy != nullptr && pEnemy->ConYards.Count > 0)
	{
		enemyDistance = static_cast<int>(cell.DistanceFrom(pEnemy->ConYards[0]->GetMapCoords()));
	}

	// Otherwise, we can basically make the value equal to the distance
	// that the building has to our next expansion point.
	// Also, secondarily take distance into enemy into account.
	return static_cast<int>(cell.DistanceFrom(houseExt->NextExpansionPointLocation) * 100) + enemyDistance;
}

CellStruct BuildingExt::Get_Best_Expansion_Placement_Position(BuildingClass* pBuilding)
{
	const int adjacency = pBuilding->Type->Adjacent + 1 + 1; // allow cheating in adjacency by 1 cell
	const RectangleStruct baseArea = Get_Base_Rect(pBuilding->Owner, adjacency, pBuilding->Type->GetFoundationWidth(), pBuilding->Type->GetFoundationHeight(false));

	const HouseClass* pOwner = pBuilding->Owner;
	const auto houseExt = HouseExt::ExtMap.Find(pOwner);

	CellStruct bestCell = Find_Best_Building_Placement_Cell(baseArea, pBuilding, Towards_Expansion_Placement_Cell_Value, 0);

	// Fetch an expansion cell with adjacency bonus.
	// This makes the algorithm much heavier, but allows the AI to jump over some gaps that it otherwise
	// could not. Should make it able to hop over cliffs in a more reliable way.
	const CellStruct altBestCell = Find_Best_Building_Placement_Cell(baseArea, pBuilding, Towards_Expansion_Placement_Cell_Value, 1);

	// Use the "adjacency cheat" if it resulted in a bigger difference in distance than 1 cell.
	if (bestCell.DistanceFrom(altBestCell) > 1)
	{
		bestCell = altBestCell;
	}

	if (houseExt->NextExpansionPointLocation.X > 0 &&
		houseExt->NextExpansionPointLocation.Y > 0 &&
		!houseExt->ShouldBuildRefinery)
	{

		// If we can't get closer to the expansion point with this building,
		// then we are as close to the expansion point as possible and should build a refinery
		// as our next building.

		// To perform this check, fetch the nearest distance any of our buildings has to the expansion point,
		// and perform a comparison to the best cell.

		double nearestDistance = std::numeric_limits<double>::max();

		for (size_t i = 0; i < ExtData::OurBuildingCount; i++)
		{
			const BuildingClass* pOtherBuilding = ExtData::OurBuildings[i];

			const double distance = houseExt->NextExpansionPointLocation.DistanceFrom(pOtherBuilding->GetMapCoords());
			if (distance < nearestDistance)
			{
				nearestDistance = distance;
			}
		}

		const double newDistance = houseExt->NextExpansionPointLocation.DistanceFrom(bestCell);
		if (newDistance >= nearestDistance)
		{
			houseExt->ShouldBuildRefinery = true;
		}
	}

	return bestCell;
}

int BuildingExt::Barracks_Placement_Cell_Value(CellStruct cell, BuildingClass* pBuilding)
{
	// A barracks is best built close to the opponent.
	const HouseClass* pOwner = pBuilding->Owner;
	const auto houseExt = HouseExt::ExtMap.Find(pOwner);

	double expandDistance = 0;
	// If we are expanding, consider distance to expansion location as barracks are great for expanding.
	if (houseExt->NextExpansionPointLocation.X > 0 && houseExt->NextExpansionPointLocation.Y > 0)
	{
		expandDistance = cell.DistanceFrom(houseExt->NextExpansionPointLocation);
		expandDistance *= 3; // Give expansion distance more weight than distance to enemy
	}

	const HouseClass* pEnemy = nullptr;

	if (pOwner->EnemyHouseIndex != -1)
	{
		pEnemy = HouseClass::FindByCountryIndex(pOwner->EnemyHouseIndex);
	}

	if (pEnemy != nullptr)
	{
		return static_cast<int>(cell.DistanceFrom(pEnemy->Base_Center()) + expandDistance);
	}

	// If we do not have an opponent, then just consider expansion.
	if (expandDistance > 0)
	{
		return static_cast<int>(expandDistance);
	}

	// If we do not have an opponent AND do not expand, just place it somewhere on our base outskirts.
	const CellStruct center = pOwner->Base_Center();
	return Modify_Rating_By_Allied_Building_Proximity(cell, pBuilding, SHRT_MAX - static_cast<int>(cell.DistanceFrom(center)));

	// TODO: Distance to other barracks of our house could be a good factor too.
	// It would require us to go through all buildings though... which might give a significant perf hit.
	// Maybe a static list of barracks so we could only fetch it once?
}

int BuildingExt::NavalYard_Placement_Cell_Value(CellStruct cell, BuildingClass* pBuilding)
{
	const HouseClass* pOwner = pBuilding->Owner;
	const HouseClass* pEnemy = nullptr;

	if (pOwner->EnemyHouseIndex != -1)
	{
		pEnemy = HouseClass::FindByCountryIndex(pOwner->EnemyHouseIndex);
	}

	// If we have no enemy, then just place it away from the base center.
	if (pEnemy == nullptr)
	{
		const CellStruct center = pOwner->Base_Center();
		return SHRT_MAX - static_cast<int>(cell.DistanceFrom(center));
	}

	return SHRT_MAX - static_cast<int>(cell.DistanceFrom(pEnemy->Base_Center()));
}

int BuildingExt::WarFactory_Placement_Cell_Value(CellStruct cell, BuildingClass* pBuilding)
{
	// War factories are typically valuable.
	// It might be best to place them not close to the enemy, but around our base center so they're safe.

	return Near_Base_Center_Placement_Position_Value(cell, pBuilding);
}

int BuildingExt::Helipad_Placement_Cell_Value(CellStruct cell, BuildingClass* pBuilding)
{
	// Helipads don't need to be very close to the enemy.
	// Place them as far from the enemy as possible.
	return Far_From_Enemy_Placement_Position_Value(cell, pBuilding);
}

/**
 *  Calculates the best factory placement location.
 */
CellStruct BuildingExt::Get_Best_Factory_Placement_Position(BuildingClass* pBuilding)
{
	const bool isNaval = pBuilding->Type->Factory == AbstractType::UnitType && pBuilding->Type->Naval;

	const int adjacency = pBuilding->Type->Adjacent + 1;

	const RectangleStruct baseArea = Get_Base_Rect(pBuilding->Owner, adjacency, pBuilding->Type->GetFoundationWidth(), pBuilding->Type->GetFoundationHeight(false));

	if (pBuilding->Type->Factory == AbstractType::InfantryType)
	{
		return Find_Best_Building_Placement_Cell(baseArea, pBuilding, Barracks_Placement_Cell_Value);
	}

	if (pBuilding->Type->Factory == AbstractType::UnitType)
	{
		if (isNaval)
		{
			return Find_Best_Building_Placement_Cell(baseArea, pBuilding, NavalYard_Placement_Cell_Value);
		}

		return Find_Best_Building_Placement_Cell(baseArea, pBuilding, WarFactory_Placement_Cell_Value);
	}

	if (pBuilding->Type->Factory == AbstractType::AircraftType)
	{
		return Find_Best_Building_Placement_Cell(baseArea, pBuilding, Helipad_Placement_Cell_Value);
	}

	return Find_Best_Building_Placement_Cell(baseArea, pBuilding, Near_Base_Center_Placement_Position_Value);
}

CellStruct BuildingExt::ExtData::AttackCell;

int BuildingExt::Near_AttackCell_Cell_Value(CellStruct cell, BuildingClass* pBuilding)
{
	return Modify_Rating_By_Allied_Building_Proximity(cell, pBuilding, static_cast<int>(cell.DistanceFrom(ExtData::AttackCell)));
}

CellStruct BuildingExt::Get_Best_Defense_Placement_Position(BuildingClass* pBuilding)
{
	const HouseClass* pOwner = pBuilding->Owner;
	const auto houseExt = HouseExt::ExtMap.Find(pOwner);

	ExtData::AttackCell = CellStruct(0, 0);

	const int adjacency = pBuilding->Type->Adjacent + 1;
	const RectangleStruct baseArea = Get_Base_Rect(pBuilding->Owner, adjacency, pBuilding->Type->GetFoundationWidth(), pBuilding->Type->GetFoundationHeight(false));

	// If we were attacked recently, then place the defense near a damaged building of ours if one exists.
	if (pOwner->LATime + TICKS_PER_MINUTE > Unsorted::CurrentFrame)
	{
		for (const auto pOtherBuilding : *BuildingClass::Array)
		{
			if (!pOtherBuilding->IsAlive ||
				pOtherBuilding->InLimbo ||
				pOtherBuilding->Type->InvisibleInGame ||
				pOtherBuilding->Owner != pOwner)
			{
				continue;
			}

			if (pOtherBuilding->Health < pOtherBuilding->Type->Strength)
			{
				ExtData::AttackCell = pOtherBuilding->GetMapCoords();
				break;
			}
		}
	}

	if (ExtData::AttackCell.X > 0 && ExtData::AttackCell.Y > 0)
	{
		return Find_Best_Building_Placement_Cell(baseArea, pBuilding, Near_AttackCell_Cell_Value);
	}

	// Special behaviour if we are under danger of getting rushed.
	// Defend our ConYard and refinery.
	if (houseExt->IsUnderStartRushThreat)
	{
		if (pOwner->ActiveBuildingTypes.GetItemCount(pBuilding->Type->ArrayIndex) < 3)
		{
			return Find_Best_Building_Placement_Cell(baseArea, pBuilding, Near_ConYard_Placement_Position_Value);
		}

		return Find_Best_Building_Placement_Cell(baseArea, pBuilding, Near_Refinery_Placement_Position_Value);
	}

	// If we are expanding, then it's likely we should build defenses towards the expansion node.
	// However, only so if we are not under immediate threat.
	const bool percentChance50 = ScenarioClass::Instance->Random.RandomRanged(0, 99) < 50;
	if (houseExt->NextExpansionPointLocation.X > 0 && houseExt->NextExpansionPointLocation.Y > 0 && percentChance50)
	{
		return Find_Best_Building_Placement_Cell(baseArea, pBuilding, Towards_Expansion_Placement_Cell_Value);
	}

	const HouseClass* pEnemy = nullptr;
	if (pOwner->EnemyHouseIndex != -1)
	{
		pEnemy = HouseClass::FindByCountryIndex(pOwner->EnemyHouseIndex);
	}

	// Place some defenses to the backline.
	const bool percentChance20 = ScenarioClass::Instance->Random.RandomRanged(0, 99) < 20;
	if (percentChance20)
	{
		return Find_Best_Building_Placement_Cell(baseArea, pBuilding, Near_ConYard_Placement_Position_Value);
	}

	// If we have no designed enemy, look for one.
	if (pEnemy == nullptr)
	{
		pEnemy = Find_Closest_Opponent(pOwner);
	}

	// Place some defenses around the center of our base to defend against cheese and flank attacks.
	const bool percentChance30 = ScenarioClass::Instance->Random.RandomRanged(0, 99) < 20;
	if (pEnemy == nullptr || percentChance30)
	{
		return Find_Best_Building_Placement_Cell(baseArea, pBuilding, Near_Base_Center_Defense_Placement_Position_Value);
	}

	return Find_Best_Building_Placement_Cell(baseArea, pBuilding, Near_Enemy_Placement_Position_Value);
}

CellStruct BuildingExt::Get_Best_Sensor_Placement_Position(BuildingClass* pBuilding)
{
	const int adjacency = pBuilding->Type->Adjacent + 1;
	const RectangleStruct baseArea = Get_Base_Rect(pBuilding->Owner, adjacency, pBuilding->Type->GetFoundationWidth(), pBuilding->Type->GetFoundationHeight(false));
	return Find_Best_Building_Placement_Cell(baseArea, pBuilding, Near_Base_Center_Placement_Position_Value);
}

CellStruct BuildingExt::Get_Best_Placement_Position(BuildingClass* pBuilding)
{
	if (pBuilding->Type->ResourceDestination)
	{
		return Get_Best_Refinery_Placement_Position(pBuilding);
	}

	if (pBuilding->Type->SuperWeapon != static_cast<int>(SpecialWeaponType::None) || pBuilding->Type->SuperWeapon2 != static_cast<int>(SpecialWeaponType::None))
	{
		return Get_Best_SuperWeapon_Building_Placement_Position(pBuilding);
	}

	if (pBuilding->Type->Factory != AbstractType::None)
	{
		return Get_Best_Factory_Placement_Position(pBuilding);
	}

	if (pBuilding->Type->GetWeapon(static_cast<size_t>(WeaponSlotType::Primary), false).WeaponType != nullptr)
	{
		return Get_Best_Defense_Placement_Position(pBuilding);
	}

	if (pBuilding->Type->SensorArray)
	{
		return Get_Best_Sensor_Placement_Position(pBuilding);
	}

	return Get_Best_Expansion_Placement_Position(pBuilding);
}

int BuildingExt::Exit_Object_Custom_Position(BuildingClass* pBuilding)
{
	const HouseClass* owner = pBuilding->Owner;
	ExtData::OurBuildingCount = 0;

	for (const auto pOtherBuilding : *BuildingClass::Array)
	{
		if (!pOtherBuilding->IsAlive ||
			pOtherBuilding->InLimbo ||
			pOtherBuilding->Type->InvisibleInGame ||
			pOtherBuilding->Owner != owner)
		{
			continue;
		}

		ExtData::OurBuildings[ExtData::OurBuildingCount] = pOtherBuilding;
		ExtData::OurBuildingCount++;

		if (ExtData::OurBuildingCount >= std::size(ExtData::OurBuildings))
		{
			break;
		}
	}

	const CellStruct placementCell = Get_Best_Placement_Position(pBuilding);

	// If we couldn't find any place for the building, refund it.
	if (placementCell.X <= 0 || placementCell.Y <= 0)
	{
		return 0;
	}

	return Try_Place(pBuilding, placementCell);
}
