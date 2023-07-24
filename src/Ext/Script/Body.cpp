#include "Body.h"
#include "../Techno/Body.h"
#include "../BuildingType/Body.h"

#include <Ext/Scenario/Body.h>

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
	default:
		// Do nothing because or it is a wrong Action number or it is an Ares/YR action...
		if (action > 70 && !IsExtVariableAction(action))
		{
			// Unknown new action. This action finished
			pTeam->StepCompleted = true;
			Debug::Log("[%s] [%s] (line %d): Unknown Script Action: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action);
		}
		break;
	}

	if (IsExtVariableAction(action))
		VariablesHandler(pTeam, static_cast<PhobosScripts>(action), argument);
}

void ScriptExt::ExecuteTimedAreaGuardAction(TeamClass* pTeam)
{
	auto pScript = pTeam->CurrentScript;
	auto pScriptType = pScript->Type;

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
	DynamicVectorClass<FootClass*> transports;

	// Collect available transports
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto const pType = pUnit->GetTechnoType();

		if (pType->Passengers > 0)
			if (pUnit->Passengers.NumPassengers < pType->Passengers)
				if (pUnit->Passengers.GetTotalSize() < pType->Passengers)
					transports.AddItem(pUnit);
	}

	if (transports.Count == 0)
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
				&& !pUnit->InLimbo
				&& !pUnitType->ConsideredAircraft
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
		if (pUnit->GetCurrentMission() == Mission::Enter)
			return;

	auto pExt = TeamExt::ExtMap.Find(pTeam);
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
				if (auto pAircraft = abstract_cast<AircraftClass*>(pUnit))
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
	auto pExt = TeamExt::ExtMap.Find(pTeam);

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
		if (!pLeaderUnit
			|| !pLeaderUnit->IsAlive
			|| pLeaderUnit->Health <= 0
			|| pLeaderUnit->InLimbo
			|| !(pLeaderUnit->IsOnMap || (pLeaderUnit->GetTechnoType()->IsSubterranean))
			|| pLeaderUnit->Transporter
			|| pLeaderUnit->Absorbed)
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
			if (pUnit
				&& pUnit->IsAlive
				&& pUnit->Health > 0
				&& !pUnit->InLimbo
				&& pUnit->IsOnMap
				&& !pUnit->Absorbed)
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
						pUnit->QueueMission(Mission::Return, false);
						pUnit->Mission_Enter();

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

