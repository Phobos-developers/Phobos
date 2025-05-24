#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Scenario/Body.h>

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
{ }

ScriptExt::ExtContainer::~ExtContainer() = default;

void ScriptExt::ProcessAction(TeamClass* pTeam)
{
	const int action = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action;
	const int argument = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	switch (static_cast<PhobosScripts>(action))
	{
	case PhobosScripts::TimedAreaGuard:
		ScriptExt::ExecuteTimedAreaGuardAction(pTeam);
		break;
	case PhobosScripts::LoadIntoTransports:
		ScriptExt::LoadIntoTransports(pTeam);
		break;
	case PhobosScripts::WaitUntilFullAmmo:
		ScriptExt::WaitUntilFullAmmoAction(pTeam);
		break;
	case PhobosScripts::RepeatAttackCloserThreat:
		// Threats that are close have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam);
		break;
	case PhobosScripts::RepeatAttackFartherThreat:
		// Threats that are far have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, 1);
		break;
	case PhobosScripts::RepeatAttackCloser:
		// Closer targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, 2);
		break;
	case PhobosScripts::RepeatAttackFarther:
		// Farther targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, 3);
		break;
	case PhobosScripts::SingleAttackCloserThreat:
		// Threats that are close have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, 0, false);
		break;
	case PhobosScripts::SingleAttackFartherThreat:
		// Threats that are far have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, 1, false);
		break;
	case PhobosScripts::SingleAttackCloser:
		// Closer targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, 2, false);
		break;
	case PhobosScripts::SingleAttackFarther:
		// Farther targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, 3, false);
		break;
	case PhobosScripts::DecreaseCurrentAITriggerWeight:
		ScriptExt::DecreaseCurrentTriggerWeight(pTeam);
		break;
	case PhobosScripts::IncreaseCurrentAITriggerWeight:
		ScriptExt::IncreaseCurrentTriggerWeight(pTeam);
		break;
	case PhobosScripts::RepeatAttackTypeCloserThreat:
		// Threats specific targets that are close have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam);
		break;
	case PhobosScripts::RepeatAttackTypeFartherThreat:
		// Threats specific targets that are far have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, 1);
		break;
	case PhobosScripts::RepeatAttackTypeCloser:
		// Closer specific targets targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, 2);
		break;
	case PhobosScripts::RepeatAttackTypeFarther:
		// Farther specific targets targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, 3);
		break;
	case PhobosScripts::SingleAttackTypeCloserThreat:
		// Threats specific targets that are close have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, 0, false);
		break;
	case PhobosScripts::SingleAttackTypeFartherThreat:
		// Threats specific targets that are far have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, 1, false);
		break;
	case PhobosScripts::SingleAttackTypeCloser:
		// Closer specific targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, 2, false);
		break;
	case PhobosScripts::SingleAttackTypeFarther:
		// Farther specific targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, 3, false);
		break;
	case PhobosScripts::WaitIfNoTarget:
		ScriptExt::WaitIfNoTarget(pTeam);
		break;
	case PhobosScripts::TeamWeightReward:
		ScriptExt::TeamWeightReward(pTeam);
		break;
	case PhobosScripts::PickRandomScript:
		ScriptExt::PickRandomScript(pTeam);
		break;
	case PhobosScripts::MoveToEnemyCloser:
		// Move to the closest enemy target
		ScriptExt::Mission_Move(pTeam, 2);
		break;
	case PhobosScripts::MoveToEnemyFarther:
		// Move to the farther enemy target
		ScriptExt::Mission_Move(pTeam, 3);
		break;
	case PhobosScripts::MoveToFriendlyCloser:
		// Move to the closest friendly target
		ScriptExt::Mission_Move(pTeam, 2, true);
		break;
	case PhobosScripts::MoveToFriendlyFarther:
		// Move to the farther friendly target
		ScriptExt::Mission_Move(pTeam, 3, true);
		break;
	case PhobosScripts::MoveToTypeEnemyCloser:
		// Move to the closest specific enemy target
		ScriptExt::Mission_Move_List(pTeam, 2);
		break;
	case PhobosScripts::MoveToTypeEnemyFarther:
		// Move to the farther specific enemy target
		ScriptExt::Mission_Move_List(pTeam, 3);
		break;
	case PhobosScripts::MoveToTypeFriendlyCloser:
		// Move to the closest specific friendly target
		ScriptExt::Mission_Move_List(pTeam, 2, true);
		break;
	case PhobosScripts::MoveToTypeFriendlyFarther:
		// Move to the farther specific friendly target
		ScriptExt::Mission_Move_List(pTeam, 3, true);
		break;
	case PhobosScripts::ModifyTargetDistance:
		// AISafeDistance equivalent for Mission_Move()
		ScriptExt::SetCloseEnoughDistance(pTeam);
		break;
	case PhobosScripts::RandomAttackTypeCloser:
		// Pick 1 closer random objective from specific list for attacking it
		ScriptExt::Mission_Attack_List1Random(pTeam, 2);
		break;
	case PhobosScripts::RandomAttackTypeFarther:
		// Pick 1 farther random objective from specific list for attacking it
		ScriptExt::Mission_Attack_List1Random(pTeam, 3);
		break;
	case PhobosScripts::RandomMoveToTypeEnemyCloser:
		// Pick 1 closer enemy random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 2);
		break;
	case PhobosScripts::RandomMoveToTypeEnemyFarther:
		// Pick 1 farther enemy random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 3);
		break;
	case PhobosScripts::RandomMoveToTypeFriendlyCloser:
		// Pick 1 closer friendly random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 2, true);
		break;
	case PhobosScripts::RandomMoveToTypeFriendlyFarther:
		// Pick 1 farther friendly random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 3, true);
		break;
	case PhobosScripts::SetMoveMissionEndMode:
		// Set the condition for ending the Mission_Move Actions.
		ScriptExt::SetMoveMissionEndMode(pTeam);
		break;
	case PhobosScripts::UnregisterGreatSuccess:
		// Un-register success for AITrigger weight adjustment (this is the opposite of 49,0)
		ScriptExt::UnregisterGreatSuccess(pTeam);
		break;
	case PhobosScripts::GatherAroundLeader:
		ScriptExt::Mission_Gather_NearTheLeader(pTeam);
		break;
	case PhobosScripts::RandomSkipNextAction:
		ScriptExt::SkipNextAction(pTeam);
		break;
	case PhobosScripts::StopForceJumpCountdown:
		// Stop Timed Jump
		ScriptExt::Stop_ForceJump_Countdown(pTeam);
		break;
	case PhobosScripts::NextLineForceJumpCountdown:
		// Start Timed Jump that jumps to the next line when the countdown finish (in frames)
		ScriptExt::Set_ForceJump_Countdown(pTeam);
		break;
	case PhobosScripts::SameLineForceJumpCountdown:
		// Start Timed Jump that jumps to the same line when the countdown finish (in frames)
		ScriptExt::Set_ForceJump_Countdown(pTeam, true);
		break;
	case PhobosScripts::JumpBackToPreviousScript:
		ScriptExt::JumpBackToPreviousScript(pTeam);
		break;
	case PhobosScripts::ChronoshiftToEnemyBase:
		// Chronoshift to enemy base, argument is additional distance modifier
		ScriptExt::ChronoshiftToEnemyBase(pTeam, argument);
		break;
	case PhobosScripts::AbortActionAfterSuccessKill:
		ScriptExt::SetAbortActionAfterSuccessKill(pTeam, -1);
		break;
	case PhobosScripts::ConditionalJumpSetCounter:
		ScriptExt::ConditionalJump_SetCounter(pTeam, -100000000);
		break;
	case PhobosScripts::ConditionalJumpSetComparatorMode:
		ScriptExt::ConditionalJump_SetComparatorMode(pTeam, -1);
		break;
	case PhobosScripts::ConditionalJumpSetComparatorValue:
		ScriptExt::ConditionalJump_SetComparatorValue(pTeam, -1);
		break;
	case PhobosScripts::ConditionalJumpSetIndex:
		ScriptExt::ConditionalJump_SetIndex(pTeam, -1000000);
		break;
	case PhobosScripts::ConditionalJumpResetVariables:
		ScriptExt::ConditionalJump_ResetVariables(pTeam);
		break;
	case PhobosScripts::ConditionalJumpIfFalse:
		ScriptExt::ConditionalJumpIfFalse(pTeam, -1);
		break;
	case PhobosScripts::ConditionalJumpIfTrue:
		ScriptExt::ConditionalJumpIfTrue(pTeam, -1);
		break;
	case PhobosScripts::ConditionalJumpManageKillsCounter:
		ScriptExt::ConditionalJump_ManageKillsCounter(pTeam, -1);
		break;
	case PhobosScripts::ConditionalJumpCheckAliveHumans:
		ScriptExt::ConditionalJump_CheckAliveHumans(pTeam, -1);
		break;
	case PhobosScripts::ConditionalJumpCheckHumanIsMostHated:
		ScriptExt::ConditionalJump_CheckHumanIsMostHated(pTeam);
		break;
	case PhobosScripts::ConditionalJumpKillEvaluation:
		ScriptExt::ConditionalJump_KillEvaluation(pTeam);
		break;
	case PhobosScripts::ConditionalJumpCheckObjects:
		ScriptExt::ConditionalJump_CheckObjects(pTeam);
		break;
	case PhobosScripts::ConditionalJumpCheckCount:
		ScriptExt::ConditionalJump_CheckCount(pTeam, 0);
		break;
	case PhobosScripts::ConditionalJumpManageResetIfJump:
		ScriptExt::ConditionalJump_ManageResetIfJump(pTeam, -1);
		break;
	default:
		// Do nothing because or it is a wrong Action number or it is an Ares/YR action...
		if (action > 70 && !ScriptExt::IsExtVariableAction(action))
		{
			// Unknown new action. This action finished
			pTeam->StepCompleted = true;
			ScriptExt::Log("AI Scripts - ProcessAction: [%s] [%s] (line %d): Unknown Script Action: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action);
		}
		break;
	}

	if (ScriptExt::IsExtVariableAction(action))
		ScriptExt::VariablesHandler(pTeam, static_cast<PhobosScripts>(action), argument);
}

void ScriptExt::ExecuteTimedAreaGuardAction(TeamClass* pTeam)
{
	auto const pScript = pTeam->CurrentScript;
	auto const pScriptType = pScript->Type;

	if (pTeam->GuardAreaTimer.TimeLeft == 0 && !pTeam->GuardAreaTimer.InProgress())
	{
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			pUnit->QueueMission(Mission::Area_Guard, true);

		pTeam->GuardAreaTimer.Start(15 * pScriptType->ScriptActions[pScript->CurrentMission].Argument);
	}

	if (pTeam->GuardAreaTimer.Completed())
	{
		pTeam->GuardAreaTimer.Stop(); // Needed
		pTeam->StepCompleted = true;
	}
}

void ScriptExt::LoadIntoTransports(TeamClass* pTeam)
{
	std::vector<FootClass*> transports;

	// Collect available transports
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto const pType = pUnit->GetTechnoType();

		if (pType->Passengers > 0
			&& pUnit->Passengers.NumPassengers < pType->Passengers
			&& pUnit->Passengers.GetTotalSize() < pType->Passengers)
		{
			transports.emplace_back(pUnit);
		}
	}

	if (transports.size() == 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	// Now load units into transports
	for (auto pTransport : transports)
	{
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			auto const pTransportType = pTransport->GetTechnoType();
			auto const pUnitType = pUnit->GetTechnoType();

			if (pTransport != pUnit
				&& pUnitType->WhatAmI() != AbstractType::AircraftType
				&& !pUnit->InLimbo && !pUnitType->ConsideredAircraft
				&& pUnit->Health > 0)
			{
				if (pUnitType->Size > 0
					&& pUnitType->Size <= pTransportType->SizeLimit
					&& pUnitType->Size <= pTransportType->Passengers - pTransport->Passengers.GetTotalSize())
				{
					// If is still flying wait a bit more
					if (pTransport->IsInAir())
						return;

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
		}
	}

	// Is loading
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit->GetCurrentMission() == Mission::Enter)
			return;
	}

	auto const pExt = TeamExt::ExtMap.Find(pTeam);
	FootClass* pLeaderUnit = ScriptExt::FindTheTeamLeader(pTeam);
	pExt->TeamLeader = pLeaderUnit;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::WaitUntilFullAmmoAction(TeamClass* pTeam)
{
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!pUnit->InLimbo && pUnit->Health > 0)
		{
			auto const pUnitType = pUnit->GetTechnoType();

			if (pUnitType->Ammo > 0 && pUnit->Ammo < pUnitType->Ammo)
			{
				// If an aircraft object have AirportBound it must be evaluated
				if (auto const pAircraft = abstract_cast<AircraftClass*>(pUnit))
				{
					if (pAircraft->Type->AirportBound)
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
				else if (pUnitType->Reload != 0) // Don't skip units that can reload themselves
					return;
			}
		}
	}

	pTeam->StepCompleted = true;
}

