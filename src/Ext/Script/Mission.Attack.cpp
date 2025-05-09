#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

// Contains ScriptExt::Mission_Attack and its helper functions.

void ScriptExt::Mission_Attack(TeamClass* pTeam, bool repeatAction = true, int calcThreatMode = 0, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	auto pScript = pTeam->CurrentScript;
	bool noWaitLoop = false;

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

	auto pFocus = abstract_cast<TechnoClass*>(pTeam->Focus);

	if (!ScriptExt::IsUnitAvailable(pFocus, true))
	{
		pTeam->Focus = nullptr;
		pFocus = nullptr;
	}

	for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
	{
		auto pKillerTechnoData = TechnoExt::ExtMap.Find(pFoot);

		if (pKillerTechnoData->LastKillWasTeamTarget)
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
				for (auto pFootTeam = pTeam->FirstUnit; pFootTeam; pFootTeam = pFootTeam->NextTeamMember)
				{
					// Let's reset all Team Members objective
					auto pKillerTeamUnitData = TechnoExt::ExtMap.Find(pFootTeam);
					pKillerTeamUnitData->LastKillWasTeamTarget = false;

					if (pFootTeam->WhatAmI() == AbstractType::Aircraft)
					{
						pFootTeam->SetTarget(nullptr);
						pFootTeam->LastTarget = nullptr;
						pFootTeam->QueueMission(Mission::Guard, true);
					}
				}

				pTeamData->IdxSelectedObjectFromAIList = -1;

				// This action finished
				pTeam->StepCompleted = true;
				ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Force the jump to next line: %d = %d,%d (This action wont repeat)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

				return;
			}
		}
	}

	bool bAircraftsWithoutAmmo = false;
	bool agentMode = false;
	bool pacifistTeam = true;

	for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
	{
		if (ScriptExt::IsUnitAvailable(pFoot, true))
		{
			auto const pTechnoType = pFoot->GetTechnoType();

			if (pFoot->WhatAmI() == AbstractType::Aircraft
				&& !pFoot->IsInAir()
				&& static_cast<AircraftTypeClass*>(pTechnoType)->AirportBound
				&& pFoot->Ammo < pTechnoType->Ammo)
			{
				bAircraftsWithoutAmmo = true;
			}

			pacifistTeam &= !ScriptExt::IsUnitArmed(pFoot);

			if (auto const pTypeInf = abstract_cast<InfantryTypeClass*, true>(pTechnoType))
			{
				// Any Team member (infantry) is a special agent? If yes ignore some checks based on Weapons.
				if ((pTypeInf->Agent && pTypeInf->Infiltrate) || pTypeInf->Engineer)
					agentMode = true;
			}
		}
	}

	// Find the Leader
	auto pLeaderUnit = pTeamData->TeamLeader;

	if (!ScriptExt::IsUnitAvailable(pLeaderUnit, true))
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
		ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No Leader found | Exists Aircrafts without ammo | Team members have no weapons)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	bool leaderWeaponsHaveAG = false;
	bool leaderWeaponsHaveAA = false;
	CheckUnitTargetingCapabilities(pLeaderUnit, leaderWeaponsHaveAG, leaderWeaponsHaveAA, agentMode);

	// Special case: a Leader with OpenTopped tag
	if (pLeaderUnit->GetTechnoType()->OpenTopped && pLeaderUnit->Passengers.NumPassengers > 0)
	{
		for (NextObject obj(pLeaderUnit->Passengers.FirstPassenger->NextObject); obj; ++obj)
		{
			auto const passenger = abstract_cast<FootClass*>(*obj);
			bool passengerWeaponsHaveAG = false;
			bool passengerWeaponsHaveAA = false;
			CheckUnitTargetingCapabilities(passenger, passengerWeaponsHaveAG, passengerWeaponsHaveAA, agentMode);

			leaderWeaponsHaveAG |= passengerWeaponsHaveAG;
			leaderWeaponsHaveAA |= passengerWeaponsHaveAA;
		}
	}

	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		// This part of the code is used for picking a new target.

		HouseClass* enemyHouse = nullptr;

		// Favorite Enemy House case. If set, AI will focus against that House
		if (pTeam->Type->OnlyTargetHouseEnemy && pLeaderUnit->Owner->EnemyHouseIndex >= 0)
			enemyHouse = HouseClass::Array.GetItem(pLeaderUnit->Owner->EnemyHouseIndex);

		int targetMask = pScript->Type->ScriptActions[pScript->CurrentMission].Argument; // This is the target type
		auto const selectedTarget = GreatestThreat(pLeaderUnit, targetMask, calcThreatMode, enemyHouse, attackAITargetType, idxAITargetTypeItem, agentMode);

		if (selectedTarget)
		{
			ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Leader [%s] (UID: %lu) selected [%s] (UID: %lu) as target.\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pLeaderUnit->GetTechnoType()->get_ID(), pLeaderUnit->UniqueID, selectedTarget->GetTechnoType()->get_ID(), selectedTarget->UniqueID);

			pTeam->Focus = selectedTarget;
			pTeamData->WaitNoTargetAttempts = 0; // Disable Script Waits if there are any because a new target was selected
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0; // Disable Script Waits if there are any because a new target was selected

			for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
			{
				if (pFoot->IsAlive && !pFoot->InLimbo)
				{
					auto const pTechnoType = pFoot->GetTechnoType();

					if (pFoot != selectedTarget && pFoot->Target != selectedTarget)
					{
						if (pTechnoType->Underwater && pTechnoType->LandTargeting == LandTargetingType::Land_Not_OK
							&& selectedTarget->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
						{
							// Naval units like Submarines are unable to target ground targets
							// except if they have anti-ground weapons. Ignore the attack
							pFoot->SetTarget(nullptr);
							pFoot->SetDestination(nullptr, false);
							pFoot->QueueMission(Mission::Area_Guard, true);

							continue;
						}

						auto const whatAmI = pFoot->WhatAmI();

						// Aircraft hack. I hate how this game auto-manages the aircraft missions.
						if (whatAmI == AbstractType::Aircraft
							&& pFoot->Ammo > 0 && pFoot->GetHeight() <= 0)
						{
							pFoot->SetDestination(selectedTarget, false);
							pFoot->QueueMission(Mission::Attack, true);
						}

						pFoot->SetTarget(selectedTarget);

						if (pFoot->IsEngineer())
							pFoot->QueueMission(Mission::Capture, true);
						else if (whatAmI != AbstractType::Aircraft) // Aircraft hack. I hate how this game auto-manages the aircraft missions.
							pFoot->QueueMission(Mission::Attack, true);

						if (whatAmI == AbstractType::Infantry)
						{
							auto const pInfantryType = static_cast<InfantryTypeClass*>(pTechnoType);

							// Spy case
							if (pInfantryType->Infiltrate && pInfantryType->Agent && pFoot->GetCurrentMission() != Mission::Enter)
								pFoot->QueueMission(Mission::Enter, true); // Check if target is an structure and see if spiable

							// Tanya / Commando C4 case
							if ((pInfantryType->C4 || pFoot->HasAbility(Ability::C4)) && pFoot->GetCurrentMission() != Mission::Sabotage)
								pFoot->QueueMission(Mission::Sabotage, true);
						}
					}
					else
					{
						pFoot->QueueMission(Mission::Attack, true);
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
			ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (Leader [%s] (UID: %lu) can't find a new target)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument, pLeaderUnit->GetTechnoType()->get_ID(), pLeaderUnit->UniqueID);

			return;
		}
	}
	else
	{
		// This part of the code is used for updating the "Attack" mission in each team unit

		bool isAirOK = pFocus->IsInAir() && leaderWeaponsHaveAA;
		bool isGroundOK = !pFocus->IsInAir() && leaderWeaponsHaveAG;

		if (ScriptExt::IsUnitAvailable(pFocus, true)
			&& !pFocus->GetTechnoType()->Immune
			&& (isAirOK || isGroundOK)
			&& (!pLeaderUnit->Owner->IsAlliedWith(pFocus) || ScriptExt::IsUnitMindControlledFriendly(pLeaderUnit->Owner, pFocus)))
		{
			bool bForceNextAction = false;

			for (auto pFoot = pTeam->FirstUnit; pFoot && !bForceNextAction; pFoot = pFoot->NextTeamMember)
			{
				auto const pTechnoType = pFoot->GetTechnoType();

				if (ScriptExt::IsUnitAvailable(pFoot, true))
				{
					auto const whatAmI = pFoot->WhatAmI();

					// Aircraft case 1
					if ((whatAmI == AbstractType::Aircraft
						&& static_cast<AircraftTypeClass*>(pTechnoType)->AirportBound)
						&& pFoot->Ammo > 0
						&& (pFoot->Target != pFocus && !pFoot->InAir))
					{
						pFoot->SetTarget(pFocus);

						continue;
					}

					// Naval units like Submarines are unable to target ground targets except if they have anti-ground weapons. Ignore the attack
					if (pTechnoType->Underwater
						&& pTechnoType->LandTargeting == LandTargetingType::Land_Not_OK
						&& pFocus->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
					{
						pFoot->SetTarget(nullptr);
						pFoot->SetDestination(nullptr, false);
						pFoot->QueueMission(Mission::Area_Guard, true);
						bForceNextAction = true;

						continue;
					}

					auto const currentMission = pFoot->GetCurrentMission();

					// Aircraft case 2
					if (whatAmI == AbstractType::Aircraft
						&& currentMission != Mission::Attack
						&& currentMission != Mission::Enter)
					{
						if (pFoot->Ammo > 0)
						{
							if (pFoot->Target != pFocus)
								pFoot->SetTarget(pFocus);

							pFoot->QueueMission(Mission::Attack, true);
						}
						else
						{
							pFoot->EnterIdleMode(false, true);
						}

						continue;
					}

					// Tanya / Commando C4 case
					if ((whatAmI == AbstractType::Infantry
						&& static_cast<InfantryTypeClass*>(pTechnoType)->C4
						|| pFoot->HasAbility(Ability::C4)) && pFoot->GetCurrentMission() != Mission::Sabotage)
					{
						pFoot->QueueMission(Mission::Sabotage, true);

						continue;
					}

					// Other cases
					if (whatAmI != AbstractType::Aircraft)
					{
						if (pFoot->Target != pFocus)
							pFoot->SetTarget(pFocus);

						if (currentMission != Mission::Attack && currentMission != Mission::Unload && currentMission != Mission::Selling)
							pFoot->QueueMission(Mission::Attack, false);

						continue;
					}
				}
			}

			if (bForceNextAction)
			{
				pTeamData->IdxSelectedObjectFromAIList = -1;
				pTeam->StepCompleted = true;
				ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to NEXT line: %d = %d,%d (Naval is unable to target ground)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

				return;
			}
		}
		else
		{
			pTeam->Focus = nullptr;
		}
	}
}

TechnoClass* ScriptExt::GreatestThreat(TechnoClass* pTechno, int method, int calcThreatMode = 0, HouseClass* onlyTargetThisHouseEnemy = nullptr, int attackAITargetType = -1, int idxAITargetTypeItem = -1, bool agentMode = false)
{
	TechnoClass* bestObject = nullptr;
	double bestVal = -1;
	bool unitWeaponsHaveAA = false;
	bool unitWeaponsHaveAG = agentMode;

	// Generic method for targeting
	for (int i = 0; i < TechnoClass::Array.Count; i++)
	{
		auto pTarget = TechnoClass::Array.GetItem(i);
		auto pTargetType = pTarget->GetTechnoType();

		if (!pTargetType->LegalTarget)
			continue;

		// Discard invisible structures
		auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTargetType);

		if (pTypeBuilding && pTypeBuilding->InvisibleInGame)
			continue;

		// Note: the TEAM LEADER is picked for this task, be careful with leadership values in your mod
		auto weaponType = pTechno->GetWeapon(pTechno->SelectWeapon(pTarget))->WeaponType;

		if (weaponType)
		{
			if (weaponType->Projectile->AA)
				unitWeaponsHaveAA = true;

			if (weaponType->Projectile->AG)
				unitWeaponsHaveAG = true;
		}

		auto pTechnoType = pTechno->GetTechnoType();

		if (!agentMode)
		{
			if (weaponType && GeneralUtils::GetWarheadVersusArmor(weaponType->Warhead, pTargetType->Armor) == 0.0)
				continue;

			if (pTarget->IsInAir() && !unitWeaponsHaveAA)
				continue;

			if (!pTarget->IsInAir() && !unitWeaponsHaveAG)
				continue;

			if (pTarget->EstimatedHealth <= 0 && pTechnoType->VHPScan == 2)
				continue;

			auto const missionControl = &MissionControlClass::Array[(int)pTarget->CurrentMission];

			if (missionControl->NoThreat)
				continue;
		}

		if (pTargetType->Naval)
		{
			auto const navalTargeting = pTechnoType->NavalTargeting;

			// Submarines aren't a valid target
			if (pTarget->CloakState == CloakState::Cloaked
				&& pTargetType->Underwater
				&& (navalTargeting == NavalTargetingType::Underwater_Never
					|| navalTargeting == NavalTargetingType::Naval_None))
			{
				continue;
			}

			// Land not OK for the Naval unit
			if (pTechnoType->LandTargeting == LandTargetingType::Land_Not_OK
				&& (pTarget->GetCell()->LandType != LandType::Water))
			{
				continue;
			}
		}

		// Stealth check.
		if (pTarget->CloakState == CloakState::Cloaked)
		{
			auto const pCell = pTarget->GetCell();

			if (!pCell->Sensors_InclHouse(pTechno->Owner->ArrayIndex))
				continue;
		}

		// OnlyTargetHouseEnemy forces targets of a specific (hated) house
		if (onlyTargetThisHouseEnemy && pTarget->Owner != onlyTargetThisHouseEnemy)
			continue;

		// Check map zone
		if (!TechnoExt::AllowedTargetByZone(pTechno, pTarget, TechnoTypeExt::ExtMap.Find(pTechnoType)->TargetZoneScanType, weaponType))
			continue;

		if (pTarget != pTechno
			&& ScriptExt::IsUnitAvailable(pTarget, true)
			&& !pTargetType->Immune
			&& !pTarget->TemporalTargetingMe
			&& !pTarget->BeingWarpedOut
			&& (!pTechno->Owner->IsAlliedWith(pTarget) || ScriptExt::IsUnitMindControlledFriendly(pTechno->Owner, pTarget)))
		{
			double value = 0;

			if (EvaluateObjectWithMask(pTarget, method, attackAITargetType, idxAITargetTypeItem, pTechno))
			{
				CellStruct newCell;
				newCell.X = (short)pTarget->Location.X;
				newCell.Y = (short)pTarget->Location.Y;

				bool isGoodTarget = false;

				if (calcThreatMode == 0 || calcThreatMode == 1)
				{
					// Threat affected by distance
					double threatMultiplier = 128.0;
					double objectThreatValue = pTargetType->ThreatPosed;

					if (pTargetType->SpecialThreatValue > 0)
					{
						double const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
						objectThreatValue += pTargetType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
					}

					// Is Defender house targeting Attacker House? if "yes" then more Threat
					if (pTechno->Owner == HouseClass::Array.GetItem(pTarget->Owner->EnemyHouseIndex))
					{
						double const& EnemyHouseThreatBonus = RulesClass::Instance->EnemyHouseThreatBonus;
						objectThreatValue += EnemyHouseThreatBonus;
					}

					// Extra threat based on current health. More damaged == More threat (almost destroyed objects gets more priority)
					objectThreatValue += pTarget->Health * (1 - pTarget->GetHealthPercentage());
					value = (objectThreatValue * threatMultiplier) / ((pTechno->DistanceFrom(pTarget) / 256.0) + 1.0);

					if (pTechnoType->VHPScan == 1)
					{
						if (pTarget->EstimatedHealth <= 0)
							value /= 2;
						else if (pTarget->EstimatedHealth <= pTargetType->Strength / 2)
							value *= 2;
					}

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
						value = pTechno->DistanceFrom(pTarget); // Note: distance is in leptons (*256)

						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						if (calcThreatMode == 3)
						{
							// Is this object very FAR? then MORE THREAT against pTechno.
							// More CLOSER? LESS THREAT for pTechno.
							value = pTechno->DistanceFrom(pTarget); // Note: distance is in leptons (*256)

							if (value > bestVal || bestVal < 0)
								isGoodTarget = true;
						}
					}
				}

				if (isGoodTarget)
				{
					bestObject = pTarget;
					bestVal = value;
				}
			}
		}
	}

	return bestObject;
}

bool ScriptExt::EvaluateObjectWithMask(TechnoClass* pTechno, int mask, int attackAITargetType = -1, int idxAITargetTypeItem = -1, TechnoClass* pTeamLeader = nullptr)
{
	auto const pTechnoType = pTechno->GetTechnoType();

	// Special case: validate target if is part of a technos list in [AITargetTypes] section
	if (attackAITargetType >= 0 && RulesExt::Global()->AITargetTypesLists.size() > 0)
	{
		for (auto item : RulesExt::Global()->AITargetTypesLists[attackAITargetType])
		{
			if (pTechnoType == item)
				return true;
		}

		return false;
	}

	switch (mask)
	{
	case 1:
		// Anything ;-)
		{
			if (!pTechno->Owner->IsNeutral())
				return true;
		}
		break;

	case 2:
		// Building
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (!pTypeBuilding->IsVehicle())
					return true;
			}
		}
		break;

	case 3:
		// Harvester
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeUnit = abstract_cast<UnitTypeClass*, true>(pTechnoType))
			{
				if (pTypeUnit->Harvester || pTypeUnit->ResourceGatherer)
					return true;
			}
			else if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->ResourceGatherer)
					return true;
			}
		}
		break;

	case 4:
		// Infantry
		{
			if (!pTechno->Owner->IsNeutral() && pTechno->WhatAmI() == AbstractType::Infantry)
				return true;
		}
		break;

	case 5:
		// Vehicle, Aircraft, Deployed vehicle into structure
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			auto const whatAmI = pTechno->WhatAmI();

			if (whatAmI == AbstractType::Aircraft || whatAmI == AbstractType::Unit)
				return true;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->IsVehicle())
					return true;
			}
		}
		break;

	case 6:
		// Factory
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->Factory != AbstractType::None)
					return true;
			}
		}
		break;

	case 7:
		// Defense
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->IsBaseDefense)
					return true;
			}
		}
		break;

	case 8:
		// House threats
		{
			if (!pTeamLeader)
				return false;

			if (auto const pTarget = abstract_cast<TechnoClass*>(pTechno->Target))
			{
				// The possible Target is aiming against me? Revenge!
				if (pTarget->Owner == pTeamLeader->Owner)
					return true;

				if (pTechno->Owner->IsNeutral())
					return false;

				// Then check if this possible target is too near of the Team Leader
				const double distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / 256.0;
				auto const pWeaponPrimary = TechnoExt::GetCurrentWeapon(pTechno);

				if (pWeaponPrimary && distanceToTarget <= (WeaponTypeExt::GetRangeWithModifiers(pWeaponPrimary, pTechno) / 256.0 * 4.0))
					return true;

				auto const pWeaponSecondary = TechnoExt::GetCurrentWeapon(pTechno, true);

				if (pWeaponSecondary && distanceToTarget <= (WeaponTypeExt::GetRangeWithModifiers(pWeaponSecondary, pTechno) / 256.0 * 4.0))
					return true;

				const int guardRange = pTeamLeader->GetTechnoType()->GuardRange;

				if (guardRange > 0 && distanceToTarget <= (guardRange / 256.0 * 2.0))
					return true;
			}
		}
		break;

	case 9:
		// Power Plant
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->PowerBonus > 0)
					return true;
			}
		}
		break;

	case 10:
		// Occupied Building
		{
			if (auto const pBuilding = abstract_cast<BuildingClass*, true>(pTechno))
			{
				if (pBuilding->Occupants.Count > 0)
					return true;
			}
		}
		break;

	case 11:
		// Civilian Tech
		{
			if (pTechno->WhatAmI() == AbstractType::Building)
			{
				auto const& neutralTechBuildings = RulesClass::Instance->NeutralTechBuildings;

				if (neutralTechBuildings.Items)
				{
					for (int i = 0; i < neutralTechBuildings.Count; i++)
					{
						auto pTechObject = neutralTechBuildings.GetItem(i);
						if (_stricmp(pTechObject->ID, pTechno->get_ID()) == 0)
							return true;
					}
				}
			}
		}
		break;

	case 12:
		// Refinery
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeUnit = abstract_cast<UnitTypeClass*, true>(pTechnoType))
			{
				if (!pTypeUnit->Harvester && pTypeUnit->ResourceGatherer)
					return true;
			}
			else if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->Refinery || pTypeBuilding->ResourceGatherer)
					return true;
			}
		}
		break;

	case 13:
		// Mind Controller
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			auto const pWeaponPrimary = TechnoExt::GetCurrentWeapon(pTechno);

			if (pWeaponPrimary && pWeaponPrimary->Warhead->MindControl)
				return true;

			auto const pWeaponSecondary = TechnoExt::GetCurrentWeapon(pTechno, true);

			if (pWeaponSecondary && pWeaponSecondary->Warhead->MindControl)
				return true;
		}
		break;

	case 14:
		// Aircraft and Air Unit including landed
		{
			if (!pTechno->Owner->IsNeutral()
				&& (pTechnoType->JumpJet || pTechno->IsInAir()
				|| pTechno->WhatAmI() == AbstractType::Aircraft))
			{
				return true;
			}
		}
		break;

	case 15:
		// Naval Unit & Structure
		{
			if (!pTechno->Owner->IsNeutral()
				&& (pTechnoType->Naval
					|| pTechno->GetCell()->LandType == LandType::Water))
			{
				return true;
			}
		}
		break;

	case 16:
		// Cloak Generator, Gap Generator, Radar Jammer or Inhibitor
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			auto const pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

			if (pTypeTechnoExt->RadarJamRadius > 0 || pTypeTechnoExt->InhibitorRange.isset())
				return true;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->GapGenerator || pTypeBuilding->CloakGenerator)
					return true;
			}
		}
		break;

	case 17:
		// Ground Vehicle
		{
			if (pTechno->Owner->IsNeutral() || pTechnoType->Naval)
				return false;

			if (pTechno->WhatAmI() == AbstractType::Unit && !pTechno->IsInAir())
				return true;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->IsVehicle())
					return true;
			}
		}
		break;

	case 18:
		// Economy: Harvester, Refinery or Resource helper
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeUnit = abstract_cast<UnitTypeClass*, true>(pTechnoType))
			{
				if (pTypeUnit->Harvester || pTypeUnit->ResourceGatherer)
					return true;
			}
			else if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->Refinery || pTypeBuilding->OrePurifier || pTypeBuilding->ResourceGatherer)
					return true;
			}
		}
		break;

	case 19:
		// Infantry Factory
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->Factory == AbstractType::InfantryType)
					return true;
			}
		}
		break;

	case 20:
		// Land Vehicle Factory
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->Factory == AbstractType::UnitType && !pTypeBuilding->Naval)
					return true;
			}
		}
		break;

	case 21:
		// Aircraft Factory
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->Factory == AbstractType::AircraftType)
					return true;
			}
		}
		break;

	case 22:
		// Radar & SpySat
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->Radar || pTypeBuilding->SpySat)
					return true;
			}
		}
		break;

	case 23:
		// Buildable Tech
		{
			if (!pTechno->Owner->IsNeutral() && pTechno->WhatAmI() == AbstractType::Building)
			{
				auto const& buildTech = RulesClass::Instance->BuildTech;

				if (buildTech.Items)
				{
					for (int i = 0; i < buildTech.Count; i++)
					{
						auto pTechObject = buildTech.GetItem(i);
						if (_stricmp(pTechObject->ID, pTechno->get_ID()) == 0)
							return true;
					}
				}
			}
		}
		break;

	case 24:
		// Naval Factory
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->Factory == AbstractType::UnitType && pTypeBuilding->Naval)
					return true;
			}
		}
		break;

	case 25:
		// Super Weapon building
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->SuperWeapon >= 0 || pTypeBuilding->SuperWeapon2 >= 0)
					return true;

				if (BuildingTypeExt::ExtMap.Find(pTypeBuilding)->SuperWeapons.size() > 0)
					return true;
			}
		}
		break;

	case 26:
		// Construction Yard
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->Factory == AbstractType::BuildingType && pTypeBuilding->ConstructionYard)
					return true;
			}
			else if (pTechno->WhatAmI() == AbstractType::Unit)
			{
				auto const& baseUnit = RulesClass::Instance->BaseUnit;

				if (baseUnit.Items)
				{
					for (int i = 0; i < baseUnit.Count; i++)
					{
						auto pMCVObject = baseUnit.GetItem(i);
						if (_stricmp(pMCVObject->ID, pTechno->get_ID()) == 0)
							return true;
					}
				}
			}
		}
		break;

	case 27:
		// Any Neutral object
		{
			if (pTechno->Owner->IsNeutral())
				return true;
		}
		break;

	case 28:
		// Cloak Generator & Gap Generator
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->GapGenerator || pTypeBuilding->CloakGenerator)
					return true;
			}
		}
		break;

	case 29:
		// Radar Jammer
		{
			if (!pTechno->Owner->IsNeutral() && TechnoTypeExt::ExtMap.Find(pTechnoType)->RadarJamRadius > 0)
				return true;
		}
		break;

	case 30:
		// Inhibitor
		{
			if (!pTechno->Owner->IsNeutral() && TechnoTypeExt::ExtMap.Find(pTechnoType)->InhibitorRange.isset())
				return true;
		}
		break;

	case 31:
		// Naval Unit
		{
			if (!pTechno->Owner->IsNeutral()
				&& pTechno->WhatAmI() != AbstractType::Building
				&& (pTechnoType->Naval || pTechno->GetCell()->LandType == LandType::Water))
			{
				return true;
			}
		}
		break;

	case 32:
		// Any non-building unit
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->IsVehicle() || pTypeBuilding->ResourceGatherer)
					return true;
			}
			else
			{
				return true;
			}
		}
		break;

	case 33:
		// Capturable Structure or Repair Hut
		{
			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->Capturable || (pTypeBuilding->BridgeRepairHut && pTypeBuilding->Repairable))
					return true;
			}
		}
		break;

	case 34:
		// Inside the Area Guard of the Team Leader
		{
			if (pTeamLeader)
			{
				if (pTechno->Owner->IsNeutral())
					return false;

				const double distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / 256.0; // Caution, DistanceFrom() return leptons
				const int guardRange = pTeamLeader->GetTechnoType()->GuardRange;

				if (guardRange > 0 && distanceToTarget <= ((guardRange / 256.0) * 2.0))
					return true;
			}
		}
		break;

	case 35:
		// Land Vehicle Factory & Naval Factory
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (pTypeBuilding->Factory == AbstractType::UnitType)
					return true;
			}
		}
		break;

	case 36:
		// Building that isn't a defense
		{
			if (pTechno->Owner->IsNeutral())
				return false;

			if (auto const pTypeBuilding = abstract_cast<BuildingTypeClass*, true>(pTechnoType))
			{
				if (!pTypeBuilding->IsBaseDefense && !pTypeBuilding->IsVehicle())
					return true;
			}
		}
		break;

	default:
		break;
	}

	// The possible target doesn't fit in the masks
	return false;
}

