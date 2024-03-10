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
{
}

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
		ScriptExt::Mission_Attack(pTeam, true, 0, -1, -1);
		break;
	case PhobosScripts::RepeatAttackFartherThreat:
		// Threats that are far have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 1, -1, -1);
		break;
	case PhobosScripts::RepeatAttackCloser:
		// Closer targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 2, -1, -1);
		break;
	case PhobosScripts::RepeatAttackFarther:
		// Farther targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 3, -1, -1);
		break;
	case PhobosScripts::SingleAttackCloserThreat:
		// Threats that are close have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 0, -1, -1);
		break;
	case PhobosScripts::SingleAttackFartherThreat:
		// Threats that are far have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 1, -1, -1);
		break;
	case PhobosScripts::SingleAttackCloser:
		// Closer targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 2, -1, -1);
		break;
	case PhobosScripts::SingleAttackFarther:
		// Farther targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 3, -1, -1);
		break;
	case PhobosScripts::DecreaseCurrentAITriggerWeight:
		ScriptExt::DecreaseCurrentTriggerWeight(pTeam, true, 0);
		break;
	case PhobosScripts::IncreaseCurrentAITriggerWeight:
		ScriptExt::IncreaseCurrentTriggerWeight(pTeam, true, 0);
		break;
	case PhobosScripts::RepeatAttackTypeCloserThreat:
		// Threats specific targets that are close have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 0, -1);
		break;
	case PhobosScripts::RepeatAttackTypeFartherThreat:
		// Threats specific targets that are far have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 1, -1);
		break;
	case PhobosScripts::RepeatAttackTypeCloser:
		// Closer specific targets targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 2, -1);
		break;
	case PhobosScripts::RepeatAttackTypeFarther:
		// Farther specific targets targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 3, -1);
		break;
	case PhobosScripts::SingleAttackTypeCloserThreat:
		// Threats specific targets that are close have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 0, -1);
		break;
	case PhobosScripts::SingleAttackTypeFartherThreat:
		// Threats specific targets that are far have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 1, -1);
		break;
	case PhobosScripts::SingleAttackTypeCloser:
		// Closer specific targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 2, -1);
		break;
	case PhobosScripts::SingleAttackTypeFarther:
		// Farther specific targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 3, -1);
		break;
	case PhobosScripts::WaitIfNoTarget:
		ScriptExt::WaitIfNoTarget(pTeam, -1);
		break;
	case PhobosScripts::TeamWeightReward:
		ScriptExt::TeamWeightReward(pTeam, 0);
		break;
	case PhobosScripts::PickRandomScript:
		ScriptExt::PickRandomScript(pTeam, -1);
		break;
	case PhobosScripts::MoveToEnemyCloser:
		// Move to the closest enemy target
		ScriptExt::Mission_Move(pTeam, 2, false, -1, -1);
		break;
	case PhobosScripts::MoveToEnemyFarther:
		// Move to the farther enemy target
		ScriptExt::Mission_Move(pTeam, 3, false, -1, -1);
		break;
	case PhobosScripts::MoveToFriendlyCloser:
		// Move to the closest friendly target
		ScriptExt::Mission_Move(pTeam, 2, true, -1, -1);
		break;
	case PhobosScripts::MoveToFriendlyFarther:
		// Move to the farther friendly target
		ScriptExt::Mission_Move(pTeam, 3, true, -1, -1);
		break;
	case PhobosScripts::MoveToTypeEnemyCloser:
		// Move to the closest specific enemy target
		ScriptExt::Mission_Move_List(pTeam, 2, false, -1);
		break;
	case PhobosScripts::MoveToTypeEnemyFarther:
		// Move to the farther specific enemy target
		ScriptExt::Mission_Move_List(pTeam, 3, false, -1);
		break;
	case PhobosScripts::MoveToTypeFriendlyCloser:
		// Move to the closest specific friendly target
		ScriptExt::Mission_Move_List(pTeam, 2, true, -1);
		break;
	case PhobosScripts::MoveToTypeFriendlyFarther:
		// Move to the farther specific friendly target
		ScriptExt::Mission_Move_List(pTeam, 3, true, -1);
		break;
	case PhobosScripts::ModifyTargetDistance:
		// AISafeDistance equivalent for Mission_Move()
		ScriptExt::SetCloseEnoughDistance(pTeam, -1);
		break;
	case PhobosScripts::RandomAttackTypeCloser:
		// Pick 1 closer random objective from specific list for attacking it
		ScriptExt::Mission_Attack_List1Random(pTeam, true, 2, -1);
		break;
	case PhobosScripts::RandomAttackTypeFarther:
		// Pick 1 farther random objective from specific list for attacking it
		ScriptExt::Mission_Attack_List1Random(pTeam, true, 3, -1);
		break;
	case PhobosScripts::RandomMoveToTypeEnemyCloser:
		// Pick 1 closer enemy random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 2, false, -1, -1);
		break;
	case PhobosScripts::RandomMoveToTypeEnemyFarther:
		// Pick 1 farther enemy random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 3, false, -1, -1);
		break;
	case PhobosScripts::RandomMoveToTypeFriendlyCloser:
		// Pick 1 closer friendly random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 2, true, -1, -1);
		break;
	case PhobosScripts::RandomMoveToTypeFriendlyFarther:
		// Pick 1 farther friendly random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 3, true, -1, -1);
		break;
	case PhobosScripts::SetMoveMissionEndMode:
		// Set the condition for ending the Mission_Move Actions.
		ScriptExt::SetMoveMissionEndMode(pTeam, -1);
		break;
	case PhobosScripts::UnregisterGreatSuccess:
		// Un-register success for AITrigger weight adjustment (this is the opposite of 49,0)
		ScriptExt::UnregisterGreatSuccess(pTeam);
		break;
	case PhobosScripts::GatherAroundLeader:
		ScriptExt::Mission_Gather_NearTheLeader(pTeam, -1);
		break;
	case PhobosScripts::RandomSkipNextAction:
		ScriptExt::SkipNextAction(pTeam, -1);
		break;
	case PhobosScripts::SetHouseAngerModifier:
		ScriptExt::SetHouseAngerModifier(pTeam, 0);
		break;
	case PhobosScripts::OverrideOnlyTargetHouseEnemy:
		ScriptExt::OverrideOnlyTargetHouseEnemy(pTeam, -1);
		break;
	case PhobosScripts::ModifyHateHouseIndex:
		ScriptExt::ModifyHateHouse_Index(pTeam, -1);
		break;
	case PhobosScripts::ModifyHateHousesList:
		ScriptExt::ModifyHateHouses_List(pTeam, -1);
		break;
	case PhobosScripts::ModifyHateHousesList1Random:
		ScriptExt::ModifyHateHouses_List1Random(pTeam, -1);
		break;
	case PhobosScripts::SetTheMostHatedHouseMinorNoRandom:
		// <, no random
		ScriptExt::SetTheMostHatedHouse(pTeam, 0, 0, false);
		break;
	case PhobosScripts::SetTheMostHatedHouseMajorNoRandom:
		// >, no random
		ScriptExt::SetTheMostHatedHouse(pTeam, 0, 1, false);
		break;
	case PhobosScripts::SetTheMostHatedHouseRandom:
		// random
		ScriptExt::SetTheMostHatedHouse(pTeam, 0, 0, true);
		break;
	case PhobosScripts::ResetAngerAgainstHouses:
		ScriptExt::ResetAngerAgainstHouses(pTeam);
		break;
	case PhobosScripts::AggroHouse:
		ScriptExt::AggroHouse(pTeam, -1);
		break;
	case PhobosScripts::StopForceJumpCountdown:
		// Stop Timed Jump
		ScriptExt::Stop_ForceJump_Countdown(pTeam);
		break;
	case PhobosScripts::NextLineForceJumpCountdown:
		// Start Timed Jump that jumps to the next line when the countdown finish (in frames)
		ScriptExt::Set_ForceJump_Countdown(pTeam, false, -1);
		break;
	case PhobosScripts::SameLineForceJumpCountdown:
		// Start Timed Jump that jumps to the same line when the countdown finish (in frames)
		ScriptExt::Set_ForceJump_Countdown(pTeam, true, -1);
		break;
	case PhobosScripts::ChronoshiftToEnemyBase:
		// Chronoshift to enemy base, argument is additional distance modifier
		ScriptExt::ChronoshiftToEnemyBase(pTeam, argument);
		break;
	default:
		// Do nothing because or it is a wrong Action number or it is an Ares/YR action...
		if (action > 70 && !IsExtVariableAction(action))
		{
			// Unknown new action. This action finished
			pTeam->StepCompleted = true;
			ScriptExt::Log("AI Scripts - ProcessAction: [%s] [%s] (line %d): Unknown Script Action: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action);
		}
		break;
	}

	if (IsExtVariableAction(action))
		VariablesHandler(pTeam, static_cast<PhobosScripts>(action), argument);
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
				if (pUnit->GetTechnoType()->Size > 0
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

	if (pExt)
	{
		FootClass* pLeaderUnit = FindTheTeamLeader(pTeam);
		pExt->TeamLeader = pLeaderUnit;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::WaitUntilFullAmmoAction(TeamClass* pTeam)
{
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!pUnit->InLimbo && pUnit->Health > 0)
		{
			if (pUnit->GetTechnoType()->Ammo > 0 && pUnit->Ammo < pUnit->GetTechnoType()->Ammo)
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
				else if (pUnit->GetTechnoType()->Reload != 0) // Don't skip units that can reload themselves
					return;
			}
		}
	}

	pTeam->StepCompleted = true;
}

