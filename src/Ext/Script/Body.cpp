#include "Body.h"
#include "../Techno/Body.h"
#include "../BuildingType/Body.h"

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
		// Threats that are close have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 0, -1);
		break;
	case 75:
		// Threats that are far have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 1, -1);
		break;
	case 76:
		// Closer targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 2, -1);
		break;
	case 77:
		// Farther targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 3, -1);
		break;
	case 78:
		// Threats that are close have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 0, -1);
		break;
	case 79:
		// Threats that are far have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 1, -1);
		break;
	case 80:
		// Closer targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 2, -1);
		break;
	case 81:
		// Farther targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 3, -1);
		break;
	case 82:
		ScriptExt::DecreaseCurrentTriggerWeight(pTeam, true, 0);
		break;
	case 83:
		ScriptExt::IncreaseCurrentTriggerWeight(pTeam, true, 0);
		break;
	case 84:
		// Threats specific targets that are close have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 0, -1);
		break;
	case 85:
		// Threats specific targets that are far have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 1, -1);
		break;
	case 86:
		// Closer specific targets targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 2, -1);
		break;
	case 87:
		// Farther specific targets targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 3, -1);
		break;
	case 88:
		// Threats specific targets that are close have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 0, -1);
		break;
	case 89:
		// Threats specific targets that are far have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 1, -1);
		break;
	case 90:
		// Closer specific targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 2, -1);
		break;
	case 91:
		// Farther specific targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 3, -1);
		break;
	case 92:
		ScriptExt::WaitIfNoTarget(pTeam, -1);
		break;
	case 93:
		ScriptExt::TeamWeightAward(pTeam, 0);
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
		} while (pUnit);
	}

	pUnit = pTeam->FirstUnit;
	do
	{
		if (pUnit->GetCurrentMission() == Mission::Enter)
			return;
		pUnit = pUnit->NextTeamMember;
	} while (pUnit);

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
	} while (pUnit);

	// This action finished
	/*if (pTeam->CurrentScript->HasNextAction())
	{
		pTeam->CurrentScript->idxCurrentLine += 1;
	}*/
	pTeam->StepCompleted = true;
}