void ScriptExt::Mission_Attack_List(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType)
{
	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);
	auto const pScript = pTeam->CurrentScript;
	pTeamData->IdxSelectedObjectFromAIList = -1;

	if (attackAITargetType < 0)
		attackAITargetType = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (RulesExt::Global()->AITargetTypesLists.size() > 0
		&& RulesExt::Global()->AITargetTypesLists[attackAITargetType].size() > 0)
	{
		ScriptExt::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, -1);
	}
}

void ScriptExt::Mission_Attack_List1Random(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType)
{
	auto const pScript = pTeam->CurrentScript;
	bool selected = false;
	int idxSelectedObject = -1;
	std::vector<int> validIndexes;

	auto const pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (pTeamData->IdxSelectedObjectFromAIList >= 0)
	{
		idxSelectedObject = pTeamData->IdxSelectedObjectFromAIList;
		selected = true;
	}

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (attackAITargetType >= 0
		&& (size_t)attackAITargetType < RulesExt::Global()->AITargetTypesLists.size())
	{
		auto& objectsList = RulesExt::Global()->AITargetTypesLists[attackAITargetType];

		if (idxSelectedObject < 0 && objectsList.size() > 0 && !selected)
		{
			// Finding the objects from the list that actually exists in the map
			for (int i = 0; i < TechnoClass::Array.Count; i++)
			{
				auto pTechno = TechnoClass::Array.GetItem(i);
				auto pTechnoType = TechnoClass::Array.GetItem(i)->GetTechnoType();
				bool found = false;

				for (auto j = 0u; j < objectsList.size() && !found; j++)
				{
					auto objectFromList = objectsList[j];
					auto const pFirstUnit = pTeam->FirstUnit;

					if (pTechnoType == objectFromList
						&& ScriptExt::IsUnitAvailable(pTechno, true)
						&& (!pFirstUnit->Owner->IsAlliedWith(pTechno) || ScriptExt::IsUnitMindControlledFriendly(pFirstUnit->Owner, pTechno)))
					{
						validIndexes.push_back(j);
						found = true;
					}
				}
			}

			if (validIndexes.size() > 0)
			{
				idxSelectedObject = validIndexes[ScenarioClass::Instance->Random.RandomRanged(0, validIndexes.size() - 1)];
				selected = true;
				ScriptExt::Log("AI Scripts - AttackListRandom: [%s] [%s] (line: %d = %d,%d) Picked a random Techno from the list index [AITargetTypes][%d][%d] = %s\n",
					pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, attackAITargetType, idxSelectedObject, objectsList[idxSelectedObject]->ID);
			}
		}

		if (selected)
			pTeamData->IdxSelectedObjectFromAIList = idxSelectedObject;

		ScriptExt::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, idxSelectedObject);
	}

	// This action finished
	if (!selected)
	{
		pTeam->StepCompleted = true;
		ScriptExt::Log("AI Scripts - AttackListRandom: [%s] [%s] (line: %d = %d,%d) Failed to pick a random Techno from the list index [AITargetTypes][%d]! Valid Technos in the list: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, attackAITargetType, validIndexes.size());
	}
}

