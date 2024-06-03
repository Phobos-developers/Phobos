#include "Body.h"

#include <AircraftClass.h>
#include <TerrainClass.h>

#define TICKS_PER_SECOND    15
#define TICKS_PER_MINUTE    (TICKS_PER_SECOND * 60)
#define TICKS_PER_HOUR      (TICKS_PER_MINUTE * 60)

///
/// Advanced AI
///	Credits to Rampastring
///

bool HouseExt::AdvAI_House_Search_For_Next_Expansion_Point(HouseClass* pHouse)
{
	const auto ext = ExtMap.Find(pHouse);

	if (ext->NextExpansionPointLocation.X != 0 && ext->NextExpansionPointLocation.Y != 0)
	{
		return false;
	}

	// Fetch our first ConYard.
	BuildingClass* firstBuilding = nullptr;
	for (const auto pBuilding : *BuildingClass::Array)
	{
		if (pBuilding->IsAlive && !pBuilding->InLimbo && pBuilding->Owner == pHouse && pBuilding->Type->Factory == AbstractType::BuildingType)
		{
			firstBuilding = pBuilding;
			break;
		}
	}

	if (firstBuilding == nullptr)
	{
		return false;
	}

	// Scan through terrain objects that spawn Tiberium, pick the closest one that does not have a refinery near it yet
	double nearestDistance = std::numeric_limits<double>::max();
	CellStruct target = CellStruct();

	for (const auto pTerrain : *TerrainClass::Array)
	{
		if (pTerrain->IsAlive && !pTerrain->InLimbo && pTerrain->Type->SpawnsTiberium)
		{
			CellStruct terrainCell = pTerrain->GetMapCoords();

			// Fetch the cell of the terrain. If the cell has overlay on it,
			// we should not expand towards it. This allows a way for mappers to mark
			// that the AI should not expand towards specific Tiberium trees.
			const CellClass& cell = (*MapClass::Instance)[terrainCell];
			if (cell.OverlayTypeIndex != -1)
			{
				continue;
			}

			bool found = false;
			for (const auto pBuilding : *BuildingClass::Array)
			{
				if (!pBuilding->IsAlive || pBuilding->InLimbo || !pBuilding->Type->Refinery)
				{
					continue;
				}

				// Check if any existing AI refinery has been assigned for this expansion point yet.
				// If yes, consider it occupied, but only if it is ours.
				const auto buildingExt = BuildingExt::ExtMap.Find(pBuilding);
				if (pBuilding->Owner == pHouse && buildingExt->AssignedExpansionPoint == terrainCell)
				{
					found = true;
					break;
				}

				// Not all refineries have an assigned expansion point. For example, initial 
				// base refineries and human players' refineries do not.
				// For these refineries, we rely on a distance check.
				const double dist = pBuilding->GetMapCoords().DistanceFrom(terrainCell);
				if (dist < 15)
				{
					found = true;
					break;
				}
			}

			if (found)
				continue; // Someone is already occupying this Tiberium tree

			const double distance = firstBuilding->GetMapCoords().DistanceFrom(pTerrain->GetMapCoords());
			if (distance < nearestDistance)
			{
				// Don't expand super far.
				if (distance / 256 < RulesExt::Global()->AdvancedAIMaxExpansionDistance)
				{
					nearestDistance = distance;
					target = pTerrain->GetMapCoords();
				}
			}
		}
	}

	if (target.X == 0 || target.Y == 0)
	{
		// We couldn't find anywhere to expand towards
		return false;
	}

	ext->NextExpansionPointLocation = target;

	return true;
}


