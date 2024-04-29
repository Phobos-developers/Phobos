#include "Body.h"

#include <Ext/Rules/Body.h>

void ScriptExt::ResetAngerAgainstHouses(TeamClass* pTeam)
{
	// Invalid team
	if (!pTeam)
		return;

	for (auto& angerNode : pTeam->Owner->AngerNodes)
	{
		angerNode.AngerLevel = 0;
	}

	pTeam->Owner->EnemyHouseIndex = -1;
	ScriptExt::DebugAngerNodesData(); // Remove this line before merging into develop branch!

	// This action finished
	pTeam->StepCompleted = true; // This action finished - FS-21
}

void ScriptExt::SetHouseAngerModifier(TeamClass* pTeam, int modifier = 0)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
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
	if (!pTeam)
		return;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	bool changeFailed = true;

	if (idxHousesList <= 0)
		idxHousesList = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (idxHousesList >= 0
		&& idxHousesList < (int)RulesExt::Global()->AIHousesLists.size()
		&& RulesExt::Global()->AIHousesLists[idxHousesList].size() > 0)
	{
		std::vector<HouseTypeClass*> objectsList = RulesExt::Global()->AIHousesLists[idxHousesList];

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

	// This action finished
	if (changeFailed)
	{
		int currentMission = pTeam->CurrentScript->CurrentMission;

		pTeam->StepCompleted = true;
		ScriptExt::Log("[%s][%s] (line: %d = %d,%d): Failed to modify AngerNode values against other houses.\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			pTeam->CurrentScript->Type->ScriptActions[currentMission].Action,
			pTeam->CurrentScript->Type->ScriptActions[currentMission].Argument);
	}

	ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);
	ScriptExt::DebugAngerNodesData(); // Remove this line before merging into develop branch!

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ModifyHateHouses_List1Random(TeamClass* pTeam, int idxHousesList = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData || pTeamData->AngerNodeModifier == 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	int changes = 0;
	int currentMission = pTeam->CurrentScript->CurrentMission;

	if (idxHousesList < 0)
	{
		idxHousesList = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

		if (idxHousesList < 0)
		{
			// This action finished
			pTeam->StepCompleted = true;
			ScriptExt::Log("[%s][%s] (line: %d = %d,%d): Invalid [AIHousesLists] index for modifying randomly AngerNode values.\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			pTeam->CurrentScript->Type->ScriptActions[currentMission].Action,
			pTeam->CurrentScript->Type->ScriptActions[currentMission].Argument);

			return;
		}
	}

	if (idxHousesList < (int)RulesExt::Global()->AIHousesLists.size()
		&& RulesExt::Global()->AIHousesLists[idxHousesList].size() > 0)
	{
		std::vector<HouseTypeClass*> objectsList = RulesExt::Global()->AIHousesLists[idxHousesList];
		int IdxSelectedObject = ScenarioClass::Instance->Random.RandomRanged(0, objectsList.size() - 1);
		HouseTypeClass* pHouseType = objectsList[IdxSelectedObject];

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

	if (changes > 0)
	{
		ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);
		ScriptExt::DebugAngerNodesData(); // Remove this line before merging into develop branch!
	}
	else
	{
		ScriptExt::Log("[%s][%s] (line: %d = %d,%d): No AngerNode values were modified.\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			pTeam->CurrentScript->CurrentMission,
			pTeam->CurrentScript->Type->ScriptActions[currentMission].Action,
			pTeam->CurrentScript->Type->ScriptActions[currentMission].Argument);
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::SetTheMostHatedHouse(TeamClass* pTeam, int mask = 0, int mode = 1, bool random = false)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (mask == 0)
	{
		mask = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

		if (mask == 0)
		{
			// This action finished
			pTeam->StepCompleted = true;
			return;
		}
	}

	std::vector<HouseClass*> objectsList;
	int idxSelectedObject = -1;
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
			|| angerNode.House->Type->MultiplayPassive
			|| pTeam->Owner->IsAlliedWith(angerNode.House)
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
			idxSelectedObject = ScenarioClass::Instance->Random.RandomRanged(0, objectsList.size() - 1);
			selectedHouse = objectsList.at(idxSelectedObject);
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
				ScriptExt::Log("[%s][%s] (line: %d = %d,%d): Picked a new house as enemy [%s]\n",
					pTeam->Type->ID,
					pTeam->CurrentScript->Type->ID,
					pTeam->CurrentScript->CurrentMission,
					pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action,
					pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument,
					angerNode.House->Type->ID);
			}
		}

		ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);
	}
	else
	{
		ScriptExt::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Failed to pick a new hated house\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument);
	}

	// This action finished
	pTeam->StepCompleted = true;
}

