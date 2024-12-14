// For selected units, pair the loadable vehicle and infantry.
// 1. If transport can load vehicle, it won't load if the vehicle can be paired with infantry.
// 2. It will always try to load the unit type with least amount of units, eg. 2 GI and 3 GGI, it will match 2 GI to transport first.
// 3. For transport with multiple seats, it will try to fill the seats diversely.

#include "AutoLoad.h"
#include "Utilities/GeneralUtils.h"
#include "Ext/Techno/Body.h"
#include <unordered_map>

const char *AutoLoadCommandClass::GetName() const
{
	return "AutoLoad";
}

const wchar_t *AutoLoadCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_LOAD", L"Auto Load");
}

const wchar_t *AutoLoadCommandClass::GetUICategory() const
{
	return CATEGORY_SELECTION;
}

const wchar_t *AutoLoadCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_LOAD_DESC", L"Auto Load");
}

void DebugPrintTransport(std::vector<std::pair<TechnoClass *, int>> &tTransports)
{
	Debug::Log("AutoLoadCommandClass::DebugPrintTransport: Transport count: %d\n", tTransports.size());
	// print address of each transport and its passengers
	for (auto transport : tTransports)
	{
		Debug::Log("AutoLoadCommandClass::DebugPrintTransport: Transport address: %p, Now size: %d, Virtual size: %d\n", transport.first, transport.first->Passengers.GetTotalSize(), transport.second);
	}
}

void DebugPrintPassenger(std::vector<TechnoClass *> &pPassengers)
{
	Debug::Log("AutoLoadCommandClass::DebugPrintPassenger: Passenger count: %d\n", pPassengers.size());
	// print address of each passenger
	for (auto passenger : pPassengers)
	{
		Debug::Log("AutoLoadCommandClass::DebugPrintPassenger: Passenger address: %p\n", passenger);
	}
}

template <typename P, typename T>
void SpreadPassengersToTransports(std::vector<P> &pPassengers, std::vector<std::pair<T, int>> &tTransports)
{
	// 1. Get the least kind of passengers
	// 2. Send the passengers to the transport in round robin, if the transport is full, remove it from the vector tTransports;
	// if the pID vector is empty, remove it from the map, if not, move it to passengerMapIdle. We try to fill the transport evenly for each kind of passengers.
	// 3. Repeat until all passengers are sent or the vector tTransports is empty.
	std::unordered_map<const char *, std::vector<P>> passengerMap;
	std::unordered_map<const char *, std::vector<P>> passengerMapIdle;

	for (auto pPassenger : pPassengers)
	{
		auto pID = pPassenger->get_ID();
		if (passengerMap.find(pID) == passengerMap.end())
		{
			passengerMap[pID] = std::vector<P>();
		}
		passengerMap[pID].push_back(pPassenger);
	}

	while (true)
	{
		while (passengerMap.size() > 0 && tTransports.size() > 0)
		{
			const char *leastpID = nullptr;
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
				for (; index < leastSize && index < tTransports.size(); index++)
				{
					auto pPassenger = passengerMap[leastpID][index];
					auto pTransport = tTransports[index].first;
					auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType());
					bool bySize = true;
					// Check passenger filter here.
					if (pTypeExt)
					{
						if (!pTypeExt->CanLoadPassenger(pPassenger))
						{
							continue;
						}
						bySize = pTypeExt->Passengers_BySize;
					}
					if (pPassenger->GetCurrentMission() != Mission::Enter)
					{
						pPassenger->QueueMission(Mission::Enter, false);
						pPassenger->SetTarget(nullptr);
						pPassenger->SetDestination(pTransport, true);
						if (bySize)
						{
							tTransports[index].second += abstract_cast<TechnoClass*>(pPassenger)->GetTechnoType()->Size; // increase the virtual size of transport
						}
						else
						{
							// If "Passengers.BySize=false" then only the number of passengers matter.
							tTransports[index].second++;
						}
					}
				}
				passengerMap[leastpID].erase(passengerMap[leastpID].begin(), passengerMap[leastpID].begin() + index);
			}

			tTransports.erase(
				std::remove_if(tTransports.begin(), tTransports.end(),
							   [](auto transport)
							   {
								   return transport.second == transport.first->GetTechnoType()->Passengers;
							   }),
				tTransports.end());

			if (passengerMap[leastpID].size() == 0)
			{
				passengerMap.erase(leastpID);
			}
			else
			{
				passengerMapIdle[leastpID] = passengerMap[leastpID];
			}
		}
		if (passengerMapIdle.size() != 0 && tTransports.size() != 0)
		{
			std::swap(passengerMap, passengerMapIdle);
		}
		else
		{
			break;
		}
	}
}

