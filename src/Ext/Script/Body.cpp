#include "Body.h"
#include "../Techno/Body.h"
#include "../BuildingType/Body.h"

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
    case 74:
        // Threats that are close have more priority. Kill until no more targets.
        ScriptExt::Mission_Attack(pTeam, true, 0);
        break;
    case 75:
        // Threats that are far have more priority. Kill until no more targets.
        ScriptExt::Mission_Attack(pTeam, true, 1);
        break;
    case 76:
        // Closer targets from Team Leader have more priority. Kill until no more targets.
        ScriptExt::Mission_Attack(pTeam, true, 2);
        break;
    case 77:
        // Farther targets from Team Leader have more priority. Kill until no more targets.
        ScriptExt::Mission_Attack(pTeam, true, 3);
        break;
    case 78:
        // Threats that are close have more priority. 1 kill only (good for xx=49,0 combos)
        ScriptExt::Mission_Attack(pTeam, false, 0);
        break;
    case 79:
        // Threats that are far have more priority. 1 kill only (good for xx=49,0 combos)
        ScriptExt::Mission_Attack(pTeam, false, 1);
        break;
    case 80:
        // Closer targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
        ScriptExt::Mission_Attack(pTeam, false, 2);
        break;
    case 81:
        // Farther targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
        ScriptExt::Mission_Attack(pTeam, false, 3);
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

                    if (pUnit->GetHeight() > 0)
                    {
                        if (pUnit->CurrentMission != Mission::Enter)
                        {
                            pUnit->QueueMission(Mission::Enter, true);
                            //pUnit->ForceMission(Mission::Enter);
                        }
                    }
                    else
                    {
                        //if (pUnit->Locomotor->Is_Moving() || pUnit->Destination)
                        //{
                        pUnit->SetDestination(nullptr, true);
                        pUnit->StopMoving();
                        pUnit->Mission_AreaGuard(); //pUnit->ForceMission(Mission::Area_Guard);
                   // }
                    }

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

