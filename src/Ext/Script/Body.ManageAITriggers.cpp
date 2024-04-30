#include "Body.h"

void ScriptExt::ManageTriggersFromList(TeamClass* pTeam, int idxAITriggerType = -1, bool isEnabled = false)
{
	if (!pTeam)
		return;

	auto pScript = pTeam->CurrentScript;

	if (idxAITriggerType < 0)
		idxAITriggerType = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (idxAITriggerType < 0)
		return;

	if (RulesExt::Global()->AITriggersLists.size() <= 0)
		return;

	DynamicVectorClass<AITriggerTypeClass*> objectsList;
	for (auto obj : RulesExt::Global()->AITriggersLists[idxAITriggerType])
	{
		objectsList.AddUnique(obj);
	}

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
	if (!pTeam)
		return;

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
	if (!pTeam)
		return;

	auto pScript = pTeam->CurrentScript;

	if (sideIdx < 0)
		sideIdx = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (sideIdx < -1)
		sideIdx = -1;

	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
		pTeamData->TriggersSideIdx = sideIdx;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::SetHouseIdxForManagingTriggers(TeamClass* pTeam, int houseIdx = 1000000)
{
	if (!pTeam)
		return;

	auto pScript = pTeam->CurrentScript;

	if (houseIdx == 1000000)
		houseIdx = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	houseIdx = HouseExt::GetHouseIndex(houseIdx, pTeam, nullptr);

	if (houseIdx < -1)
		houseIdx = -1;

	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
		pTeamData->TriggersHouseIdx = houseIdx;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ManageAITriggers(TeamClass* pTeam, int enabled = -1)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
	}

	int sideIdx = pTeamData->TriggersSideIdx;
	int houseIdx = pTeamData->TriggersHouseIdx;
	pTeamData->TriggersSideIdx = -1;
	pTeamData->TriggersHouseIdx = -1;
	auto pScript = pTeam->CurrentScript;
	bool isEnabled = false;

	if (enabled < 0)
		enabled = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (enabled >= 1)
		isEnabled = true;

	ScriptExt::ManageAllTriggersFromHouse(pTeam, nullptr, sideIdx, houseIdx, isEnabled);

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ManageTriggersWithObjects(TeamClass* pTeam, int idxAITargetType = -1, bool isEnabled = false)
{
	if (!pTeam)
		return;

	auto pScript = pTeam->CurrentScript;

	if (idxAITargetType < 0)
		idxAITargetType = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (idxAITargetType < 0)
		return;

	if (RulesExt::Global()->AITargetTypesLists.size() <= 0)
		return;

	DynamicVectorClass<TechnoTypeClass*> objectsList;
	for (auto obj : RulesExt::Global()->AITargetTypesLists[idxAITargetType])
	{
		objectsList.AddUnique(obj);
	}

	if (objectsList.Count == 0)
		return;

	for (auto pTrigger : *AITriggerTypeClass::Array)
	{
		DynamicVectorClass<TechnoTypeClass*> entriesList;

		if (pTrigger->Team1)
		{
			for (auto entry : pTrigger->Team1->TaskForce->Entries)
			{
				if (entry.Amount > 0)
					entriesList.AddItem(entry.Type);
			}
		}

		if (pTrigger->Team2)
		{
			for (auto entry : pTrigger->Team2->TaskForce->Entries)
			{
				if (entry.Amount > 0)
					entriesList.AddItem(entry.Type);
			}
		}

		if (entriesList.Count > 0)
		{
			for (auto entry : entriesList)
			{
				if (objectsList.FindItemIndex(entry) >= 0)
				{
					pTrigger->IsEnabled = isEnabled;
					break;
				}
			}
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
}
