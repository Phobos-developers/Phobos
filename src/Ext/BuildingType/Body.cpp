#include "Body.h"

#include <EventClass.h>
#include <TacticalClass.h>
#include <TunnelLocomotionClass.h>

#include <Utilities/GeneralUtils.h>
#include <Ext/House/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Scenario/Body.h>

BuildingTypeExt::ExtContainer BuildingTypeExt::ExtMap;

// Assuming SuperWeapon & SuperWeapon2 are used (for the moment)
int BuildingTypeExt::ExtData::GetSuperWeaponCount() const
{
	// The user should only use SuperWeapon and SuperWeapon2 if the attached sw count isn't bigger than 2
	return 2 + this->SuperWeapons.size();
}

int BuildingTypeExt::ExtData::GetSuperWeaponIndex(const int index, HouseClass* pHouse) const
{
	auto idxSW = this->GetSuperWeaponIndex(index);

	if (auto pSuper = pHouse->Supers.GetItemOrDefault(idxSW))
	{
		auto pExt = SWTypeExt::ExtMap.Find(pSuper->Type);

		if (!pExt->IsAvailable(pHouse))
			return -1;
	}

	return idxSW;
}

int BuildingTypeExt::ExtData::GetSuperWeaponIndex(const int index) const
{
	const auto pThis = this->OwnerObject();

	// 2 = SuperWeapon & SuperWeapon2
	if (index < 2)
		return !index ? pThis->SuperWeapon : pThis->SuperWeapon2;
	else if (index - 2 < (int)this->SuperWeapons.size())
		return this->SuperWeapons[index - 2];

	return -1;
}

int BuildingTypeExt::GetEnhancedPower(BuildingClass* pBuilding, HouseClass* pHouse)
{
	int nAmount = 0;
	float fFactor = 1.0f;

	auto const pHouseExt = HouseExt::ExtMap.Find(pHouse);

	for (const auto& [pExt, nCount] : pHouseExt->PowerPlantEnhancers)
	{
		if (pExt->PowerPlantEnhancer_Buildings.Contains(pBuilding->Type))
		{
			fFactor *= std::powf(pExt->PowerPlantEnhancer_Factor, static_cast<float>(nCount));
			nAmount += pExt->PowerPlantEnhancer_Amount * nCount;
		}
	}

	return static_cast<int>(std::round(pBuilding->GetPowerOutput() * fFactor)) + nAmount;
}

int BuildingTypeExt::GetUpgradesAmount(BuildingTypeClass* pBuilding, HouseClass* pHouse) // not including producing upgrades
{
	int result = 0;
	bool isUpgrade = false;
	auto pPowersUp = pBuilding->PowersUpBuilding;

	auto checkUpgrade = [pHouse, pBuilding, &result, &isUpgrade](BuildingTypeClass* pTPowersUp)
	{
		isUpgrade = true;
		for (auto const& pBld : pHouse->Buildings)
		{
			if (pBld->Type == pTPowersUp)
			{
				for (auto const& pUpgrade : pBld->Upgrades)
				{
					if (pUpgrade == pBuilding)
						++result;
				}
			}
		}
	};

	if (pPowersUp[0])
	{
		if (auto const pTPowersUp = BuildingTypeClass::Find(pPowersUp))
			checkUpgrade(pTPowersUp);
	}

	if (auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pBuilding))
	{
		for (auto pTPowersUp : pBuildingExt->PowersUp_Buildings)
			checkUpgrade(pTPowersUp);
	}

	return isUpgrade ? result : -1;
}

bool BuildingTypeExt::ShouldExistGreyCameo(const HouseClass* const pHouse, const TechnoTypeClass* const pType, const TechnoTypeClass* const pPreType)
{
	const int techLevel = pType->TechLevel;

	if (techLevel <= 0 || techLevel > Game::TechLevel)
		return false;

	if (!pHouse->InOwners(pType))
		return false;

	if (!pHouse->InRequiredHouses(pType))
		return false;

	if (pHouse->InForbiddenHouses(pType))
		return false;

	if (!pPreType)
	{
		const int sideIndex = pType->AIBasePlanningSide;

		return (sideIndex == -1 || sideIndex == pHouse->Type->SideIndex);
	}

	if (pHouse->CountOwnedAndPresent(pPreType))
		return true;

	TechnoTypeExt::ExtData* const pPreTypeExt = TechnoTypeExt::ExtMap.Find(pPreType);

	if (pPreTypeExt->CameoCheckMutex)
		return false;

	pPreTypeExt->CameoCheckMutex = true;
	const bool exist = BuildingTypeExt::ShouldExistGreyCameo(pHouse, pPreType, pPreTypeExt->PrerequisiteForCameo);
	pPreTypeExt->CameoCheckMutex = false;

	return exist;
}