void ScriptExt::Mission_Attack(TeamClass *pTeam, bool repeatAction = true, int calcThreatMode = 0)
{
    auto pScript = pTeam->CurrentScript;
    int scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument; // This is the target type
    TechnoClass *selectedTarget = nullptr;
    HouseClass* enemyHouse = nullptr;
    bool noWaitLoop = false;
    FootClass *pLeaderUnit = nullptr;
    int bestUnitLeadershipValue = -1;
    bool bAircraftsWithoutAmmo = false;
    TechnoClass* pFocus = nullptr;

    // When the new target wasn't found it sleeps some few frames before the new attempt.
    if (pTeam->GuardAreaTimer.TimeLeft != 0 || pTeam->GuardAreaTimer.InProgress())
    {
        auto pScript = pTeam->CurrentScript;
        int scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument;

        pTeam->GuardAreaTimer.TimeLeft--;

        if (pTeam->GuardAreaTimer.TimeLeft == 0)
        {
            pTeam->GuardAreaTimer.Stop(); // Needed
            noWaitLoop = true;
        }
        else
            return;
    }

    // This team has no units! END
    if (!pTeam)
    {
        // This action finished
        pTeam->StepCompleted = true;
        pTeam->CurrentScript->NextAction();
        Debug::Log("DEBUG: ScripType: [%s] [%s] Jump to NEXT line: %d = %d,%d -> (End Team: without units)\n", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
        return;
    }

    if (!repeatAction)
    {
        for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
        {
            // Check 1: If the previous Team Target was killed by this team check if the Action must be finished.
            auto pKillerTechnoData = TechnoExt::ExtMap.Find(pUnit);
            if (pKillerTechnoData && pKillerTechnoData->LastKillWasTeamTarget)
            {
                //pKillerTechnoData->LastKillWasTeamTarget = false;
                pTeam->QueuedFocus = nullptr;
                pTeam->Focus = nullptr;

                for (auto pTeamUnit = pTeam->FirstUnit; pTeamUnit; pTeamUnit = pTeamUnit->NextTeamMember)
                {
                    auto pKillerTechnoData = TechnoExt::ExtMap.Find(pTeamUnit);
                    pKillerTechnoData->LastKillWasTeamTarget = false;
                    pTeamUnit->SetTarget(nullptr);
                    pTeamUnit->LastTarget = nullptr;
                    pTeamUnit->SetFocus(nullptr); // Lets see if this works
                    pTeamUnit->CurrentTargets.Clear(); // Lets see if this works

                    pTeamUnit->QueueMission(Mission::Guard, true);
                }

                // This action finished
                pTeam->StepCompleted = true;
                pTeam->CurrentScript->NextAction();
                Debug::Log("DEBUG: ScripType: [%s] [%s] Jump to NEXT line: %d = %d,%d\n", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
                return;
            }
        }
    }

    for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
    {
        if (pUnit && pUnit->IsAlive && pUnit->Health > 0 && !pUnit->InLimbo)
        {
            if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType
                && !pUnit->IsInAir()
                && abstract_cast<AircraftTypeClass*>(pUnit->GetTechnoType())->AirportBound
                && pUnit->Ammo < pUnit->GetTechnoType()->Ammo)
            {
                bAircraftsWithoutAmmo = true;
            }

            // Check 2: The team leader will be used for selecting targets, always exists 1.
            int unitLeadershipRating = pUnit->GetTechnoType()->LeadershipRating;
            if (unitLeadershipRating > bestUnitLeadershipValue)
            {
                pLeaderUnit = pUnit;
                bestUnitLeadershipValue = unitLeadershipRating;
            }
        }
    }

    if (!pLeaderUnit || bAircraftsWithoutAmmo)
    {
        // This action finished
        pTeam->StepCompleted = true;

        Debug::Log("DEBUG: ScripType: [%s] [%s] Jump to NEXT line: %d = %d,%d -> (End Team: No Leader | Aircraft without ammo)\n", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
        return;
    }

    bool unitWeaponsHaveAA = false;
    bool unitWeaponsHaveAG = false;
    // Note: Replace these lines when I have access to Combat_Damage() method in YRpp if that is better
    WeaponTypeClass* WeaponType1 = pLeaderUnit->Veterancy.IsElite() ?
        pLeaderUnit->GetTechnoType()->EliteWeapon[0].WeaponType :
        pLeaderUnit->GetTechnoType()->Weapon[0].WeaponType;
    WeaponTypeClass* WeaponType2 = pLeaderUnit->Veterancy.IsElite() ?
        pLeaderUnit->GetTechnoType()->EliteWeapon[0].WeaponType :
        pLeaderUnit->GetTechnoType()->Weapon[0].WeaponType;
    WeaponTypeClass* WeaponType3 = WeaponType1;
    if (pLeaderUnit->GetTechnoType()->IsGattling)
    {
        WeaponType3 = pLeaderUnit->Veterancy.IsElite() ?
            pLeaderUnit->GetTechnoType()->EliteWeapon[pLeaderUnit->CurrentWeaponNumber].WeaponType :
            pLeaderUnit->GetTechnoType()->Weapon[pLeaderUnit->CurrentWeaponNumber].WeaponType;

        WeaponType1 = WeaponType3;
    }

    // Weapon check used for filtering targets.
    // Note: the TEAM LEADER is picked for this task, be careful with leadership values in your mod
    if ((WeaponType1 && WeaponType1->Projectile->AA) || (WeaponType2 && WeaponType2->Projectile->AA))
        unitWeaponsHaveAA = true;
    if ((WeaponType1 && WeaponType1->Projectile->AG) || (WeaponType2 && WeaponType2->Projectile->AG))
        unitWeaponsHaveAG = true;

    // Special case: a Leader with OpenTopped tag
    if (pLeaderUnit->GetTechnoType()->OpenTopped && pLeaderUnit->Passengers.NumPassengers > 0)
    {
        for (NextObject j(pLeaderUnit->Passengers.FirstPassenger->NextObject); j && abstract_cast<FootClass*>(*j); ++j)
        {
            auto passenger = static_cast<FootClass*>(*j);

            // Note: Replace these lines when I have access to Combat_Damage() method in YRpp if that is better
            WeaponTypeClass* passengerWeaponType1 = passenger->Veterancy.IsElite() ?
                passenger->GetTechnoType()->EliteWeapon[0].WeaponType :
                passenger->GetTechnoType()->Weapon[0].WeaponType;
            WeaponTypeClass* passengerWeaponType2 = passenger->Veterancy.IsElite() ?
                passenger->GetTechnoType()->EliteWeapon[0].WeaponType :
                passenger->GetTechnoType()->Weapon[0].WeaponType;
            if (passenger->GetTechnoType()->IsGattling)
            {
                WeaponTypeClass* passengerWeaponType3 = passenger->Veterancy.IsElite() ?
                    passenger->GetTechnoType()->EliteWeapon[passenger->CurrentWeaponNumber].WeaponType :
                    passenger->GetTechnoType()->Weapon[passenger->CurrentWeaponNumber].WeaponType;

                passengerWeaponType1 = passengerWeaponType3;
            }

            // Used for filtering targets.
            // Note: the units inside a openTopped Leader are used for this task
            if ((passengerWeaponType1 && passengerWeaponType1->Projectile->AA) || (passengerWeaponType2 && passengerWeaponType2->Projectile->AA))
                unitWeaponsHaveAA = true;
            if ((passengerWeaponType1 && passengerWeaponType1->Projectile->AG) || (passengerWeaponType2 && passengerWeaponType2->Projectile->AG))
                unitWeaponsHaveAG = true;
        }
    }

    pFocus = abstract_cast<TechnoClass*>(pTeam->Focus);
    if (!pFocus && !bAircraftsWithoutAmmo)
    {
        int targetMask = scriptArgument;

        selectedTarget = GreatestThreat(pLeaderUnit, targetMask, calcThreatMode, enemyHouse);

        if (selectedTarget)
        {
            Debug::Log("DEBUG: selected target [%s] for [%s] [%s] line: %d = %d,%d (Mission_Attack)\n", selectedTarget->GetTechnoType()->get_ID(), pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
            pTeam->Focus = selectedTarget;

            for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
            {
                if (pUnit != selectedTarget && pUnit->Target != selectedTarget)
                {
                    if (pUnit->GetTechnoType()->Naval && pUnit->GetTechnoType()->LandTargeting == 1 && selectedTarget->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
                    {
                        // Naval units like Submarines are unable to target ground targets, ignore the attack
                        pUnit->CurrentTargets.Clear();
                        pUnit->SetTarget(nullptr);
                        pUnit->SetFocus(nullptr);
                        pUnit->SetDestination(nullptr, false);
                        pUnit->QueueMission(Mission::Area_Guard, true);

                        continue;
                    }
                    
                    if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0 && pUnit->GetHeight() <= 0)
                    {
//                        if (pUnit->GetTechnoType()->Naval) Debug::Log("DEBUG: 1212 [%s] from [%s] [%s], line: %d = %d,%d \n", pUnit->GetTechnoType()->ID ? pUnit->GetTechnoType()->ID : "<NULL>", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
                        pUnit->SetDestination(selectedTarget, false);

                        pUnit->QueueMission(Mission::Attack, true);
                    }
                    pUnit->SetTarget(selectedTarget);
                    if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0 && pUnit->GetHeight() > 0)
                    {
//                        if (pUnit->GetTechnoType()->Naval) Debug::Log("DEBUG: 1010 [%s] from [%s] [%s], line: %d = %d,%d \n", pUnit->GetTechnoType()->ID ? pUnit->GetTechnoType()->ID : "<NULL>", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
                        pUnit->ClickedAction(Action::Attack, selectedTarget, 0);
                        pUnit->QueueMission(Mission::Attack, false);
                    }

                    if (pUnit->IsEngineer())
                        pUnit->QueueMission(Mission::Capture, true);
                    else
                    {
//                        if (pUnit->GetTechnoType()->Naval) Debug::Log("DEBUG: 9999 [%s] from [%s] [%s], line: %d = %d,%d \n", pUnit->GetTechnoType()->ID ? pUnit->GetTechnoType()->ID : "<NULL>", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
                        if (pUnit->GetTechnoType()->WhatAmI() != AbstractType::AircraftType)
                        {
                            pUnit->ClickedAction(Action::Attack, selectedTarget, 0);
                            pUnit->QueueMission(Mission::Attack, true);
                        }
                    }

                    if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::InfantryType && abstract_cast<InfantryTypeClass*>(pUnit->GetTechnoType())->C4 || pUnit->HasAbility(Ability::C4)) // Tanya C4 case
                        pUnit->SetDestination(selectedTarget, false);
                }
                else
                {
 //                   if (pUnit->GetTechnoType()->Naval) Debug::Log("DEBUG: 1313 [%s] from [%s] [%s], line: %d = %d,%d \n", pUnit->GetTechnoType()->ID ? pUnit->GetTechnoType()->ID : "<NULL>", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
                    pUnit->ClickedAction(Action::Attack, selectedTarget, 0);
                }
            }
        }
        else
        {
            if (!noWaitLoop)
                pTeam->GuardAreaTimer.Start(16);
            Debug::Log("DEBUG: selectedTarget NOT found... JUMP TO NEXT Action!\n");
            // This action finished
            pTeam->StepCompleted = true;
            pTeam->CurrentScript->NextAction();
            Debug::Log("DEBUG: ScripType: [%s] [%s] next line will be: %d = %d,%d (End Team: New Target not found)\n", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
            return;
        }
    }
    else
    {
        // Team already have a Focused target
        bool validFocus = false;

        if (pFocus && pFocus->IsAlive
            && !pFocus->InLimbo
            && !pFocus->GetTechnoType()->Immune
            && ((pFocus->IsInAir() && unitWeaponsHaveAA) || (!pFocus->IsInAir() && unitWeaponsHaveAG))
            && !pFocus->Transporter
            && pFocus->IsOnMap
            && !pFocus->Absorbed
            && pFocus->Owner != pLeaderUnit->Owner //->FindByCountryIndex(pTechno->Owner->EnemyHouseIndex)
            && (!pLeaderUnit->Owner->IsAlliedWith(pFocus)) || (pLeaderUnit->Owner->IsAlliedWith(pFocus) && pFocus->IsMindControlled() && !pLeaderUnit->Owner->IsAlliedWith(pFocus->MindControlledBy)))
        {
            validFocus = true;
        }

        //Debug::Log("DEBUG: Updating Focus TechnoClass object in Team Focus.\n");
        bool bForceNextAction = false;

        for (auto pUnit = pTeam->FirstUnit; pUnit && !bForceNextAction; pUnit = pUnit->NextTeamMember)
        {
            if (validFocus)
            {
                if (pUnit->IsAlive && !pUnit->InLimbo && (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && abstract_cast<AircraftTypeClass*>(pUnit->GetTechnoType())->AirportBound))
                {
                    pUnit->SetTarget(pFocus);
                    if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0 && pUnit->Target != pFocus)
                    {
 //                       if (pUnit->GetTechnoType()->Naval) Debug::Log("DEBUG: 8888 [%s] from [%s] [%s], line: %d = %d,%d \n", pUnit->GetTechnoType()->ID ? pUnit->GetTechnoType()->ID : "<NULL>", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
                        pUnit->ClickedAction(Action::Attack, pFocus, 0);
                    }
                    if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0 && pUnit->GetHeight() <= 0 && pUnit->Target != pFocus)
                    {
                        //                        if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType)  Debug::Log("DEBUG: 7777 [%s] from [%s] [%s], line: %d = %d,%d \n", pUnit->GetTechnoType()->ID ? pUnit->GetTechnoType()->ID : "<NULL>", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
                        pUnit->ClickedAction(Action::Attack, pFocus, 0);
                    }
                }

                if (pUnit->GetTechnoType()->Naval && pUnit->GetTechnoType()->LandTargeting == 1 && pFocus->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
                {
                    // Naval units like Submarines are unable to target ground targets, ignore the attack
                    pUnit->CurrentTargets.Clear();
                    pUnit->SetTarget(nullptr);
                    pUnit->SetFocus(nullptr);
                    pUnit->SetDestination(nullptr, false);
                    pUnit->QueueMission(Mission::Guard, true);
                    //                    if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType) Debug::Log("DEBUG: 6666 [%s] from [%s] [%s], line: %d = %d,%d \n", pUnit->GetTechnoType()->ID ? pUnit->GetTechnoType()->ID : "<NULL>", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
                    pUnit->ClickedAction(Action::Attack, pFocus, 0);
                    bForceNextAction = true;
                    continue;
                }

                if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0 && pUnit->GetHeight() > 0 && pUnit->GetCurrentMission() != Mission::Attack && pUnit->GetCurrentMission() != Mission::Enter)
                {
                    //pUnit->QueueMission(Mission::Attack, false);
//                    if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType) Debug::Log("DEBUG: 1111 [%s] from [%s] [%s], line: %d = %d,%d \n", pUnit->GetTechnoType()->ID ? pUnit->GetTechnoType()->ID : "<NULL>",pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
                    if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType)
                    {
                        if (pUnit->Ammo > 0)
                            pUnit->ClickedAction(Action::Attack, pFocus, 0);
                        else
                            pUnit->Mission_Enter();
                    }
                }
                
                //Debug::Log("DEBUG: Unit Focus updated!\n");
//                if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType) Debug::Log("DEBUG: 3333 [%s] from [%s] [%s], line: %d = %d,%d \n", pUnit->GetTechnoType()->ID, pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
//                pUnit->ClickedAction(Action::Attack, pFocus, 0);
//                pUnit->SetTarget(pFocus);

                //pUnit->SetDestination(pFocus, false);
                if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType)
                {
                    if (pUnit->Ammo > 0)
                    {
                        //                        if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType) Debug::Log("DEBUG: 3131 [%s] from [%s] [%s], line: %d = %d,%d \n", pUnit->GetTechnoType()->ID, pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
                        pUnit->Mission_Attack();
                    }
                    else
                    {
                        //                       if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType) Debug::Log("DEBUG: 3232 [%s] from [%s] [%s], line: %d = %d,%d \n", pUnit->GetTechnoType()->ID, pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
                        pUnit->ForceMission(Mission::Enter);
                        pUnit->Mission_Enter();
                        pUnit->SetFocus(pUnit);
                        pUnit->LastTarget = nullptr;
                        pUnit->SetTarget(pUnit);
                    }
                }
               
                // Tanya C4 case
                if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::InfantryType
                    && abstract_cast<InfantryTypeClass*>(pUnit->GetTechnoType())->C4
                    || pUnit->HasAbility(Ability::C4))
                    pUnit->SetDestination(selectedTarget, false);
            }
            else
            {
                //Debug::Log("DEBUG: Clearing Team Focus!\n");
                pTeam->Focus = nullptr;
                pTeam->QueuedFocus = nullptr;
                //               if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType) Debug::Log("DEBUG: 5555 [%s] from [%s] [%s], line: %d = %d,%d \n", pUnit->GetTechnoType()->ID ? pUnit->GetTechnoType()->ID : "<NULL>", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
                pUnit->ClickedAction(Action::Attack, pFocus, 0);
                pUnit->CurrentTargets.Clear();
                pUnit->SetTarget(nullptr);
                pUnit->SetFocus(nullptr);
                pUnit->SetDestination(nullptr, true);
                pUnit->QueueMission(Mission::Guard, 0);
            }

        }

        if (bForceNextAction)
        {
            pTeam->StepCompleted = true;

            if (pTeam->CurrentScript->HasNextAction())
                pTeam->CurrentScript->NextAction();
            Debug::Log("DEBUG: ScripType: [%s] [%s] Jump to NEXT line: %d = %d,%d -> (End Team: Naval unable against ground target)\n", pTeam->Type->ID, pScript->Type->ID, pScript->idxCurrentLine, pScript->Type->ScriptActions[pScript->idxCurrentLine].Action, pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument);
            return;
        }
    }
}

