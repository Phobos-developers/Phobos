// For selected units, pair the loadable vehicle and infantry.
// 1. If transport can load vehicle, it won't load if the vehicle can be paired with infantry.
// 2. It will always try to load the unit type with least amount of units, eg. 2 GI and 3 GGI, it will match 2 GI to transport first.
// 3. For transport with multiple seats, it will try to fill the seats diversely.

#include "AutoLoad.h"
#include "Utilities/GeneralUtils.h"
#include "Ext/Techno/Body.h"
#include <unordered_map>

const char* AutoLoadCommandClass::GetName() const
{
	return "AutoLoad";
}

const wchar_t* AutoLoadCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_LOAD", L"Auto Load");
}

const wchar_t* AutoLoadCommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t* AutoLoadCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_LOAD_DESC", L"Auto Load");
}

void DebugPrintTransport(std::vector<std::pair<TechnoClass*, int>>& transports)
{
	Debug::Log("AutoLoadCommandClass::DebugPrintTransport: Transport count: %d\n", transports.size());
	// print address of each transport and its passengers
	for (auto transport : transports)
		Debug::Log("AutoLoadCommandClass::DebugPrintTransport: Transport address: %p, Now size: %d, Virtual size: %d\n", transport.first, transport.first->Passengers.GetTotalSize(), transport.second);
}

void DebugPrintPassenger(std::vector<TechnoClass*>& passengers)
{
	Debug::Log("AutoLoadCommandClass::DebugPrintPassenger: Passenger count: %d\n", passengers.size());
	// print address of each passenger
	for (auto passenger : passengers)
		Debug::Log("AutoLoadCommandClass::DebugPrintPassenger: Passenger address: %p\n", passenger);
}

// filter function: tells if this techno is mind controlling something.
inline static const bool IsMindControlling(TechnoClass* pTechno)
{
	return pTechno->CaptureManager && pTechno->CaptureManager->IsControllingSomething();
}

// Gets the passenger budgets of a building.
// If return value <= 0 then the building can't be a "transport".
inline static const int GetBuildingPassengerBudget(BuildingClass* pBuilding)
{
	auto pBuildingType = abstract_cast<BuildingTypeClass*>(pBuilding->Type);
	// Bio Reactor
	if (pBuildingType->Passengers > 0 && pBuildingType->InfantryAbsorb)
	{
		return pBuildingType->Passengers - pBuilding->Passengers.NumPassengers;
	}
	// garrisonable structure
	else if (pBuildingType->CanBeOccupied)
	{
		return pBuildingType->MaxNumberOccupants - pBuilding->Occupants.Count;
	}
	// Tank Bunker
	else if (pBuildingType->Bunker)
	{
		return pBuilding->BunkerLinkedItem ? 0 : 1;
	}
	return 0;
}

// Gets if a unit can potentially be a building's passenger.
inline static const bool CanBeBuildingPassenger(TechnoClass* pPassenger)
{
	if (pPassenger->WhatAmI() == AbstractType::Infantry)
	{
		// Bio Reactor & garrisonable structure
		return !IsMindControlling(pPassenger);
	}
	else if (pPassenger->WhatAmI() == AbstractType::Unit)
	{
		// Tank Bunker
		return pPassenger->GetTechnoType()->Turret
			&& pPassenger->GetTechnoType()->Bunkerable
			&& pPassenger->GetTechnoType()->SpeedType != SpeedType::Hover
			&& !abstract_cast<FootClass*>(pPassenger)->ParasiteEatingMe
			&& !pPassenger->BunkerLinkedItem;
	}
	return false;
}

// Gets the passenger budgets of a transport.
// If return value <= 0 then it can't be a transport.
inline static const int GetVehiclePassengerBudget(TechnoClass* pTransport)
{
	if (pTransport->WhatAmI() == AbstractType::Unit)
	{
		auto pTechnoType = pTransport->GetTechnoType();
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
		if (pTechnoType->Passengers > 0 && !pTypeExt->NoManualEnter)
		{
			if (pTypeExt->Passengers_BySize)
				return pTechnoType->Passengers - pTransport->Passengers.GetTotalSize();
			else
				return pTechnoType->Passengers - pTransport->Passengers.NumPassengers;
		}
	}
	return 0;
}

// Gets if a unit can potentially be a vehicle's passenger.
inline static const bool CanBeVehiclePassenger(TechnoClass* pPassenger)
{
	if (pPassenger->WhatAmI() == AbstractType::Infantry
		|| pPassenger->WhatAmI() == AbstractType::Unit)
	{
		return !IsMindControlling(pPassenger)
			&& !pPassenger->IsMindControlled()
			&& !pPassenger->BunkerLinkedItem;
	}
	return false;
}