void ScriptExt::Mission_Attack(TeamClass *pTeam, bool repeatAction = true, int calcThreatMode = 0, int attackAITargetType = -1)
{
	auto pScript = pTeam->CurrentScript;
	int scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument; // This is the target type
	TechnoClass* selectedTarget = nullptr;
	HouseClass* enemyHouse = nullptr;
	bool noWaitLoop = false;
	FootClass *pLeaderUnit = nullptr;
	TechnoTypeClass* pLeaderUnitType = nullptr;
	int bestUnitLeadershipValue = -1;
	bool bAircraftsWithoutAmmo = false;
	TechnoClass* pFocus = nullptr;
	
	// When the new target wasn't found it sleeps some few frames before the new attempt. This can save cycles and cycles of unnecessary executed lines.
	if (pTeam->GuardAreaTimer.TimeLeft != 0 || pTeam->GuardAreaTimer.InProgress())
	{
		pTeam->GuardAreaTimer.TimeLeft--;
		//if(pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] AAA   (Sleeper function: %d)\n", pTeam->Type->ID, pTeam->GuardAreaTimer.TimeLeft);
		if (pTeam->GuardAreaTimer.TimeLeft == 0)
		{
			pTeam->GuardAreaTimer.Stop(); // Needed
			noWaitLoop = true;

			auto pTeamData = TeamExt::ExtMap.Find(pTeam);
			if (pTeamData)
			{
				if (pTeamData->WaitNoTargetAttempts > 0)
				{
					pTeamData->WaitNoTargetAttempts--;
					//Debug::Log("DEBUG: [%s] [%s] AAA ENDED Script line: %d = %d,%d selectedTarget WaitIfNoTarget: %d attempts left\n", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument, pTeamData->WaitNoTargetAttempts);
				}
				else
				{
					if (pTeamData->WaitNoTargetAttempts < 0)
					{
						//Debug::Log("DEBUG: [%s] [%s] AAA ENDED Script line: %d = %d,%d selectedTarget WaitIfNoTarget: infinite attempts left\n", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
						//pTeamData->WaitNoTargetAttempts = 0;
					}
				}
			}
		}
		else
			return;
	}

	// This team has no units! END
	if (!pTeam)
	{
		//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] BBB   (This team has no units! END)\n", pTeam->Type->ID);
		// This action finished
		pTeam->StepCompleted = true;
		//pTeam->CurrentScript->NextAction();
		Debug::Log("DEBUG: ScripType: [%s] [%s] Jump to NEXT line: %d = %d,%d -> (End Team script: no team members)\n", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
		return;
	}

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto pKillerTechnoData = TechnoExt::ExtMap.Find(pUnit);
		if (pKillerTechnoData && pKillerTechnoData->LastKillWasTeamTarget)
		{
			// Time for Team award check! (if set any)
			auto pTeamData = TeamExt::ExtMap.Find(pTeam);
			if (pTeamData)
			{
				if (pTeamData->NextSuccessWeightAward > 0)
				{
					IncreaseCurrentTriggerWeight(pTeam, false, pTeamData->NextSuccessWeightAward);
					Debug::Log("DEBUG: [%s] [%s] Script line: %d = %d,%d TeamWeightAward: Team got reward for killing the Target: +%f\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->idxCurrentLine, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Argument, pTeamData->NextSuccessWeightAward);

					pTeamData->NextSuccessWeightAward = 0;
				}
			}

			// Let's clean the Killer mess ;-)
			//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] DDD   (LastKillWasTeamTarget)\n", pTeam->Type->ID);
			pTeam->QueuedFocus = nullptr;
			pTeam->Focus = nullptr;
			pKillerTechnoData->LastKillWasTeamTarget = false;

			if (!repeatAction)
			{
				// If the previous Team's Target was killed by this Team Member and the script was a 1-time-use then this script action must be finished.
				for (auto pTeamUnit = pTeam->FirstUnit; pTeamUnit; pTeamUnit = pTeamUnit->NextTeamMember)
				{
					// Let's reset all Team Members objective (for precaution)
					auto pKillerTeamUnitData = TechnoExt::ExtMap.Find(pTeamUnit);
					pKillerTeamUnitData->LastKillWasTeamTarget = false;

					if (pTeamUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType)
					{
						//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] EEE   (Removing Airc Target)\n", pTeam->Type->ID);
						pTeamUnit->SetTarget(nullptr);
						pTeamUnit->LastTarget = nullptr;
						pTeamUnit->SetFocus(nullptr); // Lets see if this works
						pTeamUnit->CurrentTargets.Clear(); // Lets see if this works
						pTeamUnit->QueueMission(Mission::Guard, true);
					}
				}

				// This action finished
				pTeam->StepCompleted = true;
				//pTeam->CurrentScript->NextAction();
				Debug::Log("DEBUG: [%s] [%s]: Force the jump to NEXT line: %d = %d,%d (!repeatAction)\n", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine + 1, pScript->Type->ScriptActions[pScript->idxCurrentLine + 1].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine + 1].Argument);
				return;
			}
		}
	}

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit && pUnit->IsAlive && pUnit->Health > 0 && !pUnit->InLimbo)
		{
			auto pUnitType = pUnit->GetTechnoType();
			if (pUnitType)
			{
				if (pUnitType->WhatAmI() == AbstractType::AircraftType
					&& !pUnit->IsInAir()
					&& abstract_cast<AircraftTypeClass*>(pUnitType)->AirportBound
					&& pUnit->Ammo < pUnitType->Ammo)
				{
					//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] FFF   ( :-( Landed Airc has no ammo)\n", pTeam->Type->ID);
					bAircraftsWithoutAmmo = true;
					pUnit->CurrentTargets.Clear();
				}

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

	if (!pLeaderUnit || bAircraftsWithoutAmmo)
	{
		//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] GGG   (!pLeaderUnit || bAircraftsWithoutAmmo)\n", pTeam->Type->ID);
		// This action finished
		pTeam->StepCompleted = true;

		Debug::Log("DEBUG: ScripType: [%s] [%s] Jump to NEXT line: %d = %d,%d -> (End Team: No Leader or Aircraft without ammo)\n", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
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

	// Weapon check used for filtering targets.
	// Note: the Team Leader is picked for this task, be careful with leadership values in your mod
	if ((WeaponType1 && WeaponType1->Projectile->AA) || (WeaponType2 && WeaponType2->Projectile->AA))
		leaderWeaponsHaveAA = true;
	if ((WeaponType1 && WeaponType1->Projectile->AG) || (WeaponType2 && WeaponType2->Projectile->AG))
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
				if ((passengerWeaponType1 && passengerWeaponType1->Projectile->AA) || (passengerWeaponType2 && passengerWeaponType2->Projectile->AA))
					leaderWeaponsHaveAA = true;
				if ((passengerWeaponType1 && passengerWeaponType1->Projectile->AG) || (passengerWeaponType2 && passengerWeaponType2->Projectile->AG))
					leaderWeaponsHaveAG = true;
			}
		}
	}

	pFocus = abstract_cast<TechnoClass*>(pTeam->Focus);
	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		int targetMask = scriptArgument;

		selectedTarget = GreatestThreat(pLeaderUnit, targetMask, calcThreatMode, enemyHouse, attackAITargetType);

		if (selectedTarget)
		{
			Debug::Log("DEBUG: [%s]: Leader [%s] selected [%s] as target. [%s] Script line: %d = %d,%d\n", pTeam->Type->ID, pLeaderUnit->GetTechnoType()->get_ID(), selectedTarget->GetTechnoType()->get_ID(), pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
			pTeam->Focus = selectedTarget;
			
			auto pTeamData = TeamExt::ExtMap.Find(pTeam);
			if (pTeamData && pTeamData->WaitNoTargetAttempts != 0)
			{
				//Debug::Log("DEBUG: [%s] [%s] Script line: %d = %d,%d WaitIfNoTarget: disabled\n", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
				pTeamData->WaitNoTargetAttempts = 0;
			}

			for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			{
				if (pUnit->IsAlive && pUnit->Health > 0 && !pUnit->InLimbo)
				{
					auto pUnitType = pUnit->GetTechnoType();
					//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] III   Looking 4 valid Team unit\n", pTeam->Type->ID);
					if (pUnit && pUnitType && pUnit != selectedTarget && pUnit->Target != selectedTarget)
					{
						//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] JJJ   (Upd target, was diff from Team Focus)\n", pTeam->Type->ID);
						pUnit->CurrentTargets.Clear();
						if (pUnitType->Underwater && pUnitType->LandTargeting == 1 && selectedTarget->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
						{
							//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] KKK   (Naval Will do Nothing)\n", pTeam->Type->ID);
							// Naval units like Submarines are unable to target ground targets except if they have anti-ground weapons. Ignore the attack
							pUnit->CurrentTargets.Clear();
							pUnit->SetTarget(nullptr);
							pUnit->SetFocus(nullptr);
							pUnit->SetDestination(nullptr, false);
							pUnit->QueueMission(Mission::Area_Guard, true);
							continue;
						}

						// Aircraft hack. I hate how this game manages the aircraft missions.
						if (pUnitType->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0 && pUnit->GetHeight() <= 0)
						{
							//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] LLL   (Landed AirC with ammo start attack)\n", pTeam->Type->ID);
							pUnit->SetDestination(selectedTarget, false);
							pUnit->QueueMission(Mission::Attack, true);
						}

						pUnit->SetTarget(selectedTarget);

						if (pUnit->IsEngineer())
							pUnit->QueueMission(Mission::Capture, true);
						else
						{
							// Aircraft hack. I hate how this game manages the aircraft missions.
							if (pUnitType->WhatAmI() != AbstractType::AircraftType)
							{
								//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] MMM   (Attack & isn't a aircraft)\n", pTeam->Type->ID);
								pUnit->QueueMission(Mission::Attack, true);
								pUnit->ClickedAction(Action::Attack, selectedTarget, false);
								if (pUnit->GetCurrentMission() != Mission::Attack)
								{
									pUnit->Mission_Attack();
								}
									

								if (pUnit->GetCurrentMission() == Mission::Move && pUnitType->JumpJet)
								{
									//pUnit->SetDestination(selectedTarget, false);
									pUnit->Mission_Attack();
								}

							}
						}

						// Tanya / Commando C4 case
						if (pUnitType->WhatAmI() == AbstractType::InfantryType && abstract_cast<InfantryTypeClass*>(pUnitType)->C4 || pUnit->HasAbility(Ability::C4))
						{
							pUnit->SetDestination(selectedTarget, false);
							pUnit->QueueMission(Mission::Sabotage, true);
						}
					}
					else
					{
						//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] NNN   (Don't Upd target, attack the Team Focus)\n", pTeam->Type->ID);
						{
							pUnit->QueueMission(Mission::Attack, true);
							pUnit->ClickedAction(Action::Attack, selectedTarget, false);
							pUnit->Mission_Attack();
						}
					}

				}
			}
		}
		else
		{
			auto pTeamData = TeamExt::ExtMap.Find(pTeam);

			if (pTeamData && pTeamData->WaitNoTargetAttempts != 0)
			{
				//Debug::Log("DEBUG: [%s] [%s] Script line: %d = %d,%d !selectedTarget WaitIfNoTarget: %d attempts left\n", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument, pTeamData->WaitNoTargetAttempts);
				pTeam->GuardAreaTimer.Start(16);
				return;
			}

			if (!noWaitLoop)
				pTeam->GuardAreaTimer.Start(16);
			//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] OOO   (selectedTarget not found)\n", pTeam->Type->ID);
			// This action finished
			pTeam->StepCompleted = true;
			//pTeam->CurrentScript->NextAction();
			Debug::Log("DEBUG: Next script action line for [%s] (%s) will be: %d = %d,%d (reason: New target NOT FOUND)\n", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine + 1, pScript->Type->ScriptActions[pScript->idxCurrentLine + 1].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine + 1].Argument);
			return;
		}
	}
	else
	{
		// Team already have a focused target
		bool validFocus = false;

		if (pFocus && pFocus->IsAlive
			&& !pFocus->InLimbo
			&& !pFocus->GetTechnoType()->Immune
			&& ((pFocus->IsInAir() && leaderWeaponsHaveAA) || (!pFocus->IsInAir() && leaderWeaponsHaveAG))
			&& !pFocus->Transporter
			&& pFocus->IsOnMap
			&& !pFocus->Absorbed
			&& pFocus->Owner != pLeaderUnit->Owner //->FindByCountryIndex(pTechno->Owner->EnemyHouseIndex)
			&& (!pLeaderUnit->Owner->IsAlliedWith(pFocus) || (pLeaderUnit->Owner->IsAlliedWith(pFocus) && pFocus->IsMindControlled() && !pLeaderUnit->Owner->IsAlliedWith(pFocus->MindControlledBy))))
		{
			validFocus = true;
		}

		//Debug::Log("DEBUG: Updating Focus TechnoClass object in Team Focus.\n");
		bool bForceNextAction = false;

		for (auto pUnit = pTeam->FirstUnit; pUnit && !bForceNextAction; pUnit = pUnit->NextTeamMember)
		{
			if (validFocus)
			{
				auto pUnitType = pUnit->GetTechnoType();

				if (pUnitType)
				{
					if (pUnit->IsAlive
						&& !pUnit->InLimbo
						&& (pUnitType->WhatAmI() == AbstractType::AircraftType && abstract_cast<AircraftTypeClass*>(pUnitType)->AirportBound)
						&& pUnit->Ammo > 0
						&& (pUnit->Target != pTeam->Focus && !pUnit->InAir))
					{
						//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] QQQ   (AirC start attack vs pFocus)\n", pTeam->Type->ID);
						pUnit->SetTarget(pFocus);
					}

					if (pUnitType->Underwater
						&& pUnitType->LandTargeting == 1
						&& pFocus->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
					{
						//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] RRR Subms Will do nothing now\n", pTeam->Type->ID);
						// Naval units like Submarines are unable to target ground targets except if they have anti-ground weapons. Ignore the attack
						pUnit->CurrentTargets.Clear();
						pUnit->SetTarget(nullptr);
						pUnit->SetFocus(nullptr);
						pUnit->SetDestination(nullptr, false);
						pUnit->QueueMission(Mission::Area_Guard, true);

						bForceNextAction = true;
						continue;
					}

					if (pUnitType->WhatAmI() == AbstractType::AircraftType
						&& pUnit->GetCurrentMission() != Mission::Attack
						&& pUnit->GetCurrentMission() != Mission::Enter)
					{
						//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] SSS2   (AirC start attack vs pFocus)\n", pTeam->Type->ID);
						if (pUnit->InAir)
						{
							if (pUnit->Ammo > 0)
							{
								//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] TTT1   (AirC start Mission_attack)\n", pTeam->Type->ID);
								pUnit->QueueMission(Mission::Attack, true);
								pUnit->ClickedAction(Action::Attack, pFocus, false);
								pUnit->Mission_Attack();
							}
							else
							{
								//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] UUU1   (AirC stops, no ammo)\n", pTeam->Type->ID);
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
								//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] TTT2   (AirC start Mission_attack)\n", pTeam->Type->ID);
								pUnit->QueueMission(Mission::Attack, true);
								pUnit->ClickedAction(Action::Attack, pFocus, false);
								pUnit->Mission_Attack();
							}
							else
							{
								//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] UUU2   (AirC stops, no ammo)\n", pTeam->Type->ID);
								pUnit->ForceMission(Mission::Enter);
								pUnit->Mission_Enter();
								pUnit->SetFocus(pUnit);
								pUnit->LastTarget = nullptr;
								pUnit->SetTarget(pUnit);
							}
						}

					}

					// Tanya C4 case
					if (pUnitType->WhatAmI() == AbstractType::InfantryType
						&& abstract_cast<InfantryTypeClass*>(pUnitType)->C4
						|| pUnit->HasAbility(Ability::C4))
					{
						pUnit->SetDestination(selectedTarget, false);
						pUnit->QueueMission(Mission::Sabotage, true);
					}
				}
			}
			else
			{
				//Debug::Log("DEBUG: Clearing Team Focus!\n");
				pTeam->Focus = nullptr;
				pTeam->QueuedFocus = nullptr;
				//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] VVV   (validFocus = false, do nothing)\n", pTeam->Type->ID);
				pUnit->ClickedAction(Action::Attack, pFocus, false);
				pUnit->CurrentTargets.Clear();
				pUnit->SetTarget(nullptr);
				pUnit->SetFocus(nullptr);
				pUnit->SetDestination(nullptr, true);

				if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType)
					pUnit->QueueMission(Mission::Guard, false);
				else
					pUnit->QueueMission(Mission::Area_Guard, false);
			}

		}

		if (bForceNextAction)
		{
			pTeam->StepCompleted = true;
			//if (pTeam->Type->ID[0] == 'C' && pTeam->Type->ID[1] == '0') Debug::Log("DEBUG: [%s] WWW   (bForceNextAction)\n", pTeam->Type->ID);
			//if (pTeam->CurrentScript->HasNextAction())
				//pTeam->CurrentScript->NextAction();
			Debug::Log("DEBUG: ScripType: [%s] [%s] Jump to NEXT line: %d = %d,%d -> (End Team: Naval unable against ground target)\n", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine + 1, pScript->Type->ScriptActions[pScript->idxCurrentLine + 1].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine + 1].Argument);
			return;
		}
	}
}

