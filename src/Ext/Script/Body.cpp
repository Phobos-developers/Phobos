#include "Body.h"
#include "../Techno/Body.h"
#include "../BuildingType/Body.h"

template<> const DWORD Extension<ScriptClass>::Canary = 0x3B3B3B3B;
ScriptExt::ExtContainer ScriptExt::ExtMap;

// =============================
// load / save

void ScriptExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	// Nothing yet
}

void ScriptExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	// Nothing yet
}

// =============================
// container

ScriptExt::ExtContainer::ExtContainer() : Container("ScriptClass")
{
}

ScriptExt::ExtContainer::~ExtContainer() = default;

void ScriptExt::ProcessAction(TeamClass* pTeam)
{
	const int& action = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Action;

	switch (action)
	{
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
		ScriptExt::UnsetConditionalJumpVariable(pTeam);
		break;
	case 75:
		ScriptExt::SetConditionalJumpCondition(pTeam, -1);
			break;
	case 76:
		ScriptExt::SetConditionalCountCondition(pTeam, -1);
			break;
	case 77:
		ScriptExt::SetKillsLimitComparator(pTeam, -1);
			break;
	case 78:
			ScriptExt::SetAbortActionAfterSuccessKill(pTeam, -1);
			break;
	case 79:
		ScriptExt::ConditionalJumpIfFalse(pTeam, -1);
			break;
	case 80:
		ScriptExt::ConditionalJumpIfTrue(pTeam, -1);
			break;
	default:
		// Do nothing because or it is a wrong Action number or it is an Ares/YR action...
		//Debug::Log("[%s] [%s] %d = %d,%d\n", pTeam->Type->ID, pScriptType->ID, pScript->idxCurrentLine, currentLineAction->Action, currentLineAction->Argument);
		break;
	}
}

void ScriptExt::ExecuteTimedAreaGuardAction(TeamClass* pTeam)
{
	auto pScript = pTeam->CurrentScript;
	auto pScriptType = pScript->Type;

	if (pTeam->GuardAreaTimer.TimeLeft == 0 && !pTeam->GuardAreaTimer.InProgress())
	{
		auto pUnit = pTeam->FirstUnit;

		pUnit->QueueMission(Mission::Area_Guard, true);
		while (pUnit->NextTeamMember)
		{
			pUnit = pUnit->NextTeamMember;
			pUnit->QueueMission(Mission::Area_Guard, true);
		}
		pTeam->GuardAreaTimer.Start(15 * pScriptType->ScriptActions[pScript->idxCurrentLine].Argument);
	}
	/*else {
		Debug::Log("[%s] [%s] %d = %d,%d (Countdown: %d)\n", pTeam->Type->ID, pScriptType->ID, pScript->idxCurrentLine, currentLineAction->Action, currentLineAction->Argument, pTeam->GuardAreaTimer.GetTimeLeft());
	}
	*/

	if (pTeam->GuardAreaTimer.Completed())
	{
		pTeam->GuardAreaTimer.Stop(); // Needed
		pTeam->StepCompleted = true;
	}
}