// Check the cameo change
CanBuildResult BuildingTypeExt::CheckAlwaysExistCameo(const HouseClass* const pHouse, const TechnoTypeClass* const pType, CanBuildResult canBuild)
{
	TechnoTypeExt::ExtData* const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->AlwaysExistTheCameo.Get(RulesExt::Global()->AlwaysExistTheCameo))
	{
		auto& vec = HouseExt::ExtMap.Find(HouseClass::CurrentPlayer)->OwnedExistCameoTechnoTypes;

		if (canBuild == CanBuildResult::Unbuildable) // Unbuildable + Satisfy basic limitations = Change it to TemporarilyUnbuildable
		{
			pTypeExt->CameoCheckMutex = true;
			const bool exist = BuildingTypeExt::ShouldExistGreyCameo(pHouse, pType, pTypeExt->PrerequisiteForCameo);
			pTypeExt->CameoCheckMutex = false;

			if (exist)
			{
				if (std::find(vec.begin(), vec.end(), pTypeExt) == vec.end()) // … + Not in the list = Need to add it into list
				{
					vec.push_back(pTypeExt);
					SidebarClass::Instance->SidebarNeedsRepaint();
					EventClass event
					(
						pHouse->ArrayIndex,
						EventType::AbandonAll,
						(int)pType->WhatAmI(),
						pType->GetArrayIndex(),
						pType->Naval
					);
					EventClass::AddEvent(event);
				}

				canBuild = CanBuildResult::TemporarilyUnbuildable;
			}
		}
		else if (std::find(vec.begin(), vec.end(), pTypeExt) != vec.end()) // Not Unbuildable + In the list = remove it from the list and play EVA
		{
			vec.erase(std::remove(vec.begin(), vec.end(), pTypeExt), vec.end());
			SidebarClass::Instance->SidebarNeedsRepaint();

			if (pHouse->IsControlledByCurrentPlayer())
				VoxClass::Play(&Make_Global<const char>(0x83FA64)); // 0x83FA64 -> EVA_NewConstructionOptions
		}
	}

	return canBuild;
}

// Check whether can call the occupiers leave
bool BuildingTypeExt::CheckOccupierCanLeave(HouseClass* pBuildingHouse, HouseClass* pOccupierHouse)
{
	if (!pOccupierHouse)
		return false;
	else if (pBuildingHouse == pOccupierHouse)
		return true;
	else if (SessionClass::Instance->GameMode == GameMode::Campaign && pOccupierHouse->IsInPlayerControl)
		return true;
	else if (!pOccupierHouse->IsControlledByHuman() && pOccupierHouse->IsAlliedWith(pBuildingHouse))
		return true;

	return false;
}