void ScriptExt::Mission_Attack(TeamClass *pTeam, bool repeatAction = true, int calcThreatMode = 0, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	int scriptArgument = pScript->Type->ScriptActions[pScript->CurrentMission].Argument; // This is the target type
	TechnoClass* selectedTarget = nullptr;
	HouseClass* enemyHouse = nullptr;
	bool noWaitLoop = false;
	FootClass *pLeaderUnit = nullptr;
	TechnoTypeClass* pLeaderUnitType = nullptr;
	bool bAircraftsWithoutAmmo = false;
	TechnoClass* pFocus = nullptr;
	bool agentMode = false;
	bool pacifistTeam = true;
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pScript)
		return;

	if (!pTeamData)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: ExtData found)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	// When the new target wasn't found it sleeps some few frames before the new attempt. This can save cycles and cycles of unnecessary executed lines.
	if (pTeamData->WaitNoTargetCounter > 0)
	{
		if (pTeamData->WaitNoTargetTimer.InProgress())
			return;

		pTeamData->WaitNoTargetTimer.Stop();
		noWaitLoop = true;
		pTeamData->WaitNoTargetCounter = 0;

		if (pTeamData->WaitNoTargetAttempts > 0)
			pTeamData->WaitNoTargetAttempts--;
	}

	// This team has no units!
	if (!pTeam)
	{
		if (pTeamData->CloseEnough > 0)
			pTeamData->CloseEnough = -1;

		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No team members alive)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	pFocus = abstract_cast<TechnoClass*>(pTeam->Focus);
	if (!pFocus
		|| !pFocus->IsAlive
		|| pFocus->Health <= 0
		|| pFocus->InLimbo
		|| !pFocus->IsOnMap
		|| pFocus->Transporter
		|| pFocus->Absorbed)
	{
		pTeam->Focus = nullptr;
		pFocus = nullptr;
	}

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto pKillerTechnoData = TechnoExt::ExtMap.Find(pUnit);
		if (pKillerTechnoData && pKillerTechnoData->LastKillWasTeamTarget)
		{
			// Time for Team award check! (if set any)
			if (pTeamData->NextSuccessWeightAward > 0)
			{
				IncreaseCurrentTriggerWeight(pTeam, false, pTeamData->NextSuccessWeightAward);
				pTeamData->NextSuccessWeightAward = 0;
			}

			// Let's clean the Killer mess
			pKillerTechnoData->LastKillWasTeamTarget = false;
			pFocus = nullptr;
			pTeam->Focus = nullptr;

			if (!repeatAction)
			{
				// If the previous Team's Target was killed by this Team Member and the script was a 1-time-use then this script action must be finished.
				for (auto pTeamUnit = pTeam->FirstUnit; pTeamUnit; pTeamUnit = pTeamUnit->NextTeamMember)
				{
					// Let's reset all Team Members objective
					auto pKillerTeamUnitData = TechnoExt::ExtMap.Find(pTeamUnit);
					pKillerTeamUnitData->LastKillWasTeamTarget = false;

					if (pTeamUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType)
					{
						pTeamUnit->SetTarget(nullptr);
						pTeamUnit->LastTarget = nullptr;
						pTeamUnit->SetFocus(nullptr); // Lets see if this works or bug my little aircrafts
						pTeamUnit->CurrentTargets.Clear(); // Lets see if this works or bug my little aircrafts
						pTeamUnit->QueueMission(Mission::Guard, true);
					}
				}

				pTeamData->IdxSelectedObjectFromAIList = -1;

				// This action finished
				pTeam->StepCompleted = true;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Force the jump to next line: %d = %d,%d (This action wont repeat)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

				return;
			}
		}
	}

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit && pUnit->IsAlive && !pUnit->InLimbo)
		{
			auto pUnitType = pUnit->GetTechnoType();
			if (pUnitType)
			{
				if (pUnitType->WhatAmI() == AbstractType::AircraftType
					&& !pUnit->IsInAir()
					&& abstract_cast<AircraftTypeClass*>(pUnitType)->AirportBound
					&& pUnit->Ammo < pUnitType->Ammo)
				{
					bAircraftsWithoutAmmo = true;
					pUnit->CurrentTargets.Clear();
				}

				bool pacifistUnit = true;
				if (pUnit->Veterancy.IsElite())
				{
					if (pUnitType->EliteWeapon[0].WeaponType || pUnitType->EliteWeapon[1].WeaponType
						|| (pUnitType->IsGattling && pUnitType->EliteWeapon[pUnit->CurrentWeaponNumber].WeaponType))
					{
						pacifistTeam = false;
						pacifistUnit = false;
					}
				}
				else
				{
					if (pUnitType->Weapon[0].WeaponType || pUnitType->Weapon[1].WeaponType
						|| (pUnitType->IsGattling && pUnitType->Weapon[pUnit->CurrentWeaponNumber].WeaponType))
					{
						pacifistTeam = false;
						pacifistUnit = false;
					}
				}

				// Any Team member (infantry) is a special agent? If yes ignore some checks based on Weapons.
				if (pUnitType->WhatAmI() == AbstractType::InfantryType)
				{
					auto pTypeInf = abstract_cast<InfantryTypeClass*>(pUnitType);
					if ((pTypeInf->Agent && pTypeInf->Infiltrate) || pTypeInf->Engineer)
					{
						agentMode = true;
					}
				}
			}
		}
	}

	// Find the Leader
	pLeaderUnit = pTeamData->TeamLeader;
	if (!pLeaderUnit
		|| !pLeaderUnit->IsAlive
		|| pLeaderUnit->Health <= 0
		|| pLeaderUnit->InLimbo
		|| !(pLeaderUnit->IsOnMap || (pLeaderUnit->GetTechnoType()->IsSubterranean))
		|| pLeaderUnit->Transporter
		|| pLeaderUnit->Absorbed)
	{
		pLeaderUnit = FindTheTeamLeader(pTeam);
		pTeamData->TeamLeader = pLeaderUnit;
	}

	if (!pLeaderUnit || bAircraftsWithoutAmmo || (pacifistTeam && !agentMode))
	{
		pTeamData->IdxSelectedObjectFromAIList = -1;
		if (pTeamData->WaitNoTargetAttempts != 0)
		{
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0;
			pTeamData->WaitNoTargetAttempts = 0;
		}

		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No Leader found | Exists Aircrafts without ammo | Team members have no weapons)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	pLeaderUnitType = pLeaderUnit->GetTechnoType();
	bool leaderWeaponsHaveAA = false;
	bool leaderWeaponsHaveAG = false;
	// Note: Replace these lines when I have access to Combat_Damage() method in YRpp if that is better
	WeaponTypeClass* WeaponType1 = pLeaderUnit->Veterancy.IsElite() ?
		pLeaderUnitType->EliteWeapon[0].WeaponType :
		pLeaderUnitType->Weapon[0].WeaponType;

	WeaponTypeClass* WeaponType2 = pLeaderUnit->Veterancy.IsElite() ?
		pLeaderUnitType->EliteWeapon[1].WeaponType :
		pLeaderUnitType->Weapon[1].WeaponType;

	WeaponTypeClass* WeaponType3 = WeaponType1;

	if (pLeaderUnitType->IsGattling)
	{
		WeaponType3 = pLeaderUnit->Veterancy.IsElite() ?
			pLeaderUnitType->EliteWeapon[pLeaderUnit->CurrentWeaponNumber].WeaponType :
			pLeaderUnitType->Weapon[pLeaderUnit->CurrentWeaponNumber].WeaponType;

		WeaponType1 = WeaponType3;
	}

	// Weapon check used for filtering Leader targets.
	// Note: the Team Leader is picked for this task, be careful with leadership rating values in your mod
	if ((WeaponType1 && WeaponType1->Projectile->AA) || (WeaponType2 && WeaponType2->Projectile->AA))
		leaderWeaponsHaveAA = true;

	if ((WeaponType1 && WeaponType1->Projectile->AG) || (WeaponType2 && WeaponType2->Projectile->AG) || agentMode)
		leaderWeaponsHaveAG = true;

	// Special case: a Leader with OpenTopped tag
	if (pLeaderUnitType->OpenTopped && pLeaderUnit->Passengers.NumPassengers > 0)
	{
		for (NextObject j(pLeaderUnit->Passengers.FirstPassenger->NextObject); j && abstract_cast<FootClass*>(*j); ++j)
		{
			auto passenger = static_cast<FootClass*>(*j);
			auto pPassengerType = passenger->GetTechnoType();

			if (pPassengerType)
			{
				// Note: Replace these lines when I have access to Combat_Damage() method in YRpp if that is better
				WeaponTypeClass* passengerWeaponType1 = passenger->Veterancy.IsElite() ?
					pPassengerType->EliteWeapon[0].WeaponType :
					pPassengerType->Weapon[0].WeaponType;

				WeaponTypeClass* passengerWeaponType2 = passenger->Veterancy.IsElite() ?
					pPassengerType->EliteWeapon[1].WeaponType :
					pPassengerType->Weapon[1].WeaponType;

				if (pPassengerType->IsGattling)
				{
					WeaponTypeClass* passengerWeaponType3 = passenger->Veterancy.IsElite() ?
						pPassengerType->EliteWeapon[passenger->CurrentWeaponNumber].WeaponType :
						pPassengerType->Weapon[passenger->CurrentWeaponNumber].WeaponType;

					passengerWeaponType1 = passengerWeaponType3;
				}

				// Used for filtering targets.
				// Note: the units inside a openTopped Leader are used for this task
				if ((passengerWeaponType1 && passengerWeaponType1->Projectile->AA)
					|| (passengerWeaponType2 && passengerWeaponType2->Projectile->AA))
				{
					leaderWeaponsHaveAA = true;
				}

				if ((passengerWeaponType1 && passengerWeaponType1->Projectile->AG)
					|| (passengerWeaponType2 && passengerWeaponType2->Projectile->AG))
				{
					leaderWeaponsHaveAG = true;
				}
			}
		}
	}

	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		// This part of the code is used for picking a new target.

		// Favorite Enemy House case. If set, AI will focus against that House
		if (pTeam->Type->OnlyTargetHouseEnemy && pLeaderUnit->Owner->EnemyHouseIndex >= 0)
			enemyHouse = HouseClass::Array->GetItem(pLeaderUnit->Owner->EnemyHouseIndex);

		int targetMask = scriptArgument;
		selectedTarget = GreatestThreat(pLeaderUnit, targetMask, calcThreatMode, enemyHouse, attackAITargetType, idxAITargetTypeItem, agentMode);

		if (selectedTarget)
		{
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Leader [%s] (UID: %lu) selected [%s] (UID: %lu) as target.\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pLeaderUnit->GetTechnoType()->get_ID(), pLeaderUnit->UniqueID, selectedTarget->GetTechnoType()->get_ID(), selectedTarget->UniqueID);

			pTeam->Focus = selectedTarget;
			pTeamData->WaitNoTargetAttempts = 0; // Disable Script Waits if there are any because a new target was selected
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0; // Disable Script Waits if there are any because a new target was selected

			for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			{
				if (pUnit->IsAlive && !pUnit->InLimbo)
				{
					auto pUnitType = pUnit->GetTechnoType();
					if (pUnitType && pUnit != selectedTarget && pUnit->Target != selectedTarget)
					{
						pUnit->CurrentTargets.Clear();
						if (pUnitType->Underwater && pUnitType->LandTargeting == LandTargetingType::Land_Not_OK
							&& selectedTarget->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
						{
							// Naval units like Submarines are unable to target ground targets
							// except if they have anti-ground weapons. Ignore the attack
							pUnit->CurrentTargets.Clear();
							pUnit->SetTarget(nullptr);
							pUnit->SetFocus(nullptr);
							pUnit->SetDestination(nullptr, false);
							pUnit->QueueMission(Mission::Area_Guard, true);

							continue;
						}

						// Aircraft hack. I hate how this game auto-manages the aircraft missions.
						if (pUnitType->WhatAmI() == AbstractType::AircraftType
							&& pUnit->Ammo > 0 && pUnit->GetHeight() <= 0)
						{
							pUnit->SetDestination(selectedTarget, false);
							pUnit->QueueMission(Mission::Attack, true);
						}

						pUnit->SetTarget(selectedTarget);

						if (pUnit->IsEngineer())
						{
							pUnit->QueueMission(Mission::Capture, true);
						}
						else
						{
							// Aircraft hack. I hate how this game auto-manages the aircraft missions.
							if (pUnitType->WhatAmI() != AbstractType::AircraftType)
							{
								pUnit->QueueMission(Mission::Attack, true);
								pUnit->ObjectClickedAction(Action::Attack, selectedTarget, false);

								if (pUnit->GetCurrentMission() != Mission::Attack)
									pUnit->Mission_Attack();

								if (pUnit->GetCurrentMission() == Mission::Move && pUnitType->JumpJet)
									pUnit->Mission_Attack();
							}
						}

						// Spy case
						if (pUnitType->WhatAmI() == AbstractType::InfantryType)
						{
							auto pInfantryType = abstract_cast<InfantryTypeClass*>(pUnitType);

							if (pInfantryType && pInfantryType->Infiltrate && pInfantryType->Agent)
							{
								// Check if target is an structure and see if spiable
								if (pUnit->GetCurrentMission() != Mission::Enter)
									pUnit->Mission_Enter();
							}
						}

						// Tanya / Commando C4 case
						if ((pUnitType->WhatAmI() == AbstractType::InfantryType && (abstract_cast<InfantryTypeClass*>(pUnitType)->C4 || pUnit->HasAbility(Ability::C4))) && pUnit->GetCurrentMission() != Mission::Sabotage)
						{
							pUnit->Mission_Attack();
							pUnit->QueueMission(Mission::Sabotage, true);
						}
					}
					else
					{
						pUnit->QueueMission(Mission::Attack, true);
						pUnit->ObjectClickedAction(Action::Attack, selectedTarget, false);
						pUnit->Mission_Attack();
					}
				}
			}
		}
		else
		{
			// No target was found with the specific criteria.
			if (!noWaitLoop && pTeamData->WaitNoTargetTimer.Completed())
			{
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);
			}

			if (pTeamData->IdxSelectedObjectFromAIList >= 0)
				pTeamData->IdxSelectedObjectFromAIList = -1;

			if (pTeamData->WaitNoTargetAttempts != 0 && pTeamData->WaitNoTargetTimer.Completed())
			{
				// No target? let's wait some frames
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);

				return;
			}

			// This action finished
			pTeam->StepCompleted = true;
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (Leader [%s] (UID: %lu) can't find a new target)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument, pLeaderUnit->GetTechnoType()->get_ID(), pLeaderUnit->UniqueID);

			return;
		}
	}
	else
	{
		// This part of the code is used for updating the "Attack" mission in each team unit

		if (pFocus
			&& pFocus->IsAlive
			&& pFocus->Health > 0
			&& !pFocus->InLimbo
			&& !pFocus->GetTechnoType()->Immune
			&& ((pFocus->IsInAir() && leaderWeaponsHaveAA)
				|| (!pFocus->IsInAir() && leaderWeaponsHaveAG))
			&& pFocus->IsOnMap
			&& !pFocus->Absorbed
			&& pFocus->Owner != pLeaderUnit->Owner
			&& (!pLeaderUnit->Owner->IsAlliedWith(pFocus)
				|| (pLeaderUnit->Owner->IsAlliedWith(pFocus)
					&& pFocus->IsMindControlled()
					&& !pLeaderUnit->Owner->IsAlliedWith(pFocus->MindControlledBy))))
		{

			bool bForceNextAction = false;

			for (auto pUnit = pTeam->FirstUnit; pUnit && !bForceNextAction; pUnit = pUnit->NextTeamMember)
			{
				auto pUnitType = pUnit->GetTechnoType();

				if (pUnitType
					&& pUnit->IsAlive
					&& pUnit->Health > 0
					&& !pUnit->InLimbo)
				{
					// Aircraft case 1
					if ((pUnitType->WhatAmI() == AbstractType::AircraftType
						&& abstract_cast<AircraftTypeClass*>(pUnitType)->AirportBound)
						&& pUnit->Ammo > 0
						&& (pUnit->Target != pFocus && !pUnit->InAir))
					{
						pUnit->SetTarget(pFocus);
						continue;
					}

					// Naval units like Submarines are unable to target ground targets except if they have nti-ground weapons. Ignore the attack
					if (pUnitType->Underwater
						&& pUnitType->LandTargeting == LandTargetingType::Land_Not_OK
						&& pFocus->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
					{
						pUnit->CurrentTargets.Clear();
						pUnit->SetTarget(nullptr);
						pUnit->SetFocus(nullptr);
						pUnit->SetDestination(nullptr, false);
						pUnit->QueueMission(Mission::Area_Guard, true);
						bForceNextAction = true;

						continue;
					}

					// Aircraft case 2
					if (pUnitType->WhatAmI() == AbstractType::AircraftType
						&& pUnit->GetCurrentMission() != Mission::Attack
						&& pUnit->GetCurrentMission() != Mission::Enter)
					{
						if (pUnit->InAir)
						{
							if (pUnit->Ammo > 0)
							{
								pUnit->QueueMission(Mission::Attack, true);

								if (pFocus)
									pUnit->ObjectClickedAction(Action::Attack, pFocus, false);

								pUnit->Mission_Attack();
							}
							else
							{
								pUnit->ForceMission(Mission::Enter);
								pUnit->Mission_Enter();
								pUnit->SetFocus(pUnit);
								pUnit->LastTarget = nullptr;
								pUnit->SetTarget(pUnit);
							}
						}
						else
						{
							if (pUnit->Ammo > 0)
							{
								pUnit->QueueMission(Mission::Attack, true);

								if (pFocus)
									pUnit->ObjectClickedAction(Action::Attack, pFocus, false);

								pUnit->Mission_Attack();
							}
							else
							{
								pUnit->ForceMission(Mission::Enter);
								pUnit->Mission_Enter();
								pUnit->SetFocus(pUnit);
								pUnit->LastTarget = nullptr;
								pUnit->SetTarget(pUnit);
							}
						}

						continue;
					}

					// Tanya / Commando C4 case
					if ((pUnitType->WhatAmI() == AbstractType::InfantryType
						&& abstract_cast<InfantryTypeClass*>(pUnitType)->C4
						|| pUnit->HasAbility(Ability::C4)) && pUnit->GetCurrentMission() != Mission::Sabotage)
					{
						pUnit->Mission_Attack();
						pUnit->QueueMission(Mission::Sabotage, true);

						continue;
					}

					// Other cases
					if (pUnitType->WhatAmI() != AbstractType::AircraftType)
					{
						if (pUnit->Target != pFocus)
							pUnit->SetTarget(pFocus);

						if (pUnit->GetCurrentMission() != Mission::Attack
							&& pUnit->GetCurrentMission() != Mission::Unload
							&& pUnit->GetCurrentMission() != Mission::Selling)
						{
							pUnit->QueueMission(Mission::Attack, false);
						}

						continue;
					}
				}
			}

			if (bForceNextAction)
			{
				pTeamData->IdxSelectedObjectFromAIList = -1;
				pTeam->StepCompleted = true;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to NEXT line: %d = %d,%d (Naval is unable to target ground)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

				return;
			}
		}
		else
		{
			pTeam->Focus = nullptr;
		}
	}
}