bool HouseExt::AdvAI_Can_Build_Building(HouseClass* pHouse, BuildingTypeClass* pBuildingType, bool checkPrereqs)
{
	if (BuildingTypeClass::Array->FindItemIndex(pBuildingType) != pBuildingType->ArrayIndex ||
		pBuildingType->What_Am_I() != AbstractType::BuildingType)
	{
		Debug::FatalErrorAndExit("Invalid BuildingTypeClass pointer in AdvAI_Can_Build_Building!!!");
	}

	// Debug::Log("Checking if AI %d can build %s. ", house->ArrayIndex, int->Name);

	if (pBuildingType->TechLevel > pHouse->TechLevel)
	{
		// Debug::Log("Result: false (TechLevel)\n");
		return false;
	}

	if (!pBuildingType->AIBuildThis)
	{
		// Debug::Log("Result: false (AIBuildThis)\n");
		return false;
	}

	if ((pBuildingType->OwnerFlags & (1 << pHouse->Type->ArrayIndex)) != (1 << pHouse->Type->ArrayIndex))
	{
		// Debug::Log("Result: false (Ownable)\n");
		return false;
	}

	const auto buildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuildingType);

	if (checkPrereqs && !buildingTypeExt->IsAdvancedAIIgnoresPrerequisites)
	{
		for (const auto prerequisite : pBuildingType->Prerequisite)
		{
			if (prerequisite < 0)
			{
				// If we want a refinery, check if we have a slave miner unit
				if (prerequisite == -6 && pHouse->ActiveUnitTypes.GetItemCount(RulesClass::Instance->PrerequisiteProcAlternate->ArrayIndex) > 0)
					continue;

				TypeList<int>* prerequisites;
				switch (prerequisite)
				{
				case -6:
					prerequisites = &RulesClass::Instance->PrerequisiteProc;
					break;
				case -5:
					prerequisites = &RulesClass::Instance->PrerequisiteTech;
					break;
				case -4:
					prerequisites = &RulesClass::Instance->PrerequisiteRadar;
					break;
				case -3:
					prerequisites = &RulesClass::Instance->PrerequisiteBarracks;
					break;
				case -2:
					prerequisites = &RulesClass::Instance->PrerequisiteFactory;
					break;
				case -1:
					prerequisites = &RulesClass::Instance->PrerequisitePower;
					break;
				default:
					Debug::FatalErrorAndExit("Invalid prerequisite %d in AdvAI_Can_Build_Building!!!", prerequisite);
				}

				for (const auto prerequisiteIndex : *prerequisites)
				{
					if (pHouse->ActiveBuildingTypes.GetItemCount(prerequisiteIndex) > 0)
					{
						goto prerequisiteFound;
					}
				}

				return false;

			prerequisiteFound:
				continue;

			}
			else if (prerequisite >= 0 && pHouse->ActiveBuildingTypes.GetItemCount(prerequisite) == 0)
			{
				//Debug::Log("Result: false, building: (Prerequisite: %d %s)\n", prerequisite, (*BuildingTypeClass::Array)[prerequisite]->Name);
				return false;
			}
		}
	}

	// If this is an upgrade, do we have a building we could upgrade with it?
	if (pBuildingType->PowersUpBuilding[0] != '\0')
	{
		const BuildingTypeClass* base = BuildingTypeClass::FindOrMake(pBuildingType->PowersUpBuilding);

		if (pHouse->ActiveBuildingTypes.GetItemCount(base->ArrayIndex) == 0)
		{
			// Debug::Log("Result: false (no upgradeable buildings)\n");
			return false;
		}

		bool found = false;

		// Scan through the buildings...
		for (const auto pBuilding : *BuildingClass::Array)
		{
			if (!pBuilding->IsAlive ||
				pBuilding->InLimbo ||
				pBuilding->Type != base ||
				pBuilding->Owner != pHouse)
			{
				continue;
			}

			if (pBuilding->UpgradeLevel >= base->Upgrades)
			{
				continue;
			}

			found = true;
		}

		if (!found)
		{
			// Debug::Log("Result: false (no upgradeable building found in scan)\n");
			return false;
		}
	}

	// Debug::Log("Result: true\n");
	return true;
}


bool HouseExt::AdvAI_Is_Recently_Attacked(HouseClass* pHouse)
{
	return pHouse->LATime + TICKS_PER_MINUTE > Unsorted::CurrentFrame;
}


/**
 *  Checks if AdvAI is under threat of being start rushed.
 *  Start rushes require specific tactics to counter.
 *
 *  Author: Rampastring
 */
bool HouseExt::AdvAI_Is_Under_Start_Rush_Threat(HouseClass* pHouse, int enemyAircraftCount)
{
	// If the game has progressed for long enough, it is no longer considered a start rush.
	if (Unsorted::CurrentFrame > 10000)
	{
		return false;
	}

	if (enemyAircraftCount > 0 || AdvAI_Is_Recently_Attacked(pHouse))
	{
		return true;
	}

	// Counter infantry rushing. If a human enemy has more infantry than we do, we are at risk.

	static int houseInfantryStrength[10] = {};

	// Go through all infantry on the map and gather infantry strength of all enemy human houses.
	for (const auto pInfantry : *InfantryClass::Array)
	{
		if (pInfantry->InLimbo)
		{
			continue;
		}

		// Also calculate our own infantry strength for comparison.
		if (pInfantry->Owner == pHouse)
		{
			houseInfantryStrength[pHouse->ArrayIndex] += pInfantry->Type->Points;
			continue;
		}

		if (pInfantry->Owner->Type->MultiplayPassive)
		{
			continue;
		}

		if (!pInfantry->Owner->IsControlledByHuman())
		{
			continue;
		}

		if (pInfantry->Owner->IsAlliedWith(pHouse))
		{
			continue;
		}

		if (pInfantry->Owner->ArrayIndex >= std::size(houseInfantryStrength))
		{
			continue;
		}

		// Humans can typically micromanage better than the AI, so increase points for human infantry.
		houseInfantryStrength[pInfantry->Owner->ArrayIndex] += pInfantry->Type->Points * 2;
	}

	const int ourInfantryStrength = houseInfantryStrength[pHouse->ArrayIndex];
	for (const int i : houseInfantryStrength)
	{
		if (i > ourInfantryStrength)
		{
			return true;
		}
	}

	return false;
}


/**
 *  Calculates the total number of enemy aircraft in the game.
 *
 *  Author: Rampastring
 */