HouseClass* ScriptExt::GetTheMostHatedHouse(TeamClass* pTeam, int mask = 0, int mode = 1)
{
	if (!pTeam)
		return nullptr;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData || mask == 0)
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

	bool currentMission = pTeam->CurrentScript->CurrentMission;
	double objectDistance = -1;
	double enemyDistance = -1;
	int nHouses = HouseClass::Array->Count;
	std::vector<double> enemyThreatValue = std::vector<double>(nHouses);
	enemyThreatValue[nHouses] = { 0.0 };
	HouseClass* enemyHouse = nullptr;
	double const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
	long houseMoney = -1;
	int enemyPower = -1000000000;
	int enemyKills = -1;
	int enemyAirDocks = -1;
	int enemyStructures = -1;
	int enemyNavalUnits = -1;

	if (mask == -2) // Based on House economy
	{
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

			bool isValidCandidate = false;

			if (mode == 0)
				isValidCandidate = pHouse->Available_Money() < houseMoney; // The poorest is selected
			else
				isValidCandidate = pHouse->Available_Money() > houseMoney; // The richest is selected

			if (isValidCandidate || houseMoney < 0)
			{
				houseMoney = pHouse->Available_Money();
				enemyHouse = pHouse;
			}
		}

		if (enemyHouse)
		{
			ScriptExt::Log("[%s][%s] (line: %d = %d,%d): Selected house [%s] (index: %d).\n",
				pTeam->Type->ID,
				pTeam->CurrentScript->Type->ID,
				currentMission,
				pTeam->CurrentScript->Type->ScriptActions[currentMission].Action,
				pTeam->CurrentScript->Type->ScriptActions[currentMission].Argument,
				enemyHouse->Type->ID,
				enemyHouse->ArrayIndex);
		}

		return enemyHouse;
	}

	if (mask == -3) // Based on human controlled check
	{
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
			bool isValidCandidate = false;

			if (mode == 0)
				isValidCandidate = objectDistance < enemyDistance; // Based in nearest human enemy unit
			else
				isValidCandidate = objectDistance > enemyDistance; // Based in farthest human enemy unit

			if (isValidCandidate || enemyDistance < 0)
			{
				enemyDistance = objectDistance;
				enemyHouse = pHouse;
			}
		}
	}

	if (mask == -4 || mask == -5 || mask == -6) // House power check
	{
		int checkedHousePower;

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

			if (mask == -4) // Related to the house's total power demand
				checkedHousePower = pHouse->Power_Drain();

			if (mask == -5) // Related to the house's total produced power
				checkedHousePower = pHouse->PowerOutput;

			if (mask == -6) // Related to the house's unused power
				checkedHousePower = pHouse->PowerOutput - pHouse->Power_Drain();

			bool isValidCandidate = false;

			if (mode == 0)
				isValidCandidate = checkedHousePower < enemyPower; // Selection based in lower value power in house
			else
				isValidCandidate = checkedHousePower > enemyPower; // Selection based in higher value power in house

			if (isValidCandidate || enemyPower == -10000000000)
			{
				enemyPower = checkedHousePower;
				enemyHouse = pHouse;
			}
		}
	}

	if (mask == -7) // Based on house's kills
	{
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

			int currentKills = pHouse->TotalKilledBuildings + pHouse->TotalKilledUnits;
			bool isValidCandidate = false;

			if (mode == 0)
				isValidCandidate = currentKills < enemyKills; // The pacifist is selected
			else
				isValidCandidate = currentKills > enemyKills; // The major mass murder is selected

			if (isValidCandidate || enemyKills < 0)
			{
				enemyKills = currentKills;
				enemyHouse = pHouse;
			}
		}
	}

	if (mask == -8) // Based on number of house's naval units
	{
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

			bool isValidCandidate = false;

			if (mode == 0)
				isValidCandidate = currentNavalUnits < enemyNavalUnits; // The house with less naval units is selected
			else
				isValidCandidate = currentNavalUnits > enemyNavalUnits; // The house with more naval units is selected

			if (isValidCandidate || enemyNavalUnits < 0)
			{
				enemyNavalUnits = currentNavalUnits;
				enemyHouse = pHouse;
			}
		}
	}

	if (mask == -9) // Based on number of House aircraft docks
	{
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
			bool isValidCandidate = false;

			if (mode == 0)
				isValidCandidate = currentAirDocks < enemyAirDocks; // The house with less aircraft docks is selected
			else
				isValidCandidate = currentAirDocks > enemyAirDocks; // The house with more aircraft docks is selected

			if (isValidCandidate || enemyAirDocks < 0)
			{
				enemyAirDocks = currentAirDocks;
				enemyHouse = pHouse;
			}
		}
	}

	if (mask == -10) // Based on number of house's factories (except aircraft factories)
	{
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
			bool isValidCandidate = false;

			if (mode == 0)
				isValidCandidate = currentFactories < enemyStructures; // The house with less factories is selected
			else
				isValidCandidate = currentFactories > enemyStructures; // The house with more factories is selected

			if (isValidCandidate || enemyStructures < 0)
			{
				enemyStructures = currentFactories;
				enemyHouse = pHouse;
			}
		}
	}

	if (enemyHouse)
	{
		ScriptExt::Log("[%s][%s] (line: %d = %d,%d): Selected house [%s] (index: %d).\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			currentMission,
			pTeam->CurrentScript->Type->ScriptActions[currentMission].Action,
			pTeam->CurrentScript->Type->ScriptActions[currentMission].Argument,
			enemyHouse->Type->ID,
			enemyHouse->ArrayIndex);

		return enemyHouse;
	}

	// Other cases: Check all the technos and depending of the mode compare what house will be selected as the most hated
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
			if (mask > 0) // Threat based on the new attack types of the new attack actions
			{
				if (ScriptExt::EvaluateObjectWithMask(pTechno, mask, -1, -1, pLeaderUnit)) // Check if the object type is valid
				{
					if (auto const pTechnoType = pTechno->GetTechnoType())
					{
						enemyThreatValue[pTechno->Owner->ArrayIndex] += pTechnoType->ThreatPosed;

						if (pTechnoType->SpecialThreatValue > 0)
							enemyThreatValue[pTechno->Owner->ArrayIndex] += pTechnoType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
					}
				}
			}
			else if (mask == -1) // Based on enemy object distances
			{
				objectDistance = pLeaderUnit->DistanceFrom(pTechno); // Note: distance is in leptons (*256)
				bool isValidCandidate = false;

				if (mode == 0)
					isValidCandidate = objectDistance < enemyDistance; // The house with the nearest enemy unit
				else
					isValidCandidate = objectDistance > enemyDistance; // The house with the farthest enemy unit

				if (isValidCandidate || enemyDistance == -1)
				{
					enemyDistance = objectDistance;
					enemyHouse = pTechno->Owner;
				}
			}
		}
	}

	if (mask > 0) // Pick the house with major thread
	{
		double enemyThreat = -1;

		for (int i = 0; i < nHouses; i++)
		{
			auto const pHouse = HouseClass::Array->GetItem(i);

			if (pHouse->Defeated || pHouse->Type->MultiplayPassive || pHouse->IsObserver())
				continue;

			bool isValidCandidate = false;

			if (mode == 0)
				isValidCandidate = enemyThreatValue[i] < enemyThreat; // The house with the nearest enemy unit
			else
				isValidCandidate = enemyThreatValue[i] > enemyThreat; // The house with the farthest enemy unit

			if (isValidCandidate || enemyThreat < 0)
			{
				enemyThreat = enemyThreatValue[i];
				enemyHouse = pHouse;
			}
		}
	}

	if (enemyHouse)
	{
		ScriptExt::Log("[%s][%s] (line: %d = %d,%d): Selected house [%s] (index: %d).\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			currentMission,
			pTeam->CurrentScript->Type->ScriptActions[currentMission].Action,
			pTeam->CurrentScript->Type->ScriptActions[currentMission].Argument,
			enemyHouse->Type->ID,
			enemyHouse->ArrayIndex);
	}

	return enemyHouse;
}