TechnoClass* ScriptExt::GreatestThreat(TechnoClass *pTechno, int method, int calcThreatMode = 0, HouseClass* onlyTargetThisHouseEnemy = nullptr, int attackAITargetType = -1, int idxAITargetTypeItem = -1, bool agentMode = false)
{
	TechnoClass *bestObject = nullptr;
	double bestVal = -1;
	bool unitWeaponsHaveAA = false;
	bool unitWeaponsHaveAG = false;

	// Generic method for targeting
	for (int i = 0; i < TechnoClass::Array->Count; i++)
	{
		auto object = TechnoClass::Array->GetItem(i);
		auto objectType = object->GetTechnoType();
		auto pTechnoType = pTechno->GetTechnoType();

		if (!object || !objectType || !pTechnoType)
			continue;

		// Note: the TEAM LEADER is picked for this task, be careful with leadership values in your mod
		int weaponIndex = pTechno->SelectWeapon(object);
		auto weaponType = pTechno->GetWeapon(weaponIndex)->WeaponType;

		if (weaponType && weaponType->Projectile->AA)
			unitWeaponsHaveAA = true;

		if ((weaponType && weaponType->Projectile->AG) || agentMode)
			unitWeaponsHaveAG = true;

		/*
		int weaponDamage = 0;

		if (weaponType)
		{
			if (weaponType->AmbientDamage > 0)
				weaponDamage = MapClass::GetTotalDamage(weaponType->AmbientDamage, weaponType->Warhead, objectType->Armor, 0) + MapClass::GetTotalDamage(weaponType->Damage, weaponType->Warhead, objectType->Armor, 0);
			else
				weaponDamage = MapClass::GetTotalDamage(weaponType->Damage, weaponType->Warhead, objectType->Armor, 0);
		}

		// If the target can't be damaged then isn't a valid target
		if (weaponType && weaponDamage <= 0 && !agentMode)
			continue;
		*/

		if (!agentMode)
		{
			if (weaponType && GeneralUtils::GetWarheadVersusArmor(weaponType->Warhead, objectType->Armor) == 0.0)
				continue;

			if (object->IsInAir() && !unitWeaponsHaveAA)
				continue;

			if (!object->IsInAir() && !unitWeaponsHaveAG)
				continue;
		}

		// Don't pick underground units
		if (object->InWhichLayer() == Layer::Underground)
			continue;

		// Stealth ground unit check
		if (object->CloakState == CloakState::Cloaked && !objectType->Naval)
			continue;

		// Submarines aren't a valid target
		if (object->CloakState == CloakState::Cloaked
			&& objectType->Underwater
			&& (pTechnoType->NavalTargeting == NavalTargetingType::Underwater_Never || pTechnoType->NavalTargeting == NavalTargetingType::Naval_None))
		{
			continue;
		}

		// Land not OK for the Naval unit
		if (objectType->Naval
			&& pTechnoType->LandTargeting == LandTargetingType::Land_Not_OK
			&& object->GetCell()->LandType != LandType::Water)
		{
			continue;
		}

		// OnlyTargetHouseEnemy forces targets of a specific (hated) house
		if (onlyTargetThisHouseEnemy && object->Owner != onlyTargetThisHouseEnemy)
			continue;

		if (object != pTechno
			&& object->IsAlive
			&& object->Health > 0
			&& !object->InLimbo
			&& !objectType->Immune
			&& !object->Transporter
			&& object->IsOnMap
			&& !object->Absorbed
			&& !object->TemporalTargetingMe
			&& !object->BeingWarpedOut
			&& object->Owner != pTechno->Owner
			&& (!pTechno->Owner->IsAlliedWith(object)
				|| (pTechno->Owner->IsAlliedWith(object)
					&& object->IsMindControlled()
					&& !pTechno->Owner->IsAlliedWith(object->MindControlledBy))))
		{
			double value = 0;

			if (EvaluateObjectWithMask(object, method, attackAITargetType, idxAITargetTypeItem, pTechno))
			{
				CellStruct newCell;
				newCell.X = (short)object->Location.X;
				newCell.Y = (short)object->Location.Y;

				bool isGoodTarget = false;
				if (calcThreatMode == 0 || calcThreatMode == 1)
				{
					// Threat affected by distance
					double threatMultiplier = 128.0;
					double objectThreatValue = objectType->ThreatPosed;

					if (objectType->SpecialThreatValue > 0)
					{
						double const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
						objectThreatValue += objectType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
					}

					// Is Defender house targeting Attacker House? if "yes" then more Threat
					if (pTechno->Owner == HouseClass::Array->GetItem(object->Owner->EnemyHouseIndex))
					{
						double const& EnemyHouseThreatBonus = RulesClass::Instance->EnemyHouseThreatBonus;
						objectThreatValue += EnemyHouseThreatBonus;
					}

					// Extra threat based on current health. More damaged == More threat (almost destroyed objects gets more priority)
					objectThreatValue += object->Health * (1 - object->GetHealthPercentage());
					value = (objectThreatValue * threatMultiplier) / ((pTechno->DistanceFrom(object) / 256.0) + 1.0);

					if (calcThreatMode == 0)
					{
						// Is this object very FAR? then LESS THREAT against pTechno.
						// More CLOSER? MORE THREAT for pTechno.
						if (value > bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						// Is this object very FAR? then MORE THREAT against pTechno.
						// More CLOSER? LESS THREAT for pTechno.
						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
				}
				else
				{
					// Selection affected by distance
					if (calcThreatMode == 2)
					{
						// Is this object very FAR? then LESS THREAT against pTechno.
						// More CLOSER? MORE THREAT for pTechno.
						value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						if (calcThreatMode == 3)
						{
							// Is this object very FAR? then MORE THREAT against pTechno.
							// More CLOSER? LESS THREAT for pTechno.
							value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

							if (value > bestVal || bestVal < 0)
								isGoodTarget = true;
						}
					}
				}

				if (isGoodTarget)
				{
					bestObject = object;
					bestVal = value;
				}
			}
		}
	}

	return bestObject;
}

// TODO: Too many redundant abstract_cast here
bool ScriptExt::EvaluateObjectWithMask(TechnoClass *pTechno, int mask, int attackAITargetType = -1, int idxAITargetTypeItem = -1, TechnoClass *pTeamLeader = nullptr)
{
	if (!pTechno)
		return false;

	WeaponTypeClass* WeaponType1 = nullptr;
	WeaponTypeClass* WeaponType2 = nullptr;
	WeaponTypeClass* WeaponType3 = nullptr;
	BuildingClass* pBuilding = nullptr;
	BuildingTypeClass* pTypeBuilding = nullptr;
	TechnoTypeExt::ExtData* pTypeTechnoExt = nullptr;
	BuildingTypeExt::ExtData* pBuildingExt = nullptr;
	TechnoTypeClass* pTechnoType = pTechno->GetTechnoType();
	auto const& BuildTech = RulesClass::Instance->BuildTech;
	auto const& BaseUnit = RulesClass::Instance->BaseUnit;
	auto const& NeutralTechBuildings = RulesClass::Instance->NeutralTechBuildings;
	int nSuperWeapons = 0;
	double distanceToTarget = 0;
	TechnoClass* pTarget = nullptr;

	// Special case: validate target if is part of a technos list in [AITargetTypes] section
	if (attackAITargetType >= 0 && RulesExt::Global()->AITargetTypesLists.Count > 0)
	{
		DynamicVectorClass<TechnoTypeClass*> objectsList = RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType);

		for (int i = 0; i < objectsList.Count; i++)
		{
			if (objectsList.GetItem(i) == pTechnoType)
				return true;
		}

		return false;
	}

	switch (mask)
	{
	case 1:
		// Anything ;-)
		if (!pTechno->Owner->IsNeutral())
			return true;

		break;

	case 2:
		// Building
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::BuildingType
				|| (pTypeBuilding
					&& !(pTypeBuilding->Artillary
						|| pTypeBuilding->TickTank
						|| pTypeBuilding->ICBMLauncher
						|| pTypeBuilding->SensorArray))))
		{
			return true;
		}

		break;

	case 3:
		// Harvester
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechno->GetTechnoType());

		if (!pTechno->Owner->IsNeutral()
			&& ((pTechnoType->WhatAmI() == AbstractType::UnitType
				&& (abstract_cast<UnitTypeClass*>(pTechnoType)->Harvester
					|| abstract_cast<UnitTypeClass*>(pTechnoType)->ResourceGatherer))
				|| (pTypeBuilding
					&& pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& pTypeBuilding->ResourceGatherer)))
		{
			return true;
		}

		break;

	case 4:
		// Infantry
		if (!pTechno->Owner->IsNeutral() && pTechnoType->WhatAmI() == AbstractType::InfantryType)
			return true;

		break;

	case 5:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Vehicle, Aircraft, Deployed vehicle into structure
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::UnitType
				|| (pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& (pTypeBuilding
						&& (pTypeBuilding->Artillary
							|| pTypeBuilding->TickTank
							|| pTypeBuilding->ICBMLauncher
							|| pTypeBuilding->SensorArray)))
				|| (pTechnoType->WhatAmI() == AbstractType::AircraftType)))
		{
			return true;
		}

		break;

	case 6:
		// Factory
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& abstract_cast<BuildingClass*>(pTechno)->Factory != nullptr)
		{
			return true;
		}

		break;

	case 7:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Defense
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->IsBaseDefense)
		{
			return true;
		}

		break;

	case 8:
		pTarget = abstract_cast<TechnoClass*>(pTechno->Target);

		if (pTeamLeader && pTarget)
		{
			// The possible Target is aiming against me? Revenge!
			if (abstract_cast<TechnoClass*>(pTechno->Target)->Owner == pTeamLeader->Owner)
				return true;

			for (int i = 0; i < pTechno->CurrentTargets.Count; i++)
			{
				if (abstract_cast<TechnoClass*>(pTechno->CurrentTargets.GetItem(i))->Owner == pTeamLeader->Owner)
					return true;
			}

			// Note: Replace these lines when I have access to Combat_Damage() method in YRpp if that is better
			WeaponType1 = pTechno->Veterancy.IsElite() ?
				pTechnoType->EliteWeapon[0].WeaponType :
				pTechnoType->Weapon[0].WeaponType;

			WeaponType2 = pTechno->Veterancy.IsElite() ?
				pTechnoType->EliteWeapon[1].WeaponType :
				pTechnoType->Weapon[1].WeaponType;

			WeaponType3 = WeaponType1;

			if (pTechnoType->IsGattling)
			{
				WeaponType3 = pTechno->Veterancy.IsElite() ?
					pTechnoType->EliteWeapon[pTechno->CurrentWeaponNumber].WeaponType :
					pTechnoType->Weapon[pTechno->CurrentWeaponNumber].WeaponType;

				WeaponType1 = WeaponType3;
			}

			// Then check if this possible target is too near of the Team Leader
			distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / 256.0;

			if (!pTechno->Owner->IsNeutral()
				&& ((WeaponType1 && distanceToTarget <= (WeaponType1->Range / 256.0 * 4.0))
					|| (WeaponType2 && distanceToTarget <= (WeaponType2->Range / 256.0 * 4.0))
					|| (pTeamLeader->GetTechnoType()->GuardRange > 0
						&& distanceToTarget <= (pTeamLeader->GetTechnoType()->GuardRange / 256.0 * 2.0))))
			{
				return true;
			}
		}

		break;

	case 9:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Power Plant
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->PowerBonus > 0)
		{
			return true;
		}

		break;

	case 10:
		// Occupied Building
		if (pTechnoType->WhatAmI() == AbstractType::BuildingType)
		{
			pBuilding = abstract_cast<BuildingClass*>(pTechno);

			if (pBuilding && pBuilding->Occupants.Count > 0)
				return true;
		}

		break;

	case 11:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Civilian Tech
		if (pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& NeutralTechBuildings.Items)
		{
			for (int i = 0; i < NeutralTechBuildings.Count; i++)
			{
				auto pTechObject = NeutralTechBuildings.GetItem(i);
				if (_stricmp(pTechObject->ID, pTechno->get_ID()) == 0)
					return true;
			}
		}

		// Other cases of civilian Tech Structures
		if (pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Unsellable
			&& pTypeBuilding->Capturable
			&& pTypeBuilding->TechLevel < 0
			&& pTypeBuilding->NeedsEngineer
			&& !pTypeBuilding->BridgeRepairHut)
		{
			return true;
		}

		break;

	case 12:
		// Refinery
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& ((pTechnoType->WhatAmI() == AbstractType::UnitType
				&& !abstract_cast<UnitTypeClass*>(pTechnoType)->Harvester
				&& abstract_cast<UnitTypeClass*>(pTechnoType)->ResourceGatherer)
				|| (pTypeBuilding
					&& pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& (pTypeBuilding->Refinery
						|| pTypeBuilding->ResourceGatherer))))
		{
			return true;
		}

		break;

	case 13:
		// Mind Controller
		WeaponType1 = pTechno->Veterancy.IsElite() ?
			pTechnoType->EliteWeapon[0].WeaponType :
			pTechnoType->Weapon[0].WeaponType;

		WeaponType2 = pTechno->Veterancy.IsElite() ?
			pTechnoType->EliteWeapon[1].WeaponType :
			pTechnoType->Weapon[1].WeaponType;

		WeaponType3 = WeaponType1;

		if (pTechnoType->IsGattling)
		{
			WeaponType3 = pTechno->Veterancy.IsElite() ?
				pTechnoType->EliteWeapon[pTechno->CurrentWeaponNumber].WeaponType :
				pTechnoType->Weapon[pTechno->CurrentWeaponNumber].WeaponType;

			WeaponType1 = WeaponType3;
		}

		if (!pTechno->Owner->IsNeutral()
			&& ((WeaponType1 && WeaponType1->Warhead->MindControl)
				|| (WeaponType2 && WeaponType2->Warhead->MindControl)))
		{
			return true;
		}

		break;

	case 14:
		// Aircraft and Air Unit
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::AircraftType
				|| pTechnoType->JumpJet
				|| pTechno->IsInAir()))
		{
			return true;
		}

		break;

	case 15:
		// Naval Unit & Structure
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->Naval
				|| pTechno->GetCell()->LandType == LandType::Water))
		{
			return true;
		}

		break;

	case 16:
		// Cloak Generator, Gap Generator, Radar Jammer or Inhibitor
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);
		pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& ((pTypeTechnoExt
				&& (pTypeTechnoExt->RadarJamRadius > 0
					|| pTypeTechnoExt->InhibitorRange.isset()))
				|| (pTypeBuilding && (pTypeBuilding->GapGenerator
					|| pTypeBuilding->CloakGenerator))))
		{
			return true;
		}

		break;

	case 17:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Ground Vehicle
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::UnitType
				|| (pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& pTypeBuilding->UndeploysInto
					&& !pTypeBuilding->BaseNormal)
				&& !pTechno->IsInAir()
				&& !pTechnoType->Naval))
		{
			return true;
		}

		break;

	case 18:
		// Economy: Harvester, Refinery or Resource helper
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& ((pTechnoType->WhatAmI() == AbstractType::UnitType
				&& (abstract_cast<UnitTypeClass*>(pTechnoType)->Harvester
					|| abstract_cast<UnitTypeClass*>(pTechnoType)->ResourceGatherer))
				|| (pTypeBuilding
					&& pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& (pTypeBuilding->Refinery
						|| pTypeBuilding->OrePurifier
						|| pTypeBuilding->ResourceGatherer))))
		{
			return true;
		}

		break;

	case 19:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Infantry Factory
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Factory == AbstractType::InfantryType)
		{
			return true;
		}

		break;

	case 20:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Land Vehicle Factory
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Factory == AbstractType::UnitType
			&& !pTypeBuilding->Naval)
		{
			return true;
		}

		break;

	case 21:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// is Aircraft Factory
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::BuildingType
				&& (pTypeBuilding->Factory == AbstractType::AircraftType
					|| pTypeBuilding->Helipad)))
		{
			return true;
		}

		break;

	case 22:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);
		// Radar & SpySat
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::BuildingType
				&& (pTypeBuilding->Radar
					|| pTypeBuilding->SpySat)))
		{
			return true;
		}

		break;

	case 23:
		// Buildable Tech
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& BuildTech.Items)
		{
			for (int i = 0; i < BuildTech.Count; i++)
			{
				auto pTechObject = BuildTech.GetItem(i);
				if (_stricmp(pTechObject->ID, pTechno->get_ID()) == 0)
					return true;
			}
		}

		break;

	case 24:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Naval Factory
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Factory == AbstractType::UnitType
			&& pTypeBuilding->Naval)
		{
			return true;
		}

		break;

	case 25:
		// Super Weapon building
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);
		pBuildingExt = BuildingTypeExt::ExtMap.Find(static_cast<BuildingTypeClass*>(pTypeBuilding));

		if (pBuildingExt)
			nSuperWeapons = pBuildingExt->SuperWeapons.Count;

		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& (pTypeBuilding->SuperWeapon >= 0
				|| pTypeBuilding->SuperWeapon2 >= 0
				|| nSuperWeapons > 0))
		{
			return true;
		}

		break;

	case 26:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Construction Yard
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Factory == AbstractType::BuildingType
			&& pTypeBuilding->ConstructionYard)
		{
			return true;
		}
		else
		{
			if (pTechnoType->WhatAmI() == AbstractType::UnitType && BaseUnit.Items)
			{
				for (int i = 0; i < BaseUnit.Count; i++)
				{
					auto pMCVObject = BaseUnit.GetItem(i);
					if (_stricmp(pMCVObject->ID, pTechno->get_ID()) == 0)
						return true;
				}
			}
		}

		break;

	case 27:
		// Any Neutral object
		if (pTechno->Owner->IsNeutral())
			return true;

		break;

	case 28:
		// Cloak Generator & Gap Generator
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& (pTypeBuilding && (pTypeBuilding->GapGenerator
				|| pTypeBuilding->CloakGenerator)))
		{
			return true;
		}

		break;

	case 29:
		// Radar Jammer
		pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& (pTypeTechnoExt
				&& (pTypeTechnoExt->RadarJamRadius > 0)))
			return true;

		break;

	case 30:
		// Inhibitor
		pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& (pTypeTechnoExt
				&& pTypeTechnoExt->InhibitorRange.isset()))
		{
			return true;
		}

		break;

	case 31:
		// Naval Unit
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() != AbstractType::BuildingType
			&& (pTechnoType->Naval
				|| pTechno->GetCell()->LandType == LandType::Water))
		{
			return true;
		}

		break;

	case 32:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Any non-building unit
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() != AbstractType::BuildingType
				|| (pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& pTypeBuilding
					&& (pTypeBuilding->Artillary
						|| pTypeBuilding->TickTank
						|| pTypeBuilding->ICBMLauncher
						|| pTypeBuilding->SensorArray
						|| pTypeBuilding->ResourceGatherer))))
		{
			return true;
		}

		break;

	case 33:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Capturable Structure or Repair Hut
		if (pTypeBuilding
			&& (pTypeBuilding->Capturable
				|| (pTypeBuilding->BridgeRepairHut
					&& pTypeBuilding->Repairable)))
		{
			return true;
		}

		break;

	case 34:
		if (pTeamLeader)
		{
			// Inside the Area Guard of the Team Leader
			distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / 256.0; // Caution, DistanceFrom() return leptons

			if (!pTechno->Owner->IsNeutral()
				&& (pTeamLeader->GetTechnoType()->GuardRange > 0
					&& distanceToTarget <= ((pTeamLeader->GetTechnoType()->GuardRange / 256.0) * 2.0)))
			{
				return true;
			}
		}

		break;

	case 35:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Land Vehicle Factory & Naval Factory
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Factory == AbstractType::UnitType)
		{
			return true;
		}

		break;

	case 36:
		// Building that isn't a defense
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& pTypeBuilding
			&& !pTypeBuilding->IsBaseDefense
			&& !(pTypeBuilding->Artillary
				|| pTypeBuilding->TickTank
				|| pTypeBuilding->ICBMLauncher
				|| pTypeBuilding->SensorArray))
		{
			return true;
		}

		break;

	default:
		break;
	}

	// The possible target doesn't fit in te masks
	return false;
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