void ScriptExt::Mission_Gather_NearTheLeader(TeamClass* pTeam, int countdown)
{
	FootClass* pLeaderUnit = nullptr;
	int initialCountdown = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;
	bool gatherUnits = false;
	auto const pExt = TeamExt::ExtMap.Find(pTeam);

	// Load countdown
	if (pExt->Countdown_RegroupAtLeader >= 0)
		countdown = pExt->Countdown_RegroupAtLeader;

	// Gather permanently until all the team members are near of the Leader
	if (initialCountdown == 0)
		gatherUnits = true;

	// Countdown updater
	if (initialCountdown > 0)
	{
		if (countdown > 0)
		{
			countdown--; // Update countdown
			gatherUnits = true;
		}
		else if (countdown == 0) // Countdown ended
			countdown = -1;
		else // Start countdown.
		{
			countdown = initialCountdown * 15;
			gatherUnits = true;
		}

		// Save counter
		pExt->Countdown_RegroupAtLeader = countdown;
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

		// Find the Leader
		pLeaderUnit = pExt->TeamLeader;

		if (!ScriptExt::IsUnitAvailable(pLeaderUnit, true))
		{
			pLeaderUnit = ScriptExt::FindTheTeamLeader(pTeam);
			pExt->TeamLeader = pLeaderUnit;
		}

		if (!pLeaderUnit)
		{
			pExt->Countdown_RegroupAtLeader = -1;
			// This action finished
			pTeam->StepCompleted = true;

			return;
		}

		// Leader's area radius where the Team members are considered "near" to the Leader
		if (pExt->CloseEnough > 0)
		{
			closeEnough = pExt->CloseEnough;
			pExt->CloseEnough = -1; // This a one-time-use value
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
			if (!ScriptExt::IsUnitAvailable(pUnit, true))
			{
				auto pTypeUnit = pUnit->GetTechnoType();

				if (pUnit == pLeaderUnit)
				{
					nUnits++;
					continue;
				}

				// Aircraft case
				if (pTypeUnit->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo <= 0 && pTypeUnit->Ammo > 0)
				{
					auto pAircraft = static_cast<AircraftTypeClass*>(pTypeUnit);

					if (pAircraft->AirportBound)
					{
						// This aircraft won't count for the script action
						pUnit->EnterIdleMode(false, true);

						continue;
					}
				}

				nUnits++;

				if ((pUnit->DistanceFrom(pLeaderUnit) / 256.0) > closeEnough)
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
					auto mission = pUnit->GetCurrentMission();

					// Is near of the leader, then protect the area
					if (mission != Mission::Area_Guard || mission != Mission::Attack)
						pUnit->QueueMission(Mission::Area_Guard, true);

					nTogether++;
				}
			}
		}


		if (nUnits >= 0
			&& nUnits == nTogether
			&& (initialCountdown == 0
				|| (initialCountdown > 0
					&& countdown <= 0)))
		{
			pExt->Countdown_RegroupAtLeader = -1;
			// This action finished
			pTeam->StepCompleted = true;

			return;
		}
	}
}

