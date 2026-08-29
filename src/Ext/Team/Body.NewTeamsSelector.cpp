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

		percentageUnclassifiedTriggers = (percentageUnclassifiedTriggers < 0.0) ? 0.0 : percentageUnclassifiedTriggers;
		percentageGroundTriggers = (percentageGroundTriggers < 0.0) ? 0.0 : percentageGroundTriggers;
		percentageNavalTriggers = (percentageNavalTriggers < 0.0) ? 0.0 : percentageNavalTriggers;
		percentageAirTriggers = (percentageAirTriggers < 0.0) ? 0.0 : percentageAirTriggers;
		percentageInfantryTriggers = (percentageInfantryTriggers < 0.0) ? 0.0 : percentageInfantryTriggers;

		double const totalPercentages = percentageUnclassifiedTriggers + percentageGroundTriggers
			+ percentageNavalTriggers + percentageAirTriggers + percentageInfantryTriggers;

		if (totalPercentages <= 0.0)
		{
			splitTriggersByCategory = false;
		}
		else
		{
			double const categoryRoll = ScenarioClass::Instance->Random.RandomDouble() * totalPercentages;
			double accumulated = 0.0;

			// Pick what type of team will be selected in this round
			if (percentageUnclassifiedTriggers > 0.0 && categoryRoll < (accumulated += percentageUnclassifiedTriggers))
			{
				validCategory = teamCategory::Unclassified;
			}
			else if (percentageGroundTriggers > 0.0 && categoryRoll < (accumulated += percentageGroundTriggers))
			{
				validCategory = teamCategory::Ground;
			}
			else if (percentageAirTriggers > 0.0 && categoryRoll < (accumulated += percentageAirTriggers))
			{
				validCategory = teamCategory::Air;
			}
			else if (percentageNavalTriggers > 0.0 && categoryRoll < (accumulated += percentageNavalTriggers))
			{
				validCategory = teamCategory::Naval;
			}
			else if (percentageInfantryTriggers > 0.0 && categoryRoll < (accumulated += percentageInfantryTriggers))
			{
				validCategory = teamCategory::Infantry;
			}
			else
			{
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

	auto houseDifficulty = pHouse->AIDifficulty;
	int minBaseDefenseTeams = RulesClass::Instance->MinimumAIDefensiveTeams.GetItem((int)houseDifficulty);
	int maxBaseDefenseTeams = RulesClass::Instance->MaximumAIDefensiveTeams.GetItem((int)houseDifficulty);
	int activeDefenseTeamsCount = 0;
	int maxTeamsLimit = RulesClass::Instance->TotalAITeamCap.GetItem((int)houseDifficulty);

	// Check running teams owned by this house
	std::unordered_map<TeamTypeClass*, int> activeTeamCounts;

	for (auto const pRunningTeam : TeamClass::Array)
	{
		totalActiveTeams++;
		if (pRunningTeam->Owner != pHouse)
			continue;

		activeTeams++;
		activeTeamCounts[pRunningTeam->Type]++;

		if (pRunningTeam->Type->IsBaseDefense)
			activeDefenseTeamsCount++;
	}

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
		Debug::Log("AITeamsSelector - House %d [%s](%s) reached the TotalAITeamCap (%d)! Skipping operation this time...\n", pHouse->ArrayIndex, pHouse->PlainName, pHouse->Type->ID, maxTeamsLimit);
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
		if (!TechnoExt::IsValidTechno(pTechno) || !pTechno->IsAlive || pTechno->Health <= 0)
			continue;

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
			else if (pBuildingType->BridgeRepairHut)
			{
				CellStruct cell = pTechno->GetCell()->MapCoords;

				if (MapClass::Instance.IsLinkedBridgeDestroyed(cell))
					destroyedBridgesCount++;
				else
					undamagedBridgesCount++;
			}

			continue;
		}

		auto const pFoot = static_cast<FootClass*>(pTechno);

		if (!pFoot
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

		// If VIP triggers were found, discard any non-VIP trigger immediately
		if (onlyCheckImportantTriggers && pTrigger->Weight_Current < maxPriority)
			continue;

		if (pTrigger->Team1->TechLevel > pHouse->TechLevel)
			continue;

		// Ignore offensive teams if the next trigger must be defensive
		if ((onlyPickDefensiveTeams && !pTrigger->IsForBaseDefense) || (hasReachedMaxDefensiveTeamsLimit && pTrigger->IsForBaseDefense))
			continue;

		// If this type of Team reached the max then skip it immediately
		auto const itTeam1 = activeTeamCounts.find(pTrigger->Team1);
		int const currentTeam1Count = (itTeam1 != activeTeamCounts.end()) ? itTeam1->second : 0;

		if (currentTeam1Count >= pTrigger->Team1->Max)
			continue;

		teamCategory teamIsCategory = teamCategory::None;

		// Analyze what kind of category is this main team if the feature is enabled
		if (splitTriggersByCategory)
		{
			// TaskForces are limited to 6 entries
			for (int i = 0; i < 6; i++)
			{
				const auto& entry = pTrigger->Team1->TaskForce->Entries[i];
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

					// If a team has multiple categories it will be a mixed category
					teamIsCategory = (teamIsCategory == teamCategory::None || teamIsCategory == entryIsCategory) ? entryIsCategory : teamCategory::Unclassified;

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

			// Early discard if fallback is disabled and category does not match
			if (!isFallbackEnabled && validCategory != teamCategory::None && validCategory != teamIsCategory)
				continue;
		}

		// The trigger must be compatible with the owner ("ConditionType=-1" is always valid)
		if ((int)pTrigger->ConditionType >= 0)
		{
			bool conditionMet = true;

			switch ((int)pTrigger->ConditionType)
			{
			case 0:
				// Case 0: "enemy owns"
				conditionMet = pTrigger->ConditionObject && TeamExt::EnemyOwns(pTrigger, pHouse, targetHouse, true, pTrigger->ConditionObject);
				break;

			case 1:
				// Case 1: "house owns"
				conditionMet = pTrigger->ConditionObject && TeamExt::HouseOwns(pTrigger, pHouse, false, pTrigger->ConditionObject);
				break;

			case 4:
				// Case 4: "Enemy house economy threshold?"
				conditionMet = pTrigger->HouseCredits(nullptr, targetHouse);
				break;

			case 5:
				// Case 5: "Iron Curtain is charged?"
				conditionMet = pTrigger->IronCurtainCharged(pHouse, nullptr);
				break;

			case 6:
				// Case 6: "Chronosphere is charged?"
				conditionMet = pTrigger->ChronoSphereCharged(pHouse, nullptr);
				break;

			case 7:
				// Case 7: "civilian owns"
				conditionMet = pTrigger->ConditionObject && TeamExt::NeutralOwns(pTrigger, pTrigger->ConditionObject);
				break;

			case 8:
				// Case 8: "enemy owns" across all enemies
				conditionMet = pTrigger->ConditionObject && TeamExt::EnemyOwns(pTrigger, pHouse, nullptr, false, pTrigger->ConditionObject);
				break;

			case 9:
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					conditionMet = (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
						&& TeamExt::EnemyOwns(pTrigger, pHouse, targetHouse, false, RulesExt::Global()->AITargetTypesLists[listIdx]);
				}
				break;

			case 10:
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					conditionMet = (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
						&& TeamExt::HouseOwns(pTrigger, pHouse, false, RulesExt::Global()->AITargetTypesLists[listIdx]);
				}
				break;

			case 11:
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					conditionMet = (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
						&& TeamExt::NeutralOwns(pTrigger, RulesExt::Global()->AITargetTypesLists[listIdx]);
				}
				break;

			case 12:
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					conditionMet = (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
						&& TeamExt::EnemyOwns(pTrigger, pHouse, nullptr, false, RulesExt::Global()->AITargetTypesLists[listIdx]);
				}
				break;

			case 13:
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					conditionMet = (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
						&& TeamExt::HouseOwns(pTrigger, pHouse, true, RulesExt::Global()->AITargetTypesLists[listIdx]);
				}
				break;

			case 14:
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					conditionMet = (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
						&& TeamExt::EnemyOwnsAll(pTrigger, pHouse, targetHouse, RulesExt::Global()->AITargetTypesLists[listIdx]);
				}
				break;

			case 15:
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					conditionMet = (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
						&& TeamExt::HouseOwnsAll(pTrigger, pHouse, RulesExt::Global()->AITargetTypesLists[listIdx]);
				}
				break;

			case 16:
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					conditionMet = (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
						&& TeamExt::NeutralOwnsAll(pTrigger, RulesExt::Global()->AITargetTypesLists[listIdx]);
				}
				break;

			case 17:
				{
					auto const listIdx = pTrigger->Conditions[3].ComparatorOperand;
					conditionMet = (listIdx >= 0 && (size_t)listIdx < RulesExt::Global()->AITargetTypesLists.size())
						&& TeamExt::EnemyOwnsAll(pTrigger, pHouse, nullptr, RulesExt::Global()->AITargetTypesLists[listIdx]);
				}
				break;

			case 18:
				// Case 18: Check destroyed bridges
				conditionMet = TeamExt::CountConditionMet(pTrigger, destroyedBridgesCount);
				break;

			case 19:
				// Case 19: Check undamaged bridges
				conditionMet = TeamExt::CountConditionMet(pTrigger, undamagedBridgesCount);
				break;

			default:
				// Other cases from vanilla game
				conditionMet = pTrigger->ConditionMet(pHouse, targetHouse, hasReachedMaxDefensiveTeamsLimit);
				break;
			}

			if (!conditionMet)
				continue;
		}

		if (!pTrigger->Team1->Autocreate && !pTrigger->Team1->Recruiter)
			continue;

		bool canProduce = false;

		// Check if the team can be produced via autocreate
		if (pTrigger->Team1->Autocreate && canAutocreate)
		{
			canProduce = true;

			for (const auto& entry : pTrigger->Team1->TaskForce->Entries)
			{
				if (!entry.Type || entry.Amount == 0)
					break;

				bool hasFactory = false;
				switch (entry.Type->WhatAmI())
				{
				case AbstractType::InfantryType:
					hasFactory = hasInfantryFactory;
					break;

				case AbstractType::AircraftType:
					hasFactory = hasAircraftFactory;
					break;

				case AbstractType::UnitType:
					hasFactory = entry.Type->Naval ? hasNavalFactory : hasUnitFactory;
					break;

				default:
					break;
				}

				if (!hasFactory || !HouseExt::PrerequisitesMet(pHouse, entry.Type, false))
				{
					canProduce = false;
					break;
				}
			}
		}

		// Check if the team can be recruited from existing map units
		if (!canProduce && pTrigger->Team1->Recruiter)
		{
			bool canRecruit = true;

			for (const auto& entry : pTrigger->Team1->TaskForce->Entries)
			{
				if (!entry.Type || entry.Amount == 0)
					break;

				auto const it = ownedRecruitables.find(entry.Type);
				int const availableRecruits = (it != ownedRecruitables.end()) ? it->second : 0;

				if (availableRecruits < entry.Amount)
				{
					canRecruit = false;
					break;
				}
			}

			canProduce = canRecruit;
		}

		if (!canProduce)
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

	if (validCandidates.empty())
	{
		Debug::Log("AITeamsSelector - The house %d [%s](%s) has no valid triggers for now. A new attempt will be done later...\n", pHouse->ArrayIndex, pHouse->PlainName, pHouse->Type->ID);
		return SkipCode;
	}

	// Calculate category stats and candidate counts
	double totalPoolWeight = 0.0;
	int poolCandidatesCount = 0;

	for (const auto& candidate : validCandidates)
	{
		if (validCategory == teamCategory::None || candidate.Category == validCategory)
		{
			totalPoolWeight += candidate.Weight;
			poolCandidatesCount++;
		}
	}

	if (validCategory != teamCategory::None && poolCandidatesCount == 0)
	{
		Debug::Log("AITeamsSelector - The house %d [%s](%s) has no valid triggers of category %d. A new attempt should be done later...\n",
			pHouse->ArrayIndex, pHouse->PlainName, pHouse->Type->ID, static_cast<int>(validCategory));

		if (!isFallbackEnabled)
			return SkipCode;

		Debug::Log("... but FALLBACK MODE is enabled so now all available triggers will be checked.\n");
		validCategory = teamCategory::None;

		for (const auto& candidate : validCandidates)
			totalPoolWeight += candidate.Weight;

		poolCandidatesCount = static_cast<int>(validCandidates.size());
	}

	AITriggerTypeClass* selectedTrigger = nullptr;

	// Roulette Wheel Selection (Discrete Distribution)
	if (totalPoolWeight > 0.0)
	{
		double const weightDice = ScenarioClass::Instance->Random.RandomDouble() * totalPoolWeight;

		if (validCategory != teamCategory::None)
		{
			Debug::Log("AITeamsSelector - Picking 1 team (of category %d) from the %d available (Total Weight: %f, Roll: %f)...\n",
				static_cast<int>(validCategory), poolCandidatesCount, totalPoolWeight, weightDice);
		}
		else
		{
			Debug::Log("AITeamsSelector - Picking 1 team from the %d available (Total Weight: %f, Roll: %f)...\n",
				poolCandidatesCount, totalPoolWeight, weightDice);
		}

		double accumulated = 0.0;
		for (const auto& candidate : validCandidates)
		{
			if (validCategory == teamCategory::None || candidate.Category == validCategory)
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
	auto const itTeam1 = activeTeamCounts.find(selectedTrigger->Team1);
	int const count1 = (itTeam1 != activeTeamCounts.end()) ? itTeam1->second : 0;

	if (count1 < selectedTrigger->Team1->Max)
	{
		if (TeamClass* newTeam1 = selectedTrigger->Team1->CreateTeam(pHouse))
			newTeam1->NeedsToDisappear = false;
	}

	// Team 2 creation (if set)
	if (auto const pTriggerTeam2Type = selectedTrigger->Team2)
	{
		auto const itTeam2 = activeTeamCounts.find(pTriggerTeam2Type);
		int const count2 = (itTeam2 != activeTeamCounts.end()) ? itTeam2->second : 0;

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
		int const civilianSideIndex = SideClass::FindIndex("Civilian");
		for (auto const pOtherHouse : HouseClass::Array)
		{
			if (pOtherHouse->Type->SideIndex == civilianSideIndex)
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
	int const civilianSideIndex = SideClass::FindIndex("Civilian");
	for (auto const pItem : list)
	{
		if (!pItem) continue;

		for (auto const pOtherHouse : HouseClass::Array)
		{
			if (pOtherHouse->Type->SideIndex == civilianSideIndex)
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

	int const civilianSideIndex = SideClass::FindIndex("Civilian");
	for (auto const pOtherHouse : HouseClass::Array)
	{
		if (pOtherHouse->Type->SideIndex != civilianSideIndex)
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