TechnoClass* ScriptExt::GreatestThreat(TechnoClass *pTechno, int method, int calcThreatMode = 0, HouseClass* onlyTargetThisHouseEnemy = nullptr, int attackAITargetType = -1)
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
		if (weaponType && weaponType->Projectile->AG)
			unitWeaponsHaveAG = true;

		//Debug::Log("DEBUG: TechnoClass::Array[%d] -> [%s] can target [%s] ?? (AA: %d, AG: %d, Idx: %d)\n", i, pTechno->GetTechnoType()->ID, object->GetTechnoType()->ID, unitWeaponsHaveAA, unitWeaponsHaveAG, weaponIndex);
		if (object->IsInAir() && !unitWeaponsHaveAA)
			continue;
		if (!object->IsInAir() && !unitWeaponsHaveAG) // I don't know if underground is a special case
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
			&& (pTechnoType->NavalTargeting == 0
				|| pTechnoType->NavalTargeting == 6))
			continue;
		
		// Land not OK for the Naval unit
		if (objectType->Naval
			&& pTechnoType->LandTargeting == 1
			&& object->GetCell()->LandType != LandType::Water)
			continue;
		
		// OnlyTargetHouseEnemy forces targets of a specific (hated) house
		if (onlyTargetThisHouseEnemy && object->Owner != onlyTargetThisHouseEnemy)
			continue;
		
		if (object != pTechno
			&& object->IsAlive
			&& !object->InLimbo
			&& !objectType->Immune
			&& !object->Transporter
			&& object->IsOnMap
			&& !object->Absorbed
			&& object->Owner != pTechno->Owner
			&& (!pTechno->Owner->IsAlliedWith(object)
				|| (pTechno->Owner->IsAlliedWith(object)
					&& object->IsMindControlled()
					&& !pTechno->Owner->IsAlliedWith(object->MindControlledBy))))
		{
			double value = 0;
			//Debug::Log("DEBUG: Possible candidate!!! Go to EvaluateObjectWithMask check.\n");
			if (EvaluateObjectWithMask(object, method, attackAITargetType))
			{
				CellStruct newCell;
				newCell.X = (short)object->Location.X;
				newCell.Y = (short)object->Location.Y;
				
				bool isGoodTarget = false;
				if (calcThreatMode == 0)
				{
					// Threat affected by distance [recommended default]
					// Is this object very FAR? then LESS THREAT against pTechno. More CLOSER? MORE THREAT for pTechno
					double objectThreatValue = objectType->ThreatPosed;
					int threatMultiplier = 100000;

					if (objectType->SpecialThreatValue > 0)
					{
						auto const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
						objectThreatValue = objectType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
					}

					value = objectThreatValue * threatMultiplier / (pTechno->DistanceFrom(object) + 1);

					if (value > bestVal || bestVal < 0)
						isGoodTarget = true;
				}
				else
				{
					if (calcThreatMode == 1)
					{
						// Threat affected by distance
						// Is this object very FAR? then MORE THREAT against pTechno. More CLOSER? LESS THREAT for pTechno
						double objectThreatValue = objectType->ThreatPosed;

						if (objectType->SpecialThreatValue > 0)
						{
							auto const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
							objectThreatValue = objectType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
						}
						value = objectThreatValue * 200000 / (pTechno->DistanceFrom(object) + 1);

						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						if (calcThreatMode == 2)
						{
							// Selection affected by distance
							// Is this object very FAR? then LESS THREAT against pTechno. More CLOSER? MORE THREAT for pTechno
							value = pTechno->DistanceFrom(object);

							if (value < bestVal || bestVal < 0)
								isGoodTarget = true;
						}
						else
						{
							if (calcThreatMode == 3)
							{
								// Selection affected by distance
								// Is this object very FAR? then MORE THREAT against pTechno. More CLOSER? LESS THREAT for pTechno
								value = pTechno->DistanceFrom(object);

								if (value > bestVal || bestVal < 0)
									isGoodTarget = true;
							}
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

	if (bestObject != nullptr) {
		return bestObject;
	}
	return nullptr;
}

bool ScriptExt::EvaluateObjectWithMask(TechnoClass *pTechno, int mask, int attackAITargetType = -1)
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
	int nSuperWeapons = 0;

	// Special case: validate target if is part of a technos list in [AITargetType] section
	if (attackAITargetType >= 0 && RulesExt::Global()->AITargetTypeLists.Count > 0)
	{
		DynamicVectorClass<TechnoTypeClass*> objectsList = RulesExt::Global()->AITargetTypeLists.GetItem(attackAITargetType);

		for (int i = 0; i < objectsList.Count; i++)
		{
			if (objectsList.GetItem(i) == pTechnoType)
			{
				return true;
			}
		}
		return false;
	}

	switch (mask)
	{
	case 1:
		// Anything ;-)
		if (!pTechno->Owner->IsNeutral())
		{
			return true;
		}
		break;

	case 2:
		// Building
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::BuildingType
				|| (pTypeBuilding
					&& !(pTypeBuilding->Artillary || pTypeBuilding->TickTank || pTypeBuilding->ICBMLauncher || pTypeBuilding->SensorArray))))
		{
			return true;
		}
		break;

	case 3:
		// Harvester
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechno->GetTechnoType());

		if (!pTechno->Owner->IsNeutral()
			&& ((pTechnoType->WhatAmI() == AbstractType::UnitType
				&& (abstract_cast<UnitTypeClass *>(pTechnoType)->Harvester
					|| abstract_cast<UnitTypeClass *>(pTechnoType)->ResourceGatherer))
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
		{
			return true;
		}
		break;

	case 5:
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		// Vehicle or landed Aircraft
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
			&& abstract_cast<BuildingClass *>(pTechno)->Factory != nullptr)
		{
			return true;
		}
		break;

	case 7:
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		// Defense
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->IsBaseDefense)
		{
			return true;
		}
		break;

	case 8:
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

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

	case 9:
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		// Power Plant
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->PowerBonus > 0)
		{
			return true;
		}
		break;

	case 10:
		// Aircraft and Air Unit
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::AircraftType || pTechnoType->JumpJet || pTechno->IsInAir()))
		{
			return true;
		}
		break;

	case 11:
		// Economy: Harvester, Refinery or Resource helper
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& ((pTechnoType->WhatAmI() == AbstractType::UnitType
				&& (abstract_cast<UnitTypeClass *>(pTechnoType)->Harvester
					|| abstract_cast<UnitTypeClass *>(pTechnoType)->ResourceGatherer))
				|| (pTypeBuilding
					&& pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& (pTypeBuilding->Refinery 
						|| pTypeBuilding->OrePurifier 
						|| pTypeBuilding->ResourceGatherer))))
		{
			return true;
		}
		break;

	case 12:
		// Refinery
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& ((pTechnoType->WhatAmI() == AbstractType::UnitType
				&& !abstract_cast<UnitTypeClass *>(pTechnoType)->Harvester
				&& abstract_cast<UnitTypeClass *>(pTechnoType)->ResourceGatherer)
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
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

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

		if (!pTechno->Owner->IsNeutral()
			&& ((WeaponType1 && WeaponType1->Warhead->MindControl)
				|| (WeaponType2 && WeaponType2->Warhead->MindControl)))
		{
			return true;
		}
		break;

	case 14:
		// Occupied Building
		if (pTechnoType->WhatAmI() == AbstractType::BuildingType)
		{
			pBuilding = abstract_cast<BuildingClass *>(pTechno);

			if (pBuilding && pBuilding->Occupants.Count > 0)
			{
				return true;
			}
		}
		break;

	case 15:
		// Naval Unit & Structure
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->Naval || pTechno->GetCell()->LandType == LandType::Water))
		{
			return true;
		}
		break;

	case 16:
		// Cloak Generator, Gap Generator, Radar Jammer or Inhibitor
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);
		pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& ((pTypeTechnoExt
				&& (pTypeTechnoExt->RadarJamRadius > 0
					|| pTypeTechnoExt->InhibitorRange > 0))
				|| (pTypeBuilding && (pTypeBuilding->GapGenerator
					|| pTypeBuilding->CloakGenerator))))
		{
			return true;
		}
		break;

	case 17:
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

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
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		// Civilian Tech
		if (pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Unsellable
			&& pTypeBuilding->Capturable
			&& pTypeBuilding->TechLevel < 0
			&& !pTypeBuilding->BridgeRepairHut)
		{
			return true;
		}
		break;

	case 19:
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		// Infantry Factory
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Factory == AbstractType::InfantryType)
		{
			return true;
		}
		break;

	case 20:
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		// Vehicle Factory
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Factory == AbstractType::UnitType)
		{
			return true;
		}
		break;

	case 21:
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		// is Aircraft Factory
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::BuildingType
				&& (pTypeBuilding->Factory == AbstractType::AircraftType || pTypeBuilding->Helipad)))
		{
			return true;
		}
		break;

	case 22:
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);
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
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		// Buildable Tech
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& BuildTech.Items)
		{
			for (int i = 0; i < BuildTech.Count; i++)
			{
				auto pTechObject = BuildTech.GetItem(i);
				if (pTechObject->ID == pTechno->get_ID())
				{
					return true;
				}
			}
		}
		break;

	case 24:
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

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
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);
		pBuildingExt = BuildingTypeExt::ExtMap.Find(static_cast<BuildingTypeClass*>(pTypeBuilding));

		if (pBuildingExt)
			nSuperWeapons = pBuildingExt->SuperWeapons.size();

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
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

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

					if (pMCVObject->ID == pTechno->get_ID())
					{
						return true;
					}
				}
			}
		}
		break;

	case 27:
		// Any Neutral object
		if (pTechno->Owner->IsNeutral())
		{
			return true;
		}
		break;

	case 28:
		// Cloak Generator & Gap Generator
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		if (!pTechno->Owner->IsNeutral() && (pTypeBuilding && (pTypeBuilding->GapGenerator || pTypeBuilding->CloakGenerator)))
		{
			return true;
		}
		break;

	case 29:
		// Radar Jammer
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);
		pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		if (!pTechno->Owner->IsNeutral() && (pTypeTechnoExt && (pTypeTechnoExt->RadarJamRadius > 0)))
		{
			return true;
		}
		break;

	case 30:
		// Inhibitor
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);
		pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& (pTypeTechnoExt && pTypeTechnoExt->InhibitorRange > 0))
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

	default:
		break;
	}

	// The possible target doesn't fit in te masks
	return false;
}

