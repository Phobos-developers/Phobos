#include <Helpers/Macro.h>

#include "Body.h"

#include <MapClass.h>

DEFINE_HOOK(0x6E9443, TeamClass_AI, 0x8) {
	GET(TeamClass *, pTeam, ESI);

	if (!pTeam)
		return 0;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
		return 0;

	// Force a line jump. This should support vanilla YR Actions
	if (pTeamData->ForceJump_InitialCountdown > 0 && pTeamData->ForceJump_Countdown.Expired())
	{
		auto pScript = pTeam->CurrentScript;

		if (pTeamData->ForceJump_RepeatMode)
		{
			pScript->CurrentMission--;
			pTeam->Focus = nullptr;
			pTeam->QueuedFocus = nullptr;
			//pTeamData->selectedTarget; // TO-DO When a specific PR is merged
			ScriptExt::Log("AI Scripts - Team Update: [%s] [%s](line: %d = %d,%d): Jump to the same line -> (Reason: Timed Jump loop)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

			if (pTeamData->ForceJump_InitialCountdown > 0)
			{
				pTeamData->ForceJump_Countdown.Start(pTeamData->ForceJump_InitialCountdown);
				pTeamData->ForceJump_RepeatMode = true;
			}
		}
		else
		{
			pTeamData->ForceJump_InitialCountdown = -1;
			ScriptExt::Log("AI Scripts - Team Update: [%s] [%s](line: %d = %d,%d): Jump to line: %d = %d,%d -> (Reason: Timed Jump)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);
		}

		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (pUnit
				&& pUnit->IsAlive
				&& pUnit->Health > 0
				&& !pUnit->InLimbo)
			{
				pUnit->SetTarget(nullptr);
				pUnit->SetDestination(nullptr, false);
				pUnit->QueueMission(Mission::Guard, true);
			}
		}

		pTeam->StepCompleted = true;
		return 0;
	}

	ScriptExt::ProcessAction(pTeam);

	return 0;
}

// Take NewINIFormat into account just like the other classes does
// Author: secsome
DEFINE_HOOK(0x6E95B3, TeamClass_AI_MoveToCell, 0x6)
{
	if (!R->BL())
		return 0x6E95A4;

	GET(int, nCoord, ECX);
	REF_STACK(CellStruct, cell, STACK_OFFSET(0x38, -0x28));

	// if ( NewINIFormat < 4 ) then divide 128
	// in other times we divide 1000
	const int nDivisor = ScenarioClass::NewINIFormat() < 4 ? 128 : 1000;
	cell.X = static_cast<short>(nCoord % nDivisor);
	cell.Y = static_cast<short>(nCoord / nDivisor);

	R->EAX(MapClass::Instance->GetCellAt(cell));
	return 0x6E959C;
}
