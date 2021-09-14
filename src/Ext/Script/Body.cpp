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
	case 74: // rename from "74" to "110" when PR #296 "New Script Actions" is merged into develop!
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
	int bestUnitLeadershipValue = -1;
	int initialCountdown = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Argument;
	bool gatherUnits = false;

	// This team has no units! END
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	// Load countdown
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
	{
		if (pTeamData->Countdown_regroupAtLeader >= 0)
			countdown = pTeamData->Countdown_regroupAtLeader;
	}
	else
	{
		// Looks like an error...
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	// Gather permanently until all the team members are near of the Leader
	if (initialCountdown == 0)
	{
		gatherUnits = true;
	}
	
	// Countdown updater
	if (initialCountdown > 0)
	{
		if (countdown > 0)
		{
			countdown--; // Update countdown
			gatherUnits = true;
		}
		else
		{
			if (countdown == 0)
			{
				// Countdown ended
				countdown = -1;
			}
			else
			{
				// Start countdown.
				countdown = initialCountdown * 15;
				gatherUnits = true;
			}
		}

		// Save counter
		pTeamData->Countdown_regroupAtLeader = countdown;
	}
	
	if (!gatherUnits)
	{
		// This action finished
		pTeam->StepCompleted = true;
		
		return;
	}
	else
	{
		// Move all around the leader, the leader always in "Guard Area" Mission or simply in Guard Mission
		int nTogether = 0;
		int nUnits = -1; // Leader counts here
		double closeEnough;

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

			return;
		}

		// Leader's area radius where the Team members are considered "near" to the Leader
		if (pTeamData->CloseEnough > 0)
		{
			closeEnough = pTeamData->CloseEnough;
			pTeamData->CloseEnough = -1; // This a one-time-use value
		}
		else
		{
			closeEnough = RulesClass::Instance->CloseEnough / 256.0;
		}

		// The leader should stay calm & be the group's center
		if (pLeaderUnit->Locomotor->Is_Moving_Now())
			pLeaderUnit->SetDestination(nullptr, false);
		pLeaderUnit->QueueMission(Mission::Guard, false);

		// Check if units are around the leader
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (pUnit
				&& pUnit->IsAlive
				&& pUnit->Health > 0
				&& !pUnit->InLimbo
				&& pUnit->IsOnMap
				&& !pUnit->Absorbed)
			{
				auto pTypeUnit = pUnit->GetTechnoType();

				if (!pTypeUnit)
				{
					continue;
				}

				if (pUnit == pLeaderUnit)
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
						// This aircraft won't count for the script action
						pUnit->QueueMission(Mission::Return, false);
						pUnit->Mission_Enter();

						continue;
					}
				}

				nUnits++;

				if (pUnit->DistanceFrom(pLeaderUnit) / 256.0 > closeEnough)
				{
					// Leader's location is too far from me. Regroup
					if (pUnit->Destination != pLeaderUnit)
					{
						pUnit->SetDestination(pLeaderUnit, false);
						pUnit->QueueMission(Mission::Move, false);
					}
				}
				else
				{
					// Is near of the leader, then protect the area
					if (pUnit->GetCurrentMission() != Mission::Area_Guard || pUnit->GetCurrentMission() != Mission::Attack)
						pUnit->QueueMission(Mission::Area_Guard, true);

					nTogether++;
				}
			}
		}
		

		if (nUnits >= 0 && nUnits == nTogether && (initialCountdown == 0 || (initialCountdown > 0 && countdown <= 0)))
		{
			pTeamData->Countdown_regroupAtLeader = -1;

			// This action finished
			pTeam->StepCompleted = true;
			
			return;
		}
	}
}