void ScriptExt::DecreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier)
{
	if (modifier <= 0)
		modifier = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (modifier <= 0)
		modifier = RulesClass::Instance->AITriggerFailureWeightDelta;
	else
		modifier = modifier * (-1);

	ScriptExt::ModifyCurrentTriggerWeight(pTeam, forceJumpLine, modifier);

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;

	return;
}

void ScriptExt::IncreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier)
{
	if (modifier <= 0)
		modifier = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (modifier <= 0)
		modifier = abs(RulesClass::Instance->AITriggerSuccessWeightDelta);

	ScriptExt::ModifyCurrentTriggerWeight(pTeam, forceJumpLine, modifier);

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier)
{
	AITriggerTypeClass* pTriggerType = nullptr;
	auto const pTeamType = pTeam->Type;
	bool found = false;

	for (int i = 0; i < AITriggerTypeClass::Array.Count && !found; i++)
	{
		auto pTriggerTeam1Type = AITriggerTypeClass::Array.GetItem(i)->Team1;
		auto pTriggerTeam2Type = AITriggerTypeClass::Array.GetItem(i)->Team2;

		if ((pTriggerTeam1Type && pTriggerTeam1Type == pTeamType)
			|| (pTriggerTeam2Type && pTriggerTeam2Type == pTeamType))
		{
			found = true;
			pTriggerType = AITriggerTypeClass::Array.GetItem(i);
		}
	}

	if (found)
	{
		pTriggerType->Weight_Current += modifier;

		if (pTriggerType->Weight_Current > pTriggerType->Weight_Maximum)
		{
			pTriggerType->Weight_Current = pTriggerType->Weight_Maximum;
		}
		else
		{
			if (pTriggerType->Weight_Current < pTriggerType->Weight_Minimum)
				pTriggerType->Weight_Current = pTriggerType->Weight_Minimum;
		}
	}
}