void ScriptExt::WaitIfNoTarget(TeamClass *pTeam, int attempts = 0)
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

void ScriptExt::TeamWeightReward(TeamClass *pTeam, double award = 0)
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
		if (idxScriptsList < RulesExt::Global()->AIScriptsLists.Count)
		{
			DynamicVectorClass<ScriptTypeClass*> objectsList = RulesExt::Global()->AIScriptsLists.GetItem(idxScriptsList);

			if (objectsList.Count > 0)
			{
				int IdxSelectedObject = ScenarioClass::Instance->Random.RandomRanged(0, objectsList.Count - 1);

				ScriptTypeClass* pNewScript = objectsList.GetItem(IdxSelectedObject);
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
					Debug::Log("DEBUG: [%s] Aborting Script change because [%s] has 0 Action scripts!\n", pTeam->Type->ID, pNewScript->ID);

					return;
				}
			}
		}
	}

	// This action finished
	if (changeFailed)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] Failed to change the Team Script with a random one!\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID);
	}
}

void ScriptExt::Mission_Attack_List(TeamClass *pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
		pTeamData->IdxSelectedObjectFromAIList = -1;

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (RulesExt::Global()->AITargetTypesLists.Count > 0
		&& RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType).Count > 0)
	{
		ScriptExt::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, -1);
	}
}

