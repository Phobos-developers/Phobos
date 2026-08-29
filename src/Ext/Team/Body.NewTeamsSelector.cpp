#include "Body.h"
#include <unordered_map>

enum class teamCategory : int
{
	None = 0, // No category. Should be default value
	Ground = 1, // Ground vehicles (Tanks, etc.)
	Air = 2, // Aircrafts & jumpjets
	Naval = 3, // Naval units
	Infantry = 4, // Infantry units
	Unclassified = 5 // Mixed teams
};

struct CandidateTrigger
{
	AITriggerTypeClass* Trigger = nullptr;
	double Weight = 0.0;
	teamCategory Category = teamCategory::None;
};

DEFINE_HOOK(0x4F8A27, TeamTypeClass_SuggestedNewTeam_NewTeamsSelector, 0x5)
{
	enum { UseOriginalSelector = 0x4F8A63, SkipCode = 0x4F8B08 };

	GET(HouseClass*, pHouse, ESI);

	if (!RulesExt::Global()->NewTeamsSelector)
		return UseOriginalSelector;

	bool houseIsHuman = pHouse->IsHumanPlayer;
	bool isCampaign = SessionClass::IsCampaign();

	if (isCampaign)
		houseIsHuman = pHouse->IsHumanPlayer || pHouse->IsInPlayerControl;

	if (houseIsHuman || pHouse->Type->MultiplayPassive || !pHouse->AITriggersActive)
		return SkipCode;

	auto pHouseTypeExt = HouseTypeExt::ExtMap.Find(pHouse->Type);
	if (!pHouseTypeExt)
		return SkipCode;

	// Reset Team selection countdown
	int countdown = RulesClass::Instance->TeamDelays[(int)pHouse->AIDifficulty];
	pHouse->TeamDelayTimer.Start(countdown);

	int totalActiveTeams = 0;
	int activeTeams = 0;

	std::vector<CandidateTrigger> validCandidates;

	int dice = ScenarioClass::Instance->Random.RandomRanged(1, 100);

	// This house must have the triggers enabled
	if (dice > pHouse->RatioAITriggerTeam || !pHouse->AITriggersActive)
		return SkipCode;

	bool splitTriggersByCategory = RulesExt::Global()->NewTeamsSelector_SplitTriggersByCategory;
	bool isFallbackEnabled = RulesExt::Global()->NewTeamsSelector_EnableFallback;
	teamCategory validCategory = teamCategory::None;
	int mergeUnclassifiedCategoryWith = -1;

	double percentageUnclassifiedTriggers = 0.0;
	double percentageGroundTriggers = 0.0;
	double percentageNavalTriggers = 0.0;
	double percentageAirTriggers = 0.0;
	double percentageInfantryTriggers = 0.0;

	if (splitTriggersByCategory)
	{
		mergeUnclassifiedCategoryWith = pHouseTypeExt->NewTeamsSelector_MergeUnclassifiedCategoryWith.isset() ? pHouseTypeExt->NewTeamsSelector_MergeUnclassifiedCategoryWith.Get() : RulesExt::Global()->NewTeamsSelector_MergeUnclassifiedCategoryWith;  // Should mixed teams be merged into another category?
		percentageUnclassifiedTriggers = pHouseTypeExt->NewTeamsSelector_UnclassifiedCategoryPercentage.isset() ? pHouseTypeExt->NewTeamsSelector_UnclassifiedCategoryPercentage.Get() : RulesExt::Global()->NewTeamsSelector_UnclassifiedCategoryPercentage; // Mixed teams
		percentageGroundTriggers = pHouseTypeExt->NewTeamsSelector_GroundCategoryPercentage.isset() ? pHouseTypeExt->NewTeamsSelector_GroundCategoryPercentage.Get() : RulesExt::Global()->NewTeamsSelector_GroundCategoryPercentage; // Only ground vehicles
		percentageNavalTriggers = pHouseTypeExt->NewTeamsSelector_NavalCategoryPercentage.isset() ? pHouseTypeExt->NewTeamsSelector_NavalCategoryPercentage.Get() : RulesExt::Global()->NewTeamsSelector_NavalCategoryPercentage; // Only Naval=yes
		percentageAirTriggers = pHouseTypeExt->NewTeamsSelector_AirCategoryPercentage.isset() ? pHouseTypeExt->NewTeamsSelector_AirCategoryPercentage.Get() : RulesExt::Global()->NewTeamsSelector_AirCategoryPercentage; // Only Aircrafts & jumpjets
		percentageInfantryTriggers = pHouseTypeExt->NewTeamsSelector_InfantryCategoryPercentage.isset() ? pHouseTypeExt->NewTeamsSelector_InfantryCategoryPercentage.Get() : RulesExt::Global()->NewTeamsSelector_InfantryCategoryPercentage; // Only infantry

		// Merge mixed category with another category, if set
		if (mergeUnclassifiedCategoryWith >= 0)
		{
			switch (mergeUnclassifiedCategoryWith)
			{
			case (int)teamCategory::Ground:
				percentageGroundTriggers += percentageUnclassifiedTriggers;
				break;

			case (int)teamCategory::Air:
				percentageAirTriggers += percentageUnclassifiedTriggers;
				break;

			case (int)teamCategory::Naval:
				percentageNavalTriggers += percentageUnclassifiedTriggers;
				break;

			case (int)teamCategory::Infantry:
				percentageInfantryTriggers += percentageUnclassifiedTriggers;
				break;

			default:
				break;
			}

			percentageUnclassifiedTriggers = 0.0;
		}

		percentageUnclassifiedTriggers = percentageUnclassifiedTriggers < 0.0 || percentageUnclassifiedTriggers > 1.0 ? 0.0 : percentageUnclassifiedTriggers;
		percentageGroundTriggers = percentageGroundTriggers < 0.0 || percentageGroundTriggers > 1.0 ? 0.0 : percentageGroundTriggers;
		percentageNavalTriggers = percentageNavalTriggers < 0.0 || percentageNavalTriggers > 1.0 ? 0.0 : percentageNavalTriggers;
		percentageAirTriggers = percentageAirTriggers < 0.0 || percentageAirTriggers > 1.0 ? 0.0 : percentageAirTriggers;
		percentageInfantryTriggers = percentageInfantryTriggers < 0.0 || percentageInfantryTriggers > 1.0 ? 0.0 : percentageInfantryTriggers;

		double totalPercengates = percentageUnclassifiedTriggers + percentageGroundTriggers + percentageNavalTriggers + percentageAirTriggers + percentageInfantryTriggers;
		if (totalPercengates > 1.0 || totalPercengates <= 0.0)
			splitTriggersByCategory = false;

		if (splitTriggersByCategory)
		{
			int categoryDice = ScenarioClass::Instance->Random.RandomRanged(1, 100);
			int unclassifiedValue = (int)(percentageUnclassifiedTriggers * 100.0);
			int groundValue = (int)(percentageGroundTriggers * 100.0);
			int airValue = (int)(percentageAirTriggers * 100.0);
			int navalValue = (int)(percentageNavalTriggers * 100.0);
			int infantryValue = (int)(percentageInfantryTriggers * 100.0);

			// Pick what type of team will be selected in this round
			if (percentageUnclassifiedTriggers > 0.0 && categoryDice <= unclassifiedValue)
			{
				validCategory = teamCategory::Unclassified;
			}
			else if (percentageGroundTriggers > 0.0 && categoryDice <= (unclassifiedValue + groundValue))
			{
				validCategory = teamCategory::Ground;
			}
			else if (percentageAirTriggers > 0.0 && categoryDice <= (unclassifiedValue + groundValue + airValue))
			{
				validCategory = teamCategory::Air;
			}
			else if (percentageNavalTriggers > 0.0 && categoryDice <= (unclassifiedValue + groundValue + airValue + navalValue))
			{
				validCategory = teamCategory::Naval;
			}
			else if (percentageInfantryTriggers > 0.0 && categoryDice <= (unclassifiedValue + groundValue + airValue + navalValue + infantryValue))
			{
				validCategory = teamCategory::Infantry;
			}
			else
			{
				// If the sum of all percentages is less than 100% then that empty space will work like "no categories"
				splitTriggersByCategory = false;
			}
		}
	}

	if (splitTriggersByCategory)
	{
		switch (validCategory)
		{
		case teamCategory::Ground:
			Debug::Log("AITeamsSelector - This time only GROUND VEHICLE teams will be picked.\n");
			break;

		case teamCategory::Infantry:
			Debug::Log("AITeamsSelector - This time only INFANTRY teams will be picked.\n");
			break;

		case teamCategory::Unclassified:
			Debug::Log("AITeamsSelector - This time only MIXED teams will be picked.\n");
			break;

		case teamCategory::Naval:
			Debug::Log("AITeamsSelector - This time only NAVAL teams will be picked.\n");
			break;

		case teamCategory::Air:
			Debug::Log("AITeamsSelector - This time only AIR teams will be picked.\n");
			break;

		default:
			Debug::Log("AITeamsSelector - This time teams categories are DISABLED. Anyone can be picked\n");
			break;
		}
	}

	int houseIdx = pHouse->ArrayIndex;

	auto houseDifficulty = pHouse->AIDifficulty;
	int minBaseDefenseTeams = RulesClass::Instance->MinimumAIDefensiveTeams.GetItem((int)houseDifficulty);
	int maxBaseDefenseTeams = RulesClass::Instance->MaximumAIDefensiveTeams.GetItem((int)houseDifficulty);
	int activeDefenseTeamsCount = 0;
	int maxTeamsLimit = RulesClass::Instance->TotalAITeamCap.GetItem((int)houseDifficulty);

	// Check if the running teams by the house already reached all the limits
	DynamicVectorClass<TeamClass*> activeTeamsList;

	for (auto const pRunningTeam : TeamClass::Array)
	{
		totalActiveTeams++;
		int teamHouseIdx = pRunningTeam->Owner->ArrayIndex;

		if (teamHouseIdx != houseIdx)
			continue;

		activeTeamsList.AddItem(pRunningTeam);

		if (pRunningTeam->Type->IsBaseDefense)
			activeDefenseTeamsCount++;
	}

	activeTeams = activeTeamsList.Count;

	// We will use these values for discarding triggers
	int defensiveTeamsLimit = RulesClass::Instance->UseMinDefenseRule ? minBaseDefenseTeams : maxBaseDefenseTeams;
	bool hasReachedMaxTeamsLimit = activeTeams >= maxTeamsLimit;
	bool hasReachedMaxDefensiveTeamsLimit = activeDefenseTeamsCount >= defensiveTeamsLimit;

	// Check if the next team must be a defensive team
	bool onlyPickDefensiveTeams = false;
	int defensiveDice = ScenarioClass::Instance->Random.RandomRanged(0, 99);
	int defenseTeamSelectionThreshold = 50;

	if ((defensiveDice < defenseTeamSelectionThreshold) && !hasReachedMaxDefensiveTeamsLimit && !isCampaign)
		onlyPickDefensiveTeams = true;

	if (hasReachedMaxTeamsLimit)
	{
		Debug::Log("AITeamsSelector - House %d [%s](%s) reached the TotalAITeamCap (%d)! Skipping operation this time...\n", pHouse->ArrayIndex, pHouse->PlainName, pHouse->Type->ID, defenseTeamSelectionThreshold);
		return SkipCode;
	}

	if (onlyPickDefensiveTeams)
		Debug::Log("AITeamsSelector - House %d [%s](%s) currently has %d/%d active DEFENSIVE teams and in total %d/%d active teams...\n", pHouse->ArrayIndex, pHouse->PlainName, pHouse->Type->ID, activeDefenseTeamsCount, defensiveTeamsLimit, totalActiveTeams, maxTeamsLimit);
	else
		Debug::Log("AITeamsSelector - House %d [%s](%s) currently has %d/%d active teams...\n", pHouse->ArrayIndex, pHouse->PlainName, pHouse->Type->ID, totalActiveTeams, maxTeamsLimit);

	int destroyedBridgesCount = 0;
	int undamagedBridgesCount = 0;
	std::unordered_map<TechnoTypeClass*, int> ownedRecruitables;
	bool hasInfantryFactory = false;
	bool hasUnitFactory = false;
	bool hasNavalFactory = false;
	bool hasAircraftFactory = false;
	bool canAutocreate = false;

	for (auto const pTechno : TechnoClass::Array)
	{
		if (!TechnoExt::IsValidTechno(pTechno)) continue;

		if (pTechno->WhatAmI() == AbstractType::Building)
		{
			auto const pBuildingType = static_cast<BuildingTypeClass*>(pTechno->GetTechnoType());

			if (!pBuildingType)
				continue;

			if (pTechno->Owner == pHouse)
			{
				switch (pBuildingType->Factory)
				{
				case AbstractType::InfantryType:
					hasInfantryFactory = true;
					break;

				case AbstractType::AircraftType:
					hasAircraftFactory = true;
					break;

				case AbstractType::UnitType:
					if (pBuildingType->Naval)
						hasNavalFactory = true;
					else
						hasUnitFactory = true;
					break;

				default:
					break;
				}
			}
			else
			{
				if (pBuildingType->BridgeRepairHut)
				{
					CellStruct cell = pTechno->GetCell()->MapCoords;

					if (MapClass::Instance.IsLinkedBridgeDestroyed(cell))
						destroyedBridgesCount++;
					else
						undamagedBridgesCount++;
				}
			}

			continue;
		}

		auto const pFoot = static_cast<FootClass*>(pTechno);

		if (!pFoot
			|| !pTechno->IsAlive
			|| pTechno->Health <= 0
			|| !pTechno->IsOnMap // Note: underground movement is considered "IsOnMap == false"
			|| pTechno->Transporter
			|| pTechno->Absorbed
			|| !pFoot->CanBeRecruited(pHouse))
		{
			continue;
		}

		++ownedRecruitables[pTechno->GetTechnoType()];
	}

	if (hasInfantryFactory || hasUnitFactory || hasAircraftFactory || hasNavalFactory)
		canAutocreate = true;

	HouseClass* targetHouse = nullptr;
	if (pHouse->EnemyHouseIndex >= 0)
		targetHouse = HouseClass::Array.GetItem(pHouse->EnemyHouseIndex);

	bool onlyCheckImportantTriggers = false;

	const auto pHouseExt = HouseExt::ExtMap.Find(pHouse);
	double maxPriority = 5000.0;

	// Gather all the trigger candidates into one place for posterior fast calculations
	for (int triggerIdx : pHouseExt->AITriggers_ValidList)
	{
		const auto pTrigger = AITriggerTypeClass::Array.GetItem(triggerIdx);

		if (!pTrigger || !pTrigger->Team1 || !pTrigger->IsEnabled)
			continue;

		// Ignore offensive teams if the next trigger must be defensive
		if ((onlyPickDefensiveTeams && !pTrigger->IsForBaseDefense) || (hasReachedMaxDefensiveTeamsLimit && pTrigger->IsForBaseDefense))
			continue;

		if (pTrigger->Team1->TechLevel > pHouse->TechLevel)
			continue;

		// ignore it if isn't set for the house AI difficulty
		if ((int)houseDifficulty == 0 && !pTrigger->Enabled_Hard
			|| (int)houseDifficulty == 1 && !pTrigger->Enabled_Normal
			|| (int)houseDifficulty == 2 && !pTrigger->Enabled_Easy)
		{
			continue;
		}

		// The trigger must be compatible with the owner
		{
			// "ConditionType=-1" will be skipped, always is valid
			if ((int)pTrigger->ConditionType >= 0)
			{
				if ((int)pTrigger->ConditionType == 0)
				{
					// Simulate case 0: "enemy owns"
					if (!pTrigger->ConditionObject || !TeamExt::EnemyOwns(pTrigger, pHouse, targetHouse, true, pTrigger->ConditionObject))
						continue;
				}
				else if ((int)pTrigger->ConditionType == 1)
				{
					// Simulate case 1: "house owns"
					if (!pTrigger->ConditionObject || !TeamExt::HouseOwns(pTrigger, pHouse, false, pTrigger->ConditionObject))
						continue;
				}
				else if ((int)pTrigger->ConditionType == 4)
				{
					// Simulate case 4: "Enemy house economy threshold?"
					if (!pTrigger->HouseCredits(nullptr, targetHouse))
						continue;
				}
				else if ((int)pTrigger->ConditionType == 5)
				{
					// Simulate case 5: "Iron Curtain is charged?"
					if (!pTrigger->IronCurtainCharged(pHouse, nullptr))
						continue;
				}
				else if ((int)pTrigger->ConditionType == 6)
				{
					// Simulate case 6: "Chronosphere is charged?"
					if (!pTrigger->ChronoSphereCharged(pHouse, nullptr))
						continue;
				}
				else if ((int)pTrigger->ConditionType == 7)
				{
					// Simulate case 7: "civilian owns"
					if (!pTrigger->ConditionObject || !TeamExt::NeutralOwns(pTrigger, pTrigger->ConditionObject))
						continue;
				}
				else if ((int)pTrigger->ConditionType == 8)
				{
					// Simulate case 8: "enemy owns" across all enemies
					if (!pTrigger->ConditionObject || !TeamExt::EnemyOwns(pTrigger, pHouse, nullptr, false, pTrigger->ConditionObject))
						continue;
				}
				else if ((int)pTrigger->ConditionType == 9)
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					if (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
					{
						const auto& list = RulesExt::Global()->AITargetTypesLists[listIdx];
						if (!TeamExt::EnemyOwns(pTrigger, pHouse, targetHouse, false, list))
							continue;
					}
				}
				else if ((int)pTrigger->ConditionType == 10)
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					if (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
					{
						const auto& list = RulesExt::Global()->AITargetTypesLists[listIdx];
						if (!TeamExt::HouseOwns(pTrigger, pHouse, false, list))
							continue;
					}
				}
				else if ((int)pTrigger->ConditionType == 11)
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					if (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
					{
						const auto& list = RulesExt::Global()->AITargetTypesLists[listIdx];
						if (!TeamExt::NeutralOwns(pTrigger, list))
							continue;
					}
				}
				else if ((int)pTrigger->ConditionType == 12)
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					if (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
					{
						const auto& list = RulesExt::Global()->AITargetTypesLists[listIdx];
						if (!TeamExt::EnemyOwns(pTrigger, pHouse, nullptr, false, list))
							continue;
					}
				}
				else if ((int)pTrigger->ConditionType == 13)
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					if (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
					{
						const auto& list = RulesExt::Global()->AITargetTypesLists[listIdx];
						if (!TeamExt::HouseOwns(pTrigger, pHouse, true, list))
							continue;
					}
				}
				else if ((int)pTrigger->ConditionType == 14)
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					if (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
					{
						const auto& list = RulesExt::Global()->AITargetTypesLists[listIdx];
						if (!TeamExt::EnemyOwnsAll(pTrigger, pHouse, targetHouse, list))
							continue;
					}
				}
				else if ((int)pTrigger->ConditionType == 15)
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					if (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
					{
						const auto& list = RulesExt::Global()->AITargetTypesLists[listIdx];
						if (!TeamExt::HouseOwnsAll(pTrigger, pHouse, list))
							continue;
					}
				}
				else if ((int)pTrigger->ConditionType == 16)
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					if (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
					{
						const auto& list = RulesExt::Global()->AITargetTypesLists[listIdx];
						if (!TeamExt::NeutralOwnsAll(pTrigger, list))
							continue;
					}
				}
				else if ((int)pTrigger->ConditionType == 17)
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					if (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
					{
						const auto& list = RulesExt::Global()->AITargetTypesLists[listIdx];
						if (!TeamExt::EnemyOwnsAll(pTrigger, pHouse, nullptr, list))
							continue;
					}
				}
				else if ((int)pTrigger->ConditionType == 18)
				{
					// New case 18: Check destroyed bridges
					if (!TeamExt::CountConditionMet(pTrigger, destroyedBridgesCount))
						continue;
				}
				else if ((int)pTrigger->ConditionType == 19)
				{
					// New case 19: Check undamaged bridges
					if (!TeamExt::CountConditionMet(pTrigger, undamagedBridgesCount))
						continue;
				}
				else
				{
					// Other cases from vanilla game
					if (!pTrigger->ConditionMet(pHouse, targetHouse, hasReachedMaxDefensiveTeamsLimit))
						continue;
				}
			}

			// All triggers below maxPriority (usually 5000) in current weight will get discarded if this mode is enabled
			if (onlyCheckImportantTriggers)
			{
				if (pTrigger->Weight_Current < maxPriority)
					continue;
			}

			// No more defensive teams needed
			if (pTrigger->Team1->IsBaseDefense && hasReachedMaxDefensiveTeamsLimit)
				continue;

			// If this type of Team reached the max then skip it
			int count = 0;

			for (auto team : activeTeamsList)
			{
				if (team->Type == pTrigger->Team1)
					count++;
			}

			if (count >= pTrigger->Team1->Max)
				continue;

			teamCategory teamIsCategory = teamCategory::None;

			// Analyze what kind of category is this main team if the feature is enabled
			if (splitTriggersByCategory)
			{
				// TaskForces are limited to 6 entries
				for (int i = 0; i < 6; i++)
				{
					auto entry = pTrigger->Team1->TaskForce->Entries[i];
					teamCategory entryIsCategory = teamCategory::Ground;

					if (entry.Amount > 0)
					{
						if (!entry.Type)
							continue;

						if (entry.Type->WhatAmI() == AbstractType::AircraftType
							|| entry.Type->ConsideredAircraft)
						{
							// This unit is from air category
							entryIsCategory = teamCategory::Air;
						}
						else
						{
							auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(entry.Type);

							if (pTechnoTypeExt && (pTechnoTypeExt->ConsideredNaval
								|| (entry.Type->Naval
									&& (entry.Type->MovementZone != MovementZone::Amphibious
										&& entry.Type->MovementZone != MovementZone::AmphibiousDestroyer
										&& entry.Type->MovementZone != MovementZone::AmphibiousCrusher))))
							{
								// This unit is from naval category
								entryIsCategory = teamCategory::Naval;
							}
							else if (entry.Type->WhatAmI() == AbstractType::InfantryType
								&& (!pTechnoTypeExt || !pTechnoTypeExt->ConsideredVehicle))
							{
								// This unit is from infantry category
								entryIsCategory = teamCategory::Infantry;
							}
							else
							{
								// This unit is from ground vehicles category
								entryIsCategory = teamCategory::Ground;
							}
						}

						// if a team have multiple categories it will be a mixed category
						teamIsCategory = teamIsCategory == teamCategory::None || teamIsCategory == entryIsCategory ? entryIsCategory : teamCategory::Unclassified;

						if (teamIsCategory == teamCategory::Unclassified)
							break;
					}
					else
					{
						break;
					}
				}

				// If this value is set and the team is MIXED, override category type
				if (teamIsCategory == teamCategory::Unclassified
					&& mergeUnclassifiedCategoryWith >= 0)
				{
					teamIsCategory = (teamCategory)mergeUnclassifiedCategoryWith;
				}

				if (!isFallbackEnabled && validCategory != teamCategory::None && validCategory != teamIsCategory)
					continue;
			}

			bool allObjectsCanBeBuiltOrRecruited = true;

			if (pTrigger->Team1->Autocreate && canAutocreate)
			{
				for (auto entry : pTrigger->Team1->TaskForce->Entries)
				{
					if (!entry.Type)
						continue;

					if (entry.Amount == 0) // As soon as there are empty slots the check finish
						break;

					bool canBeBuilt = true;

					// Exists the required factory for producing the checked unit?
					switch (entry.Type->WhatAmI())
					{
					case AbstractType::InfantryType:
						if (!hasInfantryFactory)
							canBeBuilt = false;
						break;

					case AbstractType::AircraftType:
						if (!hasAircraftFactory)
							canBeBuilt = false;
						break;

					case AbstractType::UnitType:
						if (entry.Type->Naval)
						{
							if (!hasNavalFactory)
								canBeBuilt = false;
						}
						else
						{
							if (!hasUnitFactory)
								canBeBuilt = false;
						}
						break;

					default:
						break;
					}

					// Meets the production prerequisites?
					if (canBeBuilt)
						canBeBuilt = HouseExt::PrerequisitesMet(pHouse, entry.Type, false);

					if (!canBeBuilt)
					{
						allObjectsCanBeBuiltOrRecruited = false;
						break;
					}
				}
			}
			else
			{
				allObjectsCanBeBuiltOrRecruited = false;
			}

			if (!allObjectsCanBeBuiltOrRecruited && pTrigger->Team1->Recruiter)
			{
				allObjectsCanBeBuiltOrRecruited = true;

				for (auto entry : pTrigger->Team1->TaskForce->Entries)
				{
					if (!entry.Type)
						continue;

					// Check if each unit in the taskforce has the available recruitable units in the map
					if (allObjectsCanBeBuiltOrRecruited && entry.Amount > 0)
					{
						auto const it = ownedRecruitables.find(entry.Type);
						int const recruits = (it != ownedRecruitables.end()) ? it->second : 0;

						if (recruits < entry.Amount)
						{
							allObjectsCanBeBuiltOrRecruited = false;
							break;
						}
					}
				}
			}

			// We can't let AI cheat in this trigger because doesn't have the required tech tree available
			if (!allObjectsCanBeBuiltOrRecruited)
				continue;

			// Special case: triggers become very important if they reach the max priority (usually 5000, see maxPriority).
			// They get stored in an elitist list and all previous triggers are discarded
			if (pTrigger->Weight_Current >= maxPriority && !onlyCheckImportantTriggers)
			{
				validCandidates.clear();
				onlyCheckImportantTriggers = true;
				validCategory = teamCategory::None;
				splitTriggersByCategory = false; // VIP teams break the categories logic (on purpose)
			}

			// Passed all checks, save this trigger for later.
			double const weight = (pTrigger->Weight_Current < 1.0) ? 1.0 : pTrigger->Weight_Current;
			validCandidates.push_back({ pTrigger, weight, teamIsCategory });
		}
	}

	if (validCandidates.empty())
	{
		Debug::Log("AITeamsSelector - The house %d [%s](%s) has no valid triggers for now. A new attempt will be done later...\n", pHouse->ArrayIndex, pHouse->PlainName, pHouse->Type->ID);
		return SkipCode;
	}

	// Calculate category stats and candidate counts
	double categoryTotalWeight = 0.0;
	int categoryCandidatesCount = 0;
	if (validCategory != teamCategory::None)
	{
		for (const auto& candidate : validCandidates)
		{
			if (candidate.Category == validCategory)
			{
				categoryTotalWeight += candidate.Weight;
				categoryCandidatesCount++;
			}
		}
	}

	if (validCategory != teamCategory::None && categoryCandidatesCount == 0)
	{
		Debug::Log("AITeamsSelector - The house %d [%s](%s) has no valid triggers of category %d. A new attempt should be done later...\n",
			pHouse->ArrayIndex, pHouse->PlainName, pHouse->Type->ID, static_cast<int>(validCategory));

		if (!isFallbackEnabled)
			return SkipCode;

		Debug::Log("... but FALLBACK MODE is enabled so now all available triggers will be checked.\n");
		validCategory = teamCategory::None;
	}

	AITriggerTypeClass* selectedTrigger = nullptr;

	// Roulette Wheel Selection (Discrete Distribution)
	if (validCategory != teamCategory::None && categoryTotalWeight > 0.0)
	{
		double const weightDice = ScenarioClass::Instance->Random.RandomDouble() * categoryTotalWeight;
		Debug::Log("AITeamsSelector - Picking 1 team (of category %d) from the %d available (Total Weight: %f, Roll: %f)...\n",
			static_cast<int>(validCategory), categoryCandidatesCount, categoryTotalWeight, weightDice);

		double accumulated = 0.0;
		for (const auto& candidate : validCandidates)
		{
			if (candidate.Category == validCategory)
			{
				accumulated += candidate.Weight;
				if (weightDice < accumulated)
				{
					selectedTrigger = candidate.Trigger;
					break;
				}
			}
		}
	}
	else
	{
		double totalWeight = 0.0;
		for (const auto& candidate : validCandidates)
			totalWeight += candidate.Weight;

		if (totalWeight > 0.0)
		{
			double const weightDice = ScenarioClass::Instance->Random.RandomDouble() * totalWeight;
			Debug::Log("AITeamsSelector - Picking 1 team from the %d available (Total Weight: %f, Roll: %f)...\n",
				validCandidates.size(), totalWeight, weightDice);

			double accumulated = 0.0;
			for (const auto& candidate : validCandidates)
			{
				accumulated += candidate.Weight;
				if (weightDice < accumulated)
				{
					selectedTrigger = candidate.Trigger;
					break;
				}
			}
		}
	}

	if (!selectedTrigger)
	{
		Debug::Log("AITeamsSelector - Failed picking a new trigger for the House %d [%s](%s). A new attempt will be done later...\n", pHouse->ArrayIndex, pHouse->PlainName, pHouse->Type->ID);
		return SkipCode;
	}

	if (selectedTrigger->Weight_Current >= maxPriority
		&& selectedTrigger->Weight_Minimum <= (maxPriority - 1))
	{
		// Next time this trigger will be out of the elitist triggers list
		selectedTrigger->Weight_Current = maxPriority - 1;
	}

	// We have a winner trigger here
	Debug::Log("AITeamsSelector - House %d [%s](%s) selected trigger [%s]: %s.\n", pHouse->ArrayIndex, pHouse->PlainName, pHouse->Type->ID, selectedTrigger->ID, selectedTrigger->Team1->Name);

	// Team 1 creation
	int count1 = 0;

	for (auto team : activeTeamsList)
	{
		if (team->Type == selectedTrigger->Team1)
			count1++;
	}

	if (count1 < selectedTrigger->Team1->Max)
	{
		if (TeamClass* newTeam1 = selectedTrigger->Team1->CreateTeam(pHouse))
			newTeam1->NeedsToDisappear = false;
	}

	// Team 2 creation (if set)
	auto pTriggerTeam2Type = selectedTrigger->Team2;
	if (pTriggerTeam2Type)
	{
		int count2 = 0;

		for (auto team : activeTeamsList)
		{
			if (team->Type == pTriggerTeam2Type)
				count2++;
		}

		if (count2 < pTriggerTeam2Type->Max)
		{
			if (TeamClass* newTeam2 = pTriggerTeam2Type->CreateTeam(pHouse))
				newTeam2->NeedsToDisappear = false;
		}
	}

	return SkipCode;
}

