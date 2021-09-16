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

void ScriptExt::UnsetConditionalJumpVariable(TeamClass* pTeam)
{
	// This team has no units! END
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
	{
		pTeamData->ConditionalJumpEvaluation = false;
		pTeamData->ConditionalEvaluationType = -1;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ConditionalJumpIfTrue(TeamClass* pTeam)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument;

	if (pTeamData->ConditionalEvaluationType != -1)
	{
		if (pTeamData->ConditionalJumpEvaluation)
		{
			// Start conditional jump!
			pTeamData->ConditionalEvaluationType = -1;
			pTeamData->ConditionalJumpEvaluation = false;

			// Ready for jumping to the new line of the script
			pTeam->CurrentScript->idxCurrentLine = scriptArgument - 1;
			pTeam->StepCompleted = true;
			return;
		}

	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJumpIfFalse(TeamClass* pTeam)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}
	
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument;

	if (pTeamData->ConditionalEvaluationType != -1)
	{
		if (!pTeamData->ConditionalJumpEvaluation)
		{
			// Start conditional jump!
			pTeamData->ConditionalEvaluationType = -1;

			// Ready for jumping to the new line of the script
			pTeam->CurrentScript->idxCurrentLine = scriptArgument - 1;
			pTeam->StepCompleted = true;
			return;
		}

	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::SetConditionalJumpCondition(TeamClass* pTeam)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = pScript->Type->ScriptActions[pScript->idxCurrentLine].Argument;

	if (pTeamData->ConditionalEvaluationType != scriptArgument)
	{
		pTeamData->ConditionalEvaluationType = scriptArgument;
		pTeamData->ConditionalJumpEvaluation = false;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

/*


[DRAFT] UNCONDITIONAL SCRIPT ACTIONS DESCRIPTION:

[x]• 1 boolean variable in TeamExt for saving the evaluation. For example:
bool ConditionalJumpEvaluation = false;

[x]• 1 integer variable in TeamExt for saving the type of evaluation. For example:
int ConditionalEvaluationType = -1; // "-1" means no evaluation when a unit is killed by a Team member, maybe I should use the value 0 instead...

• When a unit is killed by a unit that belongs to a Team it will check if "ConditionalEvaluationType" != -1 so in that case it could be evaluated TRUE / FALSE and stored in "ConditionalJumpEvaluation".
If "ConditionalEvaluationType" is -1 (or 0 ?) then the evaluation process is skipped.

• Functions:
[x]-> Unset conditional variable (set to 0 / false). Self-explanatory.

[x]-> Jump to line "nn" (0-based) if conditional variable "ConditionalJumpEvaluation" is TRUE.
When the conditional jump will start the variable "ConditionalEvaluationType" is reset to -1 & the variable "ConditionalJumpEvaluation" is reset to 0 / false.
if the variable is FALSE then skip jump and go to the next script line.

[x]-> Jump to line "nn" (0-based) if conditional variable "ConditionalJumpEvaluation" is FALSE.
When the conditional jump will start the variable "ConditionalEvaluationType" is reset to -1.
if the variable is TRUE then skip jump and go to the next script line.

-> Set conditional jump variable "ConditionalJumpEvaluation" to 1 / true if the killed object is in the specified "nn" list in rulesmd.ini > [AITargetType] section.

-> Set conditional jump variable "ConditionalJumpEvaluation" to 1 / true if the killed object is part of one of the specified list of triggers/teams/taskforces (not yet evaluated this possible function if is viable or not).

[x]-> Set a "nn" type of evaluation in "ConditionalEvaluationType" like the next ones:
Case 1: Just enable it to value 1 / True. (unconditional jump like the classic "6,nn+1".
Case 2: For ANY successful kill from the team members. No extra evaluations.
Case 3: For a destroyed BUILDING by the team.
Case 4: For a ground object kill (infantry, landed aircraft, vehicles, structures).
Case 5: For a ground vehicle kill
Case 6: For a soldier kill
Case 7: For an air unit kill
Case 8: For a naval object kill (Structures, units)
Case 8: For a naval unit kill (not submarines)
Case 9: For a submerged unit kill
Case 10: For a stealth unit kill
Case 11: For a mind controller kill
Case 12: For a civilian structure kill
Case 13: For a civilian unit kill
Case 14: For a harvester kill
Case 15: For a "Economy" object kill
Case 16: For a Refinery kill
Case 17: For a Factory kill
...
...
...

I forgot anything?

*/
