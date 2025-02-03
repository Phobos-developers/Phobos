#include "AutoGarrison.h"
#include "Utilities/GeneralUtils.h"
#include "Ext/Techno/Body.h"
#include <Utilities/Helpers.Alex.h>
#include "AutoLoad.h"
#include <TacticalClass.h>

const char* AutoGarrisonCommandClass::GetName() const
{
	return "AutoGarrison";
}

const wchar_t* AutoGarrisonCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_GARRISON", L"Auto Garrison");
}

const wchar_t* AutoGarrisonCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* AutoGarrisonCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_GARRISON_DESC", L"Auto Garrison");
}

void AutoGarrisonCommandClass::Execute(WWKey eInput) const
{
	MapClass::Instance->SetTogglePowerMode(0);
	MapClass::Instance->SetWaypointMode(0, false);
	MapClass::Instance->SetRepairMode(0);
	MapClass::Instance->SetSellMode(0);

	std::vector<TechnoClass*> occupantVector;

	for (const auto& pUnit : ObjectClass::CurrentObjects())
	{
		// try to cast to InfantryClass
		InfantryClass* pInfantry = abstract_cast<InfantryClass*>(pUnit);

		// If not an infantry, or is not under control of the current player, or is in air, then exclude it from the iteration.
		if (!pInfantry || pInfantry->Berzerk || !pInfantry->Owner->IsControlledByCurrentPlayer() || pInfantry->IsInAir())
			continue;

		if (AutoLoadCommandClass::CanBeBuildingPassenger(pInfantry))
		{
			occupantVector.push_back(pInfantry);
		}
	}

	// If no valid infantry selected, we don't do anything in advance.
	if (occupantVector.empty())
		return;

	std::vector<std::pair<TechnoClass*, int>> occupiedVectorOwned;

	std::vector<std::pair<TechnoClass*, int>> occupiedVectorNeutral;

	static auto copy_dvc = []<typename T>(const DynamicVectorClass<T>&dvc)
	{
		std::vector<T> vec(dvc.Count);
		std::copy(dvc.begin(), dvc.end(), vec.begin());
		return vec;
	};

	// find garrisonable structures in the player's camera
	for (auto pBuilding : copy_dvc(*BuildingClass::Array))
	{
		if (!pBuilding || !pBuilding->IsOnMap || !pBuilding->IsAlive || pBuilding->InLimbo || pBuilding->IsSinking)
			continue;

		// checks if the building is visible in the player's camera
		if (!MapClass::Instance->IsWithinUsableArea(pBuilding->GetCoords())
			|| !pBuilding->DiscoveredByCurrentPlayer
			|| !TacticalClass::Instance->CoordsToClient(pBuilding->GetCoords()).second)
			continue;

		if (pBuilding->IsVisible && pBuilding->Type->CanBeOccupied)
		{
			int const budget = AutoLoadCommandClass::GetBuildingPassengerBudget(pBuilding);
			if (budget > 0)
			{
				if (pBuilding->Owner->IsControlledByCurrentPlayer())
				{
					occupiedVectorOwned.push_back(std::make_pair(pBuilding, budget));
				}
				else if (pBuilding->Owner->IsNeutral() && !pBuilding->IsRedHP())
				{
					occupiedVectorNeutral.push_back(std::make_pair(pBuilding, budget));
				}
			}
		}
	}

	if (!occupiedVectorOwned.empty())
	{
		auto foundTransportSet = AutoLoadCommandClass::SpreadPassengersToTransports(occupantVector, occupiedVectorOwned);
		occupantVector.erase(
			std::remove_if(occupantVector.begin(), occupantVector.end(),
				[foundTransportSet](auto pPassenger)
				{
					return foundTransportSet.contains(pPassenger);
				}),
			occupantVector.end());
		if (occupantVector.empty())
			return;
	}

	if (!occupiedVectorNeutral.empty())
	{
		AutoLoadCommandClass::SpreadPassengersToTransports(occupantVector, occupiedVectorNeutral);
	}
}