bool TeamExt::HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, TechnoTypeClass* pItem)
{
	int counter = 0;
	if (pItem && pHouse)
	{
		if (!allies)
		{
			counter = pHouse->CountOwnedAndPresent(pItem);
		}
		else
		{
			for (auto const pOtherHouse : HouseClass::Array)
			{
				if (pOtherHouse != pHouse && pHouse->IsAlliedWith(pOtherHouse) && !pOtherHouse->Type->MultiplayPassive)
				{
					counter += pOtherHouse->CountOwnedAndPresent(pItem);
				}
			}
		}
	}
	return TeamExt::CountConditionMet(pThis, counter);
}

bool TeamExt::HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, const std::vector<TechnoTypeClass*>& list)
{
	int counter = 0;
	if (pHouse)
	{
		for (auto const pItem : list)
		{
			if (!pItem) continue;

			if (!allies)
			{
				counter += pHouse->CountOwnedAndPresent(pItem);
			}
			else
			{
				for (auto const pOtherHouse : HouseClass::Array)
				{
					if (pOtherHouse != pHouse && pHouse->IsAlliedWith(pOtherHouse) && !pOtherHouse->Type->MultiplayPassive)
					{
						counter += pOtherHouse->CountOwnedAndPresent(pItem);
					}
				}
			}
		}
	}
	return TeamExt::CountConditionMet(pThis, counter);
}