void ScriptExt::Mission_Gather_NearTheLeader(TeamClass* pTeam, int countdown = -1)
{
	FootClass* pLeaderUnit = nullptr;
	int initialCountdown = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;
	bool gatherUnits = false;
	auto const pExt = TeamExt::ExtMap.Find(pTeam);

	// This team has no units! END
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

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

		if (!IsUnitAvailable(pLeaderUnit, true))
		{
			pLeaderUnit = FindTheTeamLeader(pTeam);
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
			if (!IsUnitAvailable(pUnit, true))
			{
				auto pTypeUnit = pUnit->GetTechnoType();

				if (!pTypeUnit)
					continue;

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
					// Is near of the leader, then protect the area
					if (pUnit->GetCurrentMission() != Mission::Area_Guard || pUnit->GetCurrentMission() != Mission::Attack)
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

void ScriptExt::DecreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	if (modifier <= 0)
		modifier = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (modifier <= 0)
		modifier = RulesClass::Instance->AITriggerFailureWeightDelta;
	else
		modifier = modifier * (-1);

	ModifyCurrentTriggerWeight(pTeam, forceJumpLine, modifier);

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;

	return;
}

void ScriptExt::IncreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
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

void ScriptExt::ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	AITriggerTypeClass* pTriggerType = nullptr;
	auto pTeamType = pTeam->Type;
	bool found = false;

	for (int i = 0; i < AITriggerTypeClass::Array->Count && !found; i++)
	{
		auto pTriggerTeam1Type = AITriggerTypeClass::Array->GetItem(i)->Team1;
		auto pTriggerTeam2Type = AITriggerTypeClass::Array->GetItem(i)->Team2;

		if (pTeamType
			&& ((pTriggerTeam1Type && pTriggerTeam1Type == pTeamType)
				|| (pTriggerTeam2Type && pTriggerTeam2Type == pTeamType)))
		{
			found = true;
			pTriggerType = AITriggerTypeClass::Array->GetItem(i);
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

void ScriptExt::WaitIfNoTarget(TeamClass* pTeam, int attempts = 0)
{
	// This method modifies the new attack actions preventing Team's Trigger to jump to next script action
	// attempts == number of times the Team will wait if Mission_Attack(...) can't find a new target.
	if (attempts < 0)
		attempts = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
	{
		if (attempts <= 0)
			pTeamData->WaitNoTargetAttempts = -1; // Infinite waits if no target
		else
			pTeamData->WaitNoTargetAttempts = attempts;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::TeamWeightReward(TeamClass* pTeam, double award = 0)
{
	if (award <= 0)
		award = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
	{
		if (award > 0)
			pTeamData->NextSuccessWeightAward = award;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::PickRandomScript(TeamClass* pTeam, int idxScriptsList = -1)
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

void ScriptExt::SetCloseEnoughDistance(TeamClass* pTeam, double distance = -1)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (distance <= 0)
		distance = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (pTeamData)
	{
		if (distance > 0)
			pTeamData->CloseEnough = distance;
	}

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

void ScriptExt::SetMoveMissionEndMode(TeamClass* pTeam, int mode = 0)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (mode < 0 || mode > 2)
		mode = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (pTeamData)
	{
		if (mode >= 0 && mode <= 2)
			pTeamData->MoveMissionEndMode = mode;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

bool ScriptExt::MoveMissionEndStatus(TeamClass* pTeam, TechnoClass* pFocus, FootClass* pLeader = nullptr, int mode = 0)
{
	if (!pTeam || !pFocus || mode < 0)
		return false;

	if (mode != 2 && mode != 1 && !pLeader)
		return false;

	double closeEnough = RulesClass::Instance->CloseEnough / 256.0;

	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (pTeamData && pTeamData->CloseEnough > 0)
		closeEnough = pTeamData->CloseEnough;

	bool bForceNextAction = false;

	if (mode == 2)
		bForceNextAction = true;

	// Team already have a focused target
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (IsUnitAvailable(pUnit, true)
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

void ScriptExt::SkipNextAction(TeamClass* pTeam, int successPercentage = 0)
{
	// This team has no units! END
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;
		ScriptExt::Log("AI Scripts - SkipNextAction: [%s] [%s] (line: %d) Jump to next line: %d = %d,%d -> (No team members alive)\n",
			pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument,
			pTeam->CurrentScript->CurrentMission + 1, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 1].Action,
			pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 1].Argument);

		return;
	}

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

void ScriptExt::ResetAngerAgainstHouses(TeamClass* pTeam)
{
	// Invalid team
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	for (auto& angerNode : pTeam->Owner->AngerNodes)
	{
		angerNode.AngerLevel = 0;
	}

	pTeam->Owner->EnemyHouseIndex = -1;
	ScriptExt::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true; // This action finished - FS-21
}

void ScriptExt::SetHouseAngerModifier(TeamClass* pTeam, int modifier = 0)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (modifier <= 0)
		modifier = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (modifier < 0)
		modifier = 0;

	pTeamData->AngerNodeModifier = modifier;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ModifyHateHouses_List(TeamClass* pTeam, int idxHousesList = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	bool changeFailed = true;

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (idxHousesList <= 0)
		idxHousesList = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (idxHousesList >= 0)
	{
		if (idxHousesList < (int)RulesExt::Global()->AIHousesLists.size())
		{
			std::vector<HouseTypeClass*> objectsList = RulesExt::Global()->AIHousesLists.at(idxHousesList);

			if (objectsList.size() > 0)
			{
				for (const auto pHouseType : objectsList)
				{
					for (auto& angerNode : pTeam->Owner->AngerNodes)
					{
						if (angerNode.House->IsObserver())
							continue;

						HouseTypeClass* angerNodeType = angerNode.House->Type;

						if (_stricmp(angerNodeType->ID, pHouseType->ID) == 0)
						{
							angerNode.AngerLevel += pTeamData->AngerNodeModifier;
							changeFailed = false;
						}
					}
				}
			}
		}
	}

	// This action finished
	if (changeFailed)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Failed to modify hate values against other houses\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument);
	}

	ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);
	ScriptExt::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ModifyHateHouses_List1Random(TeamClass* pTeam, int idxHousesList = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	int changes = 0;

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (idxHousesList <= 0)
		idxHousesList = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (idxHousesList >= 0)
	{
		if (idxHousesList < (int)RulesExt::Global()->AIHousesLists.size())
		{
			std::vector<HouseTypeClass*> objectsList = RulesExt::Global()->AIHousesLists.at(idxHousesList);

			if (objectsList.size() > 0)
			{
				int IdxSelectedObject = ScenarioClass::Instance->Random.RandomRanged(0, objectsList.size() - 1);
				HouseTypeClass* pHouseType = objectsList.at(IdxSelectedObject);

				for (auto& angerNode : pTeam->Owner->AngerNodes)
				{
					if (angerNode.House->Defeated || angerNode.House->IsObserver())
						continue;

					HouseTypeClass* angerNodeType = angerNode.House->Type;

					if (_stricmp(angerNodeType->ID, pHouseType->ID) == 0)
					{
						angerNode.AngerLevel += pTeamData->AngerNodeModifier;
						changes++;
					}
				}
			}
		}
	}

	// This action finished
	if (changes == 0)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Failed to modify hate values against other houses\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument);
	}

	ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);
	ScriptExt::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::SetTheMostHatedHouse(TeamClass* pTeam, int mask = 0, int mode = 1, bool random = false)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (mask == 0)
		mask = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (mask == 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	std::vector<HouseClass*> objectsList;
	int IdxSelectedObject = -1;
	HouseClass* selectedHouse = nullptr;
	int highestHateLevel = 0;
	int newHateLevel = 5000;

	if (pTeamData->AngerNodeModifier > 0)
		newHateLevel = pTeamData->AngerNodeModifier;

	// Find the highest House hate value
	for (const auto& angerNode : pTeam->Owner->AngerNodes)
	{
		if (pTeam->Owner == angerNode.House
			|| angerNode.House->Defeated
			|| pTeam->Owner->IsAlliedWith(angerNode.House)
			|| angerNode.House->Type->MultiplayPassive
			|| angerNode.House->IsObserver())
		{
			continue;
		}

		if (random)
		{
			objectsList.emplace_back(angerNode.House);
		}
		else
		{
			if (angerNode.AngerLevel > highestHateLevel)
				highestHateLevel = angerNode.AngerLevel;
		}
	}

	newHateLevel += highestHateLevel;

	// Pick a enemy house
	if (random)
	{
		if (objectsList.size() > 0)
		{
			IdxSelectedObject = ScenarioClass::Instance->Random.RandomRanged(0, objectsList.size() - 1);
			selectedHouse = objectsList.at(IdxSelectedObject);
		}
	}
	else
	{
		selectedHouse = GetTheMostHatedHouse(pTeam, mask, mode);
	}

	if (selectedHouse)
	{
		for (auto& angerNode : pTeam->Owner->AngerNodes)
		{
			if (angerNode.House->Defeated || angerNode.House->IsObserver())
				continue;

			if (angerNode.House == selectedHouse)
			{
				angerNode.AngerLevel = newHateLevel;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Picked a new house as enemy [%s]\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, angerNode.House->Type->ID);
			}
		}

		ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);
	}
	else
	{
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Failed to pick a new hated house\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument);
	}

	// This action finished
	pTeam->StepCompleted = true;
}

HouseClass* ScriptExt::GetTheMostHatedHouse(TeamClass* pTeam, int mask = 0, int mode = 1)
{
	if (!pTeam || mask == 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return nullptr;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return nullptr;
	}

	// Note regarding "mode": 1 is used for ">" comparisons and 0 for "<"
	if (mode <= 0)
		mode = 0;
	else
		mode = 1;

	// Find the Team Leader
	FootClass* pLeaderUnit = FindTheTeamLeader(pTeam);

	if (!pLeaderUnit)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return nullptr;
	}

	double objectDistance = -1;
	double enemyDistance = -1;
	double enemyThreatValue[8] = { 0 };
	HouseClass* enemyHouse = nullptr;
	double const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
	long houseMoney = -1;
	int enemyPower = -1000000000;
	int enemyKills = -1;
	int enemyAirDocks = -1;
	int enemyStructures = -1;
	int enemyNavalUnits = -1;

	if (mask == -2)
	{
		// Based on House economy
		for (const auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->IsObserver()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			if (mode == 0)
			{
				// The poorest is selected
				if (pHouse->Available_Money() < houseMoney || houseMoney < 0)
				{
					houseMoney = pHouse->Available_Money();
					enemyHouse = pHouse;
				}
			}
			else
			{
				// The richest is selected
				if (pHouse->Available_Money() > houseMoney || houseMoney < 0)
				{
					houseMoney = pHouse->Available_Money();
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Selected House [%s] (index: %d)\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, enemyHouse->Type->ID, enemyHouse->ArrayIndex);

		return enemyHouse;
	}

	if (mask == -3)
	{
		// Based on Human Controlled check
		for (const auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| !pHouse->IsControlledByHuman()
				|| pHouse->IsObserver()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			CoordStruct houseLocation;
			houseLocation.X = pHouse->BaseSpawnCell.X;
			houseLocation.Y = pHouse->BaseSpawnCell.Y;
			houseLocation.Z = 0;
			objectDistance = pLeaderUnit->Location.DistanceFrom(houseLocation); // Note: distance is in leptons (*256)

			if (mode == 0)
			{
				// mode 0: Based in NEAREST human enemy unit
				if (objectDistance < enemyDistance || enemyDistance == -1)
				{
					enemyDistance = objectDistance;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// mode 1: Based in FARTHEST human enemy unit
				if (objectDistance > enemyDistance || enemyDistance == -1)
				{
					enemyDistance = objectDistance;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, enemyHouse->Type->ID, enemyHouse->ArrayIndex);

		return enemyHouse;
	}

	if (mask == -4 || mask == -5 || mask == -6)
	{
		int checkedHousePower;

		// House power check
		for (const auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->Defeated
				|| pHouse->IsObserver()
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			if (mask == -4)
				checkedHousePower = pHouse->Power_Drain();

			if (mask == -5)
				checkedHousePower = pHouse->PowerOutput;

			if (mask == -6)
				checkedHousePower = pHouse->PowerOutput - pHouse->Power_Drain();

			if (mode == 0)
			{
				// mode 0: Selection based in lower value power in house
				if ((checkedHousePower < enemyPower) || enemyPower == -1000000000)
				{
					enemyPower = checkedHousePower;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// mode 1: Selection based in higher value power in house
				if ((checkedHousePower > enemyPower) || enemyPower == -1000000000)
				{
					enemyPower = checkedHousePower;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, enemyHouse->Type->ID, enemyHouse->ArrayIndex);

		return enemyHouse;
	}

	if (mask == -7)
	{
		// Based on House kills
		for (const auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->IsObserver()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			int currentKills = pHouse->TotalKilledUnits + pHouse->TotalKilledUnits;

			if (mode == 0)
			{
				// The pacifist is selected
				if (currentKills < enemyKills || enemyKills < 0)
				{
					enemyKills = currentKills;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// The major killer is selected
				if (currentKills > enemyKills || enemyKills < 0)
				{
					enemyKills = currentKills;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, enemyHouse->Type->ID, enemyHouse->ArrayIndex);

		return enemyHouse;
	}

	if (mask == -8)
	{
		// Based on number of House naval units
		for (const auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->IsObserver()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			int currentNavalUnits = 0;

			for (const auto& pUnit : *TechnoClass::Array)
			{
				if (pUnit->IsAlive
					&& pUnit->Health > 0
					&& pUnit->Owner == pHouse
					&& !pUnit->InLimbo
					&& pUnit->IsOnMap
					&& ScriptExt::EvaluateObjectWithMask(pUnit, 31, -1, -1, nullptr))
				{
					currentNavalUnits++;
				}
			}

			if (mode == 0)
			{
				// The House with less naval units is selected
				if (currentNavalUnits < enemyNavalUnits || enemyNavalUnits < 0)
				{
					enemyNavalUnits = currentNavalUnits;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// The House with more naval units is selected
				if (currentNavalUnits > enemyNavalUnits || enemyNavalUnits < 0)
				{
					enemyNavalUnits = currentNavalUnits;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, enemyHouse->Type->ID, enemyHouse->ArrayIndex);

		return enemyHouse;
	}

	if (mask == -9)
	{
		// Based on number of House aircraft docks
		for (const auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->IsObserver()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			int currentAirDocks = pHouse->AirportDocks;

			if (mode == 0)
			{
				// The House with less Aircraft docks is selected
				if (currentAirDocks < enemyAirDocks || enemyAirDocks < 0)
				{
					enemyAirDocks = currentAirDocks;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// The House with more Aircraft docks is selected
				if (currentAirDocks > enemyAirDocks || enemyAirDocks < 0)
				{
					enemyAirDocks = currentAirDocks;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, enemyHouse->Type->ID, enemyHouse->ArrayIndex);

		return enemyHouse;
	}

	if (mask == -10)
	{
		// Based on number of House factories (except aircraft factories)
		for (const auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->IsObserver()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			int currentFactories = pHouse->NumWarFactories + pHouse->NumConYards + pHouse->NumShipyards + pHouse->NumBarracks;

			if (mode == 0)
			{
				// The House with less factories is selected
				if (currentFactories < enemyStructures || enemyStructures < 0)
				{
					enemyStructures = currentFactories;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// The House with more factories is selected
				if (currentFactories > enemyStructures || enemyStructures < 0)
				{
					enemyStructures = currentFactories;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, enemyHouse->Type->ID, enemyHouse->ArrayIndex);

		return enemyHouse;
	}

	// Depending the mode check what house will be selected as the most hated
	for (auto pTechno : *TechnoClass::Array)
	{
		if (!pTechno->Owner->Defeated
			&& pTechno->Owner != pTeam->Owner
			&& pTechno->IsAlive
			&& !pTechno->InLimbo
			&& pTechno->IsOnMap
			&& !pTechno->Owner->IsAlliedWith(pTeam->Owner)
			&& !pTechno->Owner->Type->MultiplayPassive)
		{
			if (mask < 0)
			{
				if (mask == -1)
				{
					// mask -1: Based on object distances
					objectDistance = pLeaderUnit->DistanceFrom(pTechno); // Note: distance is in leptons (*256)

					if (mode == 0)
					{
						// mode 0: Based in NEAREST enemy unit
						if (objectDistance < enemyDistance || enemyDistance == -1)
						{
							enemyDistance = objectDistance;
							enemyHouse = pTechno->Owner;
						}
					}
					else
					{
						// mode 1: Based in FARTHEST enemy unit
						if (objectDistance > enemyDistance || enemyDistance == -1)
						{
							enemyDistance = objectDistance;
							enemyHouse = pTechno->Owner;
						}
					}
				}
			}
			else
			{
				// mask > 0 : Threat based on the new types in the new attack actions
				if (ScriptExt::EvaluateObjectWithMask(pTechno, mask, -1, -1, pLeaderUnit))
				{
					auto pTechnoType = pTechno->GetTechnoType();

					if (pTechnoType)
					{
						enemyThreatValue[pTechno->Owner->ArrayIndex] += pTechnoType->ThreatPosed;

						if (pTechnoType->SpecialThreatValue > 0)
							enemyThreatValue[pTechno->Owner->ArrayIndex] += pTechnoType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
					}
				}
			}
		}
	}

	if (mask > 0)
	{
		double value = -1;

		for (int i = 0; i < 8; i++)
		{
			if (mode == 0)
			{
				// Select House with LESS threat
				if ((enemyThreatValue[i] < value || value == -1)
					&& !HouseClass::Array->GetItem(i)->Defeated
					&& !HouseClass::Array->GetItem(i)->IsObserver())
				{
					value = enemyThreatValue[i];
					enemyHouse = HouseClass::Array->GetItem(i);
				}
			}
			else
			{
				// Select House with MORE threat
				if ((enemyThreatValue[i] > value || value == -1)
					&& !HouseClass::Array->GetItem(i)->Defeated
					&& !HouseClass::Array->GetItem(i)->IsObserver())
				{
					value = enemyThreatValue[i];
					enemyHouse = HouseClass::Array->GetItem(i);
				}
			}
		}
	}

	if (enemyHouse)
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, enemyHouse->Type->ID, enemyHouse->ArrayIndex);

	return enemyHouse;
}

void ScriptExt::OverrideOnlyTargetHouseEnemy(TeamClass* pTeam, int mode = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (mode < 0 || mode > 2)
		mode = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (mode < -1 || mode > 2)
		mode = -1;

	pTeamData->OnlyTargetHouseEnemyMode = mode;
	/*
	Modes:
		0  -> Force "False"
		1  -> Force "True"
		2  -> Force "Random boolean"
		-1 -> Use default value in OnlyTargetHouseEnemy tag
		Note: only works for new Actions, not vanilla YR actions
	*/
	switch (mode)
	{
	case 0:
		pTeamData->OnlyTargetHouseEnemy = false;
		break;

	case 1:
		pTeamData->OnlyTargetHouseEnemy = true;
		break;

	case 2:
		pTeamData->OnlyTargetHouseEnemy = (bool)ScenarioClass::Instance->Random.RandomRanged(0, 1);
		break;

	default:
		pTeamData->OnlyTargetHouseEnemy = pTeam->Type->OnlyTargetHouseEnemy;
		pTeamData->OnlyTargetHouseEnemyMode = -1;
		break;
	}

	Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): New Team -> OnlyTargetHouseEnemy value: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, pTeamData->OnlyTargetHouseEnemy);
	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ModifyHateHouse_Index(TeamClass* pTeam, int idxHouse = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (idxHouse < 0)
		idxHouse = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (idxHouse < 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}
	else
	{
		for (auto& angerNode : pTeam->Owner->AngerNodes)
		{
			if (angerNode.House->ArrayIndex == idxHouse
				&& !angerNode.House->Defeated
				&& !angerNode.House->IsObserver())
			{
				angerNode.AngerLevel += pTeamData->AngerNodeModifier;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Modified anger level against [%s](index: %d) with value: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, angerNode.House->Type->ID, angerNode.House->ArrayIndex, angerNode.AngerLevel);
			}
		}
	}

	ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);
	ScriptExt::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true;
}

// The selected house will become the most hated of the map (the effects are only visible if the other houses are enemy of the selected house)
void ScriptExt::AggroHouse(TeamClass* pTeam, int index = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	std::vector<HouseClass*> objectsList;
	HouseClass* selectedHouse = nullptr;
	int newHateLevel = 5000;

	if (pTeamData->AngerNodeModifier > 0)
		newHateLevel = pTeamData->AngerNodeModifier;

	// Store the list of playable houses for later
	for (const auto& angerNode : pTeam->Owner->AngerNodes)
	{
		if (!angerNode.House->Defeated
			&& !angerNode.House->Type->MultiplayPassive
			&& !angerNode.House->IsObserver())
		{
			objectsList.emplace_back(angerNode.House);
		}
	}

	// Include the own House if we are looking for ANY Human player
	if (index == -3)
	{
		if (!pTeam->Owner->Defeated
			&& !pTeam->Owner->Type->MultiplayPassive
			&& !pTeam->Owner->IsObserver()
			&& !pTeam->Owner->IsControlledByHuman())
		{
			objectsList.emplace_back(pTeam->Owner);
		}
	}

	// Positive indexes are specific house indexes. -1 is translated as "pick 1 random" & -2 is the owner of the Team executing the script action
	if (objectsList.size() > 0)
	{
		if (index < 0)
		{
			if (index == -1)
				index = ScenarioClass::Instance->Random.RandomRanged(0, objectsList.size() - 1);

			if (index == -2)
				index = pTeam->Owner->ArrayIndex;
		}
	}
	else
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	// Note: at most each "For" lasts 10 loops: 8 players + Civilian + Special houses
	if (index != -3)
	{
		for (const auto& pHouse : *HouseClass::Array)
		{
			if (!pHouse->Defeated && pHouse->ArrayIndex == index)
				selectedHouse = pHouse;
		}
	}

	if (selectedHouse || index == -3)
	{
		// For each playable house set the selected house as the one with highest hate value;
		for (auto& pHouse : objectsList)
		{
			int highestHateLevel = 0;

			for (const auto& angerNode : pHouse->AngerNodes)
			{
				if (angerNode.AngerLevel > highestHateLevel)
					highestHateLevel = angerNode.AngerLevel;
			}

			for (auto& angerNode : pHouse->AngerNodes)
			{
				if (index == -3)
				{
					if (angerNode.House->IsControlledByHuman())
						angerNode.AngerLevel = highestHateLevel + newHateLevel;
				}
				else
				{
					if (selectedHouse == angerNode.House)
						angerNode.AngerLevel = highestHateLevel + newHateLevel;
				}
			}

			ScriptExt::UpdateEnemyHouseIndex(pHouse);
		}
	}
	else
	{
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Failed to pick a new hated house with index: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, index);
	}

	ScriptExt::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::UpdateEnemyHouseIndex(HouseClass* pHouse)
{
	int angerLevel = 0;
	int index = -1;

	for (const auto& angerNode : pHouse->AngerNodes)
	{
		if (!angerNode.House->Defeated
			&& !angerNode.House->IsObserver()
			&& !pHouse->IsAlliedWith(angerNode.House)
			&& angerNode.AngerLevel > angerLevel)
		{
			angerLevel = angerNode.AngerLevel;
			index = angerNode.House->ArrayIndex;
		}
	}

	pHouse->EnemyHouseIndex = index;
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
		VariableOperationHandler<false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAdd:
		VariableOperationHandler<false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinus:
		VariableOperationHandler<false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiply:
		VariableOperationHandler<false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivide:
		VariableOperationHandler<false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMod:
		VariableOperationHandler<false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShift:
		VariableOperationHandler<false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShift:
		VariableOperationHandler<false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverse:
		VariableOperationHandler<false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXor:
		VariableOperationHandler<false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOr:
		VariableOperationHandler<false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAnd:
		VariableOperationHandler<false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSet:
		VariableOperationHandler<true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAdd:
		VariableOperationHandler<true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinus:
		VariableOperationHandler<true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiply:
		VariableOperationHandler<true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivide:
		VariableOperationHandler<true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMod:
		VariableOperationHandler<true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShift:
		VariableOperationHandler<true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShift:
		VariableOperationHandler<true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverse:
		VariableOperationHandler<true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXor:
		VariableOperationHandler<true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOr:
		VariableOperationHandler<true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAnd:
		VariableOperationHandler<true, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableSetByLocal:
		VariableBinaryOperationHandler<false, false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAddByLocal:
		VariableBinaryOperationHandler<false, false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinusByLocal:
		VariableBinaryOperationHandler<false, false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiplyByLocal:
		VariableBinaryOperationHandler<false, false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivideByLocal:
		VariableBinaryOperationHandler<false, false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableModByLocal:
		VariableBinaryOperationHandler<false, false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShiftByLocal:
		VariableBinaryOperationHandler<false, false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShiftByLocal:
		VariableBinaryOperationHandler<false, false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverseByLocal:
		VariableBinaryOperationHandler<false, false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXorByLocal:
		VariableBinaryOperationHandler<false, false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOrByLocal:
		VariableBinaryOperationHandler<false, false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAndByLocal:
		VariableBinaryOperationHandler<false, false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSetByLocal:
		VariableBinaryOperationHandler<false, true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAddByLocal:
		VariableBinaryOperationHandler<false, true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinusByLocal:
		VariableBinaryOperationHandler<false, true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiplyByLocal:
		VariableBinaryOperationHandler<false, true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivideByLocal:
		VariableBinaryOperationHandler<false, true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableModByLocal:
		VariableBinaryOperationHandler<false, true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShiftByLocal:
		VariableBinaryOperationHandler<false, true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShiftByLocal:
		VariableBinaryOperationHandler<false, true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverseByLocal:
		VariableBinaryOperationHandler<false, true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXorByLocal:
		VariableBinaryOperationHandler<false, true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOrByLocal:
		VariableBinaryOperationHandler<false, true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAndByLocal:
		VariableBinaryOperationHandler<false, true, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableSetByGlobal:
		VariableBinaryOperationHandler<true, false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAddByGlobal:
		VariableBinaryOperationHandler<true, false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinusByGlobal:
		VariableBinaryOperationHandler<true, false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiplyByGlobal:
		VariableBinaryOperationHandler<true, false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivideByGlobal:
		VariableBinaryOperationHandler<true, false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableModByGlobal:
		VariableBinaryOperationHandler<true, false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShiftByGlobal:
		VariableBinaryOperationHandler<true, false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShiftByGlobal:
		VariableBinaryOperationHandler<true, false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverseByGlobal:
		VariableBinaryOperationHandler<true, false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXorByGlobal:
		VariableBinaryOperationHandler<true, false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOrByGlobal:
		VariableBinaryOperationHandler<true, false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAndByGlobal:
		VariableBinaryOperationHandler<true, false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSetByGlobal:
		VariableBinaryOperationHandler<true, true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAddByGlobal:
		VariableBinaryOperationHandler<true, true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinusByGlobal:
		VariableBinaryOperationHandler<true, true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiplyByGlobal:
		VariableBinaryOperationHandler<true, true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivideByGlobal:
		VariableBinaryOperationHandler<true, true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableModByGlobal:
		VariableBinaryOperationHandler<true, true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShiftByGlobal:
		VariableBinaryOperationHandler<true, true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShiftByGlobal:
		VariableBinaryOperationHandler<true, true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverseByGlobal:
		VariableBinaryOperationHandler<true, true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXorByGlobal:
		VariableBinaryOperationHandler<true, true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOrByGlobal:
		VariableBinaryOperationHandler<true, true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAndByGlobal:
		VariableBinaryOperationHandler<true, true, operation_and>(pTeam, nLoArg, nHiArg); break;
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
		VariableOperationHandler<IsGlobal, _Pr>(pTeam, nVariable, itr->second.Value);

	pTeam->StepCompleted = true;
}

FootClass* ScriptExt::FindTheTeamLeader(TeamClass* pTeam)
{
	FootClass* pLeaderUnit = nullptr;
	int bestUnitLeadershipValue = -1;

	if (!pTeam)
		return pLeaderUnit;

	// Find the Leader or promote a new one
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!IsUnitAvailable(pUnit, true) || !(pUnit->IsInitiated || pUnit->WhatAmI() == AbstractType::Aircraft))
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

void ScriptExt::DebugAngerNodesData()
{
	Debug::Log("DEBUG: Updated AngerNodes lists of every playable House:\n");

	for (auto pHouse : *HouseClass::Array)
	{
		if (pHouse->IsObserver())
			Debug::Log("Player %d [Observer] ", pHouse->ArrayIndex);
		else
			Debug::Log("Player %d [%s]: ", pHouse->ArrayIndex, pHouse->Type->ID);

		int i = 0;

		for (auto& angerNode : pHouse->AngerNodes)
		{
			if (!pHouse->IsObserver())
			Debug::Log("%d:%d", angerNode.House->ArrayIndex, angerNode.AngerLevel);

			if (i < HouseClass::Array->Count - 2 && !pHouse->IsObserver())
				Debug::Log(", ");

			i++;
		}

		if (!pHouse->IsObserver())
			Debug::Log(" -> Main Enemy House: %d\n", pHouse->EnemyHouseIndex);
		else
			Debug::Log("\n");
	}
}

void ScriptExt::Set_ForceJump_Countdown(TeamClass *pTeam, bool repeatLine = false, int count = 0)
{
	if (!pTeam)
		return;

	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeamData)
		return;

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

	auto pScript = pTeam->CurrentScript;

	// This action finished
	pTeam->StepCompleted = true;
	ScriptExt::Log("AI Scripts - SetForceJumpCountdown: [%s] [%s](line: %d = %d,%d) Set Timed Jump -> (Countdown: %d, repeat action: %d)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, count, repeatLine);
}

void ScriptExt::Stop_ForceJump_Countdown(TeamClass* pTeam)
{
	if (!pTeam)
		return;

	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeamData)
		return;

	pTeamData->ForceJump_InitialCountdown = -1;
	pTeamData->ForceJump_Countdown.Stop();
	pTeamData->ForceJump_RepeatMode = false;

	auto pScript = pTeam->CurrentScript;

	// This action finished
	pTeam->StepCompleted = true;
	ScriptExt::Log("AI Scripts - StopForceJumpCountdown: [%s] [%s](line: %d = %d,%d): Stopped Timed Jump\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument);
}

void ScriptExt::ChronoshiftToEnemyBase(TeamClass* pTeam, int extraDistance)
{
	if (!pTeam)
		return;

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
	HouseClass* pEnemy = houseIndex != -1 ? HouseClass::Array->GetItem(houseIndex) : nullptr;

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
	if (!pTeam || !pTeamLeader || !pTarget)
		return;

	auto pScript = pTeam->CurrentScript;
	HouseClass* pOwner = pTeamLeader->Owner;
	SuperClass* pSuperChronosphere = nullptr;
	SuperClass* pSuperChronowarp = nullptr;

	for (auto const pSuper : pOwner->Supers)
	{
		if (!pSuperChronosphere && pSuper->Type->Type == SuperWeaponType::ChronoSphere)
			pSuperChronosphere = pSuper;

		if (!pSuperChronowarp && pSuper->Type->Type == SuperWeaponType::ChronoWarp)
			pSuperChronowarp = pSuper;

		if (pSuperChronosphere && pSuperChronowarp)
			break;
	}

	char logTextBase[1024];
	char logTextJump[1024];
	char jump[256];

	sprintf_s(jump, "Jump to next line: %d = %d,%d -> (Reason: %s)", pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument, "%s");
	sprintf_s(logTextBase, "AI Scripts - ChronoshiftTeamToTarget: [%s] [%s] (line: %d = %d,%d) %s\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, "%s");
	sprintf_s(logTextJump, logTextBase, jump);

	if (!pSuperChronosphere || !pSuperChronowarp)
	{
		ScriptExt::Log(logTextJump, "No Chronosphere or ChronoWarp superweapon found");
		pTeam->StepCompleted = true;
		return;
	}

	if (!pSuperChronosphere->IsReady || (pSuperChronosphere->IsPowered() && !pOwner->Is_Powered()))
	{
		if (pSuperChronosphere->IsPresent)
		{
			int rechargeTime = pSuperChronosphere->GetRechargeTime();
			int timeLeft = pSuperChronosphere->RechargeTimer.GetTimeLeft();

			if (1.0 - RulesClass::Instance->AIMinorSuperReadyPercent < timeLeft / rechargeTime)
			{
				ScriptExt::Log(logTextBase, "Chronosphere superweapon charge not at AIMinorSuperReadyPercent yet, not jumping to next line yet");
				return;
			}
		}
		else
		{
			ScriptExt::Log(logTextJump, "Chronosphere superweapon is not available");
			pTeam->StepCompleted = true;
			return;
		}
	}

	auto pTargetCell = MapClass::Instance->TryGetCellAt(pTarget->GetCoords());

	if (pTargetCell)
	{
		pOwner->Fire_SW(pSuperChronosphere->Type->ArrayIndex, pTeam->SpawnCell->MapCoords);
		pOwner->Fire_SW(pSuperChronowarp->Type->ArrayIndex, pTargetCell->MapCoords);
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
