#include "Body.h"

void ScriptExt::ManageTriggersFromList(TeamClass* pTeam, int idxAITriggerType = -1, bool isEnabled = false)
{
	auto pScript = pTeam->CurrentScript;

	if (idxAITriggerType < 0)
		idxAITriggerType = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (idxAITriggerType < 0 || RulesExt::Global()->AITriggersLists.size() <= 0)
	{
		pTeam->StepCompleted = true;
		ScriptExt::Log("AI Scripts - ManageTriggersFromList: [%s] [%s] (line: %d = %d,%d) Aborting script action because the [AITriggersLists] index %d is invalid.\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			pScript->Type->ScriptActions[pScript->CurrentMission].Action,
			pScript->Type->ScriptActions[pScript->CurrentMission].Argument,
			idxAITriggerType);

		return;
	}

	DynamicVectorClass<AITriggerTypeClass*> objectsList;
	for (auto obj : RulesExt::Global()->AITriggersLists[idxAITriggerType])
		objectsList.AddUnique(obj);

	for (auto pTrigger : *AITriggerTypeClass::Array)
	{
		if (objectsList.FindItemIndex(pTrigger) >= 0)
			pTrigger->IsEnabled = isEnabled;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ManageAllTriggersFromHouse(TeamClass* pTeam, HouseClass* pHouse = nullptr, int sideIdx = -1, int houseIdx = -1, bool isEnabled = true)
{
	// if pHouse is set then it overwrites any argument
	if (pHouse)
	{
		houseIdx = pHouse->ArrayIndex;
		sideIdx = pHouse->SideIndex;
	}

	if (sideIdx < 0)
		return;

	for (auto pTrigger : *AITriggerTypeClass::Array)
	{
		if ((houseIdx == -1 || houseIdx == pTrigger->HouseIndex) && (sideIdx == 0 || sideIdx == pTrigger->SideIndex))
			pTrigger->IsEnabled = isEnabled;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::SetSideIdxForManagingTriggers(TeamClass* pTeam, int sideIdx = -1)
{
	auto pScript = pTeam->CurrentScript;

	if (sideIdx < 0)
		sideIdx = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	// Any negative value will mark it as "Any side"
	if (sideIdx < 0)
		sideIdx = -1;

	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
		pTeamData->TriggersSideIdx = sideIdx;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::SetHouseIdxForManagingTriggers(TeamClass* pTeam, int houseIdx = 2147483647)
{
	auto pScript = pTeam->CurrentScript;

	// Note: this magic number is a default index value the game will never be able to use due to technical limitations
	if (houseIdx == 2147483647)
		houseIdx = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	houseIdx = HouseExt::GetHouseIndex(houseIdx, pTeam, nullptr);

	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
		pTeamData->TriggersHouseIdx = houseIdx;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ManageAITriggers(TeamClass* pTeam, int enabled = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	int sideIdx = pTeamData->TriggersSideIdx;
	int houseIdx = pTeamData->TriggersHouseIdx;
	pTeamData->TriggersSideIdx = -1;
	pTeamData->TriggersHouseIdx = -1;
	auto pScript = pTeam->CurrentScript;
	bool isEnabled = false;

	if (enabled < 0)
		enabled = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;
	else
		isEnabled = true;

	ScriptExt::ManageAllTriggersFromHouse(pTeam, nullptr, sideIdx, houseIdx, isEnabled);

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ManageTriggersWithObjects(TeamClass* pTeam, int idxAITargetType = -1, bool isEnabled = false)
{
	auto pScript = pTeam->CurrentScript;

	if (idxAITargetType < 0)
		idxAITargetType = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (idxAITargetType < 0 || RulesExt::Global()->AITargetTypesLists.size() <= 0)
	{
		pTeam->StepCompleted = true;
		ScriptExt::Log("AI Scripts - ManageTriggersWithObjects: [%s] [%s] (line: %d = %d,%d) Aborting script action because the [AITargetTypes] index %d is invalid.\n",
			pTeam->Type->ID,
			pScript->Type->ID,
			pScript->CurrentMission,
			pScript->Type->ScriptActions[pScript->CurrentMission].Action,
			pScript->Type->ScriptActions[pScript->CurrentMission].Argument,
			idxAITargetType);

		return;
	}

	DynamicVectorClass<TechnoTypeClass*> objectsList;
	for (auto obj : RulesExt::Global()->AITargetTypesLists[idxAITargetType])
		objectsList.AddUnique(obj);

	if (objectsList.Count == 0)
	{
		pTeam->StepCompleted = true;
		return;
	}

	auto AddTechnosFromTeam = [&](DynamicVectorClass<TechnoTypeClass*>& entriesList, TeamTypeClass* pTeam)
	{
		if (pTeam)
		{
			for (auto entry : pTeam->TaskForce->Entries)
			{
				if (entry.Amount > 0)
					entriesList.AddItem(entry.Type);
			}
		}
	};

	for (auto pTrigger : *AITriggerTypeClass::Array)
	{
		DynamicVectorClass<TechnoTypeClass*> entriesList;

		AddTechnosFromTeam(entriesList, pTrigger->Team1);
		AddTechnosFromTeam(entriesList, pTrigger->Team2);

		for (auto entry : entriesList)
		{
			if (objectsList.FindItemIndex(entry) >= 0)
			{
				pTrigger->IsEnabled = isEnabled;
				break;
			}
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
}
