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

	std::vector<std::pair<TechnoClass*, int>> capturableStructuresVector;

	static auto copy_dvc = []<typename T>(const DynamicVectorClass<T>&dvc)
	{
		std::vector<T> vec(dvc.Count);
		std::copy(dvc.begin(), dvc.end(), vec.begin());
		return vec;
	};

	auto const multiEngineer = RulesClass::Instance()->EngineerCaptureLevel < 1.0;
	auto const multiEngineerTech = multiEngineer && !RulesExt::Global()->EngineerAlwaysCaptureTech;

	// find capturable structures in the player's camera
	for (auto pBuilding : copy_dvc(*BuildingClass::Array))
	{
		if (!pBuilding || !pBuilding->IsOnMap || !pBuilding->IsAlive || pBuilding->InLimbo || pBuilding->IsSinking)
			continue;

		// checks if the building is visible in the player's camera
		if (!TacticalClass::Instance->CoordsToClient(pBuilding->GetCenterCoords()).second)
			continue;

		if (pBuilding->IsVisible
			&& pBuilding->Type->Capturable
			&& (pBuilding->Owner->IsNeutral() || !pBuilding->Owner->IsAlliedWith(pEngineerOwner)))
		{
			auto const isTechBuilding = pBuilding->Type->NeedsEngineer;
			if (TechBuildingsOnly && !isTechBuilding)
				continue;

			auto const budget = (isTechBuilding ? multiEngineerTech : multiEngineer) ? 10 : 1;
			capturableStructuresVector.push_back(std::make_pair(pBuilding, budget));
		}
	}

	if (!capturableStructuresVector.empty())
	{
		AutoLoadCommandClass::SpreadPassengersToTransports(engineerVector, capturableStructuresVector);
	}
}
