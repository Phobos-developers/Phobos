#include "Body.h"

void ScriptExt::RepairDestroyedBridge(TeamClass* pTeam, int mode = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
		return;

	auto pScript = pTeam->CurrentScript;
	int currentMission = pScript->CurrentMission;

	// The first time this team runs this kind of script the repair huts list will updated. The only reason of why it isn't stored in ScenarioClass is because always exists the possibility of a modder to make destroyable Repair Huts
	if (pTeamData->BridgeRepairHuts.size() == 0)
	{
		for (auto pTechno : TechnoClass::Array)
		{
			if (pTechno->WhatAmI() != AbstractType::Building)
				continue;

			const auto pBuilding = abstract_cast<BuildingClass*>(pTechno);
			if (!pBuilding)
				continue;

			if (pBuilding->Type->BridgeRepairHut)
				pTeamData->BridgeRepairHuts.push_back(pTechno);
		}

		if (pTeamData->BridgeRepairHuts.size() == 0)
		{
			pTeam->StepCompleted = true;
			ScriptExt::Log("AI Scripts - RepairDestroyedBridge: [%s][%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No repair huts found).\n",
				pTeam->Type->ID,
				pScript->Type->ID,
				currentMission,
				pScript->Type->ScriptActions[currentMission].Action,
				pScript->Type->ScriptActions[currentMission].Argument,
				currentMission + 1,
				pScript->Type->ScriptActions[currentMission + 1].Action,
				pScript->Type->ScriptActions[currentMission + 1].Argument);

			return;
		}
	}

	// Reset Team's target if the current target isn't a repair hut
	if (pTeam->Focus)
	{
		if (pTeam->Focus->WhatAmI() != AbstractType::Building)
		{
			pTeam->Focus = nullptr;
		}
		else
		{
			const auto pBuilding = static_cast<BuildingClass*>(pTeam->Focus);

			if (!pBuilding->Type->BridgeRepairHut)
			{
				pTeam->Focus = nullptr;
			}
			else
			{
				CellStruct cell = pBuilding->GetCell()->MapCoords;

				// If the Bridge was repaired then the repair hut isn't valid anymore
				if (!MapClass::Instance.IsLinkedBridgeDestroyed(cell))
					pTeam->Focus = nullptr;
			}
		}
	}

	TechnoClass* selectedTarget = pTeam->Focus ? static_cast<TechnoClass*>(pTeam->Focus) : nullptr;
	bool isEngineerAmphibious = false;
	std::vector<FootClass*> engineers;
	std::vector<FootClass*> otherTeamMembers;

	// Check if there are no engineers
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!ScriptExt::IsUnitAvailable(pUnit, true))
			continue;

		if (!pTeam->Focus)
		{
			pUnit->SetTarget(nullptr);
			pUnit->SetDestination(nullptr, false);
			pUnit->ForceMission(Mission::Guard);
		}

		if (pUnit->WhatAmI() == AbstractType::Infantry)
		{
			const auto pInf = static_cast<InfantryClass*>(pUnit);

			if (pInf->IsEngineer())
			{
				if (pUnit->GetTechnoType()->MovementZone == MovementZone::Amphibious
				|| pUnit->GetTechnoType()->MovementZone == MovementZone::AmphibiousCrusher
				|| pUnit->GetTechnoType()->MovementZone == MovementZone::AmphibiousDestroyer)
				{
					isEngineerAmphibious = true;
				}

				engineers.push_back(pUnit);
				continue;
			}
		}

		// Non-engineers will receive a different command
		otherTeamMembers.push_back(pUnit);
	}

	if (engineers.size() == 0)
	{
		pTeam->StepCompleted = true;
		ScriptExt::Log("AI Scripts - RepairDestroyedBridge: [%s][%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: Team has no engineers).\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			currentMission,
			pScript->Type->ScriptActions[currentMission].Action,
			pScript->Type->ScriptActions[currentMission].Argument,
			currentMission + 1,
			pScript->Type->ScriptActions[currentMission + 1].Action,
			pScript->Type->ScriptActions[currentMission + 1].Argument);

		return;
	}

	std::vector<TechnoClass*> validHuts;

	if (!selectedTarget)
	{
		for (const auto pTechno : pTeamData->BridgeRepairHuts)
		{
			CellStruct cell = pTechno->GetCell()->MapCoords;

			// Skip all huts linked to non-destroyed bridges
			if (!MapClass::Instance.IsLinkedBridgeDestroyed(cell))
				continue;

			if (isEngineerAmphibious)
			{
				validHuts.push_back(pTechno);
			}
			else
			{
				CoordStruct coords = pTechno->GetCenterCoords();

				// Only huts reachable by the (first) engineer are valid
				if (engineers.at(0)->IsInSameZoneAsCoords(pTechno->GetCenterCoords()))
					validHuts.push_back(pTechno);
			}
		}

		if (validHuts.size() == 0)
		{
			ScriptExt::Log("AI Scripts - RepairDestroyedBridge: [%s][%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (Reason: Can not select a bridge repair hut).\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			currentMission,
			pScript->Type->ScriptActions[currentMission].Action,
			pScript->Type->ScriptActions[currentMission].Argument,
			currentMission + 1,
			pScript->Type->ScriptActions[currentMission + 1].Action,
			pScript->Type->ScriptActions[currentMission + 1].Argument);

			pTeam->StepCompleted = true;
			return;
		}

		// Find the best repair hut
		int bestVal = -1;

		if (mode < 0)
			mode = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

		if (mode < 0) // Pick a random bridge
		{
			selectedTarget = validHuts.at(ScenarioClass::Instance->Random.RandomRanged(0, validHuts.size() - 1));
		}
		else
		{
			for (const auto pHut : validHuts)
			{
				int value = engineers.at(0)->DistanceFrom(pHut); // Note: distance is in leptons (*256)
				bool isValidCandidate = false;

				if (mode == 0)
					isValidCandidate = value < bestVal; // Pick the closest target
				else
					isValidCandidate = value >= bestVal; // Pick the farthest target

				if (isValidCandidate || bestVal < 0)
				{
					bestVal = value;
					selectedTarget = pHut;
				}
			}
		}
	}

	validHuts.clear();

	if (!selectedTarget)
	{
		ScriptExt::Log("AI Scripts - RepairDestroyedBridge: [%s][%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (Reason: Can not select a bridge repair hut).\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			currentMission,
			pScript->Type->ScriptActions[currentMission].Action,
			pScript->Type->ScriptActions[currentMission].Argument,
			currentMission + 1,
			pScript->Type->ScriptActions[currentMission + 1].Action,
			pScript->Type->ScriptActions[currentMission + 1].Argument);

		pTeam->StepCompleted = true;
		return;
	}

	// Setting the team's target & mission
	pTeam->Focus = selectedTarget;

	for (auto engineer : engineers)
	{
		if (engineer->Destination != selectedTarget)
		{
			engineer->SetTarget(selectedTarget);
			engineer->QueueMission(Mission::Capture, true);
		}
	}

	if (otherTeamMembers.size() > 0)
	{
		double closeEnough = RulesClass::Instance->CloseEnough; // Note: this value is in leptons (*256)

		for (auto pFoot : otherTeamMembers)
		{
			if (pTeamData && pTeamData->CloseEnough > 0)
				closeEnough = pTeamData->CloseEnough * 256.0;

			if (!pFoot->Destination
				|| (selectedTarget->DistanceFrom(pFoot->Destination) > closeEnough))
			{
				// Reset previous command
				pFoot->SetTarget(nullptr);
				pFoot->SetDestination(nullptr, false);
				pFoot->ForceMission(Mission::Guard);

				// Get a cell near the target
				pFoot->QueueMission(Mission::Move, false);
				CoordStruct coord = TechnoExt::PassengerKickOutLocation(selectedTarget, pFoot);
				CellClass* pCellDestination = MapClass::Instance.TryGetCellAt(coord);
				pFoot->SetDestination(pCellDestination, true);
			}

			// Reached destination, stay in guard until next action
			if (pFoot->DistanceFrom(pFoot->Destination) < closeEnough)
				pFoot->QueueMission(Mission::Area_Guard, false);
		}
	}
}