void ScriptExt::WaitIfNoTarget(TeamClass* pTeam, int attempts)
{
	// This method modifies the new attack actions preventing Team's Trigger to jump to next script action
	// attempts == number of times the Team will wait if Mission_Attack(...) can't find a new target.
	if (attempts < 0)
		attempts = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (attempts <= 0)
		pTeamData->WaitNoTargetAttempts = -1; // Infinite waits if no target
	else
		pTeamData->WaitNoTargetAttempts = attempts;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::TeamWeightReward(TeamClass* pTeam, double award)
{
	if (award <= 0)
		award = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (award > 0)
		pTeamData->NextSuccessWeightAward = award;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::PickRandomScript(TeamClass* pTeam, int idxScriptsList)
{
	if (idxScriptsList <= 0)
		idxScriptsList = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	bool changeFailed = true;

	if (idxScriptsList >= 0)
	{
		if ((size_t)idxScriptsList < RulesExt::Global()->AIScriptsLists.size())
		{
			auto& objectsList = RulesExt::Global()->AIScriptsLists[idxScriptsList];

			if (objectsList.size() > 0)
			{
				int IdxSelectedObject = ScenarioClass::Instance->Random.RandomRanged(0, objectsList.size() - 1);

				ScriptTypeClass* pNewScript = objectsList[IdxSelectedObject];
				if (pNewScript->ActionsCount > 0)
				{
					changeFailed = false;
					TeamExt::ExtMap.Find(pTeam)->PreviousScriptList.push_back(pTeam->CurrentScript);
					pTeam->CurrentScript = nullptr;
					pTeam->CurrentScript = GameCreate<ScriptClass>(pNewScript);

					// Ready for jumping to the first line of the new script
					pTeam->CurrentScript->CurrentMission = -1;
					pTeam->StepCompleted = true;

					return;
				}
				else
				{
					pTeam->StepCompleted = true;
					ScriptExt::Log("AI Scripts - PickRandomScript: [%s] Aborting Script change because [%s] has 0 Action scripts!\n", pTeam->Type->ID, pNewScript->ID);

					return;
				}
			}
		}
	}

	// This action finished
	if (changeFailed)
	{
		pTeam->StepCompleted = true;
		ScriptExt::Log("AI Scripts - PickRandomScript: [%s] [%s] Failed to change the Team Script with a random one!\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID);
	}
}

void ScriptExt::SetCloseEnoughDistance(TeamClass* pTeam, double distance)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (distance <= 0)
		distance = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (distance > 0)
		pTeamData->CloseEnough = distance;

	if (distance <= 0)
		pTeamData->CloseEnough = RulesClass::Instance->CloseEnough / 256.0;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::UnregisterGreatSuccess(TeamClass* pTeam)
{
	pTeam->AchievedGreatSuccess = false;
	pTeam->StepCompleted = true;
}

void ScriptExt::SetMoveMissionEndMode(TeamClass* pTeam, int mode)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (mode < 0 || mode > 2)
		mode = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (mode >= 0 && mode <= 2)
		pTeamData->MoveMissionEndMode = mode;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

bool ScriptExt::MoveMissionEndStatus(TeamClass* pTeam, TechnoClass* pFocus, FootClass* pLeader, int mode)
{
	if (!pFocus || mode < 0 || (mode != 2 && mode != 1 && !pLeader))
		return false;

	double closeEnough = RulesClass::Instance->CloseEnough / 256.0;
	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (pTeamData->CloseEnough > 0)
		closeEnough = pTeamData->CloseEnough;

	bool bForceNextAction = false;

	if (mode == 2)
		bForceNextAction = true;

	// Team already have a focused target
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (ScriptExt::IsUnitAvailable(pUnit, true)
			&& !pUnit->TemporalTargetingMe
			&& !pUnit->BeingWarpedOut)
		{
			if (mode == 2)
			{
				// Default mode: all members in range
				if ((pUnit->DistanceFrom(pFocus->GetCell()) / 256.0) > closeEnough)
				{
					bForceNextAction = false;

					if (pUnit->WhatAmI() == AbstractType::Aircraft && pUnit->Ammo > 0)
						pUnit->QueueMission(Mission::Move, false);

					continue;
				}
				else
				{
					if (pUnit->WhatAmI() == AbstractType::Aircraft && pUnit->Ammo <= 0)
					{
						pUnit->EnterIdleMode(false, true);

						continue;
					}
				}
			}
			else
			{
				if (mode == 1)
				{
					// Any member in range
					if ((pUnit->DistanceFrom(pFocus->GetCell()) / 256.0) > closeEnough)
					{
						if (pUnit->WhatAmI() == AbstractType::Aircraft && pUnit->Ammo > 0)
							pUnit->QueueMission(Mission::Move, false);

						continue;
					}
					else
					{
						bForceNextAction = true;

						if (pUnit->WhatAmI() == AbstractType::Aircraft && pUnit->Ammo <= 0)
						{
							pUnit->EnterIdleMode(false, true);

							continue;
						}
					}
				}
				else
				{
					// All other cases: Team Leader mode in range
					if (pLeader)
					{
						if ((pUnit->DistanceFrom(pFocus->GetCell()) / 256.0) > closeEnough)
						{
							if (pUnit->WhatAmI() == AbstractType::Aircraft && pUnit->Ammo > 0)
								pUnit->QueueMission(Mission::Move, false);

							continue;
						}
						else
						{
							if (pUnit->IsInitiated)
								bForceNextAction = true;

							if (pUnit->WhatAmI() == AbstractType::Aircraft && pUnit->Ammo <= 0)
							{
								pUnit->EnterIdleMode(false, true);

								continue;
							}
						}
					}
					else
					{
						break;
					}
				}
			}
		}
	}

	return bForceNextAction;
}

void ScriptExt::SkipNextAction(TeamClass* pTeam, int successPercentage)
{
	if (successPercentage < 0 || successPercentage > 100)
		successPercentage = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (successPercentage < 0)
		successPercentage = 0;

	if (successPercentage > 100)
		successPercentage = 100;

	int percentage = ScenarioClass::Instance->Random.RandomRanged(1, 100);

	if (percentage <= successPercentage)
	{
		ScriptExt::Log("AI Scripts - SkipNextAction: [%s] [%s] (line: %d = %d,%d) Next script line skipped successfuly. Next line will be: %d = %d,%d\n",
			pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, pTeam->CurrentScript->CurrentMission + 2,
			pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 2].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 2].Argument);

		pTeam->CurrentScript->CurrentMission++;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::VariablesHandler(TeamClass* pTeam, PhobosScripts eAction, int nArg)
{
	struct operation_set { int operator()(const int& a, const int& b) { return b; } };
	struct operation_add { int operator()(const int& a, const int& b) { return a + b; } };
	struct operation_minus { int operator()(const int& a, const int& b) { return a - b; } };
	struct operation_multiply { int operator()(const int& a, const int& b) { return a * b; } };
	struct operation_divide { int operator()(const int& a, const int& b) { return a / b; } };
	struct operation_mod { int operator()(const int& a, const int& b) { return a % b; } };
	struct operation_leftshift { int operator()(const int& a, const int& b) { return a << b; } };
	struct operation_rightshift { int operator()(const int& a, const int& b) { return a >> b; } };
	struct operation_reverse { int operator()(const int& a, const int& b) { return ~a; } };
	struct operation_xor { int operator()(const int& a, const int& b) { return a ^ b; } };
	struct operation_or { int operator()(const int& a, const int& b) { return a | b; } };
	struct operation_and { int operator()(const int& a, const int& b) { return a & b; } };

	int nLoArg = LOWORD(nArg);
	int nHiArg = HIWORD(nArg);

	switch (eAction)
	{
	case PhobosScripts::LocalVariableSet:
		ScriptExt::VariableOperationHandler<false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAdd:
		ScriptExt::VariableOperationHandler<false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinus:
		ScriptExt::VariableOperationHandler<false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiply:
		ScriptExt::VariableOperationHandler<false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivide:
		ScriptExt::VariableOperationHandler<false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMod:
		ScriptExt::VariableOperationHandler<false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShift:
		ScriptExt::VariableOperationHandler<false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShift:
		ScriptExt::VariableOperationHandler<false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverse:
		ScriptExt::VariableOperationHandler<false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXor:
		ScriptExt::VariableOperationHandler<false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOr:
		ScriptExt::VariableOperationHandler<false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAnd:
		ScriptExt::VariableOperationHandler<false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSet:
		ScriptExt::VariableOperationHandler<true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAdd:
		ScriptExt::VariableOperationHandler<true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinus:
		ScriptExt::VariableOperationHandler<true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiply:
		ScriptExt::VariableOperationHandler<true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivide:
		ScriptExt::VariableOperationHandler<true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMod:
		ScriptExt::VariableOperationHandler<true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShift:
		ScriptExt::VariableOperationHandler<true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShift:
		ScriptExt::VariableOperationHandler<true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverse:
		ScriptExt::VariableOperationHandler<true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXor:
		ScriptExt::VariableOperationHandler<true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOr:
		ScriptExt::VariableOperationHandler<true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAnd:
		ScriptExt::VariableOperationHandler<true, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableSetByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAddByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinusByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiplyByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivideByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableModByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShiftByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShiftByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverseByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXorByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOrByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAndByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSetByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAddByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinusByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiplyByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivideByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableModByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShiftByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShiftByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverseByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXorByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOrByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAndByLocal:
		ScriptExt::VariableBinaryOperationHandler<false, true, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableSetByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAddByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinusByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiplyByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivideByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableModByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShiftByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShiftByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverseByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXorByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOrByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAndByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSetByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAddByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinusByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiplyByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivideByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableModByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShiftByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShiftByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverseByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXorByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOrByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAndByGlobal:
		ScriptExt::VariableBinaryOperationHandler<true, true, operation_and>(pTeam, nLoArg, nHiArg); break;
	}
}

template<bool IsGlobal, class _Pr>
void ScriptExt::VariableOperationHandler(TeamClass* pTeam, int nVariable, int Number)
{
	auto itr = ScenarioExt::Global()->Variables[IsGlobal].find(nVariable);

	if (itr != ScenarioExt::Global()->Variables[IsGlobal].end())
	{
		itr->second.Value = _Pr()(itr->second.Value, Number);
		if (IsGlobal)
			TagClass::NotifyGlobalChanged(nVariable);
		else
			TagClass::NotifyLocalChanged(nVariable);
	}

	pTeam->StepCompleted = true;
}

template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
void ScriptExt::VariableBinaryOperationHandler(TeamClass* pTeam, int nVariable, int nVarToOperate)
{
	auto itr = ScenarioExt::Global()->Variables[IsSrcGlobal].find(nVarToOperate);

	if (itr != ScenarioExt::Global()->Variables[IsSrcGlobal].end())
		ScriptExt::VariableOperationHandler<IsGlobal, _Pr>(pTeam, nVariable, itr->second.Value);

	pTeam->StepCompleted = true;
}

FootClass* ScriptExt::FindTheTeamLeader(TeamClass* pTeam)
{
	FootClass* pLeaderUnit = nullptr;
	int bestUnitLeadershipValue = -1;

	// Find the Leader or promote a new one
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!ScriptExt::IsUnitAvailable(pUnit, true) || !(pUnit->IsInitiated || pUnit->WhatAmI() == AbstractType::Aircraft))
			continue;

		// The team Leader will be used for selecting targets, if there are living Team Members then always exists 1 Leader.
		int unitLeadershipRating = pUnit->GetTechnoType()->LeadershipRating;

		if (unitLeadershipRating > bestUnitLeadershipValue)
		{
			pLeaderUnit = pUnit;
			bestUnitLeadershipValue = unitLeadershipRating;
		}
	}

	return pLeaderUnit;
}