void ScriptExt::Mission_Move(TeamClass* pTeam, int calcThreatMode = 0, bool pickAllies = false, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	int scriptArgument = pScript->Type->ScriptActions[pScript->CurrentMission].Argument; // This is the target type
	TechnoClass* selectedTarget = nullptr;
	bool noWaitLoop = false;
	FootClass* pLeaderUnit = nullptr;
	TechnoTypeClass* pLeaderUnitType = nullptr;
	bool bAircraftsWithoutAmmo = false;
	TechnoClass* pFocus = nullptr;
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pScript)
		return;

	if (!pTeamData)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: ExtData found)\n", pTeam->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->Type->ID, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	// When the new target wasn't found it sleeps some few frames before the new attempt. This can save cycles and cycles of unnecessary executed lines.
	if (pTeamData->WaitNoTargetCounter > 0)
	{
		if (pTeamData->WaitNoTargetTimer.InProgress())
			return;

		pTeamData->WaitNoTargetTimer.Stop();
		noWaitLoop = true;
		pTeamData->WaitNoTargetCounter = 0;

		if (pTeamData->WaitNoTargetAttempts > 0)
			pTeamData->WaitNoTargetAttempts--;
	}

	// This team has no units!
	if (!pTeam)
	{
		if (pTeamData->CloseEnough > 0)
			pTeamData->CloseEnough = -1;

		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No team members alive)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit && pUnit->IsAlive && !pUnit->InLimbo)
		{
			auto pUnitType = pUnit->GetTechnoType();
			if (pUnitType)
			{
				if (pUnitType->WhatAmI() == AbstractType::AircraftType
					&& !pUnit->IsInAir()
					&& abstract_cast<AircraftTypeClass*>(pUnitType)->AirportBound
					&& pUnit->Ammo < pUnitType->Ammo)
				{
					bAircraftsWithoutAmmo = true;
					pUnit->CurrentTargets.Clear();
				}
			}
		}
	}

	// Find the Leader
	pLeaderUnit = pTeamData->TeamLeader;
	if (!pLeaderUnit
		|| !pLeaderUnit->IsAlive
		|| pLeaderUnit->Health <= 0
		|| pLeaderUnit->InLimbo
		|| !(pLeaderUnit->IsOnMap || (pLeaderUnit->GetTechnoType()->IsSubterranean))
		|| pLeaderUnit->Transporter
		|| pLeaderUnit->Absorbed)
	{
		pLeaderUnit = FindTheTeamLeader(pTeam);
		pTeamData->TeamLeader = pLeaderUnit;
	}

	if (!pLeaderUnit || bAircraftsWithoutAmmo)
	{
		pTeamData->IdxSelectedObjectFromAIList = -1;

		if (pTeamData->CloseEnough > 0)
			pTeamData->CloseEnough = -1;

		if (pTeamData->WaitNoTargetAttempts != 0)
		{
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0;
			pTeamData->WaitNoTargetAttempts = 0;
		}

		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reasons: No Leader | Aircrafts without ammo)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	pLeaderUnitType = pLeaderUnit->GetTechnoType();
	pFocus = abstract_cast<TechnoClass*>(pTeam->Focus);

	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		// This part of the code is used for picking a new target.
		int targetMask = scriptArgument;
		selectedTarget = FindBestObject(pLeaderUnit, targetMask, calcThreatMode, pickAllies, attackAITargetType, idxAITargetTypeItem);

		if (selectedTarget)
		{
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Leader [%s] (UID: %lu) selected [%s] (UID: %lu) as destination target.\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pLeaderUnit->GetTechnoType()->get_ID(), pLeaderUnit->UniqueID, selectedTarget->GetTechnoType()->get_ID(), selectedTarget->UniqueID);

			pTeam->Focus = selectedTarget;
			pTeamData->WaitNoTargetAttempts = 0; // Disable Script Waits if there are any because a new target was selected
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0; // Disable Script Waits if there are any because a new target was selected

			for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			{
				auto pUnitType = pUnit ? pUnit->GetTechnoType() : nullptr;
				if (!pUnitType)
					continue;

				if (pUnit->IsAlive
					&& (pUnit->IsOnMap || (pUnitType->IsSubterranean))
					&& !pUnit->InLimbo
					&& !pUnit->Transporter)
				{
					pUnit->CurrentTargets.Clear();

					if (pUnitType->Underwater && pUnitType->LandTargeting == LandTargetingType::Land_Not_OK && selectedTarget->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
					{
						// Naval units like Submarines are unable to target ground targets except if they have anti-ground weapons. Ignore the attack
						pUnit->CurrentTargets.Clear();
						pUnit->SetTarget(nullptr);
						pUnit->SetFocus(nullptr);
						pUnit->SetDestination(nullptr, false);
						pUnit->QueueMission(Mission::Area_Guard, true);

						continue;
					}

					// Reset previous command
					pUnit->SetTarget(nullptr);
					pUnit->SetFocus(nullptr);
					pUnit->SetDestination(nullptr, false);
					pUnit->ForceMission(Mission::Guard);

					// Get a cell near the target
					pUnit->QueueMission(Mission::Move, false);
					CoordStruct coord = TechnoExt::PassengerKickOutLocation(selectedTarget, pUnit, 10);
					coord = coord != CoordStruct::Empty ? coord : selectedTarget->Location;
					CellClass* pCellDestination = MapClass::Instance->TryGetCellAt(coord);
					pUnit->SetDestination(pCellDestination, true);

					// Aircraft hack. I hate how this game auto-manages the aircraft missions.
					if (pUnitType->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0 && pUnit->GetHeight() <= 0)
						pUnit->QueueMission(Mission::Move, false);
				}
			}
		}
		else
		{
			// No target was found with the specific criteria.

			if (!noWaitLoop && pTeamData->WaitNoTargetTimer.Completed())
			{
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);
			}

			if (pTeamData->IdxSelectedObjectFromAIList >= 0)
				pTeamData->IdxSelectedObjectFromAIList = -1;

			if (pTeamData->WaitNoTargetAttempts != 0 && pTeamData->WaitNoTargetTimer.Completed())
			{
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30); // No target? let's wait some frames

				return;
			}

			if (pTeamData->CloseEnough >= 0)
				pTeamData->CloseEnough = -1;

			// This action finished
			pTeam->StepCompleted = true;
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (new target NOT FOUND)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

			return;
		}
	}
	else
	{
		// This part of the code is used for updating the "Move" mission in each team unit
		int moveDestinationMode = 0;
		moveDestinationMode = pTeamData->MoveMissionEndMode;
		bool bForceNextAction = ScriptExt::MoveMissionEndStatus(pTeam, pFocus, pLeaderUnit, moveDestinationMode);

		if (bForceNextAction)
		{
			pTeamData->MoveMissionEndMode = 0;
			pTeamData->IdxSelectedObjectFromAIList = -1;

			if (pTeamData->CloseEnough >= 0)
				pTeamData->CloseEnough = -1;

			// This action finished
			pTeam->StepCompleted = true;
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (Reason: Reached destination)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

			return;
		}
	}
}

