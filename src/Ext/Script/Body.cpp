#include "Body.h"

template<> const DWORD Extension<ScriptClass>::Canary = 0x3B3B3B3B;
ScriptExt::ExtContainer ScriptExt::ExtMap;

// =============================
// load / save

void ScriptExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	// Nothing yet
}

void ScriptExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	// Nothing yet
}

// =============================
// container

ScriptExt::ExtContainer::ExtContainer() : Container("ScriptClass")
{
}

ScriptExt::ExtContainer::~ExtContainer() = default;

void ScriptExt::ProcessAction(TeamClass* pTeam)
{
	const int& action = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Action;

	switch (action)
	{
	case 71:
		ScriptExt::ExecuteTimedAreaGuardAction(pTeam);
		break;
	case 72:
		ScriptExt::LoadIntoTransports(pTeam);
		break;
	case 73:
		ScriptExt::WaitUntillFullAmmoAction(pTeam);
		break;
	case 74:
		ScriptExt::Mission_Gather_NearTheLeader(pTeam, -1);
		break;
	default:
		// Do nothing because or it is a wrong Action number or it is an Ares/YR action...
		//Debug::Log("[%s] [%s] %d = %d,%d\n", pTeam->Type->ID, pScriptType->ID, pScript->idxCurrentLine, currentLineAction->Action, currentLineAction->Argument);
		break;
	}
}

void ScriptExt::ExecuteTimedAreaGuardAction(TeamClass* pTeam)
{
	auto pScript = pTeam->CurrentScript;
	auto pScriptType = pScript->Type;

	if (pTeam->GuardAreaTimer.TimeLeft == 0 && !pTeam->GuardAreaTimer.InProgress())
	{
		auto pUnit = pTeam->FirstUnit;

		pUnit->QueueMission(Mission::Area_Guard, true);
		while (pUnit->NextTeamMember)
		{
			pUnit = pUnit->NextTeamMember;
			pUnit->QueueMission(Mission::Area_Guard, true);
		}
		pTeam->GuardAreaTimer.Start(15 * pScriptType->ScriptActions[pScript->idxCurrentLine].Argument);
	}
	/*else {
		Debug::Log("[%s] [%s] %d = %d,%d (Countdown: %d)\n", pTeam->Type->ID, pScriptType->ID, pScript->idxCurrentLine, currentLineAction->Action, currentLineAction->Argument, pTeam->GuardAreaTimer.GetTimeLeft());
	}
	*/

	if (pTeam->GuardAreaTimer.Completed())
	{
		pTeam->GuardAreaTimer.Stop(); // Needed
		pTeam->StepCompleted = true;
	}
}

void ScriptExt::LoadIntoTransports(TeamClass* pTeam)
{
	DynamicVectorClass<FootClass*> transports;

	auto pUnit = pTeam->FirstUnit;
	auto pUnitType = pUnit->GetTechnoType();
	if (pUnitType->Passengers > 0
		&& pUnit->Passengers.NumPassengers < pUnitType->Passengers
		&& pUnit->Passengers.GetTotalSize() < pUnitType->Passengers)
	{
		transports.AddItem(pUnit);
	}
	while (pUnit->NextTeamMember)
	{
		pUnit = pUnit->NextTeamMember;
		pUnitType = pUnit->GetTechnoType();
		if (pUnitType->Passengers > 0
			&& pUnit->Passengers.NumPassengers < pUnitType->Passengers
			&& pUnit->Passengers.GetTotalSize() < pUnitType->Passengers)
		{
			transports.AddItem(pUnit);
		}
	}
	// We got all the transports.

	// Now add units into transports
	for (auto pTransport : transports)
	{
		pUnit = pTeam->FirstUnit;
		auto pTransprotType = pTransport->GetTechnoType();
		do
		{
			pUnitType = pUnit->GetTechnoType();
			if (!(pTransport == pUnit
				|| pUnitType->WhatAmI() == AbstractType::AircraftType
				|| pUnit->InLimbo
				|| pUnitType->ConsideredAircraft
				|| pUnit->Health <= 0))
			{
				if (pUnit->GetTechnoType()->Size > 0
					&& pUnitType->Size <= pTransprotType->SizeLimit
					&& pUnitType->Size <= pTransprotType->Passengers - pTransport->Passengers.GetTotalSize())
				{
					pUnit->IsTeamLeader = true;
					// All fine
					if (pUnit->GetCurrentMission() != Mission::Enter)
					{
						pUnit->QueueMission(Mission::Enter, false);
						pUnit->SetTarget(nullptr);
						pUnit->SetDestination(pTransport, true);

						return;
					}
				}
			}
			pUnit = pUnit->NextTeamMember;
		}
		while (pUnit);
	}

	pUnit = pTeam->FirstUnit;
	do
	{
		if (pUnit->GetCurrentMission() == Mission::Enter)
			return;
		pUnit = pUnit->NextTeamMember;
	}
	while (pUnit);

	// This action finished
	if (pTeam->CurrentScript->HasNextAction())
		pTeam->CurrentScript->idxCurrentLine += 1;
	pTeam->StepCompleted = true;
}