int HouseExt::AdvAI_Calculate_Enemy_Aircraft_Count(HouseClass* pHouse)
{
	int enemyAircraftCount = 0;

	for (const auto pAircraft : *AircraftClass::Array)
	{
		if (!pAircraft->Owner->IsAlliedWith(pHouse) && !pAircraft->Owner->Type->MultiplayPassive)
		{
			enemyAircraftCount++;
		}
	}

	return enemyAircraftCount;
}


/**
 *  Gets the building that the Advanced AI should build in its current game situation.
 *
 *  Author: Rampastring
 */
const BuildingTypeClass* HouseExt::AdvAI_Evaluate_Get_Best_Building(HouseClass* pHouse)
{
	const auto houseExt = ExtMap.Find(pHouse);

	int ourRefinery = -1;
	int ourBasicPower = -1;
	int ourAdvancedPower = -1;

	for (const auto pRefinery : RulesClass::Instance->BuildRefinery)
	{
		if (AdvAI_Can_Build_Building(pHouse, pRefinery, true))
		{
			ourRefinery = pRefinery->ArrayIndex;
		}
	}

	for (const auto pPower : RulesClass::Instance->BuildPower)
	{
		if (AdvAI_Can_Build_Building(pHouse, pPower, true))
		{
			if (ourBasicPower != -1)
			{
				ourAdvancedPower = pPower->ArrayIndex;
			}
			else
			{
				ourBasicPower = pPower->ArrayIndex;
			}
		}
	}

	// Since we do not currently know how to handle prerequisite groups, our basic and
	// advanced power plants might be reversed.
	// Check if this is the case and if so, reverse them.
	if (ourBasicPower != -1 && ourAdvancedPower != -1 &&
		(*BuildingTypeClass::Array)[ourBasicPower]->PowerBonus > (*BuildingTypeClass::Array)[ourAdvancedPower]->PowerBonus)
	{
		const int tmp = ourBasicPower;
		ourBasicPower = ourAdvancedPower;
		ourAdvancedPower = tmp;
	}

	// If we have no power plants yet, then build one
	if (pHouse->ActiveBuildingTypes.GetItemCount(ourBasicPower) == 0)
	{
		Debug::Log("AdvAI: Making AI build %s because it has 0 basic power plants\n", (*BuildingTypeClass::Array)[ourBasicPower]->Name);
		return (*BuildingTypeClass::Array)[ourBasicPower];
	}

	// On Medium and Hard, build a barracks if we do not have any yet
	if (pHouse->AIDifficulty < AIDifficulty::Hard && pHouse->Balance >= RulesClass::Instance->AIAlternateProductionCreditCutoff)
	{
		for (const auto pBarracks : RulesClass::Instance->BuildBarracks)
		{
			if (AdvAI_Can_Build_Building(pHouse, pBarracks, true))
			{
				const int barracksCount = pHouse->ActiveBuildingTypes.GetItemCount(pBarracks->ArrayIndex);
				if (barracksCount < 1)
				{
					Debug::Log("AdvAI: Making AI build %s because it does not have a Barracks at all.\n", pBarracks->Name);
					return pBarracks;
				}
			}
		}
	}

	const bool isRecentlyAttacked = pHouse->LATime + TICKS_PER_MINUTE > Unsorted::CurrentFrame;

	// Check how many aircraft our opponents have.
	// This check could be expensive, but usually there are not very 
	// high numbers of aircraft in the game, so it's probably fine.
	const int enemyAircraftCount = AdvAI_Calculate_Enemy_Aircraft_Count(pHouse);

	// Check whether we're in threat of being rushed right in the beginning of the game.
	const bool isUnderThreat = AdvAI_Is_Under_Start_Rush_Threat(pHouse, enemyAircraftCount);
	houseExt->IsUnderStartRushThreat = isUnderThreat;

	// Build refinery if we're expanding and we're not under immediate air rush threat
	if (!isUnderThreat)
	{
		if (ourRefinery != -1 && houseExt->ShouldBuildRefinery)
		{
			Debug::Log("AdvAI: Making AI build %s because it has reached an expansion point\n", (*BuildingTypeClass::Array)[ourRefinery]->Name);
			return (*BuildingTypeClass::Array)[ourRefinery];
		}
	}

	// Build a refinery if we have 0 left
	if (ourRefinery != -1 && pHouse->ActiveBuildingTypes.GetItemCount(ourRefinery) == 0)
	{
		Debug::Log("AdvAI: Making AI build %s because it has 0 refineries\n", (*BuildingTypeClass::Array)[ourRefinery]->Name);
		return (*BuildingTypeClass::Array)[ourRefinery];
	}

	// Build power if necessary
	if (!isUnderThreat && Unsorted::CurrentFrame > 5000 && pHouse->PowerOutput - pHouse->PowerDrain < 100)
	{
		if (ourAdvancedPower != -1)
		{
			Debug::Log("AdvAI: Making AI build %s because it is out of power and can build an adv. power plant\n", (*BuildingTypeClass::Array)[ourAdvancedPower]->Name);
			return (*BuildingTypeClass::Array)[ourAdvancedPower];
		}

		if (ourBasicPower != -1)
		{
			Debug::Log("AdvAI: Making AI build %s because it is out of power and can only build a basic power plant\n", (*BuildingTypeClass::Array)[ourBasicPower]->Name);
			return (*BuildingTypeClass::Array)[ourBasicPower];
		}
	}

	// If we don't have enough barracks, then build one
	const int optimalBarracksCount = 1 + (pHouse->ActiveBuildingTypes.GetItemCount(ourRefinery) / 3);

	for (const auto pBarracks : RulesClass::Instance->BuildBarracks)
	{
		if (AdvAI_Can_Build_Building(pHouse, pBarracks, true))
		{
			int barracksCount = pHouse->ActiveBuildingTypes.GetItemCount(pBarracks->ArrayIndex);
			if (barracksCount < optimalBarracksCount)
			{
				Debug::Log("AdvAI: Making AI build %s because it does not have enough Barracks. Wanted: %d, current: %d\n",
					pBarracks->Name, optimalBarracksCount, barracksCount);

				return pBarracks;
			}
		}
	}

	// Get defenses and calculate for deficiencies on them before making
	// further decisions.
	int ourAntiInfantryDefense = -1;
	int ourAntiVehicleDefense = -1;
	int ourAntiAirDefense = -1;

	int bestAntiInfantryRating = INT_MIN;
	int bestAntiVehicleRating = INT_MIN;

	for (const auto pDefense : RulesClass::Instance->BuildDefense)
	{
		if (AdvAI_Can_Build_Building(pHouse, pDefense, true))
		{
			if (pDefense->AntiInfantryValue > bestAntiInfantryRating)
			{
				bestAntiInfantryRating = pDefense->AntiInfantryValue;
				ourAntiInfantryDefense = pDefense->ArrayIndex;
			}

			if (pDefense->AntiArmorValue > bestAntiVehicleRating)
			{
				bestAntiVehicleRating = pDefense->AntiArmorValue;
				ourAntiVehicleDefense = pDefense->ArrayIndex;
			}
		}
	}

	for (const auto pAA : RulesClass::Instance->BuildAA)
	{
		if (AdvAI_Can_Build_Building(pHouse, pAA, true))
		{
			ourAntiAirDefense = pAA->ArrayIndex;
		}
	}

	int optimalDefenseCount = pHouse->ActiveBuildingTypes.GetItemCount(ourRefinery) + (pHouse->ActiveBuildingTypes.GetItemCount(ourBasicPower) + pHouse->ActiveBuildingTypes.GetItemCount(ourAdvancedPower)) / 4;
	if (houseExt->NextExpansionPointLocation.X > 0 && houseExt->NextExpansionPointLocation.Y > 0)
	{
		optimalDefenseCount++;
	}

	// Special check for early infantry rushes.
	// If we are getting infantry-rushed, build more anti-infantry defenses.
	if (isUnderThreat && enemyAircraftCount == 0)
	{
		optimalDefenseCount *= 3;
	}

	// If we are under attack, prioritize defense.
	if (isRecentlyAttacked)
	{
		optimalDefenseCount++;
	}

	// Check which type of defense is most desperately needed.
	int antiInfDeficiency = 0;
	int antiVehicleDeficiency = 0;
	int antiAirDeficiency = 0;

	if (ourAntiInfantryDefense != -1)
	{
		const int defenseCount = pHouse->ActiveBuildingTypes.GetItemCount(ourAntiInfantryDefense);
		antiInfDeficiency = optimalDefenseCount - defenseCount;
	}

	if (ourAntiInfantryDefense != ourAntiVehicleDefense && ourAntiVehicleDefense != -1)
	{
		const int defenseCount = pHouse->ActiveBuildingTypes.GetItemCount(ourAntiVehicleDefense);
		antiVehicleDeficiency = optimalDefenseCount - defenseCount;
	}

	// We're just going to bluntly assume that we need 1 AA defense for every 2 enemy aircraft present.
	int neededAaCount = enemyAircraftCount / 2;
	// ...but don't overspend on AA.
	if (neededAaCount > optimalDefenseCount * 2)
	{
		neededAaCount = optimalDefenseCount;
	}

	int aaDefenseCount = 0;

	if (ourAntiAirDefense != -1)
	{
		aaDefenseCount = pHouse->ActiveBuildingTypes.GetItemCount(ourAntiAirDefense);
	}

	antiAirDeficiency = neededAaCount - aaDefenseCount;

	// If we are under threat of an immediate early-game rush, then skip the WF and refinery minimums.
	// Instead build defenses or tech up so we can get AA ASAP.
	if (!isUnderThreat || (antiInfDeficiency == 0 && antiAirDeficiency == 0))
	{

		// If we don't have enough weapons factory, then build one.
		const int optimalWfCount = 1 + (pHouse->ActiveBuildingTypes.GetItemCount(ourRefinery) / 4);

		for (const auto pWarFactory : RulesClass::Instance->BuildWeapons)
		{
			if (AdvAI_Can_Build_Building(pHouse, pWarFactory, true))
			{
				int wfCount = pHouse->ActiveBuildingTypes.GetItemCount(pWarFactory->ArrayIndex);
				if (wfCount < optimalWfCount)
				{

					Debug::Log("AdvAI: Making AI build %s because it does not have enough Weapons Factories. Wanted: %d, current: %d\n",
						pWarFactory->Name, optimalWfCount, wfCount);

					return pWarFactory;
				}
			}
		}

		// If we have too few refineries, build enough to match the minimum.
		// Because this is not for expanding but an emergency situation,
		// cancel any potential expanding.
		if (ourRefinery != -1 && pHouse->ActiveBuildingTypes.GetItemCount(ourRefinery) < RulesExt::Global()->AdvancedAIMinimumRefineryCount)
		{
			houseExt->NextExpansionPointLocation = CellStruct(0, 0);
			Debug::Log("AdvAI: Making AI build %s because it only has too few refineries\n", (*BuildingTypeClass::Array)[ourRefinery]->Name);
			return (*BuildingTypeClass::Array)[ourRefinery];
		}
	}

	if (antiInfDeficiency > 0 && antiInfDeficiency > antiVehicleDeficiency && antiInfDeficiency > antiAirDeficiency)
	{
		Debug::Log("AdvAI: Making AI build %s because it does not have enough anti-inf defenses. Wanted: %d, deficiency: %d\n",
			(*BuildingTypeClass::Array)[ourAntiInfantryDefense]->Name, optimalDefenseCount, antiInfDeficiency);

		return (*BuildingTypeClass::Array)[ourAntiInfantryDefense];
	}

	if (antiVehicleDeficiency > 0 && antiVehicleDeficiency >= antiAirDeficiency)
	{
		Debug::Log("AdvAI: Making AI build %s because it does not have enough anti-vehicle defenses. Wanted: %d, deficiency: %d\n",
			(*BuildingTypeClass::Array)[ourAntiVehicleDefense]->Name, optimalDefenseCount, antiVehicleDeficiency);

		return (*BuildingTypeClass::Array)[ourAntiVehicleDefense];
	}

	if (antiAirDeficiency > 0)
	{
		// If we actually can't build AA yet, then we need to tech up first.

		if (ourAntiAirDefense != -1)
		{
			Debug::Log("AdvAI: Making AI build %s because it does not have enough anti-air defenses. Deficiency: %d\n",
				(*BuildingTypeClass::Array)[ourAntiAirDefense]->Name, antiAirDeficiency);

			return (*BuildingTypeClass::Array)[ourAntiAirDefense];
		}
	}

	// If we have no radar, then build one
	for (const auto pRadar : RulesClass::Instance->BuildRadar)
	{
		if (AdvAI_Can_Build_Building(pHouse, pRadar, true))
		{
			int radarCount = pHouse->ActiveBuildingTypes.GetItemCount((int)pRadar->ArrayIndex);

			if (radarCount < 1)
			{
				Debug::Log("AdvAI: Making AI build %s because it does not have enough radars. Current count: %d\n",
					pRadar->Name, radarCount);

				return pRadar;
			}
		}
	}

	// If we don't have enough helipads, then build one
	const int optimalHelipadCount = 1 + (pHouse->ActiveBuildingTypes.GetItemCount(ourRefinery) / 4);

	for (const auto pHelipad : RulesClass::Instance->BuildHelipad)
	{
		if (AdvAI_Can_Build_Building(pHouse, pHelipad, true))
		{
			int helipadCount = pHouse->ActiveBuildingTypes.GetItemCount((int)pHelipad->ArrayIndex);

			if (helipadCount < optimalHelipadCount)
			{
				Debug::Log("AdvAI: Making AI build %s because it does not have enough helipads. Wanted: %d, current: %d\n",
					pHelipad->Name, optimalHelipadCount, helipadCount);

				return pHelipad;
			}
		}
	}

	// If we have no tech center, then build one
	for (const auto pTechCenter : RulesClass::Instance->BuildTech)
	{
		if (AdvAI_Can_Build_Building(pHouse, pTechCenter, true))
		{
			if (pHouse->ActiveBuildingTypes.GetItemCount((int)pTechCenter->ArrayIndex) < 1)
			{
				Debug::Log("AdvAI: Making AI build %s because it does not have a tech center.\n",
					pTechCenter->Name);

				return pTechCenter;
			}
		}
	}

	// Build some advanced defenses if we do not have enough
	const int optimalAdvDefenseCount = optimalDefenseCount / 2;

	int ourAdvDefense = -1;

	for (const auto pAdvDefense : RulesClass::Instance->BuildPDefense)
	{
		if (AdvAI_Can_Build_Building(pHouse, pAdvDefense, true))
		{
			ourAdvDefense = pAdvDefense->ArrayIndex;
		}
	}

	if (ourAdvDefense != -1)
	{
		const int advDefenseCount = pHouse->ActiveBuildingTypes.GetItemCount(ourAdvDefense);

		if (advDefenseCount < optimalAdvDefenseCount)
		{
			Debug::Log("AdvAI: Making AI build %s because it does not have enough. Wanted: %d, current: %d.\n",
				(*BuildingTypeClass::Array)[ourAdvDefense]->Name, optimalAdvDefenseCount, advDefenseCount);

			return (*BuildingTypeClass::Array)[ourAdvDefense];
		}
	}

	// Are there other AIBuildThis=yes buildings that we haven't built yet?
	for (const auto pBuilding : *BuildingTypeClass::Array)
	{
		if (pBuilding->AIBuildThis)
		{
			if (AdvAI_Can_Build_Building(pHouse, pBuilding, true))
			{
				if (pHouse->ActiveBuildingTypes.GetItemCount(pBuilding->ArrayIndex) < 1)
				{
					Debug::Log("AdvAI: Making AI build %s because it has AIBuildThis=yes and the AI has none.\n",
						pBuilding->Name);
					return pBuilding;
				}
			}
		}
	}

	// Build power by default, but only if we have somewhere to expand towards.
	if (houseExt->NextExpansionPointLocation.X != 0 && houseExt->NextExpansionPointLocation.Y != 0)
	{
		if (ourBasicPower != -1)
		{
			Debug::Log("AdvAI: Making AI build %s because it the AI is expanding.\n",
				(*BuildingTypeClass::Array)[ourBasicPower]->Name);
			return (*BuildingTypeClass::Array)[ourBasicPower];
		}
	}

	return nullptr;
}