TechnoClass* ScriptExt::FindBestObject(TechnoClass* pTechno, int method, int calcThreatMode = 0, bool pickAllies = false, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	TechnoClass* bestObject = nullptr;
	double bestVal = -1;
	HouseClass* enemyHouse = nullptr;

	// Favorite Enemy House case. If set, AI will focus against that House
	if (!pickAllies && pTechno->BelongsToATeam())
	{
		auto pFoot = abstract_cast<FootClass*>(pTechno);
		if (pFoot)
		{
			int enemyHouseIndex = pFoot->Team->FirstUnit->Owner->EnemyHouseIndex;

			if (pFoot->Team->Type->OnlyTargetHouseEnemy	&& enemyHouseIndex >= 0)
				enemyHouse = HouseClass::Array->GetItem(enemyHouseIndex);
		}
	}

	// Generic method for targeting
	for (int i = 0; i < TechnoClass::Array->Count; i++)
	{
		auto object = TechnoClass::Array->GetItem(i);
		auto objectType = object->GetTechnoType();
		auto pTechnoType = pTechno->GetTechnoType();

		if (!object || !objectType || !pTechnoType)
			continue;

		if (enemyHouse && enemyHouse != object->Owner)
			continue;

		// Don't pick underground units
		if (object->InWhichLayer() == Layer::Underground)
			continue;

		// Stealth ground unit check
		if (object->CloakState == CloakState::Cloaked && !objectType->Naval)
			continue;

		// Submarines aren't a valid target
		if (object->CloakState == CloakState::Cloaked
			&& objectType->Underwater
			&& (pTechnoType->NavalTargeting == NavalTargetingType::Underwater_Never
				|| pTechnoType->NavalTargeting == NavalTargetingType::Naval_None))
		{
			continue;
		}

		// Land not OK for the Naval unit
		if (objectType->Naval
			&& pTechnoType->LandTargeting == LandTargetingType::Land_Not_OK
			&& object->GetCell()->LandType != LandType::Water)
		{
			continue;
		}

		if (object != pTechno
			&& object->IsAlive
			&& !object->InLimbo
			&& object->IsOnMap
			&& !object->Transporter
			&& !object->Absorbed
			&& ((pickAllies && pTechno->Owner->IsAlliedWith(object))
				|| (!pickAllies && !pTechno->Owner->IsAlliedWith(object))))
		{
			double value = 0;

			if (EvaluateObjectWithMask(object, method, attackAITargetType, idxAITargetTypeItem, pTechno))
			{
				CellStruct newCell;
				newCell.X = (short)object->Location.X;
				newCell.Y = (short)object->Location.Y;

				bool isGoodTarget = false;

				if (calcThreatMode == 0 || calcThreatMode == 1)
				{
					// Threat affected by distance
					double threatMultiplier = 128.0;
					double objectThreatValue = objectType->ThreatPosed;

					if (objectType->SpecialThreatValue > 0)
					{
						double const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
						objectThreatValue += objectType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
					}

					// Is Defender house targeting Attacker House? if "yes" then more Threat
					if (pTechno->Owner == HouseClass::Array->GetItem(object->Owner->EnemyHouseIndex))
					{
						double const& EnemyHouseThreatBonus = RulesClass::Instance->EnemyHouseThreatBonus;
						objectThreatValue += EnemyHouseThreatBonus;
					}

					// Extra threat based on current health. More damaged == More threat (almost destroyed objects gets more priority)
					objectThreatValue += object->Health * (1 - object->GetHealthPercentage());
					value = (objectThreatValue * threatMultiplier) / ((pTechno->DistanceFrom(object) / 256.0) + 1.0);

					if (calcThreatMode == 0)
					{
						// Is this object very FAR? then LESS THREAT against pTechno.
						// More CLOSER? MORE THREAT for pTechno.
						if (value > bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						// Is this object very FAR? then MORE THREAT against pTechno.
						// More CLOSER? LESS THREAT for pTechno.
						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
				}
				else
				{
					// Selection affected by distance
					if (calcThreatMode == 2)
					{
						// Is this object very FAR? then LESS THREAT against pTechno.
						// More CLOSER? MORE THREAT for pTechno.
						value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						if (calcThreatMode == 3)
						{
							// Is this object very FAR? then MORE THREAT against pTechno.
							// More CLOSER? LESS THREAT for pTechno.
							value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

							if (value > bestVal || bestVal < 0)
								isGoodTarget = true;
						}
					}
				}

				if (isGoodTarget)
				{
					bestObject = object;
					bestVal = value;
				}
			}
		}
	}

	return bestObject;
}

void ScriptExt::Mission_Attack_List1Random(TeamClass *pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType)
{
	auto pScript = pTeam->CurrentScript;
	bool selected = false;
	int idxSelectedObject = -1;
	DynamicVectorClass<int> validIndexes;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData && pTeamData->IdxSelectedObjectFromAIList >= 0)
	{
		idxSelectedObject = pTeamData->IdxSelectedObjectFromAIList;
		selected = true;
	}

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (attackAITargetType >= 0
		&& attackAITargetType < RulesExt::Global()->AITargetTypesLists.Count)
	{
		DynamicVectorClass<TechnoTypeClass*> objectsList = RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType);

		if (idxSelectedObject < 0 && objectsList.Count > 0 && !selected)
		{
			// Finding the objects from the list that actually exists in the map
			for (int i = 0; i < TechnoClass::Array->Count; i++)
			{
				auto pTechno = TechnoClass::Array->GetItem(i);
				auto pTechnoType = TechnoClass::Array->GetItem(i)->GetTechnoType();
				bool found = false;

				for (int j = 0; j < objectsList.Count && !found; j++)
				{
					auto objectFromList = objectsList.GetItem(j);

					if (pTechnoType == objectFromList
						&& pTechno->IsAlive
						&& !pTechno->InLimbo
						&& pTechno->IsOnMap
						&& !pTechno->Absorbed
						&& (!pTeam->FirstUnit->Owner->IsAlliedWith(pTechno)
							|| (pTeam->FirstUnit->Owner->IsAlliedWith(pTechno)
								&& pTechno->IsMindControlled()
								&& !pTeam->FirstUnit->Owner->IsAlliedWith(pTechno->MindControlledBy))))
					{
						validIndexes.AddItem(j);
						found = true;
					}
				}
			}

			if (validIndexes.Count > 0)
			{
				idxSelectedObject = validIndexes.GetItem(ScenarioClass::Instance->Random.RandomRanged(0, validIndexes.Count - 1));
				selected = true;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Picked a random Techno from the list index [AITargetTypes][%d][%d] = %s\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, attackAITargetType, idxSelectedObject, objectsList.GetItem(idxSelectedObject)->ID);
			}
		}

		if (selected)
			pTeamData->IdxSelectedObjectFromAIList = idxSelectedObject;

		Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, idxSelectedObject);
	}

	// This action finished
	if (!selected)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Failed to pick a random Techno from the list index [AITargetTypes][%d]! Valid Technos in the list: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, attackAITargetType, validIndexes.Count);
	}
}