// Gets if the transport can load a passenger.
inline static const bool CanHoldPassenger(TechnoClass* pTransport, TechnoClass* pPassenger)
{
	if (pTransport->WhatAmI() == AbstractType::Unit)
	{
		auto pTechnoType = pTransport->GetTechnoType();
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
		// the check of MCing or MCed are redundant here, see inline function "CanBeVehiclePassenger".
		return pTypeExt->CanLoadPassenger(pTransport, pPassenger);
	}
	else if (pTransport->WhatAmI() == AbstractType::Building)
	{
		auto pBuilding = abstract_cast<BuildingClass*>(pTransport);
		auto pBuildingType = abstract_cast<BuildingTypeClass*>(pBuilding->Type);
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pBuildingType);
		if (pBuildingType->Passengers > 0 && pBuildingType->InfantryAbsorb)
		{
			// Bio Reactor
			// the check of MCing is redundant here, see inline function "CanBeBuildingPassenger".
			return pPassenger->WhatAmI() == AbstractType::Infantry
				&& pTypeExt->CanLoadPassenger(pTransport, pPassenger);
		}
		else if (pBuildingType->CanBeOccupied)
		{
			// garrisonable structure
			// the check of MCing is redundant here, see inline function "CanBeBuildingPassenger".
			return pPassenger->WhatAmI() == AbstractType::Infantry
				&& !pPassenger->IsMindControlled()
				&& abstract_cast<InfantryTypeClass*>(pPassenger->GetTechnoType())->Occupier
				&& pTypeExt->CanBeOccupiedBy(pPassenger);
		}
		else if (pBuildingType->Bunker)
		{
			// Tank Bunker
			// the check of "Turret=yes" and "Bunkerable=yes" are redundant here, see inline function "CanBeBuildingPassenger".
			return pPassenger->WhatAmI() == AbstractType::Unit;
		}
	}
	return false;
}

// Checks if this transport substrats passenger budget by passenger size.
inline static const bool IsBySize(TechnoClass* pTransport)
{
	if (pTransport->WhatAmI() == AbstractType::Unit)
	{
		auto pType = pTransport->GetTechnoType();
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		return pTypeExt->Passengers_BySize;
	}
	else if (pTransport->WhatAmI() == AbstractType::Building)
	{
		return false;
	}
	return true;
}

// Oddly, a Tank Bunker can't load anything when selected by the player.
// Therefore it has to be unselected.
inline static bool DeselectMe(TechnoClass* pTransport)
{
	if (pTransport->WhatAmI() == AbstractType::Building)
	{
		auto pBuilding = abstract_cast<BuildingClass*>(pTransport);
		auto pBuildingType = abstract_cast<BuildingTypeClass*>(pBuilding->Type);
		if (pBuildingType->Bunker)
		{
			pTransport->Deselect();
			return true;
		}
	}
	return false;
}