bool TeamExt::EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, TechnoTypeClass* pItem)
{
	int counter = 0;
	if (pItem && pHouse)
	{
		if (onlySelectedEnemy && pEnemy && !pHouse->IsAlliedWith(pEnemy))
		{
			counter = pEnemy->CountOwnedAndPresent(pItem);
		}
		else
		{
			for (auto const pOtherHouse : HouseClass::Array)
			{
				if (pOtherHouse != pHouse && !pHouse->IsAlliedWith(pOtherHouse) && !pOtherHouse->Type->MultiplayPassive)
				{
					counter += pOtherHouse->CountOwnedAndPresent(pItem);
				}
			}
		}
	}
	return TeamExt::CountConditionMet(pThis, counter);
}

bool TeamExt::EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, const std::vector<TechnoTypeClass*>& list)
{
	int counter = 0;
	if (pHouse)
	{
		for (auto const pItem : list)
		{
			if (!pItem) continue;

			if (onlySelectedEnemy && pEnemy && !pHouse->IsAlliedWith(pEnemy))
			{
				counter += pEnemy->CountOwnedAndPresent(pItem);
			}
			else
			{
				for (auto const pOtherHouse : HouseClass::Array)
				{
					if (pOtherHouse != pHouse && !pHouse->IsAlliedWith(pOtherHouse) && !pOtherHouse->Type->MultiplayPassive)
					{
						counter += pOtherHouse->CountOwnedAndPresent(pItem);
					}
				}
			}
		}
	}
	return TeamExt::CountConditionMet(pThis, counter);
}