void ScriptExt::Mission_Move_List(TeamClass *pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
		pTeamData->IdxSelectedObjectFromAIList = -1;

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (RulesExt::Global()->AITargetTypesLists.Count > 0
		&& RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType).Count > 0)
	{
		Mission_Move(pTeam, calcThreatMode, pickAllies, attackAITargetType, -1);
	}
}

void ScriptExt::Mission_Move_List1Random(TeamClass *pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	bool selected = false;
	int idxSelectedObject = -1;
	DynamicVectorClass<int> validIndexes;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData && pTeamData->IdxSelectedObjectFromAIList >= 0)
	{
		idxSelectedObject = pTeamData->IdxSelectedObjectFromAIList;
		selected = true;
	}

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (attackAITargetType >= 0
		&& attackAITargetType < RulesExt::Global()->AITargetTypesLists.Count)
	{
		DynamicVectorClass<TechnoTypeClass*> objectsList = RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType);

		// Still no random target selected
		if (idxSelectedObject < 0 && objectsList.Count > 0 && !selected)
		{
			// Finding the objects from the list that actually exists in the map
			for (int i = 0; i < TechnoClass::Array->Count; i++)
			{
				auto pTechno = TechnoClass::Array->GetItem(i);
				auto pTechnoType = TechnoClass::Array->GetItem(i)->GetTechnoType();
				bool found = false;

				for (int j = 0; j < objectsList.Count && !found; j++)
				{
					auto objectFromList = objectsList.GetItem(j);

					if (pTechnoType == objectFromList
						&& pTechno->IsAlive
						&& !pTechno->InLimbo
						&& pTechno->IsOnMap
						&& !pTechno->Absorbed
						&& ((pickAllies
							&& pTeam->FirstUnit->Owner->IsAlliedWith(pTechno))
							|| (!pickAllies
								&& !pTeam->FirstUnit->Owner->IsAlliedWith(pTechno))))
					{
						validIndexes.AddItem(j);
						found = true;
					}
				}
			}

			if (validIndexes.Count > 0)
			{
				idxSelectedObject = validIndexes.GetItem(ScenarioClass::Instance->Random.RandomRanged(0, validIndexes.Count - 1));
				selected = true;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Picked a random Techno from the list index [AITargetTypes][%d][%d] = %s\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, attackAITargetType, idxSelectedObject, objectsList.GetItem(idxSelectedObject)->ID);
			}
		}

		if (selected)
			pTeamData->IdxSelectedObjectFromAIList = idxSelectedObject;

		Mission_Move(pTeam, calcThreatMode, pickAllies, attackAITargetType, idxSelectedObject);
	}

	// This action finished
	if (!selected)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Failed to pick a random Techno from the list index [AITargetTypes][%d]! Valid Technos in the list: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, attackAITargetType, validIndexes.Count);
	}
}