template <typename TPassenger, typename TTransport>
std::set<TPassenger> SpreadPassengersToTransports(std::vector<TPassenger>& passengers, std::vector<std::pair<TTransport, int>>& transports)
{
	std::set<TPassenger> foundTransportVector;
	// 1. Get the least kind of passengers
	// 2. Send the passengers to the transport in round robin, if the transport is full, remove it from the vector transports;
	// if the pID vector is empty, remove it from the map, if not, move it to passengerMapIdle. We try to fill the transport evenly for each kind of passengers.
	// 3. Repeat until all passengers are sent or the vector transports is empty.
	std::unordered_map<const char*, std::vector<TPassenger>> passengerMap;
	std::unordered_map<const char*, std::vector<TPassenger>> passengerMapIdle;

	for (auto pPassenger : passengers)
	{
		auto pID = pPassenger->get_ID();
		if (passengerMap.find(pID) == passengerMap.end())
			passengerMap[pID] = std::vector<TPassenger>();

		passengerMap[pID].push_back(pPassenger);
	}

	while (true)
	{
		while (passengerMap.size() > 0 && transports.size() > 0)
		{
			const char* leastpID = nullptr;
			unsigned int leastSize = std::numeric_limits<unsigned int>::max();
			for (auto const& [pID, pPassenger] : passengerMap)
			{
				if (pPassenger.size() < leastSize)
				{
					leastSize = pPassenger.size();
					leastpID = pID;
				}
			}

			{
				unsigned int index = 0;
				for (; index < leastSize && index < transports.size(); index++)
				{
					auto pPassenger = passengerMap[leastpID][index];
					auto pTransport = transports[index].first;

					// If this transport can't hold this passenger then skip.
					if (!CanHoldPassenger(pTransport, pPassenger))
						goto seeNextTransport; // "continue" causes to skip the passenger, not to skip the transport, so we shouldn't use "continue" here

					// Gets the passenger slot budget that would be substracted.
					int passengerSize;
					if (IsBySize(pTransport))
					{
						// If by size then get the actual size of the potential passenger.
						// Otherwise, every potential passenger counts as size 1.
						passengerSize = static_cast<int>(abstract_cast<TechnoClass*>(pPassenger)->GetTechnoType()->Size);

						// Check if the transport still has the budget for the new passenger.
						if (transports[index].second < passengerSize)
							goto seeNextTransport;
					}
					else
					{
						passengerSize = 1;
						// Note that, the transport is momentarily removed from "tTransports" as soon as it's full,
						// so it is redundant to check a transport's budget if it doesn't count by size.
					}

					if (pPassenger->GetCurrentMission() != Mission::Enter)
					{
						bool deselected = DeselectMe(pTransport);
						bool moveFeedbackOld = std::exchange(Unsorted::MoveFeedback(), false);
						pPassenger->ObjectClickedAction(Action::Enter, pTransport, true);
						if (deselected)
							pTransport->Select();
						Unsorted::MoveFeedback = moveFeedbackOld;
						transports[index].second -= passengerSize; // take away that much passenger slot budgets from the transport
						foundTransportVector.insert(pPassenger);
					}

				seeNextTransport:;
				}
				passengerMap[leastpID].erase(passengerMap[leastpID].begin(), passengerMap[leastpID].begin() + index);
			}

			// Remove fully loaded transports from potential transports array.
			transports.erase(
				std::remove_if(transports.begin(), transports.end(),
					[](auto transport)
					{
						return transport.second == 0;
					}),
				transports.end());

			if (passengerMap[leastpID].size() == 0)
				passengerMap.erase(leastpID);
			else
				passengerMapIdle[leastpID] = passengerMap[leastpID];
		}

		if (passengerMapIdle.size() != 0 && transports.size() != 0)
			std::swap(passengerMap, passengerMapIdle);
		else
			break;
	}
	return foundTransportVector;
}