const BuildingTypeClass* HouseExt::AdvAI_Get_Building_To_Build(HouseClass* pHouse)
{
	const BuildingTypeClass* buildChoice = AdvAI_Evaluate_Get_Best_Building(pHouse);

	if (buildChoice == nullptr)
	{
		return nullptr;
	}

	// If our power budget couldn't afford the building, then build a power plant first instead.
	// Unless it's a refinery that we're building, those are considered more critical.
	if (buildChoice->PowerDrain > 0 && !buildChoice->Refinery && (pHouse->PowerDrain + buildChoice->PowerDrain > pHouse->PowerOutput))
	{
		int ourBasicPower = -1;
		int ourAdvancedPower = -1;

		for (const auto pPower : RulesClass::Instance->BuildPower)
		{
			if (AdvAI_Can_Build_Building(pHouse, pPower, true))
			{
				if (ourBasicPower != -1)
				{
					ourAdvancedPower = pPower->ArrayIndex;
				}
				else
				{
					ourBasicPower = pPower->ArrayIndex;
				}
			}
		}

		if (ourAdvancedPower != -1)
		{
			return (*BuildingTypeClass::Array)[ourAdvancedPower];
		}

		if (ourBasicPower != -1)
		{
			return (*BuildingTypeClass::Array)[ourBasicPower];
		}
	}

	return buildChoice;
}