// Possible mode values:
// 0  -> Force "False"
// 1  -> Force "True"
// 2  -> Force "Random boolean"
// -1 -> Use default value in OnlyTargetHouseEnemy tag
// Note: only works for new Phobos script actions, not the original ones
void ScriptExt::OverrideOnlyTargetHouseEnemy(TeamClass* pTeam, int mode = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
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

	int currentMission = pTeam->CurrentScript->CurrentMission;
	ScriptExt::Log("[%s][%s] (line: %d = %d,%d): Team's 'OnlyTargetHouseEnemy' value overwrited. Now is '%d'.\n",
		pTeam->Type->ID,
		pTeam->CurrentScript->Type->ID,
		currentMission,
		pTeam->CurrentScript->Type->ScriptActions[currentMission].Action,
		pTeam->CurrentScript->Type->ScriptActions[currentMission].Argument,
		pTeamData->OnlyTargetHouseEnemy);

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ModifyHateHouse_Index(TeamClass* pTeam, int idxHouse = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData || pTeamData->AngerNodeModifier == 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	int currentMission = pTeam->CurrentScript->CurrentMission;

	if (idxHouse < 0)
		idxHouse = pTeam->CurrentScript->Type->ScriptActions[currentMission].Argument;

	if (idxHouse < 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	for (auto& angerNode : pTeam->Owner->AngerNodes)
	{
		if (angerNode.House->ArrayIndex == idxHouse
			&& !angerNode.House->Defeated
			&& !angerNode.House->IsObserver())
		{
			angerNode.AngerLevel += pTeamData->AngerNodeModifier;
			ScriptExt::Log("[%s][%s] (line: %d = %d,%d): Modified AngerNode level of [%s](index: %d) against house [%s](index: %d). Current hate value: %d\n",
				pTeam->Type->ID,
				pTeam->CurrentScript->Type->ID,
				currentMission,
				pTeam->CurrentScript->Type->ScriptActions[currentMission].Action,
				pTeam->CurrentScript->Type->ScriptActions[currentMission].Argument,
				pTeam->Owner->Type->ID,
				pTeam->Owner->ArrayIndex,
				angerNode.House->Type->ID,
				angerNode.House->ArrayIndex,
				angerNode.AngerLevel);
		}
	}

	ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);
	ScriptExt::DebugAngerNodesData(); // Remove this line before merging into develop branch!

	// This action finished
	pTeam->StepCompleted = true;
}

// The selected house will become the most hated of the map (the effects are only visible if the other houses are enemy of the selected house)
void ScriptExt::AggroHouse(TeamClass* pTeam, int index = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	int currentMission = pTeam->CurrentScript->CurrentMission;
	std::vector<HouseClass*> objectsList;
	HouseClass* selectedHouse = nullptr;
	int extraHateLevel = 5000;
	bool onlySelectHumans = index == -3 ? true : false;
	bool onlyCivilians = index == -4 ? true : false;
	bool includeCivilians = index == -5 ? true : false;

	// Only if the additional was specified then overwrite the default value
	if (pTeamData->AngerNodeModifier > 0)
		extraHateLevel = pTeamData->AngerNodeModifier;

	if (index >= 0) // A specific house
	{
		selectedHouse = HouseClass::Array()->GetItem(index);
		objectsList.emplace_back(selectedHouse);
	}
	else if (index == -2) // Team's onwer as candidate
	{
		selectedHouse = pTeam->Owner;
		objectsList.emplace_back(pTeam->Owner);
	}
	else
	{
		if (onlySelectHumans && pTeam->Owner->IsControlledByHuman()) // Include the team's onwer as candidate if only select human players
			objectsList.emplace_back(pTeam->Owner);

		// Store the list of possible candidate houses for later
		for (auto pCandidateHouse : *HouseClass::Array)
		{
			if (pCandidateHouse->Defeated || pCandidateHouse->IsObserver() || (pCandidateHouse == pTeam->Owner))
				continue;

			if (onlySelectHumans)
			{
				if (pCandidateHouse->IsControlledByHuman())
					objectsList.emplace_back(pCandidateHouse); // Only Human players are candidate
			}
			else if(onlyCivilians && pCandidateHouse->Type->MultiplayPassive)
			{
				objectsList.emplace_back(pCandidateHouse); // Only civilians are candidate
			}
			else
			{
				if (!includeCivilians && pCandidateHouse->Type->MultiplayPassive) // Ignore civilians, just valid houses
					continue;

				objectsList.emplace_back(pCandidateHouse); // Any valid house is candidate
			}
		}
	}

	if (objectsList.size() == 0)
	{
		ScriptExt::Log("[%s][%s] (line: %d = %d,%d): [%s](index: %d) failed to pick a new house as main enemy using index '%d'.\n",
			pTeam->Type->ID,
			pTeam->CurrentScript->Type->ID,
			currentMission,
			pTeam->CurrentScript->Type->ScriptActions[currentMission].Action,
			pTeam->CurrentScript->Type->ScriptActions[currentMission].Argument,
			pTeam->Owner->Type->ID,
			pTeam->Owner->ArrayIndex,
			index);

		// No candidates. This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (!selectedHouse && index != -3) // Candidates random index. Only humans case is excluded here
		selectedHouse = objectsList[ScenarioClass::Instance->Random.RandomRanged(0, objectsList.size() - 1)];

	for (auto pHouse : *HouseClass::Array)
	{
		if (pHouse->Defeated || pHouse->IsObserver())
			continue;

		// For each valid house find the highest anger value and sum extra hate;
		int highestHateLevel = -1;

		for (const auto& angerNode : pHouse->AngerNodes)
		{
			if (angerNode.AngerLevel > highestHateLevel)
				highestHateLevel = angerNode.AngerLevel;
		}

		highestHateLevel += extraHateLevel;

		// Find the houses that must be hated more than anyone
		for (auto& angerNode : pHouse->AngerNodes)
		{
			if (index == -3)
			{
				// All humans will receive the highest hate value
				if (angerNode.House->IsControlledByHuman())
					angerNode.AngerLevel = highestHateLevel;
			}
			else
			{
				// Find the select house and set it as the highest hated house
				if (selectedHouse == angerNode.House)
					angerNode.AngerLevel = highestHateLevel;
			}
		}

		ScriptExt::UpdateEnemyHouseIndex(pHouse);
	}

	ScriptExt::DebugAngerNodesData(); // Remove this line before merging into develop branch!

	// This action finished
	pTeam->StepCompleted = true;
}

// The most hated house must be the main enemy
void ScriptExt::UpdateEnemyHouseIndex(HouseClass* pHouse)
{
	if (!pHouse)
		return;

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

void ScriptExt::DebugAngerNodesData()
{
	ScriptExt::Log("Updated AngerNodes lists of every playable House:\n");

	for (auto pHouse : *HouseClass::Array)
	{
		if (pHouse->IsObserver())
			ScriptExt::Log("Player %d [Observer] ", pHouse->ArrayIndex);
		else
			ScriptExt::Log("Player %d [%s]: ", pHouse->ArrayIndex, pHouse->Type->ID);

		int i = 0;

		for (auto& angerNode : pHouse->AngerNodes)
		{
			if (!pHouse->IsObserver())
				ScriptExt::Log("%d:%d", angerNode.House->ArrayIndex, angerNode.AngerLevel);

			if (i < HouseClass::Array->Count - 2 && !pHouse->IsObserver())
				ScriptExt::Log(", ");

			i++;
		}

		if (!pHouse->IsObserver())
			ScriptExt::Log(" -> Main Enemy House: %d\n", pHouse->EnemyHouseIndex);
		else
			ScriptExt::Log("\n");
	}
}
