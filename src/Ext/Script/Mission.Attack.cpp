#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

// Contains ScriptExt::Mission_Attack and its helper functions.

void ScriptExt::Mission_Attack(TeamClass* pTeam, int calcThreatMode, bool repeatAction, int attackAITargetType, int idxAITargetTypeItem)
{
	bool noWaitLoop = false;
	bool bAircraftsWithoutAmmo = false;
	bool agentMode = false;
	bool pacifistTeam = true;
	const auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	auto pHouseExt = HouseExt::ExtMap.Find(pTeam->Owner);
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

	const auto pScript = pTeam->CurrentScript;
	const auto pScriptType = pScript->Type;

	for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
	{
		auto pKillerTechnoData = TechnoExt::ExtMap.Find(pFoot);

		if (pKillerTechnoData->LastKillWasTeamTarget)
		{
			// Time for Team award check! (if set any)
			if (pTeamData->NextSuccessWeightAward > 0)
			{
				ScriptExt::IncreaseCurrentTriggerWeight(pTeam, false, pTeamData->NextSuccessWeightAward);
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

				const auto& node = pScriptType->ScriptActions[pScript->CurrentMission];
				const int nextMission = pScript->CurrentMission + 1;
				const auto& nextNode = pScriptType->ScriptActions[nextMission];
				ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Force the jump to next line: %d = %d,%d (This action wont repeat)\n",
					pTeam->Type->ID,
					pScriptType->ID,
					pScript->CurrentMission,
					node.Action,
					node.Argument,
					nextMission,
					nextNode.Action,
					nextNode.Argument);

				return;
			}
		}
	}

	for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
	{
		if (ScriptExt::IsUnitAvailable(pFoot, true))
		{
			const auto pTechnoType = pFoot->GetTechnoType();
			const auto whatAmI = pFoot->WhatAmI();

			if (whatAmI == AbstractType::Aircraft)
			{
				if (!pFoot->IsInAir()
					&& static_cast<AircraftTypeClass*>(pTechnoType)->AirportBound
					&& pFoot->Ammo < pTechnoType->Ammo)
				{
					bAircraftsWithoutAmmo = true;
				}
			}
			else if (whatAmI == AbstractType::Infantry)
			{
				const auto pTypeInf = static_cast<InfantryTypeClass*>(pTechnoType);

				// Any Team member (infantry) is a special agent? If yes ignore some checks based on Weapons.
				if ((pTypeInf->Agent && pTypeInf->Infiltrate) || pTypeInf->Engineer)
					agentMode = true;
			}

			pacifistTeam &= !ScriptExt::IsUnitArmed(pFoot);
		}
	}

	// Find the Leader
	auto pLeaderUnit = pTeamData->TeamLeader;

	if (!ScriptExt::IsUnitAvailable(pLeaderUnit, true))
	{
		pLeaderUnit = ScriptExt::FindTheTeamLeader(pTeam);
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

		const auto& node = pScriptType->ScriptActions[pScript->CurrentMission];
		const int nextMission = pScript->CurrentMission + 1;
		const auto& nextNode = pScriptType->ScriptActions[nextMission];
		ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No Leader found | Exists Aircrafts without ammo | Team members have no weapons)\n",
			pTeam->Type->ID,
			pScriptType->ID,
			pScript->CurrentMission,
			node.Action,
			node.Argument,
			nextMission,
			nextNode.Action,
			nextNode.Argument);

		return;
	}

	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		// This part of the code is used for picking a new target.
		HouseClass* enemyHouse = nullptr;
		const auto pTeamType = pTeam->Type;
		bool onlyTargetHouseEnemy = pTeam->Type->OnlyTargetHouseEnemy;

		if (pHouseExt->ForceOnlyTargetHouseEnemyMode != -1)
			onlyTargetHouseEnemy = pHouseExt->ForceOnlyTargetHouseEnemy;

		// Favorite Enemy House case. If set, AI will focus against that House
		if (onlyTargetHouseEnemy && pLeaderUnit->Owner->EnemyHouseIndex >= 0)
			enemyHouse = HouseClass::Array.GetItem(pLeaderUnit->Owner->EnemyHouseIndex);

		const auto& node = pScriptType->ScriptActions[pScript->CurrentMission];
		const int targetMask = node.Argument; // This is the target type
		const auto pSelectedTarget = ScriptExt::GreatestThreat(pLeaderUnit, targetMask, calcThreatMode, enemyHouse, attackAITargetType, idxAITargetTypeItem, agentMode);

		if (pSelectedTarget)
		{
			ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Leader [%s] (UID: %lu) selected [%s] (UID: %lu) as target.\n",
				pTeamType->ID,
				pScriptType->ID,
				pScript->CurrentMission,
				node.Action,
				node.Argument,
				pLeaderUnit->GetTechnoType()->get_ID(),
				pLeaderUnit->UniqueID,
				pSelectedTarget->GetTechnoType()->get_ID(),
				pSelectedTarget->UniqueID);

			pTeam->Focus = pSelectedTarget;
			pTeamData->WaitNoTargetAttempts = 0; // Disable Script Waits if there are any because a new target was selected
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0; // Disable Script Waits if there are any because a new target was selected

			for (auto pFoot = pTeam->FirstUnit; pFoot; pFoot = pFoot->NextTeamMember)
			{
				if (pFoot->IsAlive && !pFoot->InLimbo)
				{
					const auto pTechnoType = pFoot->GetTechnoType();

					if (pFoot != pSelectedTarget && pFoot->Target != pSelectedTarget)
					{
						if (pTechnoType->Underwater
							&& pTechnoType->LandTargeting == LandTargetingType::Land_Not_OK
							&& pSelectedTarget->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
						{
							// Naval units like Submarines are unable to target ground targets
							// except if they have anti-ground weapons. Ignore the attack
							pFoot->SetTarget(nullptr);
							pFoot->SetDestination(nullptr, false);
							pFoot->QueueMission(Mission::Area_Guard, true);

							continue;
						}

						const auto whatAmI = pFoot->WhatAmI();

						// Aircraft hack. I hate how this game auto-manages the aircraft missions.
						if (whatAmI == AbstractType::Aircraft
							&& pFoot->Ammo > 0
							&& pFoot->GetHeight() <= 0)
						{
							pFoot->SetDestination(pSelectedTarget, false);
							pFoot->QueueMission(Mission::Attack, true);
						}

						pFoot->SetTarget(pSelectedTarget);

						if (pFoot->IsEngineer())
							pFoot->QueueMission(Mission::Capture, true);
						else if (whatAmI != AbstractType::Aircraft) // Aircraft hack. I hate how this game auto-manages the aircraft missions.
							pFoot->QueueMission(Mission::Attack, true);

						if (whatAmI == AbstractType::Infantry)
						{
							const auto pInfantryType = static_cast<InfantryTypeClass*>(pTechnoType);

							// Spy case
							if (pInfantryType->Infiltrate
								&& pInfantryType->Agent
								&& pFoot->GetCurrentMission() != Mission::Enter)
							{
								pFoot->QueueMission(Mission::Enter, true); // Check if target is an structure and see if spiable
							}

							// Tanya / Commando C4 case
							if ((pInfantryType->C4 || pFoot->HasAbility(Ability::C4))
								&& pFoot->GetCurrentMission() != Mission::Sabotage)
							{
								pFoot->QueueMission(Mission::Sabotage, true);
							}
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

			const int nextMission = pScript->CurrentMission + 1;
			const auto& nextNode = pScriptType->ScriptActions[nextMission];
			ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (Leader [%s] (UID: %lu) can't find a new target)\n",
				pTeamType->ID,
				pScriptType->ID,
				pScript->CurrentMission,
				node.Action,
				node.Argument,
				nextMission,
				nextNode.Action,
				nextNode.Argument,
				pLeaderUnit->GetTechnoType()->get_ID(),
				pLeaderUnit->UniqueID);

			return;
		}
	}
	else
	{
		// This part of the code is used for updating the "Attack" mission in each team unit
		if (ScriptExt::IsUnitAvailable(pFocus, true)
			&& !pFocus->GetTechnoType()->Immune
			&& ScriptExt::CheckUnitTargetingCapability(pLeaderUnit, pFocus->IsInAir(), agentMode)
			&& (!pLeaderUnit->Owner->IsAlliedWith(pFocus->Owner)
				|| ScriptExt::IsMindControlledByEnemy(pLeaderUnit->Owner, pFocus)))
		{
			bool bForceNextAction = false;

			for (auto pFoot = pTeam->FirstUnit; pFoot && !bForceNextAction; pFoot = pFoot->NextTeamMember)
			{
				if (ScriptExt::IsUnitAvailable(pFoot, true))
				{
					const auto pTechnoType = pFoot->GetTechnoType();
					const auto whatAmI = pFoot->WhatAmI();

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

					const auto currentMission = pFoot->GetCurrentMission();

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
					if (currentMission != Mission::Sabotage
						&& (pFoot->HasAbility(Ability::C4)
							|| (whatAmI == AbstractType::Infantry
								&& static_cast<InfantryTypeClass*>(pTechnoType)->C4)))
					{
						pFoot->QueueMission(Mission::Sabotage, true);

						continue;
					}

					// Other cases
					if (whatAmI != AbstractType::Aircraft)
					{
						if (pFoot->Target != pFocus)
							pFoot->SetTarget(pFocus);

						if (currentMission != Mission::Attack
							&& currentMission != Mission::Unload
							&& currentMission != Mission::Selling)
						{
							pFoot->QueueMission(Mission::Attack, false);
						}

						continue;
					}
				}
			}

			if (bForceNextAction)
			{
				pTeamData->IdxSelectedObjectFromAIList = -1;
				pTeam->StepCompleted = true;

				const auto& node = pScriptType->ScriptActions[pScript->CurrentMission];
				const int nextMission = pScript->CurrentMission + 1;
				const auto& nextNode = pScriptType->ScriptActions[nextMission];
				ScriptExt::Log("AI Scripts - Attack: [%s] [%s] (line: %d = %d,%d) Jump to NEXT line: %d = %d,%d (Naval is unable to target ground)\n",
					pTeam->Type->ID,
					pScriptType->ID,
					pScript->CurrentMission,
					node.Action,
					node.Argument,
					nextMission,
					nextNode.Action,
					nextNode.Argument);

				return;
			}
		}
		else
		{
			pTeam->Focus = nullptr;
		}
	}
}

TechnoClass* ScriptExt::GreatestThreat(TechnoClass* pTechno, int method, int calcThreatMode, HouseClass* onlyTargetThisHouseEnemy, int attackAITargetType, int idxAITargetTypeItem, bool agentMode)
{
	TechnoClass* pBestObject = nullptr;
	double bestVal = -1;
	bool unitWeaponsHaveAA = false;
	bool unitWeaponsHaveAG = false;
	const auto pTechnoType = pTechno->GetTechnoType();
	const auto pTechnoOwner = pTechno->Owner;
	const auto targetZoneScanType = TechnoTypeExt::ExtMap.Find(pTechnoType)->TargetZoneScanType;

	// Generic method for targeting
	for (int i = 0; i < TechnoClass::Array.Count; i++)
	{
		const auto pTarget = TechnoClass::Array.GetItem(i);

		if (pTechnoOwner->IsAlliedWith(pTarget->Owner) && !ScriptExt::IsMindControlledByEnemy(pTechnoOwner, pTarget))
			continue;

		if (pTarget->Owner == pTechnoOwner)
			continue;

		// Exclude most of invalid target first
		if (!ScriptExt::EvaluateObjectWithMask(pTarget, method, attackAITargetType, idxAITargetTypeItem, pTechno))
			continue;

		// OnlyTargetHouseEnemy forces targets of a specific (hated) house
		if (onlyTargetThisHouseEnemy && pTarget->Owner != onlyTargetThisHouseEnemy)
			continue;

		if (pTarget->TemporalTargetingMe || pTarget->BeingWarpedOut)
			continue;

		const auto pTargetType = pTarget->GetTechnoType();

		if (!pTargetType->LegalTarget || pTargetType->Immune)
			continue;

		// Discard invisible structures
		if (const auto pTargetBuildingType = abstract_cast<BuildingTypeClass*>(pTargetType))
		{
			if (pTargetBuildingType->InvisibleInGame)
				continue;
		}

		// Note: the TEAM LEADER is picked for this task, be careful with leadership values in your mod
		const auto pWeaponType = pTechno->GetWeapon(pTechno->SelectWeapon(pTarget))->WeaponType;

		if (pWeaponType)
		{
			const auto pBulletType = pWeaponType->Projectile;

			if (pBulletType->AA)
				unitWeaponsHaveAA = true;

			if (pBulletType->AG || agentMode)
				unitWeaponsHaveAG = true;
		}

		if (!agentMode)
		{
			if (pWeaponType && GeneralUtils::GetWarheadVersusArmor(pWeaponType->Warhead, pTarget, pTargetType) == 0.0)
				continue;

			if (!(pTarget->IsInAir() ? unitWeaponsHaveAA : unitWeaponsHaveAG))
				continue;

			if (MissionControlClass::Array[(int)pTarget->CurrentMission].NoThreat)
				continue;

			if (pTarget->EstimatedHealth <= 0 && pTechnoType->VHPScan == 2)
				continue;
		}

		if (pTargetType->Naval)
		{
			// Submarines aren't a valid target
			if (pTarget->CloakState == CloakState::Cloaked
				&& pTargetType->Underwater)
			{
				const auto navalTargeting = pTechnoType->NavalTargeting;

				if (navalTargeting == NavalTargetingType::Underwater_Never
					|| navalTargeting == NavalTargetingType::Naval_None)
				{
					continue;
				}
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
			const auto pCell = pTarget->GetCell();

			if (!pCell->Sensors_InclHouse(pTechnoOwner->ArrayIndex))
				continue;
		}

		if (!ScriptExt::IsUnitAvailable(pTarget, true))
			continue;

		// Check map zone
		if (!TechnoExt::AllowedTargetByZone(pTechno, pTarget, targetZoneScanType, pWeaponType))
			continue;

		double value = 0;
		bool isGoodTarget = false;

		switch (calcThreatMode)
		{
		case 0:
		case 1:
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
			if (pTechnoOwner == HouseClass::Array.GetItem(pTarget->Owner->EnemyHouseIndex))
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

			break;
		}
		case 2:
		case 3:
		{
			// Selection affected by distance
			value = pTechno->DistanceFrom(pTarget); // Note: distance is in leptons (*256)

			if (calcThreatMode == 2)
			{
				// Is this object very FAR? then LESS THREAT against pTechno.
				// More CLOSER? MORE THREAT for pTechno.
				if (value < bestVal || bestVal < 0)
					isGoodTarget = true;
			}
			else
			{
				// Is this object very FAR? then MORE THREAT against pTechno.
				// More CLOSER? LESS THREAT for pTechno.
				if (value > bestVal || bestVal < 0)
					isGoodTarget = true;
			}

			break;
		}
		default:
		{
			break;
		}
		}

		if (isGoodTarget)
		{
			pBestObject = pTarget;
			bestVal = value;
		}
	}

	return pBestObject;
}

bool ScriptExt::EvaluateObjectWithMask(TechnoClass* pTechno, int mask, int attackAITargetType, int idxAITargetTypeItem, TechnoClass* pTeamLeader)
{
	const auto pTechnoType = pTechno->GetTechnoType();

	// Special case: validate target if is part of a technos list in [AITargetTypes] section
	if (attackAITargetType >= 0)
	{
		const auto& lists = RulesExt::Global()->AITargetTypesLists;

		if (lists.size() > static_cast<size_t>(attackAITargetType))
		{
			for (const auto& item : lists[attackAITargetType])
			{
				if (pTechnoType == item)
					return true;
			}

			return false;
		}
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

		if (!pTechno->Owner->IsNeutral())
		{
			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (!pBuildingType->IsVehicle())
					return true;
			}
		}

		break;

	case 3:
		// Harvester

		if (!pTechno->Owner->IsNeutral())
		{
			switch (pTechno->WhatAmI())
			{
			case AbstractType::Unit:

				if (static_cast<UnitTypeClass*>(pTechnoType)->Harvester)
					return true;

				// No break

			case AbstractType::Building:

				if (pTechnoType->ResourceGatherer)
					return true;

				break;

			default:
				break;
			}
		}

		break;

	case 4:
		// Infantry

		if (!pTechno->Owner->IsNeutral()
			&& pTechno->WhatAmI() == AbstractType::Infantry)
		{
			return true;
		}

		break;

	case 5:
		// Vehicle, Aircraft, Deployed vehicle into structure

		if (!pTechno->Owner->IsNeutral())
		{
			switch (pTechno->WhatAmI())
			{
			case AbstractType::Building:

				if (!static_cast<BuildingTypeClass*>(pTechnoType)->IsVehicle())
					break;

				// No break

			case AbstractType::Aircraft:
			case AbstractType::Unit:

				return true;

				break;

			default:
				break;
			}
		}

		break;

	case 6:
		// Factory

		if (!pTechno->Owner->IsNeutral())
		{
			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (pBuildingType->Factory != AbstractType::None)
					return true;
			}
		}

		break;

	case 7:
		// Defense

		if (!pTechno->Owner->IsNeutral())
		{
			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (pBuildingType->IsBaseDefense)
					return true;
			}
		}

		break;

	case 8:
		// House threats

		if (pTeamLeader)
		{
			if (const auto pTarget = abstract_cast<TechnoClass*>(pTechno->Target))
			{
				// The possible Target is aiming against me? Revenge!
				if (pTarget->Owner == pTeamLeader->Owner)
					return true;

				// Then check if this possible target is too near of the Team Leader
				if (!pTechno->Owner->IsNeutral())
				{
					const int distanceToTarget = pTeamLeader->DistanceFrom(pTechno);

					if (const auto pWeapon = TechnoExt::GetCurrentWeapon(pTechno))
					{
						if (distanceToTarget <= (WeaponTypeExt::GetRangeWithModifiers(pWeapon, pTechno) * 4))
							return true;
					}

					if (const auto pWeapon = TechnoExt::GetCurrentWeapon(pTechno, true))
					{
						if (distanceToTarget <= (WeaponTypeExt::GetRangeWithModifiers(pWeapon, pTechno) * 4))
							return true;
					}

					const int guardRange = pTeamLeader->GetTechnoType()->GuardRange;

					if (guardRange > 0
						&& distanceToTarget <= (guardRange * 2))
					{
						return true;
					}
				}
			}
		}

		break;

	case 9:
		// Power Plant

		if (!pTechno->Owner->IsNeutral())
		{
			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (pBuildingType->PowerBonus > 0)
					return true;
			}
		}

		break;

	case 10:
		// Occupied Building

		if (const auto pBuilding = abstract_cast<BuildingClass*>(pTechno))
		{
			if (pBuilding->Occupants.Count > 0)
				return true;
		}

		break;

	case 11:
		// Civilian Tech

		if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
		{
			const auto& neutralTechBuildings = RulesClass::Instance->NeutralTechBuildings;

			if (const int count = neutralTechBuildings.Count)
			{
				for (int i = 0; i < count; ++i)
				{
					if (neutralTechBuildings.GetItem(i) == pTechnoType)
						return true;
				}
			}

			// Other cases of civilian Tech Structures
			if (pBuildingType->Unsellable
				&& pBuildingType->Capturable
				&& pBuildingType->TechLevel < 0
				&& pBuildingType->NeedsEngineer
				&& !pBuildingType->BridgeRepairHut)
			{
				return true;
			}
		}

		break;

	case 12:
		// Refinery

		if (!pTechno->Owner->IsNeutral())
		{
			switch (pTechno->WhatAmI())
			{
			case AbstractType::Building:

				if (static_cast<BuildingTypeClass*>(pTechnoType)->Refinery
					|| pTechnoType->ResourceGatherer)
				{
					return true;
				}

				break;

			case AbstractType::Unit:

				if (!static_cast<UnitTypeClass*>(pTechnoType)->Harvester
					&& pTechnoType->ResourceGatherer)
				{
					return true;
				}

				break;

			default:
				break;
			}
		}

		break;

	case 13:
		// Mind Controller

		if (!pTechno->Owner->IsNeutral())
		{
			if (const auto pWeapon = TechnoExt::GetCurrentWeapon(pTechno))
			{
				if (pWeapon->Warhead->MindControl)
					return true;
			}

			if (const auto pWeapon = TechnoExt::GetCurrentWeapon(pTechno, true))
			{
				if (pWeapon->Warhead->MindControl)
					return true;
			}
		}

		break;

	case 14:
		// Aircraft and Air Unit including landed
		if (!pTechno->Owner->IsNeutral()
			&& (pTechno->WhatAmI() == AbstractType::Aircraft
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

		if (!pTechno->Owner->IsNeutral())
		{
			const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

			if (pTechnoTypeExt->RadarJamRadius > 0
				|| pTechnoTypeExt->InhibitorRange.isset())
			{
				return true;
			}

			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (pBuildingType->GapGenerator
					|| pBuildingType->CloakGenerator)
				{
					return true;
				}
			}
		}

		break;

	case 17:
		// Ground Vehicle

		if (!pTechno->Owner->IsNeutral())
		{
			switch (pTechno->WhatAmI())
			{
			case AbstractType::Building:

				if (static_cast<BuildingTypeClass*>(pTechnoType)->IsVehicle()
					&& !pTechnoType->Naval)
				{
					return true;
				}

				break;

			case AbstractType::Unit:

				if (!pTechno->IsInAir()
					&& !pTechnoType->Naval)
				{
					return true;
				}

				break;

			default:
				break;
			}
		}

		break;

	case 18:
		// Economy: Harvester, Refinery or Resource helper

		if (!pTechno->Owner->IsNeutral())
		{
			switch (pTechno->WhatAmI())
			{
			case AbstractType::Building:

				if (static_cast<BuildingTypeClass*>(pTechnoType)->Refinery
					|| static_cast<BuildingTypeClass*>(pTechnoType)->OrePurifier
					|| pTechnoType->ResourceGatherer)
				{
					return true;
				}

				break;

			case AbstractType::Unit:

				if (static_cast<UnitTypeClass*>(pTechnoType)->Harvester
					|| pTechnoType->ResourceGatherer)
				{
					return true;
				}

				break;

			default:
				break;
			}
		}

		break;

	case 19:
		// Infantry Factory

		if (!pTechno->Owner->IsNeutral())
		{
			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (pBuildingType->Factory == AbstractType::InfantryType)
					return true;
			}
		}

		break;

	case 20:
		// Land Vehicle Factory

		if (!pTechno->Owner->IsNeutral()
			&& !pTechnoType->Naval)
		{
			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (pBuildingType->Factory == AbstractType::UnitType)
					return true;
			}
		}

		break;

	case 21:
		// Aircraft Factory

		if (!pTechno->Owner->IsNeutral())
		{
			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (pBuildingType->Factory == AbstractType::AircraftType
					|| pBuildingType->Helipad)
				{
					return true;
				}
			}
		}

		break;

	case 22:
		// Radar & SpySat

		if (!pTechno->Owner->IsNeutral())
		{
			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (pBuildingType->Radar
					|| pBuildingType->SpySat)
				{
					return true;
				}
			}
		}

		break;

	case 23:
		// Buildable Tech

		if (!pTechno->Owner->IsNeutral())
		{
			if (pTechno->WhatAmI() == AbstractType::Building)
			{
				const auto& buildTech = RulesClass::Instance->BuildTech;

				if (const int count = buildTech.Count)
				{
					for (int i = 0; i < count; ++i)
					{
						if (buildTech.GetItem(i) == pTechnoType)
							return true;
					}
				}
			}
		}

		break;

	case 24:
		// Naval Factory

		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->Naval)
		{
			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (pBuildingType->Factory == AbstractType::UnitType)
					return true;
			}
		}

		break;

	case 25:
		// Super Weapon building

		if (!pTechno->Owner->IsNeutral())
		{
			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (pBuildingType->SuperWeapon >= 0
					|| pBuildingType->SuperWeapon2 >= 0
					|| BuildingTypeExt::ExtMap.Find(pBuildingType)->SuperWeapons.size() > 0)
				{
					return true;
				}
			}
		}

		break;

	case 26:
		// Construction Yard

		if (!pTechno->Owner->IsNeutral())
		{
			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (pBuildingType->Factory == AbstractType::BuildingType
					&& pBuildingType->ConstructionYard)
				{
					return true;
				}
			}

			if (pTechno->WhatAmI() == AbstractType::Unit)
			{
				const auto& baseUnit = RulesClass::Instance->BaseUnit;

				if (const int count = baseUnit.Count)
				{
					for (int i = 0; i < count; ++i)
					{
						if (baseUnit.GetItem(i) == pTechnoType)
							return true;
					}
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

		if (!pTechno->Owner->IsNeutral())
		{
			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (pBuildingType->GapGenerator
					|| pBuildingType->CloakGenerator)
				{
					return true;
				}
			}
		}

		break;

	case 29:
		// Radar Jammer

		if (!pTechno->Owner->IsNeutral()
			&& TechnoTypeExt::ExtMap.Find(pTechnoType)->RadarJamRadius > 0)
		{
			return true;
		}

		break;

	case 30:
		// Inhibitor

		if (!pTechno->Owner->IsNeutral()
			&& TechnoTypeExt::ExtMap.Find(pTechnoType)->InhibitorRange.isset())
		{
			return true;
		}

		break;

	case 31:
		// Naval Unit

		if (!pTechno->Owner->IsNeutral()
			&& (pTechno->AbstractFlags & AbstractFlags::Foot)
			&& (pTechnoType->Naval
				|| pTechno->GetCell()->LandType == LandType::Water))
		{
			return true;
		}

		break;

	case 32:
		// Any non-building unit

		if (!pTechno->Owner->IsNeutral())
		{
			const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType);

			if (!pBuildingType
				|| pBuildingType->IsVehicle()
				|| pBuildingType->ResourceGatherer)
			{
				return true;
			}
		}

		break;

	case 33:
		// Capturable Structure or Repair Hut

		if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
		{
			if (pBuildingType->Capturable
				|| (pBuildingType->BridgeRepairHut
					&& pBuildingType->Repairable))
			{
				return true;
			}
		}

		break;

	case 34:
		// Inside the Area Guard of the Team Leader

		if (pTeamLeader && !pTechno->Owner->IsNeutral())
		{
			const int distanceToTarget = pTeamLeader->DistanceFrom(pTechno);
			const int guardRange = pTeamLeader->GetTechnoType()->GuardRange;

			if (guardRange > 0
				&& distanceToTarget <= (guardRange * 2))
			{
				return true;
			}
		}

		break;

	case 35:
		// Land Vehicle Factory & Naval Factory

		if (!pTechno->Owner->IsNeutral())
		{
			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (pBuildingType->Factory == AbstractType::UnitType)
					return true;
			}
		}

		break;

	case 36:
		// Building that isn't a defense

		if (!pTechno->Owner->IsNeutral())
		{
			if (const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTechnoType))
			{
				if (!pBuildingType->IsBaseDefense
					&& !pBuildingType->IsVehicle())
				{
					return true;
				}
			}
		}

		break;

	default:
		break;
	}

	// The possible target doesn't fit in the masks
	return false;
}