/**
 *  Checks if AdvAI should raise money.
 *  If it should, then raises money.
 *
 *  Author: Rampastring
 */
void HouseExt::AdvAI_Raise_Money(HouseClass* pHouse)
{
	// We should raise money if we are low on funds and have zero refineries.

	if (pHouse->Balance > 1000)
	{
		return;
	}

	int ourRefinery = -1;

	for (const auto pRefinery : RulesClass::Instance->BuildRefinery)
	{
		if (AdvAI_Can_Build_Building(pHouse, pRefinery, true))
		{
			ourRefinery = pRefinery->ArrayIndex;
		}
	}

	if (ourRefinery == -1)
	{
		return;
	}

	const int refineryCount = pHouse->ActiveBuildingTypes.GetItemCount(ourRefinery);

	if (refineryCount > 0)
	{
		return;
	}

	// Look for buildings to sell.
	Debug::Log("AdvAI: Attempting to raise money.\n");

	BuildingClass* bestBuilding = nullptr;
	int bestCost = INT_MIN;

	for (const auto pBuilding : *BuildingClass::Array)
	{
		if (!pBuilding->IsAlive || pBuilding->InLimbo || pBuilding->Owner != pHouse || pBuilding->Type->ConstructionYard)
		{
			continue;
		}

		if (pBuilding->CurrentMission == Mission::Construction || pBuilding->QueuedMission == Mission::Construction)
		{
			// Don't sell something that we've just built.
			continue;
		}

		if (pBuilding->CurrentMission == Mission::Selling || pBuilding->QueuedMission == Mission::Selling)
		{

			// We are already in the process of selling something.
			return;
		}

		// Prefer selling the most expensive stuff first.
		// Give a lower priority to super-weapon buildings, however.
		// They'll be expensive to replace later on.
		int cost = pBuilding->Type->Cost;
		if (pBuilding->Type->SuperWeapon != static_cast<int>(SpecialWeaponType::None) && pBuilding->Type->SuperWeapon2 != static_cast<int>(SpecialWeaponType::None))
		{
			cost = cost / 3;
		}

		if (cost > bestCost)
		{
			bestBuilding = pBuilding;
		}
	}

	// If we found something to sell, then sell it.
	if (bestBuilding != nullptr)
	{
		Debug::Log("AdvAI: Found a building to sell.\n");
		bestBuilding->Sell(1);
	}
}