bool ScriptExt::IsExtVariableAction(int action)
{
	auto eAction = static_cast<PhobosScripts>(action);
	return eAction >= PhobosScripts::LocalVariableAdd && eAction <= PhobosScripts::GlobalVariableAndByGlobal;
}

void ScriptExt::Set_ForceJump_Countdown(TeamClass* pTeam, bool repeatLine, int count)
{
	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);
	auto const pScript = pTeam->CurrentScript;

	if (count <= 0)
		count = 15 * pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (count > 0)
	{
		pTeamData->ForceJump_InitialCountdown = count;
		pTeamData->ForceJump_Countdown.Start(count);
		pTeamData->ForceJump_RepeatMode = repeatLine;
	}
	else
	{
		pTeamData->ForceJump_InitialCountdown = -1;
		pTeamData->ForceJump_Countdown.Stop();
		pTeamData->ForceJump_RepeatMode = false;
	}

	// This action finished
	pTeam->StepCompleted = true;
	ScriptExt::Log("AI Scripts - SetForceJumpCountdown: [%s] [%s](line: %d = %d,%d) Set Timed Jump -> (Countdown: %d, repeat action: %d)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, count, repeatLine);
}

void ScriptExt::Stop_ForceJump_Countdown(TeamClass* pTeam)
{
	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);
	auto const pScript = pTeam->CurrentScript;
	pTeamData->ForceJump_InitialCountdown = -1;
	pTeamData->ForceJump_Countdown.Stop();
	pTeamData->ForceJump_RepeatMode = false;

	// This action finished
	pTeam->StepCompleted = true;
	ScriptExt::Log("AI Scripts - StopForceJumpCountdown: [%s] [%s](line: %d = %d,%d): Stopped Timed Jump\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument);
}