void AutoLoadCommandClass::Execute(WWKey eInput) const
{
	MapClass::Instance->SetTogglePowerMode(0);
	MapClass::Instance->SetWaypointMode(0, false);
	MapClass::Instance->SetRepairMode(0);
	MapClass::Instance->SetSellMode(0);

	std::vector<TechnoClass *> infantryIndexArray;
	std::vector<std::pair<TechnoClass *, int>> vehicleIndexArray;
	// vehicle that can hold size larger than 2 passenger index array
	std::vector<std::pair<TechnoClass *, int>> largeVehicleIndexArray;
	// full vehicle may be passenger.
	std::vector<TechnoClass *> mayBePassengerArray;
	// get current selected units.
	for (int i = 0; i < ObjectClass::CurrentObjects->Count; i++)
	{
		auto pUnit = ObjectClass::CurrentObjects->GetItem(i);
		// try to cast to TechnoClass
		TechnoClass *pTechno = abstract_cast<TechnoClass *>(pUnit);

		if (pTechno && pTechno->WhatAmI() == AbstractType::Infantry && !pTechno->IsInAir())
		{
			infantryIndexArray.push_back(pTechno);
		}
		else if (pTechno && pTechno->WhatAmI() == AbstractType::Unit && !pTechno->IsInAir())
		{
			auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());
			bool bySize = true;
			if (pTypeExt)
			{
				// If "NoManualEnter=true" then the transport is excluded from auto load feature. It may still become a passenger.
				if (pTypeExt->NoManualEnter)
				{
					mayBePassengerArray.push_back(pTechno);
					continue;
				}
				bySize = pTypeExt->Passengers_BySize;
			}

			// If "Passengers.BySize=false" then only the number of passengers matter.
			auto const pType = pTechno->GetTechnoType();
			if (pType->Passengers > 0 && pTechno->Passengers.NumPassengers < pType->Passengers && (!bySize || pTechno->Passengers.GetTotalSize() < pType->Passengers))
			{
				auto const transportTotalSize = bySize ? pTechno->Passengers.GetTotalSize() : pTechno->Passengers.NumPassengers;
				if (pTechno->GetTechnoType()->SizeLimit > 2)
				{
					largeVehicleIndexArray.push_back(std::make_pair(pTechno, transportTotalSize));
				}
				else
				{
					vehicleIndexArray.push_back(std::make_pair(pTechno, transportTotalSize));
				}
			}
			else
			{
				mayBePassengerArray.push_back(pTechno);
			}
		}
	}

	// pair the infantry and vehicle
	if (vehicleIndexArray.size() > 0 && infantryIndexArray.size() > 0)
	{
		SpreadPassengersToTransports(infantryIndexArray, vehicleIndexArray);
	}
	else if (largeVehicleIndexArray.size() > 0)
	{
		// load both infantry and vehicle into large vehicle
		auto &passengerIndexArray = infantryIndexArray;
		for (auto vehicle : vehicleIndexArray)
		{
			passengerIndexArray.push_back(vehicle.first);
		}
		for (auto mayBePassenger : mayBePassengerArray)
		{
			passengerIndexArray.push_back(mayBePassenger);
		}
		SpreadPassengersToTransports(passengerIndexArray, largeVehicleIndexArray);
	}
}
