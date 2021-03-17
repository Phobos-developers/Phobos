#include "Body.h"

template<> const DWORD Extension<ScriptClass>::Canary = 0x3B3B3B3B;
ScriptExt::ExtContainer ScriptExt::ExtMap;

// =============================
// load / save

void ScriptExt::ExtData::LoadFromStream(IStream* Stm) {
	// Nothing yet
}

void ScriptExt::ExtData::SaveToStream(IStream* Stm) {
	// Nothing yet
}

// =============================
// container

ScriptExt::ExtContainer::ExtContainer() : Container("ScriptClass") {
}

ScriptExt::ExtContainer::~ExtContainer() = default;

void ScriptExt::ProcessAction(TeamClass * pTeam)
{
	//ScriptActionNode *currentLineAction = pScript->GetCurrentAction(&pScriptType->ScriptActions[pScript->idxCurrentLine]);
	int action = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Action;

	switch (action) {
	case 71:
		ExecuteTimedAreaGuardAction(pTeam);

		break;

	default:
		// Do nothing because or it is a wrong Action number or it is an Ares/YR action...
		//Debug::Log("[%s] [%s] %d = %d,%d\n", pTeam->Type->ID, pScriptType->ID, pScript->idxCurrentLine, currentLineAction->Action, currentLineAction->Argument);
		break;
	}
}

void ScriptExt::ExecuteTimedAreaGuardAction(TeamClass *pTeam)
{
	auto pScript = pTeam->CurrentScript;
	auto pScriptType = pScript->Type;

	if (pTeam->GuardAreaTimer.TimeLeft == 0 && !pTeam->GuardAreaTimer.InProgress()) {
		auto pUnit = pTeam->FirstUnit;

		pUnit->QueueMission(Mission::Area_Guard, true);
		while (pUnit->NextTeamMember) {
			pUnit = pUnit->NextTeamMember;
			pUnit->QueueMission(Mission::Area_Guard, true);
		}
		pTeam->GuardAreaTimer.Start(15 * pScriptType->ScriptActions[pScript->idxCurrentLine].Argument);
	}
	/*else {
		Debug::Log("[%s] [%s] %d = %d,%d (Countdown: %d)\n", pTeam->Type->ID, pScriptType->ID, pScript->idxCurrentLine, currentLineAction->Action, currentLineAction->Argument, pTeam->GuardAreaTimer.GetTimeLeft());
	}
	*/

	if (pTeam->GuardAreaTimer.Completed()) {
		pTeam->GuardAreaTimer.Stop(); // Needed

		pTeam->StepCompleted = true;
	}
}
