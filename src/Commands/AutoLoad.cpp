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

void DebugPrintTransport(std::vector<std::pair<TechnoClass *, int>> &transports)
{
	Debug::Log("AutoLoadCommandClass::DebugPrintTransport: Transport count: %d\n", transports.size());
	// print address of each transport and its passengers
	for (auto transport : transports)
		Debug::Log("AutoLoadCommandClass::DebugPrintTransport: Transport address: %p, Now size: %d, Virtual size: %d\n", transport.first, transport.first->Passengers.GetTotalSize(), transport.second);
}

void DebugPrintPassenger(std::vector<TechnoClass *> &passengers)
{
	Debug::Log("AutoLoadCommandClass::DebugPrintPassenger: Passenger count: %d\n", passengers.size());
	// print address of each passenger
	for (auto passenger : passengers)
		Debug::Log("AutoLoadCommandClass::DebugPrintPassenger: Passenger address: %p\n", passenger);
}

template <typename TPassenger, typename TTransport>
void SpreadPassengersToTransports(std::vector<TPassenger> &passengers, std::vector<std::pair<TTransport, int>> &transports)
{
	// 1. Get the least kind of passengers
	// 2. Send the passengers to the transport in round robin, if the transport is full, remove it from the vector transports;
	// if the pID vector is empty, remove it from the map, if not, move it to passengerMapIdle. We try to fill the transport evenly for each kind of passengers.
	// 3. Repeat until all passengers are sent or the vector transports is empty.
	std::unordered_map<const char *, std::vector<TPassenger>> passengerMap;
	std::unordered_map<const char *, std::vector<TPassenger>> passengerMapIdle;

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
			for (auto const &[pID, pPassenger] : passengerMap)
			{
				if (pPassenger.size() < leastSize)
				{
					leastSize = pPassenger.size();
					leastpID = pID;
				}
			}

			{
				unsigned int index = 0;
				for ( ; index < leastSize && index < transports.size(); index++)
				{
					auto pPassenger = passengerMap[leastpID][index];
					auto pTransport = transports[index].first;
					auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType());
					bool bySize = true;
					int passengerSize = 1;

					if (pTypeExt)
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
						if (tTransports[index].second + passengerSize > pTransport->GetTechnoType()->Passengers)
							continue;
					}

					if (pPassenger->GetCurrentMission() != Mission::Enter)
					{
						pPassenger->QueueMission(Mission::Enter, false);
						pPassenger->SetTarget(nullptr);
						pPassenger->SetDestination(pTransport, true);
						tTransports[index].second += passengerSize; // increase the virtual size of transport
					}
				}
				passengerMap[leastpID].erase(passengerMap[leastpID].begin(), passengerMap[leastpID].begin() + index);
			}

			transports.erase(
				std::remove_if(transports.begin(), transports.end(),
							   [](auto transport)
							   {
								   return transport.second == transport.first->GetTechnoType()->Passengers;
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

	// This array is for the standard passengers, most Infantry and small vehicles like terror drones go here.
	// A techno is added to this array if:
	// 1. It's an Infantry with size <= 2;
	// 2. It's a Vehicle with size <= 2, and either has 0 passenger slots, or disallow manual loading.
	std::vector<TechnoClass *> passengerArray;

	// This array is for larger passengers like tanks. It is not necessarily larger, but has lower loading priority.
	// A techno is added to this array if:
	// 1. It's an Infantry with size >= 3;
	// 2. It's a Vehicle with size >= 3, and either has 0 passenger slots, or disallow manual loading;
	// 3. It's a Vehicle with passenger slots, allows manual loading, and has size limit <= 2.
	//
	// This array is only loaded into large vehicles if:
	//   - either "passengerArray" is empty;
	//   - or nothing in the "passengerArray" can actually be loaded into non-large vehicles.
	std::vector<TechnoClass *> largePassengerArray;

	// vehicles that can hold size <= 2
	std::vector<std::pair<TechnoClass *, int>> vehicleIndexArray;
	// vehicles that can hold size >= 3
	std::vector<std::pair<TechnoClass *, int>> largeVehicleIndexArray;

	// Get current selected units.
	// The first iteration, we find units that can't be transports, and add them to the passenger arrays.
	for (int i = 0; i < ObjectClass::CurrentObjects->Count; i++)
	{
		auto pUnit = ObjectClass::CurrentObjects->GetItem(i);
		// try to cast to TechnoClass
		TechnoClass* pTechno = abstract_cast<TechnoClass *>(pUnit);

		// If not a techno, or is in air, or is MCed, or MCs anything, then it can't be a passenger.
		if (!pTechno || pTechno->IsInAir() || pTechno->IsMindControlled() || (pTechno->CaptureManager && pTechno->CaptureManager->IsControllingSomething()))
		{
			continue;
		}

		auto pTechnoType = pTechno->GetTechnoType();
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		// If it's an Infantry, or it's a Unit with no passenger slots or unable to manually load, add it to the passenger arrays.
		if ((pTechno->WhatAmI() == AbstractType::Infantry || (pTechno->WhatAmI() == AbstractType::Unit && (pTechnoType->Passengers <= 0 || (pTypeExt && pTypeExt->NoManualEnter)))))
		{
			if (pTechnoType->Size <= 2)
				passengerArray.push_back(pTechno);
			else
				largePassengerArray.push_back(pTechno);
		}
	}

	// Get current selected units.
	// The second iteration, we find units that can be transports.
	for (int i = 0; i < ObjectClass::CurrentObjects->Count; i++)
	{
		auto pUnit = ObjectClass::CurrentObjects->GetItem(i);
		// try to cast to TechnoClass
		TechnoClass* pTechno = abstract_cast<TechnoClass *>(pUnit);

		// If not a techno, or is in air, then it can't be a vehicle.
		if (!pTechno || pTechno->IsInAir())
			continue;

		auto pTechnoType = pTechno->GetTechnoType();
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
		if (pTechno->WhatAmI() == AbstractType::Unit)
		{
			bool bySize = pTypeExt && pTypeExt->Passengers_BySize;

			// If "passengers.BySize=false" then only the number of passengers matter.
			if (pTechnoType->Passengers > 0 && pTechno->Passengers.NumPassengers < pTechnoType->Passengers && (!bySize || pTechno->Passengers.GetTotalSize() < pTechnoType->Passengers))
			{
				auto const transportTotalSize = bySize ? pTechno->Passengers.GetTotalSize() : pTechno->Passengers.NumPassengers;
				if (pTechnoType->SizeLimit > 2)
				{
					largeVehicleIndexArray.push_back(std::make_pair(pTechno, transportTotalSize));
					continue;
				}
				else if (!pTypeExt || pTypeExt->CanLoadAny(pTechno, passengerArray))
				{
					vehicleIndexArray.push_back(std::make_pair(pTechno, transportTotalSize));
					continue;
				}
			}

			// If MCed, or MCs anything, then it can't be a passenger.
			if (pTechno->IsMindControlled() || (pTechno->CaptureManager && pTechno->CaptureManager->IsControllingSomething()))
				continue;

			largePassengerArray.push_back(pTechno);
		}
	}

	// pair the infantry and vehicle
	if (vehicleIndexArray.size() > 0 && passengerArray.size() > 0)
	{
		SpreadPassengersToTransports(passengerArray, vehicleIndexArray);
	}
	else if (largeVehicleIndexArray.size() > 0)
	{
		// load both infantry and vehicle into large vehicle
		auto &passengerIndexArray = passengerArray;
		for (auto largePassenger : largePassengerArray)
		{
			passengerIndexArray.push_back(largePassenger);
		}
		SpreadPassengersToTransports(passengerIndexArray, largeVehicleIndexArray);
	}
}