TechnoClass* ScriptExt::GreatestThreat(TechnoClass *pTechno, int method, int calcThreatMode = 0, HouseClass* onlyTargetThisHouseEnemy = nullptr)
{
    TechnoClass *bestObject = nullptr;
    int bestVal = -1;
    int zone = -1;

    bool unitWeaponsHaveAA = false;
    bool unitWeaponsHaveAG = false;

    // Note: Replace these lines when I have access to Combat_Damage() method in YRpp if that is better
    WeaponTypeClass* WeaponType1 = pTechno->Veterancy.IsElite() ?
        pTechno->GetTechnoType()->EliteWeapon[0].WeaponType :
        pTechno->GetTechnoType()->Weapon[0].WeaponType;
    WeaponTypeClass* WeaponType2 = pTechno->Veterancy.IsElite() ?
        pTechno->GetTechnoType()->EliteWeapon[0].WeaponType :
        pTechno->GetTechnoType()->Weapon[0].WeaponType;
    WeaponTypeClass* WeaponType3 = WeaponType1;

    if (pTechno->GetTechnoType()->IsGattling)
    {
        WeaponType3 = pTechno->Veterancy.IsElite() ?
            pTechno->GetTechnoType()->EliteWeapon[pTechno->CurrentWeaponNumber].WeaponType :
            pTechno->GetTechnoType()->Weapon[pTechno->CurrentWeaponNumber].WeaponType;

        WeaponType1 = WeaponType3;
    }

    // Note: the TEAM LEADER is picked for this task, be careful with leadership values in your mod
    if ((WeaponType1 && WeaponType1->Projectile->AA) || (WeaponType2 && WeaponType2->Projectile->AA))
        unitWeaponsHaveAA = true;

    if ((WeaponType1 && WeaponType1->Projectile->AG) || (WeaponType2 && WeaponType2->Projectile->AG))
        unitWeaponsHaveAG = true;

    int combatDamage = -1;
    if (WeaponType1 && WeaponType1->Damage > 0)
        combatDamage = WeaponType1->Damage;
    if (WeaponType2 && WeaponType1->Damage > 0)
        combatDamage = WeaponType2->Damage;
    //Debug::Log("DEBUG: combatDamage: %d\n", combatDamage);

    // Get the max range available of all weapons
    int maxWeaponRange = -1;
    if (WeaponType1 && WeaponType1->Range > 0)
        maxWeaponRange = WeaponType1->Range;
    if (WeaponType2 && WeaponType1->Range > 0)
        maxWeaponRange = WeaponType2->Range;
    //Debug::Log("DEBUG: maxWeaponRange: %d\n", maxWeaponRange);

    // Generic method for targeting
    for (int i = 0; i < TechnoClass::Array->Count; i++)
    {
        auto object = TechnoClass::Array->GetItem(i);

        if (!object)
            continue;

        //Debug::Log("DEBUG: TechnoClass::Array[%d] -> Check if [%s] is targetable for [%s].\n", i, object->GetTechnoType()->ID, pTechno->GetTechnoType()->ID);
        if (object->IsInAir() && !unitWeaponsHaveAA)
            continue;
        if (!object->IsInAir() && !unitWeaponsHaveAG) // I don't know if underground is a special case
            continue;

        // Don't pick underground units
        if (object->InWhichLayer() == Layer::Underground)
            continue;

        // Stealth ground unit check
        if (object->CloakState == CloakState::Cloaked && !object->GetTechnoType()->Naval)
            continue;

        // Submarines aren't a valid target
        if (object->CloakState == CloakState::Cloaked
            && object->GetTechnoType()->Underwater
            && (pTechno->GetTechnoType()->NavalTargeting == 0
                || pTechno->GetTechnoType()->NavalTargeting == 6))
            continue;

        // Land not OK for the Naval unit
        if (pTechno->GetTechnoType()->Naval
            && pTechno->GetTechnoType()->LandTargeting == 1
            && object->GetCell()->LandType != LandType::Water)
            continue;

        // OnlyTargetHouseEnemy forces targets of a specific (hated) house
        if (onlyTargetThisHouseEnemy && object->Owner != onlyTargetThisHouseEnemy)
            continue;

        if (object != pTechno
            && object->IsAlive
            && !object->InLimbo
            && !object->GetTechnoType()->Immune
            && !object->Transporter
            && object->IsOnMap
            && !object->Absorbed
            && object->Owner != pTechno->Owner
            && (!pTechno->Owner->IsAlliedWith(object))
            || (pTechno->Owner->IsAlliedWith(object)
                && object->IsMindControlled()
                && !pTechno->Owner->IsAlliedWith(object->MindControlledBy)))
        {
            int value = 0;
            //Debug::Log("DEBUG: Possible candidate!!! Go to EvaluateObjectWithMask check.\n");
            if (EvaluateObjectWithMask(object, method))
            {
                CellStruct newCell;
                newCell.X = object->Location.X;
                newCell.Y = object->Location.Y;

                bool isGoodTarget = false;
                if (calcThreatMode == 0)
                {
                    // Threat affected by distance [recommended default]
                    // Is this object very far? then LESS threat against pTechno. More closer? More threat for pTechno
                    int objectThreatValue = object->GetTechnoType()->ThreatPosed;

                    if (object->GetTechnoType()->SpecialThreatValue > 0)
                    {
                        auto const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
                        objectThreatValue = object->GetTechnoType()->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
                    }

                    value = objectThreatValue * 200000 / (pTechno->DistanceFrom(object) + 1);

                    if (value > bestVal || bestVal < 0)
                        isGoodTarget = true;
                }
                else
                {
                    if (calcThreatMode == 1)
                    {
                        // Threat affected by distance
                        // Is this object very far? then MORE threat against pTechno. More closer? Less threat for pTechno
                        int objectThreatValue = object->GetTechnoType()->ThreatPosed;

                        if (object->GetTechnoType()->SpecialThreatValue > 0)
                        {
                            auto const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
                            objectThreatValue = object->GetTechnoType()->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
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
                            // Is this object very far? then LESS threat against pTechno. More closer? More threat for pTechno
                            value = pTechno->DistanceFrom(object);

                            if (value < bestVal || bestVal < 0)
                                isGoodTarget = true;
                        }
                        else
                        {
                            if (calcThreatMode == 3)
                            {
                                // Selection affected by distance
                                // Is this object very far? then MORE threat against pTechno. More closer? Less threat for pTechno
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

bool ScriptExt::EvaluateObjectWithMask(TechnoClass *pTechno, int mask)
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
                || (pTypeBuilding && pTypeBuilding->WhatAmI() == AbstractType::BuildingType
                    && pTypeBuilding->UndeploysInto
                    && !pTypeBuilding->BaseNormal && pTypeBuilding->Unsellable)
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
                || (pTypeBuilding
                    && (pTypeBuilding->Artillary || pTypeBuilding->TickTank || pTypeBuilding->ICBMLauncher || pTypeBuilding->SensorArray))))
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
                    && (pTypeBuilding->Refinery || pTypeBuilding->OrePurifier || pTypeBuilding->ResourceGatherer))))
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
                    && (pTypeBuilding->Refinery || pTypeBuilding->ResourceGatherer))))
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
            pTechnoType->EliteWeapon[0].WeaponType :
            pTechnoType->Weapon[0].WeaponType;
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
        // Naval Objects
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
                && (pTypeBuilding->Radar || pTypeBuilding->SpySat)))
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

        if (pBuildingExt = BuildingTypeExt::ExtMap.Find(static_cast<BuildingTypeClass*>(pTypeBuilding)))
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

    default:
        break;
    }

    // The possible target doesn't fit in te masks
    return false;
}