/**
 *  Perfoms some general economy maintenance.
 *  Raises money if necessary.
 *
 *  Author: Rampastring
 */
void HouseExt::AdvAI_Economy_Upkeep(HouseClass* pHouse)
{
	AdvAI_Raise_Money(pHouse);

	// Don't sell refineries on Easy mode.
	if (pHouse->AIDifficulty == AIDifficulty::Hard)
	{
		return;
	}

	int ourRefinery = -1;

	for (const auto pRefinery : RulesClass::Instance->BuildRefinery)
	{
		if (AdvAI_Can_Build_Building(pHouse, pRefinery, true))
		{
			ourRefinery = pRefinery->ArrayIndex;
		}
	}

	if (ourRefinery == -1)
	{
		return;
	}

	const int refineryCount = pHouse->ActiveBuildingTypes.GetItemCount(ourRefinery);

	int harvesterCount = 0;
	for (const auto pHarvester : RulesClass::Instance->HarvesterUnit)
	{
		harvesterCount += pHouse->ActiveUnitTypes.GetItemCount(pHarvester->ArrayIndex);
	}

	int toSellCount = refineryCount - harvesterCount;
	if (toSellCount <= 0)
	{
		return;
	}

	Debug::Log("AdvAI: Looking for a refinery to sell because we have %d excess.\n", toSellCount);

	// Sell the refinery that is closest to our primary enemy.
	// If we have extra refineries, we have lost harvesters, and harvesters are most likely
	// lost near the expansion that is closest to our primary enemy.
	// If we have no primary enemy, then sell one near our base center.
	// It probably won't go horribly wrong anyway.

	HouseClass* pEnemy = nullptr;
	if (pHouse->EnemyHouseIndex != -1)
	{
		pEnemy = HouseClass::FindByCountryIndex(pHouse->EnemyHouseIndex);
	}

	CellStruct centerPoint;

	if (pEnemy != nullptr)
	{
		centerPoint = pEnemy->Base_Center();
	}
	else
	{
		centerPoint = pHouse->Base_Center();
	}

	BuildingClass* farthest_refinery = nullptr;
	double closest_distance = std::numeric_limits<double>::max();

	for (const auto pBuilding : *BuildingClass::Array)
	{
		if (!pBuilding->IsAlive || pBuilding->InLimbo || pBuilding->Owner != pHouse || !pBuilding->Type->Refinery)
		{
			continue;
		}

		if (pBuilding->CurrentMission == Mission::Construction || pBuilding->QueuedMission == Mission::Construction)
		{
			// If a refinery is in process of being constructed, it hasn't got the spawn its FreeUnit
			// harvester yet.
			Debug::Log("AdvAI: We have a refinery in construction phase, skip.\n");
			return;
		}

		if (pBuilding->CurrentMission == Mission::Selling || pBuilding->QueuedMission == Mission::Selling)
		{

			// We are already in the process of selling a refinery, don't sell more
			// until it's finished.
			Debug::Log("AdvAI: We are already selling a refinery, skip.\n");
			return;
		}

		const double distance = centerPoint.DistanceFrom(GeneralUtils::CellFromCoordinates(pBuilding->GetCenterCoords()));
		if (distance < closest_distance)
		{
			closest_distance = distance;
			farthest_refinery = pBuilding;
		}
	}

	if (farthest_refinery != nullptr)
	{
		Debug::Log("AdvAI: Found a Refinery to sell.\n");
		farthest_refinery->Sell(1);
	}
}