void ScriptExt::LoadIntoTransports(TeamClass* pTeam)
{
	DynamicVectorClass<FootClass*> transports;

	auto pUnit = pTeam->FirstUnit;
	auto pUnitType = pUnit->GetTechnoType();
	if (pUnitType->Passengers > 0
		&& pUnit->Passengers.NumPassengers < pUnitType->Passengers
		&& pUnit->Passengers.GetTotalSize() < pUnitType->Passengers)
	{
		transports.AddItem(pUnit);
	}
	while (pUnit->NextTeamMember)
	{
		pUnit = pUnit->NextTeamMember;
		pUnitType = pUnit->GetTechnoType();
		if (pUnitType->Passengers > 0
			&& pUnit->Passengers.NumPassengers < pUnitType->Passengers
			&& pUnit->Passengers.GetTotalSize() < pUnitType->Passengers)
		{
			transports.AddItem(pUnit);
		}
	}
	// We got all the transports.

	// Now add units into transports
	for (auto pTransport : transports)
	{
		pUnit = pTeam->FirstUnit;
		auto pTransprotType = pTransport->GetTechnoType();
		do
		{
			pUnitType = pUnit->GetTechnoType();
			if (!(pTransport == pUnit
				|| pUnitType->WhatAmI() == AbstractType::AircraftType
				|| pUnit->InLimbo
				|| pUnitType->ConsideredAircraft
				|| pUnit->Health <= 0))
			{
				if (pUnit->GetTechnoType()->Size > 0
					&& pUnitType->Size <= pTransprotType->SizeLimit
					&& pUnitType->Size <= pTransprotType->Passengers - pTransport->Passengers.GetTotalSize())
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
			pUnit = pUnit->NextTeamMember;
		}
		while (pUnit);
	}

	pUnit = pTeam->FirstUnit;
	do
	{
		if (pUnit->GetCurrentMission() == Mission::Enter)
			return;
		pUnit = pUnit->NextTeamMember;
	}
	while (pUnit);

	// This action finished
	if (pTeam->CurrentScript->HasNextAction())
		pTeam->CurrentScript->idxCurrentLine += 1;
	pTeam->StepCompleted = true;
}

void ScriptExt::WaitUntillFullAmmoAction(TeamClass* pTeam)
{
	auto pUnit = pTeam->FirstUnit;

	do
	{
		if (pUnit && !pUnit->InLimbo && pUnit->Health > 0)
		{
			if (pUnit->GetTechnoType()->Ammo > 0 && pUnit->Ammo < pUnit->GetTechnoType()->Ammo)
			{
				// If an aircraft object have AirportBound it must be evaluated
				if (pUnit->WhatAmI() == AbstractType::Aircraft)
				{
					auto pAircraft = static_cast<AircraftTypeClass*>(pUnit->GetTechnoType());
					if (pAircraft->AirportBound)
					{
						// Reset last target, at long term battles this prevented the aircraft to pick a new target (rare vanilla YR bug)
						pUnit->SetTarget(nullptr);
						pUnit->LastTarget = nullptr;
						// Fix YR bug (when returns from the last attack the aircraft switch in loop between Mission::Enter & Mission::Guard, making it impossible to land in the dock)
						if (pUnit->IsInAir() && pUnit->CurrentMission != Mission::Enter)
							pUnit->QueueMission(Mission::Enter, true);
						return;
					}
				}
				else if (pUnit->GetTechnoType()->Reload != 0)
				{ // Don't skip units that can reload themselves
					return;
				}
			}
		}
		pUnit = pUnit->NextTeamMember;
	}
	while (pUnit);

	// This action finished
	/*if (pTeam->CurrentScript->HasNextAction())
	{
		pTeam->CurrentScript->idxCurrentLine += 1;
	}*/
	pTeam->StepCompleted = true;
}

bool ScriptExt::EvaluateObjectWithMask(TechnoClass *pTechno, int mask, int attackAITargetType = -1, int idxAITargetTypeItem = -1, TechnoClass *pTeamLeader = nullptr)
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
	auto const& NeutralTechBuildings = RulesClass::Instance->NeutralTechBuildings;
	int nSuperWeapons = 0;
	double distanceToTarget = 0;
	TechnoClass* pTarget = nullptr;

	// Special case: validate target if is part of a technos list in [AITargetType] section
	if (attackAITargetType >= 0 && RulesExt::Global()->AITargetTypeLists.Count > 0)
	{
		DynamicVectorClass<TechnoTypeClass*> objectsList = RulesExt::Global()->AITargetTypeLists.GetItem(attackAITargetType);

		if (idxAITargetTypeItem > 0)
		{
			if (objectsList.GetItem(idxAITargetTypeItem) == pTechnoType)
				return true;

			return false;
		}
		else
		{
			for (int i = 0; i < objectsList.Count; i++)
			{
				if (objectsList.GetItem(i) == pTechnoType)
				{
					return true;
				}
			}
			return false;
		}
	}

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

		// Vehicle, Aircraft, Deployed vehicle into structure
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::UnitType
				|| (pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& (pTypeBuilding
						&& (pTypeBuilding->Artillary
							|| pTypeBuilding->TickTank
							|| pTypeBuilding->ICBMLauncher
							|| pTypeBuilding->SensorArray)))
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
		pTarget = abstract_cast<TechnoClass *>(pTechno->Target);

		if (pTeamLeader && pTarget)
		{
			// The possible Target is aiming against me? Revenge!
			if (abstract_cast<TechnoClass *>(pTechno->Target)->Owner == pTeamLeader->Owner)
			{
				return true;
			}

			for (int i = 0; i < pTechno->CurrentTargets.Count; i++)
			{
				if (abstract_cast<TechnoClass *>(pTechno->CurrentTargets.GetItem(i))->Owner == pTeamLeader->Owner)
				{
					return true;
				}
			}

			// Note: Replace these lines when I have access to Combat_Damage() method in YRpp if that is better
			WeaponType1 = pTechno->Veterancy.IsElite() ?
				pTechnoType->EliteWeapon[0].WeaponType :
				pTechnoType->Weapon[0].WeaponType;
			WeaponType2 = pTechno->Veterancy.IsElite() ?
				pTechnoType->EliteWeapon[1].WeaponType :
				pTechnoType->Weapon[1].WeaponType;
			WeaponType3 = WeaponType1;
			if (pTechnoType->IsGattling)
			{
				WeaponType3 = pTechno->Veterancy.IsElite() ?
					pTechnoType->EliteWeapon[pTechno->CurrentWeaponNumber].WeaponType :
					pTechnoType->Weapon[pTechno->CurrentWeaponNumber].WeaponType;

				WeaponType1 = WeaponType3;
			}

			// Then check if this possible target is too near of the Team Leader
			distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / 256.0;

			if (!pTechno->Owner->IsNeutral()
				&& ((WeaponType1 && distanceToTarget <= (WeaponType1->Range / 256.0 * 4.0))
					|| (WeaponType2 && distanceToTarget <= (WeaponType2->Range / 256.0 * 4.0))
					|| (pTeamLeader->GetTechnoType()->GuardRange > 0
						&& distanceToTarget <= (pTeamLeader->GetTechnoType()->GuardRange / 256.0 * 2.0))))
			{
				return true;
			}
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

	case 11:
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		// Civilian Tech
		if (pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& NeutralTechBuildings.Items)
		{
			for (int i = 0; i < NeutralTechBuildings.Count; i++)
			{
				auto pTechObject = NeutralTechBuildings.GetItem(i);
				if (pTechObject->ID == pTechno->get_ID())
				{
					return true;
				}
			}
		}

		// Other cases of civilian Tech Structures
		if (pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Unsellable
			&& pTypeBuilding->Capturable
			&& pTypeBuilding->TechLevel < 0
			&& pTypeBuilding->NeedsEngineer
			&& !pTypeBuilding->BridgeRepairHut)
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
					&& (pTypeBuilding->Refinery
						|| pTypeBuilding->ResourceGatherer))))
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
			pTechnoType->EliteWeapon[1].WeaponType :
			pTechnoType->Weapon[1].WeaponType;
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
		// Aircraft and Air Unit
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::AircraftType || pTechnoType->JumpJet || pTechno->IsInAir()))
		{
			return true;
		}
		break;

	case 15:
		// Naval Unit & Structure
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
		// Economy: Harvester, Refinery or Resource helper
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& ((pTechnoType->WhatAmI() == AbstractType::UnitType
				&& (abstract_cast<UnitTypeClass *>(pTechnoType)->Harvester
					|| abstract_cast<UnitTypeClass *>(pTechnoType)->ResourceGatherer))
				|| (pTypeBuilding
					&& pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& (pTypeBuilding->Refinery
						|| pTypeBuilding->OrePurifier
						|| pTypeBuilding->ResourceGatherer))))
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
				&& (pTypeBuilding->Radar
					|| pTypeBuilding->SpySat)))
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
		pBuildingExt = BuildingTypeExt::ExtMap.Find(static_cast<BuildingTypeClass*>(pTypeBuilding));

		if (pBuildingExt)
			nSuperWeapons = pBuildingExt->SuperWeapons.Count;

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
		// Inhibitor
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);
		pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& (pTypeTechnoExt && pTypeTechnoExt->InhibitorRange > 0))
		{
			return true;
		}
		break;

	case 31:
		// Naval Unit
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() != AbstractType::BuildingType
			&& (pTechnoType->Naval
				|| pTechno->GetCell()->LandType == LandType::Water))
		{
			return true;
		}
		break;

	case 32:
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		// Any non-building unit
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() != AbstractType::BuildingType
				|| (pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& pTypeBuilding
					&& (pTypeBuilding->Artillary
						|| pTypeBuilding->TickTank
						|| pTypeBuilding->ICBMLauncher
						|| pTypeBuilding->SensorArray
						|| pTypeBuilding->ResourceGatherer))))
		{
			return true;
		}
		break;

	case 33:
		pTypeBuilding = abstract_cast<BuildingTypeClass *>(pTechnoType);

		// Capturable Structure or Repair Hut
		if (pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Capturable
			|| (pTypeBuilding->BridgeRepairHut
				&& pTypeBuilding->Repairable))
		{
			return true;
		}
		break;

	case 34:
		if (pTeamLeader)
		{
			// Inside the Area Guard of the Team Leader
			distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / 256.0; // Caution, DistanceFrom() return leptons

			if (!pTechno->Owner->IsNeutral()
				&& (pTeamLeader->GetTechnoType()->GuardRange > 0
					&& distanceToTarget <= ((pTeamLeader->GetTechnoType()->GuardRange / 256.0) * 2.0)))
			{
				return true;
			}
		}
		break;

	default:
		break;
	}

	// The possible target doesn't fit in te masks
	return false;
}