// Force occupiers leave, return: whether it should stop right now
bool BuildingTypeExt::CleanUpBuildingSpace(BuildingTypeClass* pBuildingType, CellStruct topLeftCell, HouseClass* pHouse, TechnoClass* pExceptTechno)
{
	// Step 1: Find the technos inside of the building place grid.
	CellStruct infantryCount { 0, 0 };
	std::vector<TechnoClass*> checkedTechnos;
	checkedTechnos.reserve(24);
	std::vector<CellClass*> checkedCells;
	checkedCells.reserve(24);

	for (auto pFoundation = pBuildingType->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
	{
		CellStruct currentCell = topLeftCell + *pFoundation;

		if (CellClass* const pCell = MapClass::Instance->GetCellAt(currentCell))
		{
			ObjectClass* pObject = pCell->FirstObject;

			while (pObject)
			{
				AbstractType const absType = pObject->WhatAmI();

				if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
				{
					TechnoClass* const pCellTechno = static_cast<TechnoClass*>(pObject);
					auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pCellTechno->GetTechnoType());

					if ((!pTypeExt || !pTypeExt->CanBeBuiltOn) && pCellTechno != pExceptTechno) // No need to check house
					{
						const FootClass* pFoot = static_cast<FootClass*>(pCellTechno);

						if (pFoot->GetCurrentSpeed() <= 0 || (locomotion_cast<TunnelLocomotionClass*>(pFoot->Locomotor) && !pFoot->Locomotor->Is_Moving()))
						{
							if (absType == AbstractType::Infantry)
								++infantryCount.X;

							checkedTechnos.push_back(pCellTechno);
						}
					}
				}

				pObject = pObject->NextObject;
			}

			checkedCells.push_back(pCell);
		}
	}

	if (checkedTechnos.size() <= 0) // All in moving
		return false;

	// Step 2: Find the cells around the building.
	std::vector<CellClass*> optionalCells;
	optionalCells.reserve(24);

	for (auto pFoundation = pBuildingType->FoundationOutside; *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
	{
		CellStruct searchCell = topLeftCell + *pFoundation;

		if (CellClass* const pSearchCell = MapClass::Instance->GetCellAt(searchCell))
		{
			if (std::find(checkedCells.begin(), checkedCells.end(), pSearchCell) == checkedCells.end() // TODO If there is a cellflag that can be used …
				&& !pSearchCell->GetBuilding()
				&& pSearchCell->IsClearToMove(SpeedType::Amphibious, true, true, -1, MovementZone::Amphibious, -1, false))
			{
				optionalCells.push_back(pSearchCell);
			}
		}
	}

	if (optionalCells.size() <= 0) // There is no place for scattering
		return true;

	// Step 3: Sort the technos by the distance out of the foundation.
	std::sort(&checkedTechnos[0], &checkedTechnos[checkedTechnos.size()],[optionalCells](TechnoClass* pTechnoA, TechnoClass* pTechnoB)
	{
		int minA = INT_MAX;
		int minB = INT_MAX;

		for (auto const& pOptionalCell : optionalCells) // If there are many valid cells at start, it means most of occupiers will near to the edge
		{
			if (minA > 65536) // If distance squared is lower or equal to 256^2, then no need to calculate any more because it is on the edge
			{
				int curA = static_cast<int>(pTechnoA->GetMapCoords().DistanceFromSquared(pOptionalCell->MapCoords));

				if (curA < minA)
					minA = curA;
			}

			if (minB > 65536)
			{
				int curB = static_cast<int>(pTechnoB->GetMapCoords().DistanceFromSquared(pOptionalCell->MapCoords));

				if (curB < minB)
					minB = curB;
			}
		}

		return minA > minB;
	});

	// Step 4: Core, successively find the farthest techno and its closest valid destination.
	std::vector<TechnoClass*> reCheckedTechnos;
	reCheckedTechnos.reserve(12);

	struct InfantryCountInCell // Temporary struct
	{
		CellClass* position;
		int count;
	};
	std::vector<InfantryCountInCell> infantryCells;
	infantryCells.reserve(4);

	struct TechnoWithDestination // Also temporary struct
	{
		TechnoClass* techno;
		CellClass* destination;
	};
	std::vector<TechnoWithDestination> finalOrder;
	finalOrder.reserve(24);

	do
	{
		// Step 4.1: Push the technos discovered just now back to the vector.
		for (auto const& pRecheckedTechno : reCheckedTechnos)
		{
			if (pRecheckedTechno->WhatAmI() == AbstractType::Infantry)
				++infantryCount.X;

			checkedTechnos.push_back(pRecheckedTechno);
		}

		reCheckedTechnos.clear();

		// Step 4.2: Check the techno vector.
		for (auto const& pCheckedTechno : checkedTechnos)
		{
			CellClass* pDestinationCell = nullptr;

			// Step 4.2.1: Search the closest valid cell to be the destination.
			do
			{
				const CellStruct location = pCheckedTechno->GetMapCoords();
				const bool isInfantry = pCheckedTechno->WhatAmI() == AbstractType::Infantry;
				TechnoTypeClass* const pCheckedType = pCheckedTechno->GetTechnoType();

				if (isInfantry) // Try to maximizing cells utilization
				{
					if (infantryCells.size() && infantryCount.Y >= (infantryCount.X / 3 + (infantryCount.X % 3 ? 1 : 0)))
					{
						std::sort(&infantryCells[0], &infantryCells[infantryCells.size()],[location](InfantryCountInCell cellA, InfantryCountInCell cellB){
							return cellA.position->MapCoords.DistanceFromSquared(location) < cellB.position->MapCoords.DistanceFromSquared(location);
						});

						for (auto& infantryCell : infantryCells)
						{
							if (static_cast<InfantryClass*>(pCheckedTechno)->Destination == infantryCell.position)
							{
								infantryCell.count = 3;
							}
							else if (infantryCell.count < 3 && infantryCell.position->IsClearToMove(pCheckedType->SpeedType, true, true, -1, pCheckedType->MovementZone, -1, false))
							{
								pDestinationCell = infantryCell.position;
								++infantryCell.count;

								break;
							}
						}

						if (pDestinationCell)
							break; // Complete
					}
				}

				std::sort(&optionalCells[0], &optionalCells[optionalCells.size()],[location](CellClass* pCellA, CellClass* pCellB){
					return pCellA->MapCoords.DistanceFromSquared(location) < pCellB->MapCoords.DistanceFromSquared(location);
				});
				const double minDistanceSquared = optionalCells[0]->MapCoords.DistanceFromSquared(location);

				for (auto const& pOptionalCell : optionalCells) // Prioritize selecting empty cells
				{
					if (!pOptionalCell->FirstObject && pOptionalCell->IsClearToMove(pCheckedType->SpeedType, true, true, -1, pCheckedType->MovementZone, -1, false))
					{
						if (isInfantry) // Not need to remove it now
						{
							infantryCells.push_back(InfantryCountInCell{ pOptionalCell, 1 });
							++infantryCount.Y;
						}

						if (pOptionalCell->MapCoords.DistanceFromSquared(location) < (minDistanceSquared * 4)) // Empty cell is not too far
							pDestinationCell = pOptionalCell;

						break;
					}
				}

				if (!pDestinationCell)
				{
					std::vector<CellClass*> deleteCells;
					deleteCells.reserve(8);

					for (auto const& pOptionalCell : optionalCells)
					{
						ObjectClass* pCurObject = pOptionalCell->FirstObject;
						std::vector<TechnoClass*> optionalTechnos;
						optionalTechnos.reserve(4);
						bool valid = true;

						while (pCurObject)
						{
							AbstractType const absType = pCurObject->WhatAmI();

							if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
							{
								TechnoClass* const pCurTechno = static_cast<TechnoClass*>(pCurObject);

								if (!BuildingTypeExt::CheckOccupierCanLeave(pHouse, pCurTechno->Owner)) // Means invalid for all
								{
									deleteCells.push_back(pOptionalCell);
									valid = false;
									break;
								}

								optionalTechnos.push_back(pCurTechno);
							}

							pCurObject = pCurObject->NextObject;
						}

						if (valid && pOptionalCell->IsClearToMove(pCheckedType->SpeedType, true, true, -1, pCheckedType->MovementZone, -1, false))
						{
							for (auto const& pOptionalTechno : optionalTechnos)
							{
								reCheckedTechnos.push_back(pOptionalTechno);
							}

							if (isInfantry) // Not need to remove it now
							{
								infantryCells.push_back(InfantryCountInCell{ pOptionalCell, 1 });
								++infantryCount.Y;
							}

							pDestinationCell = pOptionalCell;
							break;
						}
					}

					for (auto const& pDeleteCell : deleteCells) // Mark the invalid cells
					{
						checkedCells.push_back(pDeleteCell);
						optionalCells.erase(std::remove(optionalCells.begin(), optionalCells.end(), pDeleteCell), optionalCells.end());
					}
				}
			}
			while (false);

			// Step 4.2.2: Mark the cell and push back its surrounded cells, then prepare for the command.
			if (pDestinationCell)
			{
				if (std::find(checkedCells.begin(), checkedCells.end(), pDestinationCell) == checkedCells.end())
					checkedCells.push_back(pDestinationCell);

				if (std::find(optionalCells.begin(), optionalCells.end(), pDestinationCell) != optionalCells.end())
				{
					optionalCells.erase(std::remove(optionalCells.begin(), optionalCells.end(), pDestinationCell), optionalCells.end());
					CellStruct searchCell = pDestinationCell->MapCoords - CellStruct { 1, 1 };

					for (int i = 0; i < 4; ++i)
					{
						for (int j = 0; j < 2; ++j)
						{
							if (CellClass* const pSearchCell = MapClass::Instance->GetCellAt(searchCell))
							{
								if (std::find(checkedCells.begin(), checkedCells.end(), pSearchCell) == checkedCells.end()
									&& std::find(optionalCells.begin(), optionalCells.end(), pSearchCell) == optionalCells.end()
									&& !pSearchCell->GetBuilding()
									&& pSearchCell->IsClearToMove(SpeedType::Amphibious, true, true, -1, MovementZone::Amphibious, -1, false))
								{
									optionalCells.push_back(pSearchCell);
								}
							}

							if (i % 2)
								searchCell.Y += static_cast<short>((i / 2) ? -1 : 1);
							else
								searchCell.X += static_cast<short>((i / 2) ? -1 : 1);
						}
					}
				}

				const TechnoWithDestination thisOrder { pCheckedTechno, pDestinationCell };
				finalOrder.push_back(thisOrder);
			}
			else // Can not build
			{
				return true;
			}
		}

		checkedTechnos.clear();
	}
	while (reCheckedTechnos.size());

	// Step 5: Confirm command execution.
	for (auto const& pThisOrder : finalOrder)
	{
		TechnoClass* const pCheckedTechno = pThisOrder.techno;
		CellClass* const pDestinationCell = pThisOrder.destination;
		AbstractType const absType = pCheckedTechno->WhatAmI();
		pCheckedTechno->ForceMission(Mission::Guard);

		if (absType == AbstractType::Infantry)
		{
			InfantryClass* const pInfantry = static_cast<InfantryClass*>(pCheckedTechno);

			if (pInfantry->IsDeployed())
				pInfantry->PlayAnim(Sequence::Undeploy, true);

			pInfantry->SetDestination(pDestinationCell, false);
			pInfantry->QueueMission(Mission::QMove, false); // To force every three infantries gather together, it should be QMove
		}
		else if (absType == AbstractType::Unit)
		{
			UnitClass* const pUnit = static_cast<UnitClass*>(pCheckedTechno);

			if (pUnit->Deployed)
				pUnit->Undeploy();

			pUnit->SetDestination(pDestinationCell, false);
			pUnit->QueueMission(Mission::Move, false);
		}
	}

	return false;
}

void BuildingTypeExt::DrawAdjacentLines()
{
	BuildingClass* const pBuilding = abstract_cast<BuildingClass*>(DisplayClass::Instance->CurrentBuilding);

	if (!pBuilding)
		return;

	BuildingTypeClass* const pType = pBuilding->Type;
	const short adjacent = static_cast<short>(pType->Adjacent + 1);

	if (adjacent <= 0)
		return;

	const CellStruct foundation { pType->GetFoundationWidth(), pType->GetFoundationHeight(false) };

	if (foundation == CellStruct::Empty)
		return;

	const CellStruct topLeft = DisplayClass::Instance->CurrentFoundation_CenterCell + DisplayClass::Instance->CurrentFoundation_TopLeftOffset;
	const CellStruct min { topLeft.X - adjacent, topLeft.Y - adjacent };
	const CellStruct max { topLeft.X + foundation.X + adjacent - 1, topLeft.Y + foundation.Y + adjacent - 1 };

	RectangleStruct rect = DSurface::Temp->GetRect();
	rect.Height -= 32;

	if (const CellClass* const pCell = MapClass::Instance->TryGetCellAt(min))
	{
		Point2D point = TacticalClass::Instance->CoordsToClient(CellClass::Cell2Coord(pCell->MapCoords, (1 + pCell->GetFloorHeight(Point2D::Empty)))).first;
		point.Y -= 1;
		Point2D nextPoint = point;

		point.Y -= 14;
		nextPoint.X += 29;
		DSurface::Temp->DrawLineEx(&rect, &point, &nextPoint, COLOR_WHITE);

		point.X -= 1;
		nextPoint.X -= 59;
		DSurface::Temp->DrawLineEx(&rect, &point, &nextPoint, COLOR_WHITE);
	}

	if (const CellClass* const pCell = MapClass::Instance->TryGetCellAt(CellStruct{ min.X, max.Y }))
	{
		Point2D point = TacticalClass::Instance->CoordsToClient(CellClass::Cell2Coord(pCell->MapCoords, (1 + pCell->GetFloorHeight(Point2D::Empty)))).first;
		point.X -= 1;
		Point2D nextPoint = point;

		point.X -= 29;
		nextPoint.Y += 14;
		DSurface::Temp->DrawLineEx(&rect, &point, &nextPoint, COLOR_WHITE);

		point.Y -= 1;
		nextPoint.Y -= 29;
		DSurface::Temp->DrawLineEx(&rect, &point, &nextPoint, COLOR_WHITE);
	}

	if (const CellClass* const pCell = MapClass::Instance->TryGetCellAt(max))
	{
		Point2D point = TacticalClass::Instance->CoordsToClient(CellClass::Cell2Coord(pCell->MapCoords, (1 + pCell->GetFloorHeight(Point2D::Empty)))).first;
		Point2D nextPoint = point;

		point.Y += 14;
		nextPoint.X += 29;
		DSurface::Temp->DrawLineEx(&rect, &point, &nextPoint, COLOR_WHITE);

		point.X -= 1;
		nextPoint.X -= 59;
		DSurface::Temp->DrawLineEx(&rect, &point, &nextPoint, COLOR_WHITE);
	}

	if (const CellClass* const pCell = MapClass::Instance->TryGetCellAt(CellStruct{ max.X, min.Y }))
	{
		Point2D point = TacticalClass::Instance->CoordsToClient(CellClass::Cell2Coord(pCell->MapCoords, (1 + pCell->GetFloorHeight(Point2D::Empty)))).first;
		Point2D nextPoint = point;

		point.X += 29;
		nextPoint.Y += 14;
		DSurface::Temp->DrawLineEx(&rect, &point, &nextPoint, COLOR_WHITE);

		point.Y -= 1;
		nextPoint.Y -= 29;
		DSurface::Temp->DrawLineEx(&rect, &point, &nextPoint, COLOR_WHITE);
	}
}

bool BuildingTypeExt::AutoUpgradeBuilding(BuildingClass* pBuilding)
{
	BuildingTypeClass* const pBuildingType = pBuilding->Type;

	if (BuildingTypeExt::ExtData* const pTypeExt = BuildingTypeExt::ExtMap.Find(pBuildingType))
	{
		if (pTypeExt->AutoUpgrade)
		{
			HouseClass* const pHouse = pBuilding->Owner;
			HouseExt::ExtData* const pHouseExt = HouseExt::ExtMap.Find(pHouse);
			const int size = pTypeExt->PowersUp_Buildings.size();

			std::vector<BuildingTypeClass*> upgradeBuildings;
			upgradeBuildings.reserve(size + 1);

			if (BuildingTypeClass* const pUpgrade = BuildingTypeClass::Find(pBuildingType->PowersUpBuilding))
				upgradeBuildings.push_back(pUpgrade);

			if (size)
			{
				for (auto const& pUpgrade : pTypeExt->PowersUp_Buildings)
				{
					if (pUpgrade)
						upgradeBuildings.push_back(pUpgrade);
				}
			}

			for (auto const& pOwned : pHouse->Buildings)
			{
				for (auto const& pUpgradeType : upgradeBuildings)
				{
					if (pOwned->Type == pUpgradeType && pOwned->IsAlive && pOwned->Health > 0 && pOwned->IsOnMap && !pOwned->InLimbo && pOwned->CurrentMission != Mission::Selling)
					{
						if (reinterpret_cast<bool(__thiscall*)(BuildingClass*, BuildingTypeClass*, HouseClass*)>(0x452670)(pOwned, pBuildingType, pHouse))
						{
							const CellStruct cell = pOwned->GetMapCoords();

							if (cell != CellStruct::Empty && !pHouseExt->OwnsLimboDeliveredBuilding(pOwned))
							{
								EventClass event
								(
									pHouse->ArrayIndex,
									EventType::Place,
									AbstractType::Building,
									pBuildingType->GetArrayIndex(),
									pBuildingType->Naval,
									cell
								);
								EventClass::AddEvent(event);

								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}

bool BuildingTypeExt::BuildLimboBuilding(BuildingClass* pBuilding)
{
	BuildingTypeClass* const pBuildingType = pBuilding->Type;

	if (BuildingTypeExt::ExtData* const pTypeExt = BuildingTypeExt::ExtMap.Find(pBuildingType))
	{
		if (pTypeExt->LimboBuild)
		{
			EventClass event
			(
				pBuilding->Owner->ArrayIndex,
				EventType::Place,
				AbstractType::Building,
				pBuildingType->GetArrayIndex(),
				pBuildingType->Naval,
				CellStruct { 1, 1 }
			);
			EventClass::AddEvent(event);

			return true;
		}
	}

	return false;
}

void BuildingTypeExt::CreateLimboBuilding(BuildingClass* pBuilding, BuildingTypeClass* pType, HouseClass* pOwner, int ID)
{
	if (pBuilding || (pBuilding = static_cast<BuildingClass*>(pType->CreateObject(pOwner)), pBuilding))
	{
		// All of these are mandatory
		pBuilding->InLimbo = false;
		pBuilding->IsAlive = true;
		pBuilding->IsOnMap = true;

		// For reasons beyond my comprehension, the discovery logic is checked for certain logics like power drain/output in campaign only.
		// Normally on unlimbo the buildings are revealed to current player if unshrouded or if game is a campaign and to non-player houses always.
		// Because of the unique nature of LimboDelivered buildings, this has been adjusted to always reveal to the current player in singleplayer
		// and to the owner of the building regardless, removing the shroud check from the equation since they don't physically exist - Starkku
		if (SessionClass::IsCampaign())
			pBuilding->DiscoveredBy(HouseClass::CurrentPlayer);

		pBuilding->DiscoveredBy(pOwner);

		pOwner->RegisterGain(pBuilding, false);
		pOwner->UpdatePower();
		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;
		pOwner->RecheckRadar = true;
		pOwner->Buildings.AddItem(pBuilding);

		// Different types of building logics
		if (pType->ConstructionYard)
			pOwner->ConYards.AddItem(pBuilding); // why would you do that????

		if (pType->SecretLab)
			pOwner->SecretLabs.AddItem(pBuilding);

		auto const pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);
		auto const pOwnerExt = HouseExt::ExtMap.Find(pOwner);

		if (pType->FactoryPlant)
		{
			if (pBuildingExt->TypeExtData->FactoryPlant_AllowTypes.size() > 0 || pBuildingExt->TypeExtData->FactoryPlant_DisallowTypes.size() > 0)
			{
				pOwnerExt->RestrictedFactoryPlants.push_back(pBuilding);
			}
			else
			{
				pOwner->FactoryPlants.AddItem(pBuilding);
				pOwner->CalculateCostMultipliers();
			}
		}

		// BuildingClass::Place is already called in DiscoveredBy
		// it added OrePurifier and xxxGainSelfHeal to House counter already

		// LimboKill ID
		pBuildingExt->LimboID = ID;

		// Add building to list of owned limbo buildings
		pOwnerExt->OwnedLimboDeliveredBuildings.push_back(pBuilding);

		if (!pBuilding->Type->Insignificant && !pBuilding->Type->DontScore)
			pOwnerExt->AddToLimboTracking(pBuilding->Type);

		auto const pTechnoExt = TechnoExt::ExtMap.Find(pBuilding);
		auto const pTechnoTypeExt = pTechnoExt->TypeExtData;

		if (pTechnoTypeExt->AutoDeath_Behavior.isset())
		{
			ScenarioExt::Global()->AutoDeathObjects.push_back(pTechnoExt);

			if (pTechnoTypeExt->AutoDeath_AfterDelay > 0)
				pTechnoExt->AutoDeathTimer.Start(pTechnoTypeExt->AutoDeath_AfterDelay);
		}
	}
}

bool BuildingTypeExt::DeleteLimboBuilding(BuildingClass* pBuilding, int ID)
{
	BuildingExt::ExtData* const pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);

	if (pBuildingExt->LimboID != ID)
		return false;

	if (pBuildingExt->TypeExtData->LimboBuildID == ID)
	{
		HouseClass* const pHouse = pBuilding->Owner;
		const int index = pBuilding->Type->ArrayIndex;

		for (auto& pBaseNode : pHouse->Base.BaseNodes)
		{
			if (pBaseNode.BuildingTypeIndex == index)
				pBaseNode.Placed = false;
		}
	}

	return true;
}

void BuildingTypeExt::ExtData::Initialize()
{ }

// =============================
// load / save

void BuildingTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;
	auto pArtINI = &CCINIClass::INI_Art();

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	INI_EX exArtINI(pArtINI);

	this->PowersUp_Owner.Read(exINI, pSection, "PowersUp.Owner");
	this->PowersUp_Buildings.Read(exINI, pSection, "PowersUp.Buildings");
	this->PowerPlantEnhancer_Buildings.Read(exINI, pSection, "PowerPlantEnhancer.PowerPlants");
	this->PowerPlantEnhancer_Amount.Read(exINI, pSection, "PowerPlantEnhancer.Amount");
	this->PowerPlantEnhancer_Factor.Read(exINI, pSection, "PowerPlantEnhancer.Factor");
	this->Powered_KillSpawns.Read(exINI, pSection, "Powered.KillSpawns");

	if (pThis->PowersUpBuilding[0] == NULL && this->PowersUp_Buildings.size() > 0)
		strcpy_s(pThis->PowersUpBuilding, this->PowersUp_Buildings[0]->ID);

	this->AllowAirstrike.Read(exINI, pSection, "AllowAirstrike");
	this->CanC4_AllowZeroDamage.Read(exINI, pSection, "CanC4.AllowZeroDamage");

	this->InitialStrength_Cloning.Read(exINI, pSection, "InitialStrength.Cloning");
	this->ExcludeFromMultipleFactoryBonus.Read(exINI, pSection, "ExcludeFromMultipleFactoryBonus");

	this->Grinding_AllowAllies.Read(exINI, pSection, "Grinding.AllowAllies");
	this->Grinding_AllowOwner.Read(exINI, pSection, "Grinding.AllowOwner");
	this->Grinding_AllowTypes.Read(exINI, pSection, "Grinding.AllowTypes");
	this->Grinding_DisallowTypes.Read(exINI, pSection, "Grinding.DisallowTypes");
	this->Grinding_Sound.Read(exINI, pSection, "Grinding.Sound");
	this->Grinding_PlayDieSound.Read(exINI, pSection, "Grinding.PlayDieSound");
	this->Grinding_Weapon.Read<true>(exINI, pSection, "Grinding.Weapon");
	this->Grinding_Weapon_RequiredCredits.Read(exINI, pSection, "Grinding.Weapon.RequiredCredits");

	this->DisplayIncome.Read(exINI, pSection, "DisplayIncome");
	this->DisplayIncome_Houses.Read(exINI, pSection, "DisplayIncome.Houses");
	this->DisplayIncome_Offset.Read(exINI, pSection, "DisplayIncome.Offset");

	this->ConsideredVehicle.Read(exINI, pSection, "ConsideredVehicle");
	this->SellBuildupLength.Read(exINI, pSection, "SellBuildupLength");
	this->IsDestroyableObstacle.Read(exINI, pSection, "IsDestroyableObstacle");

	this->AutoUpgrade.Read(exINI, pSection, "AutoUpgrade");
	this->LimboBuild.Read(exINI, pSection, "LimboBuild");
	this->LimboBuildID.Read(exINI, pSection, "LimboBuildID");

	this->FactoryPlant_AllowTypes.Read(exINI, pSection, "FactoryPlant.AllowTypes");
	this->FactoryPlant_DisallowTypes.Read(exINI, pSection, "FactoryPlant.DisallowTypes");

	this->Units_RepairRate.Read(exINI, pSection, "Units.RepairRate");
	this->Units_RepairStep.Read(exINI, pSection, "Units.RepairStep");
	this->Units_RepairPercent.Read(exINI, pSection, "Units.RepairPercent");
	this->Units_UseRepairCost.Read(exINI, pSection, "Units.UseRepairCost");

	this->NoBuildAreaOnBuildup.Read(exINI, pSection, "NoBuildAreaOnBuildup");
	this->Adjacent_Allowed.Read(exINI, pSection, "Adjacent.Allowed");
	this->Adjacent_Disallowed.Read(exINI, pSection, "Adjacent.Disallowed");

	if (pThis->NumberOfDocks > 0)
	{
		this->AircraftDockingDirs.clear();
		this->AircraftDockingDirs.resize(pThis->NumberOfDocks);

		Nullable<DirType> nLandingDir;
		nLandingDir.Read(exINI, pSection, "AircraftDockingDir");

		if (nLandingDir.isset())
			this->AircraftDockingDirs[0] = nLandingDir.Get();

		for (int i = 0; i < pThis->NumberOfDocks; ++i)
		{
			char tempBuffer[32];
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "AircraftDockingDir%d", i);
			nLandingDir.Read(exINI, pSection, tempBuffer);

			if (nLandingDir.isset())
				this->AircraftDockingDirs[i] = nLandingDir.Get();
		}
	}

	// Ares tag
	this->SpyEffect_Custom.Read(exINI, pSection, "SpyEffect.Custom");
	if (SuperWeaponTypeClass::Array->Count > 0)
	{
		this->SuperWeapons.Read(exINI, pSection, "SuperWeapons");

		this->SpyEffect_VictimSuperWeapon.Read(exINI, pSection, "SpyEffect.VictimSuperWeapon");
		this->SpyEffect_InfiltratorSuperWeapon.Read(exINI, pSection, "SpyEffect.InfiltratorSuperWeapon");
	}

	if (pThis->MaxNumberOccupants > 10)
	{
		char tempBuffer[32];
		this->OccupierMuzzleFlashes.clear();
		this->OccupierMuzzleFlashes.resize(pThis->MaxNumberOccupants);

		for (int i = 0; i < pThis->MaxNumberOccupants; ++i)
		{
			Nullable<Point2D> nMuzzleLocation;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "MuzzleFlash%d", i);
			nMuzzleLocation.Read(exArtINI, pArtSection, tempBuffer);
			this->OccupierMuzzleFlashes[i] = nMuzzleLocation.Get(Point2D::Empty);
		}
	}

	this->Refinery_UseStorage.Read(exINI, pSection, "Refinery.UseStorage");

	// PlacementPreview
	{
		this->PlacementPreview.Read(exINI, pSection, "PlacementPreview");
		this->PlacementPreview_Shape.Read(exINI, pSection, "PlacementPreview.Shape");
		this->PlacementPreview_ShapeFrame.Read(exINI, pSection, "PlacementPreview.ShapeFrame");
		this->PlacementPreview_Offset.Read(exINI, pSection, "PlacementPreview.Offset");
		this->PlacementPreview_Remap.Read(exINI, pSection, "PlacementPreview.Remap");
		this->PlacementPreview_Palette.LoadFromINI(pINI, pSection, "PlacementPreview.Palette");
		this->PlacementPreview_Translucency.Read(exINI, pSection, "PlacementPreview.Translucency");
	}

	// Art
	this->ZShapePointMove_OnBuildup.Read(exArtINI, pSection, "ZShapePointMove.OnBuildup");
}

void BuildingTypeExt::ExtData::CompleteInitialization()
{
	auto const pThis = this->OwnerObject();
	UNREFERENCED_PARAMETER(pThis);
}

template <typename T>
void BuildingTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->PowersUp_Owner)
		.Process(this->PowersUp_Buildings)
		.Process(this->PowerPlantEnhancer_Buildings)
		.Process(this->PowerPlantEnhancer_Amount)
		.Process(this->PowerPlantEnhancer_Factor)
		.Process(this->SuperWeapons)
		.Process(this->OccupierMuzzleFlashes)
		.Process(this->Powered_KillSpawns)
		.Process(this->AllowAirstrike)
		.Process(this->CanC4_AllowZeroDamage)
		.Process(this->InitialStrength_Cloning)
		.Process(this->ExcludeFromMultipleFactoryBonus)
		.Process(this->Refinery_UseStorage)
		.Process(this->Grinding_AllowAllies)
		.Process(this->Grinding_AllowOwner)
		.Process(this->Grinding_AllowTypes)
		.Process(this->Grinding_DisallowTypes)
		.Process(this->Grinding_Sound)
		.Process(this->Grinding_PlayDieSound)
		.Process(this->Grinding_Weapon)
		.Process(this->Grinding_Weapon_RequiredCredits)
		.Process(this->DisplayIncome)
		.Process(this->DisplayIncome_Houses)
		.Process(this->DisplayIncome_Offset)
		.Process(this->PlacementPreview)
		.Process(this->PlacementPreview_Shape)
		.Process(this->PlacementPreview_ShapeFrame)
		.Process(this->PlacementPreview_Offset)
		.Process(this->PlacementPreview_Remap)
		.Process(this->PlacementPreview_Palette)
		.Process(this->PlacementPreview_Translucency)
		.Process(this->SpyEffect_Custom)
		.Process(this->SpyEffect_VictimSuperWeapon)
		.Process(this->SpyEffect_InfiltratorSuperWeapon)
		.Process(this->ConsideredVehicle)
		.Process(this->ZShapePointMove_OnBuildup)
		.Process(this->SellBuildupLength)
		.Process(this->AutoUpgrade)
		.Process(this->LimboBuild)
		.Process(this->LimboBuildID)
		.Process(this->AircraftDockingDirs)
		.Process(this->FactoryPlant_AllowTypes)
		.Process(this->FactoryPlant_DisallowTypes)
		.Process(this->IsDestroyableObstacle)
		.Process(this->Units_RepairRate)
		.Process(this->Units_RepairStep)
		.Process(this->Units_RepairPercent)
		.Process(this->Units_UseRepairCost)
		.Process(this->NoBuildAreaOnBuildup)
		.Process(this->Adjacent_Allowed)
		.Process(this->Adjacent_Disallowed)
		;
}

void BuildingTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BuildingTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BuildingTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BuildingTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool BuildingTypeExt::ExtContainer::Load(BuildingTypeClass* pThis, IStream* pStm)
{
	BuildingTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	return pData != nullptr;
};

bool BuildingTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{

	return Stm.Success();
}

bool BuildingTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{


	return Stm.Success();
}
// =============================
// container

BuildingTypeExt::ExtContainer::ExtContainer() : Container("BuildingTypeClass") { }

BuildingTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x45E50C, BuildingTypeClass_CTOR, 0x6)
{
	GET(BuildingTypeClass*, pItem, EAX);

	BuildingTypeExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x45E707, BuildingTypeClass_DTOR, 0x6)
{
	GET(BuildingTypeClass*, pItem, ESI);

	BuildingTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x465300, BuildingTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x465010, BuildingTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BuildingTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x4652ED, BuildingTypeClass_Load_Suffix, 0x7)
{
	BuildingTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46536A, BuildingTypeClass_Save_Suffix, 0x7)
{
	BuildingTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x464A56, BuildingTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x464A49, BuildingTypeClass_LoadFromINI, 0xA)
{
	GET(BuildingTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x364);

	BuildingTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