/**
 *  Checks for sleeping harvesters. If found, puts them to Harvest mode.
 *
 *  Author: Rampastring
 */
void HouseExt::AdvAI_Awaken_Sleeping_Harvesters(HouseClass* pHouse)
{
	for (const auto pUnit : *UnitClass::Array)
	{
		if (!pUnit->IsAlive || pUnit->InLimbo || pUnit->Owner != pHouse || !pUnit->Type->Harvester)
		{
			continue;
		}

		if (pUnit->CurrentMission == Mission::Sleep || pUnit->CurrentMission == Mission::Guard)
		{
			Debug::Log("AdvAI: Waking up a sleeping harvester.\n");
			pUnit->QueueMission(Mission::Harvest, true);
		}
	}
}


/**
 *  Sells extra construction yards of the specific house until there is one one left.
 *
 *  Author: Rampastring
 */
void HouseExt::AdvAI_Sell_Extra_ConYards(HouseClass* pHouse)
{
	const int toSellCount = pHouse->ConYards.Count - 1;

	Debug::Log("AdvAI: AI %d has too many Construction Yards (%d). Selling off %d of them. Frame: %d\n", pHouse->ArrayIndex, toSellCount, Unsorted::CurrentFrame.get());

	if (toSellCount < 1)
	{
		return;
	}

	int soldCount = 0;

	for (int i = pHouse->ConYards.Count - 1; i > 0; i--)
	{
		BuildingClass* building = pHouse->ConYards[i];

		if (!building->IsAlive || building->InLimbo)
		{
			continue;
		}

		if (building->CurrentMission == Mission::Selling || building->QueuedMission == Mission::Selling)
		{
			soldCount++;

			if (soldCount >= toSellCount)
			{
				break;
			}

			continue;
		}

		Debug::Log("AdvAI: Found a Construction Yard to sell.\n");

		building->Sell(1);
		soldCount++;

		if (soldCount >= toSellCount)
		{
			break;
		}
	}
}


