#include "Body.h"

template<> const DWORD Extension<ScriptClass>::Canary = 0x3B3B3B3B;
ScriptExt::ExtContainer ScriptExt::ExtMap;

// =============================
// load / save

void ScriptExt::ExtData::LoadFromStream(PhobosStreamReader& Stm) {
	// Nothing yet
}

void ScriptExt::ExtData::SaveToStream(PhobosStreamWriter& Stm) {
	// Nothing yet
}

// =============================
// container

ScriptExt::ExtContainer::ExtContainer() : Container("ScriptClass") {
}

ScriptExt::ExtContainer::~ExtContainer() = default;

void ScriptExt::ProcessAction(TeamClass * pTeam)
{
    const int& action = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Action;

	switch (action) {
	case 71:
        ScriptExt::ExecuteTimedAreaGuardAction(pTeam);
		break;
	case 72:
        ScriptExt::LoadIntoTransports(pTeam);
		break;
	case 73:
        ScriptExt::WaitUntillFullAmmoAction(pTeam);
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
	DynamicVectorClass<FootClass *> transports;

    // We get all the transports.
    for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
        if (pUnit->GetTechnoType()->Passengers > 0 &&
            pUnit->Passengers.NumPassengers < pUnit->GetTechnoType()->Passengers &&
            pUnit->Passengers.GetTotalSize() < pUnit->GetTechnoType()->Passengers)
        {
            transports.AddItem(pUnit);
        } 

	// Now add units into transports
    for (auto pTransport : transports) {
        for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
        {
            if (!(pTransport == pUnit ||
                pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType ||
                pUnit->InLimbo ||
                pUnit->GetTechnoType()->ConsideredAircraft ||
                pUnit->Health <= 0))
            {
                if ((pUnit->GetTechnoType()->Size > 0 &&
                    pUnit->GetTechnoType()->Size <= pTransport->GetTechnoType()->SizeLimit) &&
                    (pUnit->GetTechnoType()->Size <=
                        (pTransport->GetTechnoType()->Passengers - pTransport->Passengers.GetTotalSize())
                        ))
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
        }
    }

    for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
        if (pUnit->GetCurrentMission() == Mission::Enter)
            return;
	
	// This action finished
	if (pTeam->CurrentScript->HasNextAction())
		pTeam->CurrentScript->idxCurrentLine += 1;
	pTeam->StepCompleted = true;
}

void ScriptExt::WaitUntillFullAmmoAction(TeamClass *pTeam)
{
    for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
    {
        if (pUnit && !pUnit->InLimbo && pUnit->Health > 0)
        {
            if (pUnit->GetTechnoType()->Ammo > 0 && pUnit->Ammo < pUnit->GetTechnoType()->Ammo)
            {
                auto pAirType = abstract_cast<AircraftTypeClass*>(pUnit->GetTechnoType());
                if (pAirType && pAirType->AirportBound) // If an aircraft object have AirportBound it must be evaluated
                {
                    // Reset last target, at long term battles this prevented the 
                        // aircraft to pick a new target (rare vanilla YR bug)
                    pUnit->SetTarget(nullptr);
                    pUnit->LastTarget = nullptr;
                    // Fix YR bug (when returns from the last attack the aircraft switch in loop 
                    // between Mission::Enter & Mission::Guard, making it impossible to land in the dock)
                    if (pUnit->IsInAir() && pUnit->CurrentMission != Mission::Enter)
                        pUnit->QueueMission(Mission::Enter, true);

                    return;
                }
                else { // Don't skip units that can reload themselves
                    if (pUnit->GetTechnoType()->Reload != 0)
                        return;
                }
            }
        }
    }

	// This action finished
	/*if (pTeam->CurrentScript->HasNextAction())
	{
		pTeam->CurrentScript->idxCurrentLine += 1;
	}*/
	pTeam->StepCompleted = true;
}