bool TeamExt::NeutralOwns(AITriggerTypeClass* pThis, TechnoTypeClass* pItem)
{
	int counter = 0;
	if (pItem)
	{
		for (auto const pOtherHouse : HouseClass::Array)
		{
			if (_stricmp(SideClass::Array.GetItem(pOtherHouse->Type->SideIndex)->Name, "Civilian") == 0)
			{
				counter += pOtherHouse->CountOwnedAndPresent(pItem);
			}
		}
	}
	return TeamExt::CountConditionMet(pThis, counter);
}

bool TeamExt::NeutralOwns(AITriggerTypeClass* pThis, const std::vector<TechnoTypeClass*>& list)
{
	int counter = 0;
	for (auto const pItem : list)
	{
		if (!pItem) continue;

		for (auto const pOtherHouse : HouseClass::Array)
		{
			if (_stricmp(SideClass::Array.GetItem(pOtherHouse->Type->SideIndex)->Name, "Civilian") == 0)
			{
				counter += pOtherHouse->CountOwnedAndPresent(pItem);
			}
		}
	}
	return TeamExt::CountConditionMet(pThis, counter);
}

bool TeamExt::HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, const std::vector<TechnoTypeClass*>& list)
{
	if (list.empty() || !pHouse)
		return false;

	for (auto const pItem : list)
	{
		if (!pItem) continue;

		int const counter = pHouse->CountOwnedAndPresent(pItem);
		if (!TeamExt::CountConditionMet(pThis, counter))
			return false;
	}

	return true;
}

