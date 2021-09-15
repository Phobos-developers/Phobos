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

void ScriptExt::UnsetConditionalJumpVariable(TeamClass* pTeam)
{
	// This team has no units! END
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
	{
		pTeamData->ConditionalJumpEvaluation = false;
		pTeamData->ConditionalEvaluationType = -1;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ConditionalJumpIfTrue(TeamClass* pTeam)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument;

	if (pTeamData->ConditionalEvaluationType != -1)
	{
		if (pTeamData->ConditionalJumpEvaluation)
		{
			// Start conditional jump!
			pTeamData->ConditionalEvaluationType = -1;
			pTeamData->ConditionalJumpEvaluation = false;

			// Ready for jumping to the new line of the script
			pTeam->CurrentScript->idxCurrentLine = scriptArgument - 1;
			pTeam->StepCompleted = true;
			return;
		}

	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJumpIfFalse(TeamClass* pTeam)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}
	
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument;

	if (pTeamData->ConditionalEvaluationType != -1)
	{
		if (!pTeamData->ConditionalJumpEvaluation)
		{
			// Start conditional jump!
			pTeamData->ConditionalEvaluationType = -1;

			// Ready for jumping to the new line of the script
			pTeam->CurrentScript->idxCurrentLine = scriptArgument - 1;
			pTeam->StepCompleted = true;
			return;
		}

	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::SetConditionalJumpCondition(TeamClass* pTeam)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument;

	if (pTeamData->ConditionalEvaluationType != scriptArgument)
	{
		pTeamData->ConditionalEvaluationType = scriptArgument;
		pTeamData->ConditionalJumpEvaluation = false;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

/*


[DRAFT] UNCONDITIONAL SCRIPT ACTIONS DESCRIPTION:

[x]• 1 boolean variable in TeamExt for saving the evaluation. For example:
bool ConditionalJumpEvaluation = false;

[x]• 1 integer variable in TeamExt for saving the type of evaluation. For example:
int ConditionalEvaluationType = -1; // "-1" means no evaluation when a unit is killed by a Team member, maybe I should use the value 0 instead...

• When a unit is killed by a unit that belongs to a Team it will check if "ConditionalEvaluationType" != -1 so in that case it could be evaluated TRUE / FALSE and stored in "ConditionalJumpEvaluation".
If "ConditionalEvaluationType" is -1 (or 0 ?) then the evaluation process is skipped.

• Functions:
[x]-> Unset conditional variable (set to 0 / false). Self-explanatory.

[x]-> Jump to line "nn" (0-based) if conditional variable "ConditionalJumpEvaluation" is TRUE.
When the conditional jump will start the variable "ConditionalEvaluationType" is reset to -1 & the variable "ConditionalJumpEvaluation" is reset to 0 / false.
if the variable is FALSE then skip jump and go to the next script line.

[x]-> Jump to line "nn" (0-based) if conditional variable "ConditionalJumpEvaluation" is FALSE.
When the conditional jump will start the variable "ConditionalEvaluationType" is reset to -1.
if the variable is TRUE then skip jump and go to the next script line.

-> Set conditional jump variable "ConditionalJumpEvaluation" to 1 / true if the killed object is in the specified "nn" list in rulesmd.ini > [AITargetType] section.

-> Set conditional jump variable "ConditionalJumpEvaluation" to 1 / true if the killed object is part of one of the specified list of triggers/teams/taskforces (not yet evaluated this possible function if is viable or not).

[x]-> Set a "nn" type of evaluation in "ConditionalEvaluationType" like the next ones:
Case 1: Just enable it to value 1 / True. (unconditional jump like the classic "6,nn+1".
Case 2: For ANY successful kill from the team members. No extra evaluations.
Case 3: For a destroyed BUILDING by the team.
Case 4: For a ground object kill (infantry, landed aircraft, vehicles, structures).
Case 5: For a ground vehicle kill
Case 6: For a soldier kill
Case 7: For an air unit kill
Case 8: For a naval object kill (Structures, units)
Case 8: For a naval unit kill (not submarines)
Case 9: For a submerged unit kill
Case 10: For a stealth unit kill
Case 11: For a mind controller kill
Case 12: For a civilian structure kill
Case 13: For a civilian unit kill
Case 14: For a harvester kill
Case 15: For a "Economy" object kill
Case 16: For a Refinery kill
Case 17: For a Factory kill
...
...
...

I forgot anything?

*/