void AutoLoadCommandClass::Execute(WWKey eInput) const
{
	MapClass::Instance->SetTogglePowerMode(0);
	MapClass::Instance->SetWaypointMode(0, false);
	MapClass::Instance->SetRepairMode(0);
	MapClass::Instance->SetSellMode(0);

	std::map<int, std::vector<TechnoClass*>> passengerMap;						// unit size -> a list of passengers of that size
	std::set<int> passengerSizes;												// a sorted set of known passenger sizes

	std::map<int, std::vector<std::pair<TechnoClass*, int>>> transportMap;		// size limit -> a list of transports of that size limit
	std::set<int> transportSizeLimits;											// a sorted set of known size limits

	std::map<int, std::vector<TechnoClass*>> ambiguousMap;						// unit size -> a list of units in ambiguousity
	std::set<int> ambiguousSizes;												// a sorted set of known ambiguous unit sizes

	// This array is for Bio Reactors, garrisonable structures, and Tank Bunkers.
	// A Bio Reactor is a building with "Passengers > 0" and "InfantryAbsorb=yes".
	// A garrisonable structure is a building with "CanBeOccupied=yes".
	// A Tank Bunker is a building with "Bunker=yes", and is not yet docked.
	std::vector<std::pair<TechnoClass*, int>> enterableBuildingIndexArray;

	// This array is for the candidates of enterable buildings.
	std::vector<TechnoClass*> enterableBuildingCandidateArray;

	// Get current selected units.
	// The first iteration, we find units that can't be transports, and add them to the passenger arrays.
	// We also find Bio Reactors, Tank Bunkers, and candidates for them.
	for (const auto& pUnit : ObjectClass::CurrentObjects())
	{
		// try to cast to TechnoClass
		TechnoClass* pTechno = abstract_cast<TechnoClass*>(pUnit);

		// If not a techno, or is in air, then it can't be a passenger, and it can't be a Bio Reactor or Tank Bunker.
		if (!pTechno || pTechno->IsInAir())
			continue;

		// Detect enterable buildings.
		if (pTechno->WhatAmI() == AbstractType::Building)
		{
			auto pBuilding = abstract_cast<BuildingClass*>(pTechno);
			int const budget = GetBuildingPassengerBudget(pBuilding);
			if (budget > 0)
			{
				enterableBuildingIndexArray.push_back(std::make_pair(pTechno, budget));
			}
			continue;
		}

		// Detect candidates for enterable buildings
		if (CanBeBuildingPassenger(pTechno))
		{
			enterableBuildingCandidateArray.push_back(pTechno);
		}

		// Detect candidates for enterable vehicles.
		if (CanBeVehiclePassenger(pTechno) && GetVehiclePassengerBudget(pTechno) <= 0)
		{
			auto pTechnoType = pTechno->GetTechnoType();
			int const size = static_cast<int>(pTechnoType->Size);
			passengerSizes.insert(size);
			passengerMap[size].push_back(pTechno);
		}
	}

	// Get current selected units.
	// The second iteration, we find units that can be transports.
	for (const auto& pUnit : ObjectClass::CurrentObjects())
	{
		// try to cast to TechnoClass
		TechnoClass* pTechno = abstract_cast<TechnoClass*>(pUnit);

		// If not a techno, or is in air, then it can't be a vehicle.
		if (!pTechno || pTechno->IsInAir())
			continue;

		auto pTechnoType = pTechno->GetTechnoType();
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
		if (pTechno->WhatAmI() == AbstractType::Unit)
		{
			int const budget = GetVehiclePassengerBudget(pTechno);
			if (budget > 0)
			{
				if (pTypeExt->CanLoadAny(pTechno, passengerMap, passengerSizes))
				{
					int const sizeLimit = static_cast<int>(pTechnoType->SizeLimit);
					transportSizeLimits.insert(sizeLimit);
					transportMap[sizeLimit].push_back(std::make_pair(pTechno, budget));
				}
				else
				{
					// If it can't actually load any clear passenger, then put it into ambiguousity.
					int const size = static_cast<int>(pTechnoType->Size);
					ambiguousSizes.insert(size);
					ambiguousMap[size].push_back(pTechno);
				}
			}
		}
	}

	// Find a right position for units in ambiguousity.
	// A unit in ambiguousity is a unit that can't load any of the clear passengers,
	// but can potentially load other units in ambiguousity,
	// so we have to deside here if they will be passengers or transports.
	// We move units in ambiguousity from the smallest size to the largest size into passengers,
	// and if an unit in ambiguousity can somehow load them this way, then it is a transport.
	if (!ambiguousSizes.empty())
	{
		std::vector<TechnoClass*> ambiguousPassengerVector;
		for (auto ambiguousSize : ambiguousSizes)
		{
			if (ambiguousMap.contains(ambiguousSize))
			{
				auto const& ambiguousVector = ambiguousMap[ambiguousSize];
				for (auto pAmbiguousTechno : ambiguousVector)
				{
					auto const pTechnoType = pAmbiguousTechno->GetTechnoType();
					auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
					if (ambiguousPassengerVector.empty() || !pTypeExt->CanLoadAny(pAmbiguousTechno, ambiguousPassengerVector))
					{
						// This unit in ambiguousity is about to be added to passengers.
						// Before that, check if it can be a vehicle's passenger.
						// If not, this unit in ambiguousity is added to neither passengers nor transports.
						if (!CanBeVehiclePassenger(pAmbiguousTechno))
							continue;
						ambiguousPassengerVector.push_back(pAmbiguousTechno);
						int const size = static_cast<int>(pTechnoType->Size);
						passengerSizes.insert(size);
						passengerMap[size].push_back(pAmbiguousTechno);
					}
					else
					{
						// This unit in ambiguousity is added to transports.
						int const budget = GetVehiclePassengerBudget(pAmbiguousTechno);
						int const sizeLimit = static_cast<int>(pTechnoType->SizeLimit);
						transportSizeLimits.insert(sizeLimit);
						transportMap[sizeLimit].push_back(std::make_pair(pAmbiguousTechno, budget));
					}
				}
			}
		}
	}

	// Pair the passengers and the transports if possible.
	if (!passengerSizes.empty() && !transportSizeLimits.empty())
	{
		// Reversed iteration, so we load larger passengers first.
		for (auto passengerSizesItr = passengerSizes.rbegin();
			passengerSizesItr != passengerSizes.rend();
			++passengerSizesItr)
		{
			auto passengerSize = *passengerSizesItr;
			auto& passengerVector = passengerMap[passengerSize];

			for (auto transportSizeLimit : transportSizeLimits)
			{
				// If the transports are too small for the passengers then skip them.
				if (transportSizeLimit < passengerSize)
					continue;

				auto& transportVector = transportMap[transportSizeLimit];
				if (transportVector.empty())
					continue;

				auto foundTransportVector = SpreadPassengersToTransports(passengerVector, transportVector);
				passengerVector.erase(
					std::remove_if(passengerVector.begin(), passengerVector.end(),
						[foundTransportVector](auto pPassenger)
						{
							return foundTransportVector.contains(pPassenger);
						}),
					passengerVector.end());
				if (passengerVector.empty())
					break;
			}
		}
	}
	else
	{
		// If nothing can load then go find enterable buildings.
		if (enterableBuildingIndexArray.size() > 0 && enterableBuildingCandidateArray.size() > 0)
			SpreadPassengersToTransports(enterableBuildingCandidateArray, enterableBuildingIndexArray);
	}
}