// Something similar (not equal) to Script Action 10,0
bool ScriptExt::FollowTheLeader(TeamClass *pTeam, TechnoClass* pLeader = nullptr, TechnoClass* pFollower = nullptr)
{
	if (pTeam && !pLeader)
	{
		int bestUnitLeadershipValue = -1;

		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			auto pUnitType = pUnit->GetTechnoType();
			int unitLeadershipRating = pUnitType->LeadershipRating;

			// If there are living Team Members then always exists 1 Leader.
			if (pUnit && pUnitType && pUnit->IsAlive && pUnit->Health > 0 && !pUnit->InLimbo && pUnitType->WhatAmI() != AbstractType::AircraftType)
			{
				if (unitLeadershipRating > bestUnitLeadershipValue)
				{
					pLeader = pUnit;
					bestUnitLeadershipValue = unitLeadershipRating;
				}
			}
		}
	}

	if (!pLeader)
	{
		return false;
	}

	//auto const& teamStray = RulesClass::Instance->Stray;
	auto const& teamStray = RulesClass::Instance->RelaxedStray;

	if (!pFollower)
	{
		if (!pTeam)
			return false;

		// All the team members follow the Leader
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			auto pUnitType = pUnit->GetTechnoType();

			if (pUnit && pUnitType && pUnit != pLeader && pUnit->IsAlive && pUnit->Health > 0 && !pUnit->InLimbo && pUnitType->WhatAmI() != AbstractType::AircraftType)
			{
				if (pUnit->DistanceFrom(pLeader) > (teamStray * 256))
				{
					// Too far from the leader, regroup with the leader
					pUnit->SetDestination(pLeader, false);
					pUnit->QueueMission(Mission::Patrol, false);
				}
				else
				{
					pUnit->QueueMission(Mission::Area_Guard, false);
				}
			}
		}
	}
	else
	{
		// Only 1 follower will follow the Leader
		auto pFollowerType = pFollower->GetTechnoType();

		if (pFollower && pFollowerType && pFollower != pLeader && pFollower->IsAlive && pFollower->Health > 0 && !pFollower->InLimbo && pFollowerType->WhatAmI() != AbstractType::AircraftType)
		{
			if (pFollower->DistanceFrom(pLeader) > (teamStray * 256))
			{
				// Too far from the leader, regroup with the leader
				pFollower->SetDestination(pLeader, false);
				pFollower->QueueMission(Mission::Patrol, false);
			}
			else
			{
				pFollower->QueueMission(Mission::Area_Guard, false);
			}
		}
	}

	return true;
}