void ScriptExt::WaitUntillFullAmmoAction(TeamClass* pTeam)
{
	auto pUnit = pTeam->FirstUnit;

	do
	{
		if (pUnit && !pUnit->InLimbo && pUnit->Health > 0)
		{
			if (pUnit->GetTechnoType()->Ammo > 0 && pUnit->Ammo < pUnit->GetTechnoType()->Ammo)
			{
				// If an aircraft object have AirportBound it must be evaluated
				if (pUnit->WhatAmI() == AbstractType::Aircraft)
				{
					auto pAircraft = static_cast<AircraftTypeClass*>(pUnit->GetTechnoType());
					if (pAircraft->AirportBound)
					{
						// Reset last target, at long term battles this prevented the aircraft to pick a new target (rare vanilla YR bug)
						pUnit->SetTarget(nullptr);
						pUnit->LastTarget = nullptr;
						// Fix YR bug (when returns from the last attack the aircraft switch in loop between Mission::Enter & Mission::Guard, making it impossible to land in the dock)
						if (pUnit->IsInAir() && pUnit->CurrentMission != Mission::Enter)
							pUnit->QueueMission(Mission::Enter, true);
						return;
					}
				}
				else if (pUnit->GetTechnoType()->Reload != 0)
				{ // Don't skip units that can reload themselves
					return;
				}
			}
		}
		pUnit = pUnit->NextTeamMember;
	}
	while (pUnit);

	// This action finished
	/*if (pTeam->CurrentScript->HasNextAction())
	{
		pTeam->CurrentScript->idxCurrentLine += 1;
	}*/
	pTeam->StepCompleted = true;
}