bool TeamExt::EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, const std::vector<TechnoTypeClass*>& list)
{
	if (list.empty() || !pHouse)
		return false;

	if (pEnemy && pHouse->IsAlliedWith(pEnemy))
		pEnemy = nullptr;

	for (auto const pItem : list)
	{
		if (!pItem) continue;

		int counter = 0;
		if (pEnemy)
		{
			counter = pEnemy->CountOwnedAndPresent(pItem);
		}
		else
		{
			for (auto const pOtherHouse : HouseClass::Array)
			{
				if (pOtherHouse != pHouse && !pHouse->IsAlliedWith(pOtherHouse) && !pOtherHouse->Type->MultiplayPassive)
				{
					counter += pOtherHouse->CountOwnedAndPresent(pItem);
				}
			}
		}

		if (!TeamExt::CountConditionMet(pThis, counter))
			return false;
	}

	return true;
}

bool TeamExt::NeutralOwnsAll(AITriggerTypeClass* pThis, const std::vector<TechnoTypeClass*>& list)
{
	if (list.empty())
		return false;

	for (auto const pOtherHouse : HouseClass::Array)
	{
		if (_stricmp(SideClass::Array.GetItem(pOtherHouse->Type->SideIndex)->Name, "Civilian") != 0)
			continue;

		bool allMet = true;
		for (auto const pItem : list)
		{
			if (!pItem) continue;

			int const counter = pOtherHouse->CountOwnedAndPresent(pItem);
			if (!TeamExt::CountConditionMet(pThis, counter))
			{
				allMet = false;
				break;
			}
		}

		if (allMet)
			return true;
	}

	return false;
}

bool TeamExt::CountConditionMet(AITriggerTypeClass* pThis, int nObjects)
{
	if (!pThis || nObjects < 0)
		return false;

	switch (pThis->Conditions[0].ComparatorOperand)
	{
	case 0:
		return nObjects < pThis->Conditions[0].ComparatorType;
	case 1:
		return nObjects <= pThis->Conditions[0].ComparatorType;
	case 2:
		return nObjects == pThis->Conditions[0].ComparatorType;
	case 3:
		return nObjects >= pThis->Conditions[0].ComparatorType;
	case 4:
		return nObjects > pThis->Conditions[0].ComparatorType;
	case 5:
		return nObjects != pThis->Conditions[0].ComparatorType;
	default:
		return true;
	}
}