void ScriptExt::CheckUnitTargetingCapabilities(TechnoClass* pTechno, bool& hasAntiGround, bool& hasAntiAir, bool agentMode)
{
	auto const pWeaponPrimary = TechnoExt::GetCurrentWeapon(pTechno);
	auto const pWeaponSecondary = TechnoExt::GetCurrentWeapon(pTechno, true);

	if ((pWeaponPrimary && pWeaponPrimary->Projectile->AA) || (pWeaponSecondary && pWeaponSecondary->Projectile->AA))
		hasAntiAir = true;

	if (agentMode || (pWeaponPrimary && pWeaponPrimary->Projectile->AG && !BulletTypeExt::ExtMap.Find(pWeaponPrimary->Projectile)->AAOnly)
		|| (pWeaponSecondary && pWeaponSecondary->Projectile->AG && !BulletTypeExt::ExtMap.Find(pWeaponSecondary->Projectile)->AAOnly))
	{
		hasAntiGround = true;
	}
}

bool ScriptExt::IsUnitArmed(TechnoClass* pTechno)
{
	if (TechnoExt::GetCurrentWeapon(pTechno))
		return true;

	if (TechnoExt::GetCurrentWeapon(pTechno, true))
		return true;

	return false;
}

bool ScriptExt::IsUnitMindControlledFriendly(HouseClass* pHouse, TechnoClass* pTechno)
{
	return pHouse->IsAlliedWith(pTechno) && pTechno->IsMindControlled() && !pHouse->IsAlliedWith(pTechno->MindControlledBy);
}