void ScriptExt::SetCloseEnoughDistance(TeamClass *pTeam, double distance = -1)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (distance <= 0)
		distance = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
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

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
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

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData && pTeamData->CloseEnough > 0)
		closeEnough = pTeamData->CloseEnough;

	bool bForceNextAction = false;

	if (mode == 2)
		bForceNextAction = true;

	// Team already have a focused target
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit
			&& pUnit->IsAlive
			&& pUnit->Health > 0
			&& !pUnit->InLimbo
			&& !pUnit->TemporalTargetingMe
			&& !pUnit->BeingWarpedOut
			&& !pUnit->Transporter)
		{
			if (mode == 2)
			{
				// Default mode: all members in range
				if ((pUnit->DistanceFrom(pFocus->GetCell()) / 256.0) > closeEnough)
				{
					bForceNextAction = false;

					if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0)
						pUnit->QueueMission(Mission::Move, false);

					continue;
				}
				else
				{
					if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo <= 0)
					{
						pUnit->QueueMission(Mission::Return, false);
						pUnit->Mission_Enter();

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
						if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0)
							pUnit->QueueMission(Mission::Move, false);

						continue;
					}
					else
					{
						bForceNextAction = true;

						if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo <= 0)
						{
							pUnit->QueueMission(Mission::Return, false);
							pUnit->Mission_Enter();

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
							if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0)
								pUnit->QueueMission(Mission::Move, false);

							continue;
						}
						else
						{
							if (pUnit->IsTeamLeader)
								bForceNextAction = true;

							if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo <= 0)
							{
								pUnit->QueueMission(Mission::Return, false);
								pUnit->Mission_Enter();

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
		Debug::Log("DEBUG: [%s] [%s] (line: %d) Jump to next line: %d = %d,%d -> (No team members alive)\n",
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
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Next script line skipped successfuly. Next line will be: %d = %d,%d\n",
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
	bool teamLeaderFound = false;

	if (!pTeam)
		return pLeaderUnit;

	// Find the Leader or promote a new one
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!pUnit)
			continue;

		bool isValidUnit = pUnit->IsAlive
			&& pUnit->Health > 0
			&& !pUnit->InLimbo
			&& (pUnit->IsOnMap || (pUnit->GetTechnoType()->IsSubterranean))
			&& !pUnit->Transporter
			&& !pUnit->Absorbed;

		// Preventing >1 leaders in teams
		if (teamLeaderFound || !isValidUnit)
		{
			pUnit->IsTeamLeader = false;
			continue;
		}

		if (pUnit->IsTeamLeader)
		{
			pLeaderUnit = pUnit;
			teamLeaderFound = true;

			continue;
		}

		auto pUnitType = pUnit->GetTechnoType();
		if (!pUnitType)
			continue;

		// The team Leader will be used for selecting targets, if there are living Team Members then always exists 1 Leader.
		int unitLeadershipRating = pUnitType->LeadershipRating;
		if (unitLeadershipRating > bestUnitLeadershipValue)
		{
			pLeaderUnit = pUnit;
			bestUnitLeadershipValue = unitLeadershipRating;
		}
	}

	if (pLeaderUnit)
		pLeaderUnit->IsTeamLeader = true;

	return pLeaderUnit;
}

bool ScriptExt::IsExtVariableAction(int action)
{
	auto eAction = static_cast<PhobosScripts>(action);
	return eAction >= PhobosScripts::LocalVariableAdd && eAction <= PhobosScripts::GlobalVariableAndByGlobal;
}

void ScriptExt::Set_ForceJump_Countdown(TeamClass *pTeam, bool repeatLine = false, int count = 0)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
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
		pTeamData->ForceJump_Countdown = -1;
		pTeamData->ForceJump_RepeatMode = false;
	}

	auto pScript = pTeam->CurrentScript;

	// This action finished
	pTeam->StepCompleted = true;
	Debug::Log("DEBUG: [%s] [%s](line: %d = %d,%d) Set Timed Jump -> (Countdown: %d, repeat action: %d)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, count, repeatLine);
}

void ScriptExt::Stop_ForceJump_Countdown(TeamClass *pTeam)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
		return;

	pTeamData->ForceJump_InitialCountdown = -1;
	pTeamData->ForceJump_Countdown.Stop();
	pTeamData->ForceJump_Countdown = -1;
	pTeamData->ForceJump_RepeatMode = false;

	auto pScript = pTeam->CurrentScript;

	// This action finished
	pTeam->StepCompleted = true;
	Debug::Log("DEBUG: [%s] [%s](line: %d = %d,%d): Stopped Timed Jump\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument);
}