void ScriptExt::Mission_Gather_NearTheLeader(TeamClass *pTeam, int countdown = -1)
{
	FootClass *pLeaderUnit = nullptr;
	//TechnoTypeClass* pLeaderUnitType = nullptr;
	int bestUnitLeadershipValue = -1;
	int initialCountdown = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Argument;
	//auto pTypeThis = pThis->GetTechnoType();
	//auto pData = TeamExt::ExtMap.Find(pThis);

	// This team has no units! END
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] Mission_Gather_NearTheLeader: END! No team\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID);
		return;
	}

	// Load countdown
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
	{
		if (pTeamData->Countdown_regroupAtLeader >= 0)
			countdown = pTeamData->Countdown_regroupAtLeader;
		Debug::Log("DEBUG: [%s] [%s] Mission_Gather_NearTheLeader: countdown = %d, initialCountdown = %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, countdown, initialCountdown);
	}
	else
	{
		// Looks like an error...
		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] Mission_Gather_NearTheLeader: END! No pTeamData found\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID);
		return;
	}

	// Find the leader
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit && pUnit->IsAlive
			&& pUnit->Health > 0
			&& !pUnit->InLimbo
			&& pUnit->IsOnMap
			&& !pUnit->Absorbed)
		{
			auto pUnitType = pUnit->GetTechnoType();

			if (pUnitType)
			{
				// The team leader will be used for selecting targets, if there are living Team Members then always exists 1 Leader.
				int unitLeadershipRating = pUnitType->LeadershipRating;
				if (unitLeadershipRating > bestUnitLeadershipValue)
				{
					pLeaderUnit = pUnit;
					bestUnitLeadershipValue = unitLeadershipRating;
				}
			}
		}
	}

	if (!pLeaderUnit)
	{
		pTeamData->Countdown_regroupAtLeader = -1;

		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] Mission_Gather_NearTheLeader: No leader!\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID);
		return;
	}

	// Move all around the leader, the leader always in "Guard Area" Mission or simply in Guard Mission
	int nTogether = 0;
	int nUnits = -1; // Leader counts here
	double closeEnough;

	//if (pTeamData->CloseEnough > 0) // Uncomment when PR #296 "New Script Actions" is merged into develop!
	//{
	//	closeEnough = pTeamData->CloseEnough;
	//	pTeamData->CloseEnough = -1; // This a one-time-use value
	//}
	//else
	//{ // Uncomment when PR #296 "New Script Actions" is merged into develop!
	closeEnough = RulesClass::Instance->CloseEnough / 256.0;
	//}

	// The leader should stay calm & be the group's center
	if (pLeaderUnit->Locomotor->Is_Moving_Now())
		pLeaderUnit->SetDestination(nullptr, false);
	pLeaderUnit->QueueMission(Mission::Guard, false);

	// Check if units are around the leader
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit && pUnit->IsAlive && pUnit->Health > 0 && !pUnit->InLimbo)
		{
			auto pTypeUnit = pUnit->GetTechnoType();

			if (pUnit == pLeaderUnit || !pTypeUnit)
			{
				nUnits++;

				continue;
			}

			// Aircraft case
			if (pTypeUnit->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo <= 0 && pTypeUnit->Ammo > 0)
			{
				auto pAircraft = static_cast<AircraftTypeClass*>(pUnit->GetTechnoType());
				if (pAircraft->AirportBound)
				{
					pUnit->QueueMission(Mission::Return, false);
					pUnit->Mission_Enter();

					continue;
				}
			}

			nUnits++;

			if (pUnit->DistanceFrom(pLeaderUnit) / 256.0 > closeEnough)
			{
				// Leader's group is too far from me. Regroup
				if (pUnit->Destination != pLeaderUnit)
				{
					pUnit->SetDestination(pLeaderUnit, false);
					pUnit->QueueMission(Mission::Move, false);
				}
			}
			else
			{
				// Is near of the leader, then protect the area
				pUnit->QueueMission(Mission::Area_Guard, true);
				nTogether++;
			}
		}
	}
	Debug::Log("DEBUG: [%s] [%s] Mission_Gather_NearTheLeader: nUnits = %d, nTogether = %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, nUnits, nTogether);

	if (initialCountdown >= 0 && nUnits >= 0 && nUnits == nTogether && countdown > 0)
	{
		countdown--; // Update countdown
		Debug::Log("DEBUG: [%s] [%s] Mission_Gather_NearTheLeader: updated countdown = %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, countdown);
		// Save counter
		pTeamData->Countdown_regroupAtLeader = countdown;
	}

	if (initialCountdown > 0 && countdown == -1)
	{
		// Start countdown.
		countdown = initialCountdown * 15;
		Debug::Log("DEBUG: [%s] [%s] Mission_Gather_NearTheLeader: set countdown = %d [initialCountdown]\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, countdown);
		pTeamData->Countdown_regroupAtLeader = countdown;
	}

	if (nUnits >= 0 && nUnits > nTogether && countdown > 0)
	{
		Debug::Log("DEBUG: [%s] [%s] Mission_Gather_NearTheLeader: units not together yet (current countdown: %d)\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, countdown);
		return;
	}

	if (nUnits >= 0 && nUnits == nTogether)
	{
		if (initialCountdown == 0)
		{
			// Case: when this Script action have no ending set then the only ending requisite is to stay together around the leader.
			pTeamData->Countdown_regroupAtLeader = -1;

			// This action finished
			pTeam->StepCompleted = true;
			Debug::Log("DEBUG: [%s] [%s] Mission_Gather_NearTheLeader: initialCountdown == 0 that finished!\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID);
			return;
		}

		if (initialCountdown > 0 && countdown == 0)
		{
			// Countdown ended.
			Debug::Log("DEBUG: [%s] [%s] Mission_Gather_NearTheLeader: countdown = %d [END]\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, countdown);
			countdown = -1;
			pTeamData->Countdown_regroupAtLeader = -1;
			initialCountdown = 0;
		}
	}

	if (initialCountdown == 0 && nUnits >= 0 && nUnits == nTogether)
	{
		// Case: when this Script action have no ending set then the only ending requisite is to stay together around the leader.
		pTeamData->Countdown_regroupAtLeader = -1;

		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] Mission_Gather_NearTheLeader: initialCountdown == 0 that finished!\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID);
		return;
	}

	// Save counter
	pTeamData->Countdown_regroupAtLeader = countdown;

}
