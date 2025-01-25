#include "Body.h"

#include <EventClass.h>

#include <Utilities/GeneralUtils.h>
#include <Ext/TechnoType/Body.h>
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

	for (const auto& [bTypeIdx, nCount] : pHouseExt->PowerPlantEnhancers)
	{
		auto bTypeExt = BuildingTypeExt::ExtMap.Find(BuildingTypeClass::Array->Items[bTypeIdx]);
		if (bTypeExt->PowerPlantEnhancer_Buildings.Contains(pBuilding->Type))
		{
			fFactor *= std::powf(bTypeExt->PowerPlantEnhancer_Factor, static_cast<float>(nCount));
			nAmount += bTypeExt->PowerPlantEnhancer_Amount * nCount;
		}
	}

	return static_cast<int>(std::round(pBuilding->GetPowerOutput() * fFactor)) + nAmount;
}

int BuildingTypeExt::CountOwnedNowWithDeployOrUpgrade(BuildingTypeClass* pType, HouseClass* pHouse)
{
	const auto upgrades = BuildingTypeExt::GetUpgradesAmount(pType, pHouse);

	if (upgrades != -1)
		return upgrades;

	if (const auto pUndeploy = pType->UndeploysInto)
		return pHouse->CountOwnedNow(pType) + pHouse->CountOwnedNow(pUndeploy);

	return pHouse->CountOwnedNow(pType);
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

// Check whether can call the occupiers leave
bool BuildingTypeExt::CheckOccupierCanLeave(HouseClass* pBuildingHouse, HouseClass* pOccupierHouse)
{
	if (!pOccupierHouse)
		return false;
	else if (pBuildingHouse == pOccupierHouse)
		return true;
	else if (SessionClass::IsCampaign() && pOccupierHouse->IsInPlayerControl)
		return true;
	else if (!pOccupierHouse->IsControlledByHuman() && pOccupierHouse->IsAlliedWith(pBuildingHouse))
		return true;

	return false;
}

// Force occupiers leave, return: whether it should stop right now
bool BuildingTypeExt::CleanUpBuildingSpace(BuildingTypeClass* pBuildingType, CellStruct topLeftCell, HouseClass* pHouse, TechnoClass* pExceptTechno)
{
	// Step 1: Find the technos inside of the building place grid.
	auto infantryCount = CellStruct::Empty;
	std::vector<TechnoClass*> checkedTechnos;
	checkedTechnos.reserve(24);
	std::vector<CellClass*> checkedCells;
	checkedCells.reserve(24);

	for (auto pFoundation = pBuildingType->GetFoundationData(false); *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
	{
		auto currentCell = topLeftCell + *pFoundation;

		if (const auto pCell = MapClass::Instance->GetCellAt(currentCell))
		{
			auto pObject = pCell->FirstObject;

			while (pObject)
			{
				const auto absType = pObject->WhatAmI();

				if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
				{
					const auto pCellTechno = static_cast<TechnoClass*>(pObject);
					const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pCellTechno->GetTechnoType());

					if ((!pTypeExt || !pTypeExt->CanBeBuiltOn) && pCellTechno != pExceptTechno) // No need to check house
					{
						const auto pFoot = static_cast<FootClass*>(pCellTechno);

						if (pFoot->GetCurrentSpeed() <= 0 || !pFoot->Locomotor->Is_Moving())
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
		auto searchCell = topLeftCell + *pFoundation;

		if (const auto pSearchCell = MapClass::Instance->GetCellAt(searchCell))
		{
			if (std::find(checkedCells.begin(), checkedCells.end(), pSearchCell) == checkedCells.end() // TODO If there is a cellflag (or CellExt) that can be used …
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

		for (const auto& pOptionalCell : optionalCells) // If there are many valid cells at start, it means most of occupiers will near to the edge
		{
			if (minA > 65536) // If distance squared is lower or equal to 256^2, then no need to calculate any more because it is on the edge
			{
				auto curA = static_cast<int>(pTechnoA->GetMapCoords().DistanceFromSquared(pOptionalCell->MapCoords));

				if (curA < minA)
					minA = curA;
			}

			if (minB > 65536)
			{
				auto curB = static_cast<int>(pTechnoB->GetMapCoords().DistanceFromSquared(pOptionalCell->MapCoords));

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
		for (const auto& pRecheckedTechno : reCheckedTechnos)
		{
			if (pRecheckedTechno->WhatAmI() == AbstractType::Infantry)
				++infantryCount.X;

			checkedTechnos.push_back(pRecheckedTechno);
		}

		reCheckedTechnos.clear();

		// Step 4.2: Check the techno vector.
		for (const auto& pCheckedTechno : checkedTechnos)
		{
			CellClass* pDestinationCell = nullptr;

			// Step 4.2.1: Search the closest valid cell to be the destination.
			do
			{
				const auto location = pCheckedTechno->GetMapCoords();
				const bool isInfantry = pCheckedTechno->WhatAmI() == AbstractType::Infantry;
				const auto pCheckedType = pCheckedTechno->GetTechnoType();

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
				const auto minDistanceSquared = optionalCells[0]->MapCoords.DistanceFromSquared(location);

				for (const auto& pOptionalCell : optionalCells) // Prioritize selecting empty cells
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

					for (const auto& pOptionalCell : optionalCells)
					{
						auto pCurObject = pOptionalCell->FirstObject;
						std::vector<TechnoClass*> optionalTechnos;
						optionalTechnos.reserve(4);
						bool valid = true;

						while (pCurObject)
						{
							const auto absType = pCurObject->WhatAmI();

							if (absType == AbstractType::Infantry || absType == AbstractType::Unit)
							{
								const auto pCurTechno = static_cast<TechnoClass*>(pCurObject);

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
							for (const auto& pOptionalTechno : optionalTechnos)
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

					for (const auto& pDeleteCell : deleteCells) // Mark the invalid cells
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
					auto searchCell = pDestinationCell->MapCoords - CellStruct { 1, 1 };

					for (int i = 0; i < 4; ++i)
					{
						for (int j = 0; j < 2; ++j)
						{
							if (const auto pSearchCell = MapClass::Instance->GetCellAt(searchCell))
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

				const auto thisOrder = TechnoWithDestination { pCheckedTechno, pDestinationCell };
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
	for (const auto& pThisOrder : finalOrder)
	{
		const auto pCheckedTechno = pThisOrder.techno;
		const auto pDestinationCell = pThisOrder.destination;
		const auto absType = pCheckedTechno->WhatAmI();
		pCheckedTechno->ForceMission(Mission::Guard);

		if (absType == AbstractType::Infantry)
		{
			const auto pInfantry = static_cast<InfantryClass*>(pCheckedTechno);

			if (pInfantry->IsDeployed())
				pInfantry->PlayAnim(Sequence::Undeploy, true);

			pInfantry->SetDestination(pDestinationCell, false);
			pInfantry->QueueMission(Mission::QMove, false); // To force every three infantries gather together, it should be QMove
		}
		else if (absType == AbstractType::Unit)
		{
			const auto pUnit = static_cast<UnitClass*>(pCheckedTechno);

			if (pUnit->Deployed)
				pUnit->Undeploy();

			pUnit->SetDestination(pDestinationCell, false);
		}
	}

	return false;
}

bool BuildingTypeExt::AutoPlaceBuilding(BuildingClass* pBuilding)
{
	if (!Phobos::Config::AutoBuilding_Enable)
		return false;

	const auto pType = pBuilding->Type;
	const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);

	if (!pTypeExt->AutoBuilding.Get(RulesExt::Global()->AutoBuilding) || pType->LaserFence || pType->Gate || pType->ToTile)
		return false;

	const auto pHouse = pBuilding->Owner;

	if (pHouse->Buildings.Count <= 0)
		return false;

	const auto foundation = pType->GetFoundationData(true);

	auto canBuildHere = [&pType, &pHouse, &foundation](CellStruct cell)
	{
		return reinterpret_cast<bool(__thiscall*)(MapClass*, BuildingTypeClass*, int, CellStruct*, CellStruct*)>(0x4A8EB0)(MapClass::Instance(),
			pType, pHouse->ArrayIndex, foundation, &cell) // Adjacent
			&& reinterpret_cast<bool(__thiscall*)(MapClass*, BuildingTypeClass*, int, CellStruct*, CellStruct*)>(0x4A9070)(MapClass::Instance(),
			pType, pHouse->ArrayIndex, foundation, &cell); // NoShroud
	};

	const auto pHouseExt = HouseExt::ExtMap.Find(pHouse);

	auto getMapCell = [&pHouseExt](BuildingClass* pBuilding)
	{
		if (!pBuilding->IsAlive || pBuilding->Health <= 0 || !pBuilding->IsOnMap || pBuilding->InLimbo || pHouseExt->OwnsLimboDeliveredBuilding(pBuilding))
			return CellStruct::Empty;

		return pBuilding->GetMapCoords();
	};

	auto addPlaceEvent = [&pType, &pHouse](CellStruct cell)
	{
		const EventClass event (pHouse->ArrayIndex, EventType::Place, AbstractType::Building, pType->GetArrayIndex(), pType->Naval, cell);
		EventClass::AddEvent(event);
	};

	if (pType->LaserFencePost || pType->Wall)
	{
		for (const auto& pOwned : pHouse->Buildings)
		{
			const auto pOwnedType = pOwned->Type;

			if (!pOwnedType->ProtectWithWall)
				continue;

			const auto baseCell = getMapCell(pOwned);

			if (baseCell == CellStruct::Empty)
				continue;

			const auto width = pOwnedType->GetFoundationWidth();
			const auto height = pOwnedType->GetFoundationHeight(true);
			auto cell = CellStruct::Empty;
			int index = 0, check = width + 1, count = 0;

			for (auto pFoundation = pOwnedType->FoundationOutside; *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
			{
				if (++index != check)
					continue;

				check += (++count & 1) ? 1 : (height * 2 + width + 1);
				const auto outsideCell = baseCell + *pFoundation;
				const auto pCell = MapClass::Instance->TryGetCellAt(outsideCell);

				if (pCell && pCell->CanThisExistHere(pOwnedType->SpeedType, pOwnedType, pHouse) && canBuildHere(outsideCell))
				{
					addPlaceEvent(outsideCell);
					return true;
				}
			}

			for (auto pFoundation = pOwnedType->FoundationOutside; *pFoundation != CellStruct { 0x7FFF, 0x7FFF }; ++pFoundation)
			{
				const auto outsideCell = baseCell + *pFoundation;
				const auto pCell = MapClass::Instance->TryGetCellAt(outsideCell);

				if (pCell && pCell->CanThisExistHere(pOwnedType->SpeedType, pOwnedType, pHouse) && canBuildHere(outsideCell))
					cell = outsideCell;
			}

			if (cell == CellStruct::Empty)
				continue;

			addPlaceEvent(cell);
			return true;
		}

		return false;
	}
	else if (pType->PlaceAnywhere)
	{
		for (const auto& pOwned : pHouse->Buildings)
		{
			if (!pOwned->Type->BaseNormal)
				continue;

			const auto cell = getMapCell(pOwned);

			if (cell == CellStruct::Empty || !canBuildHere(cell))
				continue;

			addPlaceEvent(cell);
			return true;
		}

		return false;
	}
	else if (pType->PowersUpBuilding[0])
	{
		for (const auto& pOwned : pHouse->Buildings)
		{
			if (!reinterpret_cast<bool(__thiscall*)(BuildingClass*, BuildingTypeClass*, HouseClass*)>(0x452670)(pOwned, pType, pHouse)) // CanUpgradeBuilding
				continue;

			const auto cell = getMapCell(pOwned);

			if (cell == CellStruct::Empty || pOwned->CurrentMission == Mission::Selling || !canBuildHere(cell))
				continue;

			addPlaceEvent(cell);
			return true;
		}

		return false;
	}

	const auto buildGap = static_cast<short>(pTypeExt->AutoBuilding_Gap + pType->ProtectWithWall ? 1 : 0);
	const auto doubleGap = buildGap * 2;
	const auto width = pType->GetFoundationWidth() + doubleGap;
	const auto height = pType->GetFoundationHeight(true) + doubleGap;
	const auto speedType = pType->SpeedType == SpeedType::Float ? SpeedType::Float : SpeedType::Track;
	const auto buildable = speedType != SpeedType::Float;

	auto tryBuildAt = [&](DynamicVectorClass<BuildingClass*>& vector, bool baseNormal)
	{
		for (const auto& pBase : vector)
		{
			if (baseNormal && !pBase->Type->BaseNormal)
				continue;

			const auto baseCell = getMapCell(pBase);

			if (baseCell == CellStruct::Empty)
				continue;

			// TODO The construction area does not actually need to be so large, the surrounding space should be able to be occupied by other things
			// TODO It would be better if the Buildable check could be fit with ExtendedBuildingPlacing within this function.
			// TODO Similarly, it would be better if the following Adjacent & NoShroud check could be made within this function.
			auto cell = pType->PlaceAnywhere ? baseCell : MapClass::Instance->NearByLocation(baseCell, speedType, -1, MovementZone::Normal, false,
				width, height, false, false, false, false, CellStruct::Empty, false, buildable);

			if (cell == CellStruct::Empty)
				return false;

			cell += CellStruct { buildGap, buildGap };

			if (!canBuildHere(cell))
				continue;

			addPlaceEvent(cell);
			return true;
		}

		return false;
	};

	if (pHouse->ConYards.Count > 0 && tryBuildAt(pHouse->ConYards, false))
		return true;

	return tryBuildAt(pHouse->Buildings, true);
}

bool BuildingTypeExt::BuildLimboBuilding(BuildingClass* pBuilding)
{
	const auto pBuildingType = pBuilding->Type;

	if (BuildingTypeExt::ExtMap.Find(pBuildingType)->LimboBuild)
	{
		const EventClass event
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
	const auto pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);

	if (pBuildingExt->LimboID != ID)
		return false;

	if (pBuildingExt->TypeExtData->LimboBuildID == ID)
	{
		const auto pHouse = pBuilding->Owner;
		const auto index = pBuilding->Type->ArrayIndex;

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

	this->AutoBuilding.Read(exINI, pSection, "AutoBuilding");
	this->AutoBuilding_Gap.Read(exINI, pSection, "AutoBuilding.Gap");
	this->LimboBuild.Read(exINI, pSection, "LimboBuild");
	this->LimboBuildID.Read(exINI, pSection, "LimboBuildID");
	this->LaserFencePost_Fence.Read(exINI, pSection, "LaserFencePost.Fence");
	this->PlaceBuilding_OnLand.Read(exINI, pSection, "PlaceBuilding.OnLand");
	this->PlaceBuilding_OnWater.Read(exINI, pSection, "PlaceBuilding.OnWater");

	this->FactoryPlant_AllowTypes.Read(exINI, pSection, "FactoryPlant.AllowTypes");
	this->FactoryPlant_DisallowTypes.Read(exINI, pSection, "FactoryPlant.DisallowTypes");

	this->Units_RepairRate.Read(exINI, pSection, "Units.RepairRate");
	this->Units_RepairStep.Read(exINI, pSection, "Units.RepairStep");
	this->Units_RepairPercent.Read(exINI, pSection, "Units.RepairPercent");
	this->Units_UseRepairCost.Read(exINI, pSection, "Units.UseRepairCost");

	this->NoBuildAreaOnBuildup.Read(exINI, pSection, "NoBuildAreaOnBuildup");
	this->Adjacent_Allowed.Read(exINI, pSection, "Adjacent.Allowed");
	this->Adjacent_Disallowed.Read(exINI, pSection, "Adjacent.Disallowed");

	this->BarracksExitCell.Read(exINI, pSection, "BarracksExitCell");

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
		.Process(this->AutoBuilding)
		.Process(this->AutoBuilding_Gap)
		.Process(this->LimboBuild)
		.Process(this->LimboBuildID)
		.Process(this->LaserFencePost_Fence)
		.Process(this->PlaceBuilding_OnLand)
		.Process(this->PlaceBuilding_OnWater)
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
		.Process(this->BarracksExitCell)
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