void ScriptExt::Mission_Attack_List(TeamClass* pTeam, int calcThreatMode, bool repeatAction, int attackAITargetType)
{
	const auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	pTeamData->IdxSelectedObjectFromAIList = -1;

	if (attackAITargetType < 0)
	{
		const auto pScript = pTeam->CurrentScript;
		attackAITargetType = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;
	}

	if (RulesExt::Global()->AITargetTypesLists.size() > 0
		&& RulesExt::Global()->AITargetTypesLists[attackAITargetType].size() > 0)
	{
		ScriptExt::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, -1);
	}
}

void ScriptExt::Mission_Attack_List1Random(TeamClass* pTeam, int calcThreatMode, bool repeatAction, int attackAITargetType)
{
	bool selected = false;
	int idxSelectedObject = -1;
	std::vector<int> validIndexes;
	const auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (pTeamData->IdxSelectedObjectFromAIList >= 0)
	{
		idxSelectedObject = pTeamData->IdxSelectedObjectFromAIList;
		selected = true;
	}

	const auto pScript = pTeam->CurrentScript;
	const auto pScriptType = pScript->Type;

	if (attackAITargetType < 0)
		attackAITargetType = pScriptType->ScriptActions[pScript->CurrentMission].Argument;

	if (attackAITargetType >= 0
		&& (size_t)attackAITargetType < RulesExt::Global()->AITargetTypesLists.size())
	{
		auto& objectsList = RulesExt::Global()->AITargetTypesLists[attackAITargetType];

		if (idxSelectedObject < 0 && objectsList.size() > 0 && !selected)
		{
			const auto pFirstUnit = pTeam->FirstUnit;

			// Finding the objects from the list that actually exists in the map
			for (int i = 0; i < TechnoClass::Array.Count; i++)
			{
				const auto pTechno = TechnoClass::Array.GetItem(i);
				const auto pTechnoType = pTechno->GetTechnoType();
				bool found = false;

				for (auto j = 0u; j < objectsList.size() && !found; j++)
				{
					if (pTechnoType == objectsList[j]
						&& ScriptExt::IsUnitAvailable(pTechno, true)
						&& (!pFirstUnit->Owner->IsAlliedWith(pTechno->Owner)
							|| ScriptExt::IsMindControlledByEnemy(pFirstUnit->Owner, pTechno)))
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

				const auto& node = pScriptType->ScriptActions[pScript->CurrentMission];
				ScriptExt::Log("AI Scripts - AttackListRandom: [%s] [%s] (line: %d = %d,%d) Picked a random Techno from the list index [AITargetTypes][%d][%d] = %s\n",
					pTeam->Type->ID,
					pScriptType->ID,
					pScript->CurrentMission,
					node.Action,
					node.Argument,
					attackAITargetType,
					idxSelectedObject,
					objectsList[idxSelectedObject]->ID);
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

		const auto& node = pScriptType->ScriptActions[pScript->CurrentMission];
		ScriptExt::Log("AI Scripts - AttackListRandom: [%s] [%s] (line: %d = %d,%d) Failed to pick a random Techno from the list index [AITargetTypes][%d]! Valid Technos in the list: %d\n",
			pTeam->Type->ID,
			pScriptType->ID,
			pScript->CurrentMission,
			node.Action,
			node.Argument,
			attackAITargetType,
			validIndexes.size());
	}
}

bool ScriptExt::CheckUnitTargetingCapability(TechnoClass* pTechno, bool targetInAir, bool agentMode)
{
	if (!targetInAir && agentMode)
		return true;

	auto checkWeaponCapability = [targetInAir](TechnoClass* pTechno, bool secondary)
	{
		if (const auto pWeapon = TechnoExt::GetCurrentWeapon(pTechno, secondary))
		{
			const auto pBulletType = pWeapon->Projectile;
			return (targetInAir ? pBulletType->AA : (pBulletType->AG && !BulletTypeExt::ExtMap.Find(pBulletType)->AAOnly));
		}
		return false;
	};

	if (checkWeaponCapability(pTechno, false) || checkWeaponCapability(pTechno, true))
		return true;

	if (!pTechno->GetTechnoType()->OpenTopped || pTechno->Passengers.NumPassengers <= 0)
		return false;

	// Special case: a Leader with OpenTopped tag
	for (NextObject obj(pTechno->Passengers.FirstPassenger->NextObject); obj; ++obj)
	{
		const auto pPassenger = abstract_cast<FootClass*>(*obj);

		if (checkWeaponCapability(pPassenger, false) || checkWeaponCapability(pPassenger, true))
			return true;
	}

	return false;
}

bool ScriptExt::IsUnitArmed(TechnoClass* pTechno)
{
	return TechnoExt::GetCurrentWeapon(pTechno) || TechnoExt::GetCurrentWeapon(pTechno, true);
}

bool ScriptExt::IsMindControlledByEnemy(HouseClass* pHouse, TechnoClass* pTechno)
{
	return pTechno->IsMindControlled() && !pHouse->IsAlliedWith(pTechno->MindControlledBy);
}