void ScriptExt::DecreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	AITriggerTypeClass* pTriggerType = nullptr;
	auto pTeamType = pTeam->Type;
	bool found = false;

	if (modifier <= 0)
		modifier = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Argument;

	for (int i = 0; i < AITriggerTypeClass::Array->Count && !found; i++)
	{
		auto pTriggerTeam1Type = AITriggerTypeClass::Array->GetItem(i)->Team1;
		auto pTriggerTeam2Type = AITriggerTypeClass::Array->GetItem(i)->Team2;

		if (pTeamType && ((pTriggerTeam1Type && pTriggerTeam1Type == pTeamType) || (pTriggerTeam2Type && pTriggerTeam2Type == pTeamType)))
		{
			found = true;
			pTriggerType = AITriggerTypeClass::Array->GetItem(i);
		}
	}

	if (found)
	{
		if (modifier <= 0)
		{
			modifier = abs(RulesClass::Instance->AITriggerFailureWeightDelta);
		}

		pTriggerType->Weight_Current -= modifier;

		if (pTriggerType->Weight_Current < pTriggerType->Weight_Minimum)
			pTriggerType->Weight_Current = pTriggerType->Weight_Minimum;
	}

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;
	//pTeam->CurrentScript->NextAction();
}

void ScriptExt::IncreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	AITriggerTypeClass* pTriggerType = nullptr;
	auto pTeamType = pTeam->Type;
	bool found = false;
	
	if (modifier <= 0)
		modifier = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Argument;
	
	for (int i = 0; i < AITriggerTypeClass::Array->Count && !found; i++)
	{
		auto pTriggerTeam1Type = AITriggerTypeClass::Array->GetItem(i)->Team1;
		auto pTriggerTeam2Type = AITriggerTypeClass::Array->GetItem(i)->Team2;

		if (pTeamType && ((pTriggerTeam1Type && pTriggerTeam1Type == pTeamType) || (pTriggerTeam2Type && pTriggerTeam2Type == pTeamType)))
		{
			found = true;
			pTriggerType = AITriggerTypeClass::Array->GetItem(i);
		}
	}

	if (found)
	{
		if (modifier <= 0)
		{
			modifier = abs(RulesClass::Instance->AITriggerSuccessWeightDelta);
		}

		pTriggerType->Weight_Current += modifier;
		
		if (pTriggerType->Weight_Current > pTriggerType->Weight_Maximum)
			pTriggerType->Weight_Current = pTriggerType->Weight_Maximum;
	}

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;
	//pTeam->CurrentScript->NextAction();

	return;
}

