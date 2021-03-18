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

	case 72:
		LoadIntoTransports(pTeam);
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

void ScriptExt::LoadIntoTransports(TeamClass *pTeam)
{
	DynamicVectorClass<FootClass *>transports;

	auto pUnit = pTeam->FirstUnit;
	if (pUnit->GetTechnoType()->Passengers > 0 && pUnit->Passengers.NumPassengers < pUnit->GetTechnoType()->Passengers && pUnit->Passengers.GetTotalSize() < pUnit->GetTechnoType()->Passengers) {
		transports.AddItem(pUnit);
	}
	while (pUnit->NextTeamMember) {
		pUnit = pUnit->NextTeamMember;

		if (pUnit->GetTechnoType()->Passengers > 0 && pUnit->Passengers.NumPassengers < pUnit->GetTechnoType()->Passengers && pUnit->Passengers.GetTotalSize() < pUnit->GetTechnoType()->Passengers) {
			transports.AddItem(pUnit);
		}
	}
	// We got all the transports. Let's continue
	// Now add units into transports

	for (auto pTransport : transports) {
		pUnit = pTeam->FirstUnit;

		do {
			if (!(pTransport == pUnit || pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType || pUnit->InLimbo || pUnit->GetTechnoType()->ConsideredAircraft || pUnit->Health <= 0)) {

				if ((pUnit->GetTechnoType()->Size > 0 && pUnit->GetTechnoType()->Size <= pTransport->GetTechnoType()->SizeLimit) && (pUnit->GetTechnoType()->Size <= (pTransport->GetTechnoType()->Passengers - pTransport->Passengers.GetTotalSize()))) {
					pUnit->IsTeamLeader = true;

					// All fine
					if (pUnit->GetCurrentMission() != Mission::Enter) {
						// Transport, stop please...
						pTransport->SetTarget(nullptr);
						pTransport->QueueMission(Mission::Guard, false);
						pTransport->SetDestination(pTransport, true);

						// Unit, your turn...
						pUnit->QueueMission(Mission::Enter, false);
						pUnit->SetTarget(pTransport);
						pUnit->SetDestination(pTransport, true);

						return;
					}
				}
			}

			pUnit = pUnit->NextTeamMember;
		} while (pUnit);
	}

	pUnit = pTeam->FirstUnit;
	do {
		if (pUnit->GetCurrentMission() == Mission::Enter) {
			return;
		}
		pUnit = pUnit->NextTeamMember;
	} while (pUnit);
	
	// This action finished
	if (pTeam->CurrentScript->HasNextAction()) {
		pTeam->CurrentScript->idxCurrentLine += 1;
	}
	pTeam->StepCompleted = true;
}