void ScriptExt::JumpBackToPreviousScript(TeamClass* pTeam)
{
	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeamData->PreviousScriptList.empty())
	{
		pTeam->CurrentScript = pTeamData->PreviousScriptList.back();
		pTeamData->PreviousScriptList.pop_back();
		pTeam->StepCompleted = true;
	}
	else
	{
		auto const pScript = pTeam->CurrentScript;
		ScriptExt::Log("AI Scripts - JumpBackToPreviousScript: [%s] [%s](line: %d = %d,%d): Can't find the previous script! This script action must be used after PickRandomScript.\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument);
		pTeam->StepCompleted = true;
	}
}

void ScriptExt::ChronoshiftToEnemyBase(TeamClass* pTeam, int extraDistance)
{
	auto pScript = pTeam->CurrentScript;
	auto const pLeader = ScriptExt::FindTheTeamLeader(pTeam);

	char logText[1024];
	sprintf_s(logText, "AI Scripts - ChronoshiftToEnemyBase: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: %s)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument, "%s");

	if (!pLeader)
	{
		ScriptExt::Log(logText, "No team leader found");
		pTeam->StepCompleted = true;
		return;
	}

	int houseIndex = pLeader->Owner->EnemyHouseIndex;
	HouseClass* pEnemy = houseIndex != -1 ? HouseClass::Array.GetItem(houseIndex) : nullptr;

	if (!pEnemy)
	{
		ScriptExt::Log(logText, "No enemy house found");
		pTeam->StepCompleted = true;
		return;
	}

	auto const pTargetCell = HouseExt::GetEnemyBaseGatherCell(pEnemy, pLeader->Owner, pLeader->GetCoords(), pLeader->GetTechnoType()->SpeedType, extraDistance);

	if (!pTargetCell)
	{
		ScriptExt::Log(logText, "No target cell found");
		pTeam->StepCompleted = true;
		return;
	}

	ScriptExt::ChronoshiftTeamToTarget(pTeam, pLeader, pTargetCell);
}

void ScriptExt::ChronoshiftTeamToTarget(TeamClass* pTeam, TechnoClass* pTeamLeader, AbstractClass* pTarget)
{
	auto pScript = pTeam->CurrentScript;
	HouseClass* pOwner = pTeamLeader->Owner;
	SuperClass* pSuperCSphere = nullptr;
	SuperClass* pSuperCWarp = nullptr;

	HouseExt::GetAIChronoshiftSupers(pOwner, pSuperCSphere, pSuperCWarp);

	char logTextBase[1024];
	char logTextJump[1024];
	char jump[256];

	sprintf_s(jump, "Jump to next line: %d = %d,%d -> (Reason: %s)", pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument, "%s");
	sprintf_s(logTextBase, "AI Scripts - ChronoshiftTeamToTarget: [%s] [%s] (line: %d = %d,%d) %s\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, "%s");
	sprintf_s(logTextJump, logTextBase, jump);

	if (!pSuperCSphere || !pSuperCWarp)
	{
		ScriptExt::Log(logTextJump, "No ChronoSphere or ChronoWarp superweapon found");
		pTeam->StepCompleted = true;
		return;
	}

	if (!pSuperCSphere->IsReady || (pSuperCSphere->IsPowered() && !pOwner->Is_Powered()))
	{
		if (pSuperCSphere->IsPresent && 1.0 - RulesClass::Instance->AIMinorSuperReadyPercent < pSuperCSphere->RechargeTimer.GetTimeLeft() / pSuperCSphere->GetRechargeTime())
		{
			ScriptExt::Log(logTextBase, "ChronoSphere superweapon [%s] charge not at AIMinorSuperReadyPercent yet, not jumping to next line yet", pSuperCSphere->Type->get_ID());
			return;
		}
		else
		{
			ScriptExt::Log(logTextJump, "ChronoSphere superweapon [%s] is not available", pSuperCSphere->Type->get_ID());
			pTeam->StepCompleted = true;
			return;
		}
	}

	auto pTargetCell = MapClass::Instance.TryGetCellAt(pTarget->GetCoords());

	if (pTargetCell)
	{
		pOwner->Fire_SW(pSuperCSphere->Type->ArrayIndex, pTeam->SpawnCell->MapCoords);
		pOwner->Fire_SW(pSuperCWarp->Type->ArrayIndex, pTargetCell->MapCoords);
		pTeam->AssignMissionTarget(pTargetCell);
		ScriptExt::Log(logTextJump, "Finished successfully");
	}
	else
	{
		ScriptExt::Log(logTextJump, "No target cell found");
	}

	pTeam->StepCompleted = true;
	return;
}

bool ScriptExt::IsUnitAvailable(TechnoClass* pTechno, bool checkIfInTransportOrAbsorbed)
{
	if (!pTechno)
		return false;

	bool isAvailable = pTechno->IsAlive && pTechno->Health > 0 && !pTechno->InLimbo && pTechno->IsOnMap;

	if (checkIfInTransportOrAbsorbed)
		isAvailable &= !pTechno->Absorbed && !pTechno->Transporter;

	return isAvailable;
}

void ScriptExt::Log(const char* pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	Debug::LogWithVArgs(pFormat, args);
	va_end(args);
}