void ScriptExt::Mission_Attack_List(TeamClass *pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType)
{
	// We'll asume that the Modder used an valid Action parameter that is a Key in the [AITargetType] section
	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Argument;

	if (RulesExt::Global()->AITargetTypeLists.Count > 0 && RulesExt::Global()->AITargetTypeLists.GetItem(attackAITargetType).Count > 0)
		Mission_Attack(pTeam, true, 0, attackAITargetType);
}

void ScriptExt::WaitIfNoTarget(TeamClass *pTeam, int attempts = 0)
{
	// This passive method prevents Team's Trigger reaching the Max Weight in seconds if there is no target and the script contains a loop (6,nn).
	// attempts == number of times the Team will wait if Mission_Attack(...) can't find a new target.
	if (attempts < 0)
		attempts = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Argument;
	//Debug::Log("DEBUG: [%s] [%s] Script line: %d = %d,%d WaitIfNoTarget: inside\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->idxCurrentLine, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Argument);
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (pTeamData)
	{
		if (attempts <= 0)
			pTeamData->WaitNoTargetAttempts = -1; // Infinite waits if no target
		else
			pTeamData->WaitNoTargetAttempts = attempts;
		//Debug::Log("DEBUG: [%s] [%s] Script line: %d = %d,%d WaitIfNoTarget: set %d attempts\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->idxCurrentLine, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Argument, pTeamData->WaitNoTargetAttempts);
	}

	// This action finished
	pTeam->StepCompleted = true;
	return;
}

void ScriptExt::TeamWeightAward(TeamClass *pTeam, double award = 0)
{
	// This passive method prevents Team's Trigger reaching the Max Weight in seconds if there is no target and the script contains a loop (6,nn).
	// attempts == number of times the Team will wait if Mission_Attack(...) can't find a new target.
	if (award <= 0)
		award = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Argument;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (pTeamData)
	{
		if (award > 0)
			pTeamData->NextSuccessWeightAward = award;
		//Debug::Log("DEBUG: [%s] [%s] Script line: %d = %d,%d TeamWeightAward: Set award: %f\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->idxCurrentLine, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Argument, award);
	}

	// This action finished
	pTeam->StepCompleted = true;
	return;
}