// 1-based like the original '6,n' (so the first script line is n=1, I hate that confusing 1-based system)
void ScriptExt::ConditionalJumpIfTrue(TeamClass* pTeam, int newScriptLine = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	int scriptArgument = newScriptLine;
	if (scriptArgument < 1)
	{
		auto pScript = pTeam->CurrentScript;
		scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument;
	}
	
	if (scriptArgument <= 0)
	{
		scriptArgument = 1; // if by mistake you put as first line=0 this corrects it because for WW/EALA this script argument is 1-based
	}
	
	if (pTeamData->ConditionalEvaluationType >= 0)
	{
		if (pTeamData->ConditionalJumpEvaluation)
		{
			Debug::Log("DEBUG: [%s] [%s] %d = %d,%d - Conditional Jump was a success! - New Line: %d = %d,%d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->idxCurrentLine, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Argument, scriptArgument - 1, pTeam->CurrentScript->Type->ScriptActions[scriptArgument - 1].Action, pTeam->CurrentScript->Type->ScriptActions[scriptArgument - 1].Argument);

			// Start conditional jump!
			// This is magic: for example, for jumping into line 0 of the script list you have to point to the "-1" line so in the next AI iteration the current line will be increased by 1 and then it will point to the desired line 0
			pTeam->CurrentScript->idxCurrentLine = scriptArgument - 2;

			// Cleaning Conditional Jump related variables
			pTeamData->ConditionalJumpEvaluation = false;
			pTeamData->ConditionalEvaluationType = -1; // Disabled the Conditional jump (this was a 1-time-use)
			pTeamData->ConditionalComparatorType = 3; // >=
			pTeamData->KillsCounter = 0;
			pTeamData->KillsCountLimit = 1;
			pTeamData->AbortActionAfterKilling = false;
		}
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

// 1-based like the original '6,n' (so the first script line is n=1, I hate that confusing 1-based system)
void ScriptExt::ConditionalJumpIfFalse(TeamClass* pTeam, int newScriptLine = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	int scriptArgument = newScriptLine;
	if (scriptArgument < 1)
	{
		auto pScript = pTeam->CurrentScript;
		scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument;
	}
	
	if (scriptArgument <= 0)
	{
		scriptArgument = 1; // if by mistake you put as first line=0 this corrects it because for WW/EALA this script argument is 1-based
	}
	
	if (pTeamData->ConditionalEvaluationType >= 0)
	{
		if (!pTeamData->ConditionalJumpEvaluation)
		{
			Debug::Log("DEBUG: [%s] [%s] %d = %d,%d - Conditional Jump was a success! - New Line: %d = %d,%d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->idxCurrentLine, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->idxCurrentLine].Argument, scriptArgument - 1, pTeam->CurrentScript->Type->ScriptActions[scriptArgument - 1].Action, pTeam->CurrentScript->Type->ScriptActions[scriptArgument - 1].Argument);

			// Start conditional jump!
			// This is magic: for example, for jumping into line 0 of the script list you have to point to the "-1" line so in the next AI iteration the current line will be increased by 1 and then it will point to the desired line 0
			pTeam->CurrentScript->idxCurrentLine = scriptArgument - 2;

			// Cleaning Conditional Jump related variables
			pTeamData->ConditionalJumpEvaluation = false;
			pTeamData->ConditionalEvaluationType = -1; // Disabled the Conditional jump (this was a 1-time-use)
			pTeamData->ConditionalComparatorType = 3; // >=
			pTeamData->KillsCounter = 0;
			pTeamData->KillsCountLimit = 1;
			pTeamData->AbortActionAfterKilling = false;
		}
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

// Enables the conditional jump logic against one type of object evaluation (currently +30 types)
void ScriptExt::SetConditionalJumpCondition(TeamClass* pTeam, int evaluationType = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	int scriptArgument = evaluationType;
	if (scriptArgument < 0)
	{
		auto pScript = pTeam->CurrentScript;
		scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument;
	}

	if (pTeamData->ConditionalEvaluationType != scriptArgument && scriptArgument >= 0)
	{
		pTeamData->ConditionalEvaluationType = scriptArgument;
		pTeamData->ConditionalJumpEvaluation = false; // Reset conditional jumping value
		
		// Reset the kills evaluation
		pTeamData->ConditionalComparatorType = 3;
		pTeamData->KillsCounter = 0;
		pTeamData->KillsCountLimit = 1;
		pTeamData->AbortActionAfterKilling = false;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

// Disables the conditional jump logic
void ScriptExt::UnsetConditionalJumpVariable(TeamClass* pTeam)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	pTeamData->ConditionalJumpEvaluation = false;
	pTeamData->ConditionalEvaluationType = -1; // Disabled the Conditional jump
	pTeamData->ConditionalComparatorType = 3; // >=
	pTeamData->KillsCounter = 0;
	pTeamData->KillsCountLimit = 1;
	pTeamData->AbortActionAfterKilling = false;
	
	// This action finished
	pTeam->StepCompleted = true;
}

// Sets the comparator from: <, <=, ==, >=, >, !=
void ScriptExt::SetConditionalCountCondition(TeamClass* pTeam, int comparatorType = 3)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	int scriptArgument = comparatorType;
	if (scriptArgument < 0)
	{
		auto pScript = pTeam->CurrentScript;
		scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument;
	}

	if (scriptArgument >= 0)
	{
		if (pTeamData->ConditionalComparatorType != scriptArgument)
		{
			// Enabling new count
			pTeamData->ConditionalComparatorType = scriptArgument; // Possible values: 0 -> '<', 1 -> '<=', 2 -> '=', 3 -> '>=', 4 -> '>', 5 -> '!='
			pTeamData->ConditionalJumpEvaluation = false; // Reset conditional jumping value
			pTeamData->KillsCounter = 0; // If the new evaluation is different then the current kills count should start again
		}
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

// Sets the value that will be used for comparation with the current kills count
void ScriptExt::SetKillsLimitComparator(TeamClass* pTeam, int newLimit = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	int scriptArgument = newLimit;
	if (scriptArgument < 0)
	{
		auto pScript = pTeam->CurrentScript;
		scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument;
	}

	if (scriptArgument >= 0)
	{
		pTeamData->KillsCountLimit = scriptArgument;
	}
	else
	{
		pTeamData->KillsCountLimit = 1; // Is the default value
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

// Enable / Disable this modifier feature.
// Only used by the Conditional Jump code: Don't continue the current action if the team reached the kills goal (true value).
void ScriptExt::SetAbortActionAfterSuccessKill(TeamClass* pTeam, int enable = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	int scriptArgument = enable;
	if (scriptArgument < 0)
	{
		auto pScript = pTeam->CurrentScript;
		scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument;
	}

	if (scriptArgument == 1)
	{
		pTeamData->AbortActionAfterKilling = true;
	}
	else
	{
		pTeamData->AbortActionAfterKilling = false;
	}
	
	// This action finished
	pTeam->StepCompleted = true;

	return;
}

