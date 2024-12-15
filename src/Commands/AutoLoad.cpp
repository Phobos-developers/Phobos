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

template <typename TPassenger, typename TTransport>
void SpreadPassengersToTransports(std::vector<TPassenger>& passengers, std::vector<std::pair<TTransport, int>>& transports)
{
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
					auto pType = pTransport->GetTechnoType();
					auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

					bool bySize = true;
					int passengerSize = 1;
					if (pTransport->WhatAmI() == AbstractType::Building)
					{
						auto pBuildingType = abstract_cast<BuildingTypeClass*>(pType);
						// If a building comes here then it is either a Bio Reactor or a Tank Bunker.
						// Tank Bunkers have no passenger filter logic, while Bio Reactors have.
						if (!pBuildingType->Bunker && !pTypeExt->CanLoadPassenger(pTransport, pPassenger))
							continue;
						// Tank Bunkers and Bio Reactors are viewed as "Passengers.BySize=no".
						bySize = false;
					}
					else if (pTypeExt)
					{
						// Check passenger filter here.
						if (!pTypeExt->CanLoadPassenger(pTransport, pPassenger))
							continue;

						bySize = pTypeExt->Passengers_BySize;
					}

					if (bySize)
					{
						// If by size then get the actual size of the potential passenger.
						// Otherwise, every potential passenger counts as size 1.
						passengerSize = abstract_cast<TechnoClass*>(pPassenger)->GetTechnoType()->Size;

						// Check if the transport still has the budget for the new passenger.
						// Note that, if not by size, then the transport is momentarily removed from "tTransports" as soon as it's full,
						// so a transport not by size will not need to check against the passenger budget.
						if (transports[index].second + passengerSize > pType->Passengers)
							continue;
					}

					if (pPassenger->GetCurrentMission() != Mission::Enter)
					{
						pPassenger->QueueMission(Mission::Enter, false);
						pPassenger->SetTarget(nullptr);
						pPassenger->SetDestination(pTransport, true);
						transports[index].second += passengerSize; // increase the virtual size of transport
					}
				}
				passengerMap[leastpID].erase(passengerMap[leastpID].begin(), passengerMap[leastpID].begin() + index);
			}

			// Remove fully loaded transports from potential transports array.
			// Tank Bunkers are fully loaded by one "passenger".
			transports.erase(
				std::remove_if(transports.begin(), transports.end(),
					[](auto transport)
					{
						BuildingTypeClass* pBuildingType;
						return transport.second == transport.first->GetTechnoType()->Passengers
							|| (transport.second > 0 && transport.first->WhatAmI() == AbstractType::Building
								&& (pBuildingType = abstract_cast<BuildingTypeClass*>(transport.first->GetTechnoType()))
								&& pBuildingType->Bunker);
					}),
				transports.end());

			if (passengerMap[leastpID].size() == 0)
			{
				passengerMap.erase(leastpID);
			}
			else
			{
				passengerMapIdle[leastpID] = passengerMap[leastpID];
			}
		}

		if (passengerMapIdle.size() != 0 && transports.size() != 0)
			std::swap(passengerMap, passengerMapIdle);
		else
			break;
	}
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

	// This array is for Bio Reactors.
	// A Bio Reactor is a building with "Passengers > 0" and "InfantryAbsorb=yes".
	std::vector<std::pair<TechnoClass*, int>> bioReactorIndexArray;

	// This array is for the infantry candidates of Bio Reactors.
	// Unlike transports, mind-controlled ones can be loaded into Bio Reactors.
	std::vector<TechnoClass*> bioReactorCandidateArray;

	// This array is for Tank Bunkers.
	// A Tank Bunker is a building with "Bunker=yes", "NumberOfDocks > 0", and is not yet fully docked.
	std::vector<std::pair<TechnoClass*, int>> tankBunkerIndexArray;

	// This array is for the unit candidates for Tank Bunkers.
	// Unlike transports, mind-controlled ones and those mind-controlling something can be loaded into a Tank Bunker.
	std::vector<TechnoClass*> tankBunkerCandidateArray;

	// Get current selected units.
	// The first iteration, we find units that can't be transports, and add them to the passenger arrays.
	// We also find Bio Reactors, Tank Bunkers, and candidates for them.
	for (int i = 0; i < ObjectClass::CurrentObjects->Count; i++)
	{
		auto pUnit = ObjectClass::CurrentObjects->GetItem(i);
		// try to cast to TechnoClass
		TechnoClass* pTechno = abstract_cast<TechnoClass*>(pUnit);

		// If not a techno, or is in air, then it can't be a passenger, and it can't be a Bio Reactor or Tank Bunker.
		if (!pTechno || pTechno->IsInAir())
			continue;

		// Detect Bio Reactors and Tank Bunkers.
		if (pTechno->WhatAmI() == AbstractType::Building)
		{
			auto pBuilding = abstract_cast<BuildingClass*>(pTechno);
			auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechno->GetTechnoType());
			if (pBuildingType->Passengers > 0 && pBuildingType->InfantryAbsorb)
				bioReactorIndexArray.push_back(std::make_pair(pTechno, pTechno->Passengers.NumPassengers));
			else if (pBuildingType->Bunker && !pBuilding->BunkerLinkedItem)
				tankBunkerIndexArray.push_back(std::make_pair(pTechno, 0));
			continue;
		}

		// Detect candidates for Bio Reactors and Tank Bunkers.
		// A Bio Reactor candidate is an Infantry that isn't mind-controlling something.
		// A Tank Bunker candidate is a Vehicle with "Turret=yes" and "Bunkerable=yes".
		if (pTechno->WhatAmI() == AbstractType::Infantry && (!pTechno->CaptureManager || !pTechno->CaptureManager->IsControllingSomething()))
			bioReactorCandidateArray.push_back(pTechno);
		else if (pTechno->WhatAmI() == AbstractType::Unit && pTechno->GetTechnoType()->Turret && pTechno->GetTechnoType()->Bunkerable)
			tankBunkerCandidateArray.push_back(pTechno);

		// If MCed, or MCs anything, then it can't be a passenger.
		if (pTechno->IsMindControlled() || (pTechno->CaptureManager && pTechno->CaptureManager->IsControllingSomething()))
			continue;

		auto pTechnoType = pTechno->GetTechnoType();
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		// If it's an Infantry, or it's a Unit with no passenger slots or unable to manually load, add it to the passenger arrays.
		if ((pTechno->WhatAmI() == AbstractType::Infantry || (pTechno->WhatAmI() == AbstractType::Unit && (pTechnoType->Passengers <= 0 || (pTypeExt && pTypeExt->NoManualEnter)))))
		{
			int const size = pTechnoType->Size;
			passengerSizes.insert(size);
			passengerMap[size].push_back(pTechno);
		}
	}

	// Get current selected units.
	// The second iteration, we find units that can be transports.
	for (int i = 0; i < ObjectClass::CurrentObjects->Count; i++)
	{
		auto pUnit = ObjectClass::CurrentObjects->GetItem(i);
		// try to cast to TechnoClass
		TechnoClass* pTechno = abstract_cast<TechnoClass*>(pUnit);

		// If not a techno, or is in air, then it can't be a vehicle.
		if (!pTechno || pTechno->IsInAir())
			continue;

		auto pTechnoType = pTechno->GetTechnoType();
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
		if (pTechno->WhatAmI() == AbstractType::Unit)
		{
			auto const bySize = pTypeExt && pTypeExt->Passengers_BySize;
			auto const noManualEnter = pTypeExt && pTypeExt->NoManualEnter;

			// If "Passengers.BySize=false" then only the number of passengers matter.
			if (pTechnoType->Passengers > 0 && !noManualEnter
				&& pTechno->Passengers.NumPassengers < pTechnoType->Passengers
				&& (!bySize || pTechno->Passengers.GetTotalSize() < pTechnoType->Passengers))
			{
				if (pTypeExt->CanLoadAny(pTechno, passengerMap, passengerSizes))
				{
					int const sizeLimit = int(pTechnoType->SizeLimit);
					transportSizeLimits.insert(sizeLimit);
					auto const transportTotalSize = bySize ? pTechno->Passengers.GetTotalSize() : pTechno->Passengers.NumPassengers;
					transportMap[sizeLimit].push_back(std::make_pair(pTechno, transportTotalSize));
				}
				else
				{
					// If it can't actually load anything that is clearly a passenger,
					// then put this unit into ambiguousity.
					int const size = int(pTechnoType->Size);
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
		for (auto ambiguousSizesItr = ambiguousSizes.begin();
			ambiguousSizesItr != ambiguousSizes.end();
			ambiguousSizesItr++)
		{
			auto const ambiguousSize = *ambiguousSizesItr;

			if (ambiguousMap.contains(ambiguousSize))
			{
				auto const ambiguousVector = ambiguousMap[ambiguousSize];
				for (auto ambiguousVectorItr = ambiguousVector.begin();
					ambiguousVectorItr != ambiguousVector.end();
					ambiguousVectorItr++)
				{
					auto const pAmbiguousTechno = *ambiguousVectorItr;
					auto const pTechnoType = pAmbiguousTechno->GetTechnoType();
					auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
					if (ambiguousPassengerVector.empty() || !pTypeExt->CanLoadAny(pAmbiguousTechno, ambiguousPassengerVector))
					{
						// If MCed, or MCs anything, then it can't be a passenger.
						// In this case, this unit in ambiguousity is added to neither passengers nor transports.
						if (pAmbiguousTechno->IsMindControlled() || (pAmbiguousTechno->CaptureManager && pAmbiguousTechno->CaptureManager->IsControllingSomething()))
							continue;
						ambiguousPassengerVector.push_back(pAmbiguousTechno);
						int const size = int(pTechnoType->Size);
						passengerSizes.insert(size);
						passengerMap[size].push_back(pAmbiguousTechno);
					}
					else
					{
						int const sizeLimit = int(pTechnoType->SizeLimit);
						auto const bySize = pTypeExt && pTypeExt->Passengers_BySize;
						transportSizeLimits.insert(sizeLimit);
						auto const transportTotalSize = bySize ? pAmbiguousTechno->Passengers.GetTotalSize() : pAmbiguousTechno->Passengers.NumPassengers;
						transportMap[sizeLimit].push_back(std::make_pair(pAmbiguousTechno, transportTotalSize));
					}
				}
			}
		}
	}

	// Pair the passengers and the transports if possible.
	if (!passengerSizes.empty() && !transportSizeLimits.empty())
	{
		for (auto passengerSizesItr = passengerSizes.begin();
			passengerSizesItr != passengerSizes.end();
			passengerSizesItr++)
		{
			int passengerSize = *passengerSizesItr;
			if (passengerMap.contains(passengerSize))
			{
				auto passengerVector = passengerMap[passengerSize];

				for (auto transportSizeLimitsItr = transportSizeLimits.begin();
					transportSizeLimitsItr != transportSizeLimits.end();
					transportSizeLimitsItr++)
				{
					int transportSizeLimit = *transportSizeLimitsItr;

					// If the transports are too small for the passengers then skip them.
					if (transportSizeLimit < passengerSize)
						continue;

					if (transportMap.contains(transportSizeLimit))
					{
						auto transportVector = transportMap[transportSizeLimit];
						SpreadPassengersToTransports(passengerVector, transportVector);
					}
				}
			}
		}
	}
	else
	{
		// If nothing can load then go find the Bio Reactors and Tank Bunkers.
		if (bioReactorIndexArray.size() > 0 && bioReactorCandidateArray.size() > 0)
			SpreadPassengersToTransports(bioReactorCandidateArray, bioReactorIndexArray);
		if (tankBunkerIndexArray.size() > 0 && tankBunkerCandidateArray.size() > 0)
			SpreadPassengersToTransports(tankBunkerCandidateArray, tankBunkerIndexArray);
	}
}