/**
 *  Implements DTA's custom AI building selection logic.
 *
 *  Author: Rampastring
 */
void HouseExt::Vinifera_HouseClass_AI_Building(HouseClass* pHouse)
{
	// Decide what to build.
	// If we already have something to build, do nothing.
	if (pHouse->ProducingBuildingTypeIndex != -1)
		return;

	if (pHouse->ConYards.Count <= 0)
		return;

	const auto houseExt = ExtMap.Find(pHouse);

	if (RulesExt::Global()->IsUseAdvancedAI)
	{
		// If we have nowhere to expand towards, check for a new location to expand to.
		if (houseExt->NextExpansionPointLocation.X <= 0 || houseExt->NextExpansionPointLocation.Y <= 0)
		{
			AdvAI_House_Search_For_Next_Expansion_Point(pHouse);
		}

		const BuildingTypeClass* toBuild = AdvAI_Get_Building_To_Build(pHouse);

		if (toBuild == nullptr)
		{
			return;
		}

		Debug::Log("AI %d selected building %s to build. Frame: %d\n", pHouse->ArrayIndex, toBuild->Name, Unsorted::CurrentFrame.get());

		pHouse->ProducingBuildingTypeIndex = toBuild->ArrayIndex;
	}
	else
	{
		const BaseNodeClass* node = pHouse->Base.NextBuildable();

		if (node != nullptr)
		{
			pHouse->ProducingBuildingTypeIndex = node->BuildingTypeIndex;
		}
	}
}

/**
 *  Performs some maintenance for the Advanced AI.
 *
 *  Author: Rampastring
 */
void HouseExt::AdvAI_HouseClass_Expert_AI(HouseClass* pHouse)
{
	if (pHouse->Type->MultiplayPassive)
	{
		return;
	}

	// Only enable our custom logic when using Advanced AI.
	if (!RulesExt::Global()->IsUseAdvancedAI)
	{
		return;
	}

	// If we have more than 1 ConYard without Rules allowing it, sell some of them off
	// to avoid the "Extreme AI" syndrome.
	if (pHouse->ConYards.Count > 1 && !RulesExt::Global()->IsAdvancedAIMultiConYard)
	{
		AdvAI_Sell_Extra_ConYards(pHouse);
	}

	// If we have no enemy, then pick one.
	if (pHouse->EnemyHouseIndex == -1)
	{
		pHouse->ExpertAITimer.Start(0);
	}

	const auto houseExt = ExtMap.Find(pHouse);

	// Do some economy upkeep to keep the AI running.

	if (Unsorted::CurrentFrame > houseExt->LastExcessRefineryCheckFrame + 500)
	{
		houseExt->LastExcessRefineryCheckFrame = Unsorted::CurrentFrame;
		AdvAI_Economy_Upkeep(pHouse);
	}

	if (Unsorted::CurrentFrame > houseExt->LastSleepingHarvesterCheckFrame + 1000)
	{
		houseExt->LastSleepingHarvesterCheckFrame = Unsorted::CurrentFrame;
		AdvAI_Awaken_Sleeping_Harvesters(pHouse);
	}

	// If we have 0 ConYards and 0 War Factories, it is very unlikely we could get
	// back into the game. Send all our non-Harvester vehicles into Hunt mode.
	if (Unsorted::CurrentFrame > 5000 && pHouse->ConYards.Count == 0 && pHouse->NumWarFactories == 0 && !houseExt->HasPerformedVehicleCharge)
	{
		houseExt->HasPerformedVehicleCharge = true;

		for (const auto pUnit : *UnitClass::Array)
		{
			if (pUnit->Owner == pHouse &&
				(pUnit->Type->DeploysInto == nullptr || !pUnit->Type->DeploysInto->ConstructionYard) &&
				!pUnit->Type->Harvester &&
				!pUnit->Type->Weeder)
			{
				if (pUnit->Team != nullptr)
				{
					pUnit->Team->LiberateMember(pUnit);
				}

				pUnit->QueueMission(Mission::Hunt, false);
			}
		}
	}

	// If we are under threat of getting rushed early and our ConYard is producing something non-defensive and non-power-granting, abandon it.
	const int enemyAircraftCount = AdvAI_Calculate_Enemy_Aircraft_Count(pHouse);
	const bool isUnderThreat = AdvAI_Is_Under_Start_Rush_Threat(pHouse, enemyAircraftCount);

	if (isUnderThreat)
	{
		FactoryClass* buildingFactory = pHouse->GetPrimaryFactory(AbstractType::BuildingType, false, BuildCat::DontCare);
		if (buildingFactory != nullptr)
		{
			if (buildingFactory->GetObject() != nullptr)
			{
				BuildingClass* building = reinterpret_cast<BuildingClass*>(buildingFactory->GetObject());

				if (building->Type->PowerBonus <= 0 ||
					building->Type->GetWeapon(static_cast<int>(WeaponSlotType::Primary), false).WeaponType == nullptr ||
					building->Type->Factory != AbstractType::InfantryType)
				{
					buildingFactory->AbandonProduction();
				}
			}
		}
	}
}