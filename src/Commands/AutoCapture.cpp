#include "AutoCapture.h"
#include "Utilities/GeneralUtils.h"
#include "Ext/Techno/Body.h"
#include <Utilities/Helpers.Alex.h>
#include "AutoLoad.h"
#include <TacticalClass.h>

const char* AutoCaptureCommandClass::GetName() const
{
	if (TechBuildingsOnly)
		return "AutoCapture.Tech";
	else
		return "AutoCapture.Base";
}

const wchar_t* AutoCaptureCommandClass::GetUIName() const
{
	if (TechBuildingsOnly)
		return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_CAPTURE_T", L"Auto Capture Tech Buildings");
	else
		return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_CAPTURE_B", L"Auto Capture Base");
}

const wchar_t* AutoCaptureCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* AutoCaptureCommandClass::GetUIDescription() const
{
	if (TechBuildingsOnly)
		return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_CAPTURE_T_DESC", L"Auto Capture Tech Buildings");
	else
		return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_CAPTURE_B_DESC", L"Auto Capture Base");
}

inline const static bool CanCaptureBuilding(HouseClass* pEngineerOwner, BuildingClass* pBuilding, bool TechBuildingsOnly)
{
	if (pBuilding->Type->Capturable
		&& (pBuilding->Owner->IsNeutral() || !pBuilding->Owner->IsAlliedWith(pEngineerOwner)))
	{
		return !TechBuildingsOnly || pBuilding->Type->NeedsEngineer;
	}
	return false;
}

void AutoCaptureCommandClass::Execute(WWKey eInput) const
{
	MapClass::Instance->SetTogglePowerMode(0);
	MapClass::Instance->SetWaypointMode(0, false);
	MapClass::Instance->SetRepairMode(0);
	MapClass::Instance->SetSellMode(0);

	std::vector<TechnoClass*> engineerVector;
	HouseClass* pEngineerOwner = nullptr;

	for (const auto& pUnit : ObjectClass::CurrentObjects())
	{
		// try to cast to InfantryClass
		InfantryClass* pInfantry = abstract_cast<InfantryClass*>(pUnit);

		// If not an infantry, or is not under control of the current player, or is in air, then exclude it from the iteration.
		if (!pInfantry || pInfantry->Berzerk || !pInfantry->Owner->IsControlledByCurrentPlayer() || pInfantry->IsInAir())
			continue;

		if (pInfantry->Type->Engineer)
		{
			if (!pEngineerOwner)
				pEngineerOwner = pInfantry->Owner;
			if (pInfantry->Owner == pEngineerOwner)
				engineerVector.push_back(pInfantry);
		}
	}

	// If no valid engineer selected, we don't do anything in advance.
	if (engineerVector.empty())
		return;

	std::vector<std::pair<TechnoClass*, int>> capturableStructuresSelectedVector;

	std::vector<std::pair<TechnoClass*, int>> capturableStructuresVector;

	for (const auto& pUnit : ObjectClass::CurrentObjects())
	{
		// try to cast to BuildingClass
		BuildingClass* pBuilding = abstract_cast<BuildingClass*>(pUnit);

		// If not an building, or is not under control of the current player, or is in air, then exclude it from the iteration.
		if (!pBuilding || pBuilding->Berzerk || pBuilding->IsInAir())
			continue;

		// - There is currently no need to check if object owner is current player,
		//   because this hotkey requires at least 2 objects to be selected to do something,
		//   however the player is normally unable to select 2 objects that are not owned at a same time.
		// - Even though we can make use of "MultiSelectNeutrals.Garrisonable" and like,
		//   this hotkey does nothing anyway if no owned units are selected.

		// If it can be captured then add into the top priority list
		if (CanCaptureBuilding(pEngineerOwner, pBuilding, TechBuildingsOnly))
		{
			auto const pBuildingExt = TechnoExt::ExtMap.Find(pBuilding);
			auto const budget = pBuildingExt->NeedsMultipleEngineers() ? 10 : 1;
			capturableStructuresSelectedVector.push_back(std::make_pair(pBuilding, budget));
		}
	}

	static auto copy_dvc = []<typename T>(const DynamicVectorClass<T>&dvc)
	{
		std::vector<T> vec(dvc.Count);
		std::copy(dvc.begin(), dvc.end(), vec.begin());
		return vec;
	};

	// find capturable structures in the player's camera
	for (auto pBuilding : copy_dvc(*BuildingClass::Array))
	{
		if (!pBuilding || !pBuilding->IsOnMap || !pBuilding->IsAlive || pBuilding->InLimbo || pBuilding->IsSinking)
			continue;

		// already checked above, no need to do here
		if (pBuilding->IsSelected)
			continue;

		// checks if the building is visible in the player's camera
		if (!MapClass::Instance->IsWithinUsableArea(pBuilding->GetCoords())
			|| !pBuilding->DiscoveredByCurrentPlayer
			|| !TacticalClass::Instance->CoordsToClient(pBuilding->GetCoords()).second)
			continue;

		if (pBuilding->IsVisible
			&& CanCaptureBuilding(pEngineerOwner, pBuilding, TechBuildingsOnly))
		{
			auto const pBuildingExt = TechnoExt::ExtMap.Find(pBuilding);
			auto const budget = pBuildingExt->NeedsMultipleEngineers() ? 10 : 1;
			capturableStructuresVector.push_back(std::make_pair(pBuilding, budget));
		}
	}

	if (!capturableStructuresSelectedVector.empty())
	{
		auto foundTransportSet = AutoLoadCommandClass::SpreadPassengersToTransports(engineerVector, capturableStructuresSelectedVector, Action::Capture, true);
		engineerVector.erase(
			std::remove_if(engineerVector.begin(), engineerVector.end(),
				[foundTransportSet](auto pPassenger)
				{
					return foundTransportSet.contains(pPassenger);
				}),
			engineerVector.end());
		if (engineerVector.empty())
			return;
	}

	if (!capturableStructuresVector.empty())
	{
		AutoLoadCommandClass::SpreadPassengersToTransports(engineerVector, capturableStructuresVector, Action::Capture, true);
	}
}
