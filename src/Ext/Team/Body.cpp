#include "Body.h"

template<> const DWORD Extension<TeamClass>::Canary = 0x414B4B41;
TeamExt::ExtContainer TeamExt::ExtMap;

// =============================
// load / save

template <typename T>
void TeamExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->WaitNoTargetAttempts)
		.Process(this->NextSuccessWeightAward)
		.Process(this->IdxSelectedObjectFromAIList)
		.Process(this->CloseEnough)
		.Process(this->Countdown_RegroupAtLeader)
		.Process(this->MoveMissionEndMode)
		.Process(this->WaitNoTargetCounter)
		.Process(this->WaitNoTargetTimer)
		.Process(this->ForceJump_Countdown)
		.Process(this->ForceJump_InitialCountdown)
		.Process(this->ForceJump_RepeatMode)
		.Process(this->TeamLeader)
		;
}

void TeamExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TeamClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TeamExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TeamClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

TeamExt::ExtContainer::ExtContainer() : Container("TeamClass") { }
TeamExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

//Everything InitEd beside the Vector below this address
DEFINE_HOOK(0x6E8B46, TeamClass_CTOR, 0x7)
{
	GET(TeamClass*, pThis, ESI);

	TeamExt::ExtMap.FindOrAllocate(pThis);

	return 0;
}

//before `test` i hope not crash the game ,..
DEFINE_HOOK(0x6E8EC6, TeamClass_DTOR, 0x9)
{
	GET(TeamClass*, pThis, ESI);

	TeamExt::ExtMap.Remove(pThis);

	return 0;
}

DEFINE_HOOK_AGAIN(0x6EC450, TeamClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6EC540, TeamClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(TeamClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TeamExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6EC52F, TeamClass_Load_Suffix, 0x6)
{
	TeamExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x6EC55A, TeamClass_Save_Suffix, 0x5)
{
	TeamExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x4F8A27, TeamTypeClass_SuggestedNewTeam_NewTeamsSelector, 0x5)
{
	enum { UseOriginalSelector = 0x4F8A63, SkipCode = 0x4F8B08 };

	GET(HouseClass*, pHouse, ESI);

	bool houseIsHuman = pHouse->IsHumanPlayer;

	if (SessionClass::IsCampaign())
		houseIsHuman = pHouse->IsHumanPlayer || pHouse->IsInPlayerControl;

	if (houseIsHuman || pHouse->Type->MultiplayPassive)
		return SkipCode;

	auto pHouseTypeExt = HouseTypeExt::ExtMap.Find(pHouse->Type);
	if (!pHouseTypeExt)
		return SkipCode;

	if (!RulesExt::Global()->NewTeamsSelector)
		return UseOriginalSelector;

	// Reset Team selection countdown
	int countdown = RulesClass::Instance->TeamDelays[(int)pHouse->AIDifficulty];
	pHouse->TeamDelayTimer.Start(countdown);

	int totalActiveTeams = 0;
	int activeTeams = 0;

	int totalGroundCategoryTriggers = 0;
	int totalUnclassifiedCategoryTriggers = 0;
	int totalNavalCategoryTriggers = 0;
	int totalAirCategoryTriggers = 0;

	DynamicVectorClass<TriggerElementWeight> validTriggerCandidates;
	DynamicVectorClass<TriggerElementWeight> validTriggerCandidatesGroundOnly;
	DynamicVectorClass<TriggerElementWeight> validTriggerCandidatesNavalOnly;
	DynamicVectorClass<TriggerElementWeight> validTriggerCandidatesAirOnly;
	DynamicVectorClass<TriggerElementWeight> validTriggerCandidatesUnclassifiedOnly;

	int dice = ScenarioClass::Instance->Random.RandomRanged(1, 100);

	// This house must have the triggers enabled
	if (dice <= pHouse->RatioAITriggerTeam && pHouse->AITriggersActive)
	{
		bool splitTriggersByCategory = RulesExt::Global()->NewTeamsSelector_SplitTriggersByCategory;
		bool isFallbackEnabled = RulesExt::Global()->NewTeamsSelector_EnableFallback;
		teamCategory validCategory = teamCategory::None;
		int mergeUnclassifiedCategoryWith = -1;

		double percentageUnclassifiedTriggers = 0.0;
		double percentageGroundTriggers = 0.0;
		double percentageNavalTriggers = 0.0;
		double percentageAirTriggers = 0.0;

		if (splitTriggersByCategory)
		{
			mergeUnclassifiedCategoryWith = pHouseTypeExt->NewTeamsSelector_MergeUnclassifiedCategoryWith.isset() ? pHouseTypeExt->NewTeamsSelector_MergeUnclassifiedCategoryWith.Get() : RulesExt::Global()->NewTeamsSelector_MergeUnclassifiedCategoryWith;  // Should mixed teams be merged into another category?
			percentageUnclassifiedTriggers = pHouseTypeExt->NewTeamsSelector_UnclassifiedCategoryPercentage.isset() ? pHouseTypeExt->NewTeamsSelector_UnclassifiedCategoryPercentage.Get() : RulesExt::Global()->NewTeamsSelector_UnclassifiedCategoryPercentage; // Mixed teams
			percentageGroundTriggers = pHouseTypeExt->NewTeamsSelector_GroundCategoryPercentage.isset() ? pHouseTypeExt->NewTeamsSelector_GroundCategoryPercentage.Get() : RulesExt::Global()->NewTeamsSelector_GroundCategoryPercentage; // Only ground
			percentageNavalTriggers = pHouseTypeExt->NewTeamsSelector_NavalCategoryPercentage.isset() ? pHouseTypeExt->NewTeamsSelector_NavalCategoryPercentage.Get() : RulesExt::Global()->NewTeamsSelector_NavalCategoryPercentage; // Only Naval=yes
			percentageAirTriggers = pHouseTypeExt->NewTeamsSelector_AirCategoryPercentage.isset() ? pHouseTypeExt->NewTeamsSelector_AirCategoryPercentage.Get() : RulesExt::Global()->NewTeamsSelector_AirCategoryPercentage; // Only Aircrafts & jumpjets

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

				default:
					break;
				}

				percentageUnclassifiedTriggers = 0.0;
			}

			percentageUnclassifiedTriggers = percentageUnclassifiedTriggers < 0.0 || percentageUnclassifiedTriggers > 1.0 ? 0.0 : percentageUnclassifiedTriggers;
			percentageGroundTriggers = percentageGroundTriggers < 0.0 || percentageGroundTriggers > 1.0 ? 0.0 : percentageGroundTriggers;
			percentageNavalTriggers = percentageNavalTriggers < 0.0 || percentageNavalTriggers > 1.0 ? 0.0 : percentageNavalTriggers;
			percentageAirTriggers = percentageAirTriggers < 0.0 || percentageAirTriggers > 1.0 ? 0.0 : percentageAirTriggers;

			double totalPercengates = percentageUnclassifiedTriggers + percentageGroundTriggers + percentageNavalTriggers + percentageAirTriggers;

			if (totalPercengates > 1.0 || totalPercengates <= 0.0)
				splitTriggersByCategory = false;
			// Note: if the sum of all percentages is less than 100% then that empty space will work like "no categories"

			if (splitTriggersByCategory)
			{
				int categoryDice = ScenarioClass::Instance->Random.RandomRanged(1, 100);

				if (categoryDice == 0)
				{
					splitTriggersByCategory = false;
				}
				else if (categoryDice <= (int)(percentageUnclassifiedTriggers * 100.0))
				{
					validCategory = teamCategory::Unclassified;
				}
				else if (categoryDice <= (int)((percentageUnclassifiedTriggers + percentageGroundTriggers) * 100.0))
				{
					validCategory = teamCategory::Ground;
				}
				else if (categoryDice <= (int)((percentageUnclassifiedTriggers + percentageGroundTriggers + percentageNavalTriggers) * 100.0))
				{
					validCategory = teamCategory::Naval;
				}
				else if (categoryDice <= (int)((percentageUnclassifiedTriggers + percentageGroundTriggers + percentageNavalTriggers + percentageAirTriggers) * 100.0))
				{
					validCategory = teamCategory::Air;
				}
				else
				{
					splitTriggersByCategory = false;
				}
			}
		}

		int houseIdx = pHouse->ArrayIndex;
		int sideIdx = pHouse->SideIndex + 1;
		auto houseDifficulty = pHouse->AIDifficulty;
		int maxBaseDefenseTeams = RulesClass::Instance->MaximumAIDefensiveTeams.GetItem((int)houseDifficulty);
		int activeDefenseTeamsCount = 0;
		int maxTeamsLimit = RulesClass::Instance->TotalAITeamCap.GetItem((int)houseDifficulty);
		double totalWeight = 0.0;
		double totalWeightGroundOnly = 0.0;
		double totalWeightNavalOnly = 0.0;
		double totalWeightAirOnly = 0.0;
		double totalWeightUnclassifiedOnly = 0.0;

		// Check if the running teams by the house already reached all the limits
		DynamicVectorClass<TeamClass*> activeTeamsList;

		for (auto const pRunningTeam : *TeamClass::Array)
		{
			totalActiveTeams++;
			int teamHouseIdx = pRunningTeam->Owner->ArrayIndex;
			//auto teamHouse = pRunningTeam->GetOwningHouse();
			if (teamHouseIdx != houseIdx)
				continue;

			activeTeamsList.AddItem(pRunningTeam);

			if (pRunningTeam->Type->IsBaseDefense && activeDefenseTeamsCount < maxBaseDefenseTeams)
				activeDefenseTeamsCount++;
		}

		activeTeams = activeTeamsList.Count;

		// We will use these values for discarding triggers
		bool hasReachedMaxTeamsLimit = activeTeams < maxTeamsLimit ? false : true;
		bool hasReachedMaxDefensiveTeamsLimit = activeDefenseTeamsCount < maxBaseDefenseTeams ? false : true;

		if (hasReachedMaxDefensiveTeamsLimit)
			Debug::Log("DEBUG: House [%s] (idx: %d) reached the MaximumAIDefensiveTeams value!\n", pHouse->Type->ID, pHouse->ArrayIndex);

		if (hasReachedMaxTeamsLimit)
		{
			Debug::Log("DEBUG: House [%s] (idx: %d) reached the TotalAITeamCap value!\n", pHouse->Type->ID, pHouse->ArrayIndex);
			return SkipCode;
		}

		// Obtain the real list of structures the house have
		DynamicVectorClass<BuildingTypeClass*> ownedBuildingTypes;
		for (auto building : pHouse->Buildings)
		{
			ownedBuildingTypes.AddUnique(building->Type);
		}

		struct recruitableUnit
		{
			TechnoTypeClass* object = nullptr;
			int count = 1;

			bool operator==(const TechnoTypeClass* other) const
			{
				return (object == other);
			}

			bool operator==(const recruitableUnit other) const
			{
				return (object == other.object);
			}
		};

		// Build a list of recruitable units by the house
		DynamicVectorClass<recruitableUnit> recruitableUnits;

		for (auto pTechno : *TechnoClass::Array)
		{
			if (pTechno->WhatAmI() == AbstractType::Building)
				continue;

			FootClass* pFoot = static_cast<FootClass*>(pTechno);

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

			auto pTechnoType = pTechno->GetTechnoType();
			bool found = false;

			for (int i = 0; i < recruitableUnits.Count; i++)
			{
				if (recruitableUnits[i].object == pTechnoType)
				{
					recruitableUnits[i].count++;
					found = true;
					break;
				}
			}

			if (!found)
			{
				recruitableUnit newRecruitable;
				newRecruitable.object = pTechnoType;
				recruitableUnits.AddItem(newRecruitable);
			}
		}

		HouseClass* targetHouse = nullptr;
		if (pHouse->EnemyHouseIndex >= 0)
			targetHouse = HouseClass::Array->GetItem(pHouse->EnemyHouseIndex);

		bool onlyCheckImportantTriggers = false;

		// Gather all the trigger candidates into one place for posterior fast calculations
		for (auto const pTrigger : *AITriggerTypeClass::Array)
		{
			if (!pTrigger)
				continue;

			int triggerHouse = pTrigger->HouseIndex;
			int triggerSide = pTrigger->SideIndex;

			// Ignore the deactivated triggers
			if (pTrigger->IsEnabled)
			{
				// The trigger must be compatible with the owner
				if ((triggerHouse == -1 || houseIdx == triggerHouse) && (triggerSide == 0 || sideIdx == triggerSide))
				{
					// "ConditionType=-1" will be skipped, always is valid
					if ((int)pTrigger->ConditionType >= 0)
					{
						if ((int)pTrigger->ConditionType == 0)
						{
							// Simulate case 0: "enemy owns"
							if (!pTrigger->ConditionObject)
								continue;

							DynamicVectorClass<TechnoTypeClass*> list;
							list.AddItem(pTrigger->ConditionObject);
							bool isConditionMet = TeamExt::EnemyOwns(pTrigger, pHouse, targetHouse, true, list);

							if (!isConditionMet)
								continue;
						}
						else if ((int)pTrigger->ConditionType == 1)
						{
							// Simulate case 0: "house owns"
							if (!pTrigger->ConditionObject)
								continue;

							DynamicVectorClass<TechnoTypeClass*> list;
							list.AddItem(pTrigger->ConditionObject);
							bool isConditionMet = TeamExt::HouseOwns(pTrigger, pHouse, false, list);

							if (!isConditionMet)
								continue;
						}
						else if ((int)pTrigger->ConditionType == 7)
						{
							// Simulate case 7: "civilian owns"
							if (!pTrigger->ConditionObject)
								continue;

							DynamicVectorClass<TechnoTypeClass*> list;
							list.AddItem(pTrigger->ConditionObject);
							bool isConditionMet = TeamExt::NeutralOwns(pTrigger, list);

							if (!isConditionMet)
								continue;
						}
						else if ((int)pTrigger->ConditionType == 8)
						{
							// Simulate case 0: "enemy owns" but instead of restrict it against the main enemy house it is done against all enemies
							if (!pTrigger->ConditionObject)
								continue;

							DynamicVectorClass<TechnoTypeClass*> list;
							list.AddItem(pTrigger->ConditionObject);
							bool isConditionMet = TeamExt::EnemyOwns(pTrigger, pHouse, nullptr, false, list);

							if (!isConditionMet)
								continue;
						}
						else if ((int)pTrigger->ConditionType == 9)
						{
							if (pTrigger->Conditions[3].ComparatorOperand < RulesExt::Global()->AITargetTypesLists.Count)
							{
								// New case 9: Like in case 0 but instead of 1 unit for comparisons there is a full list from [AITargetTypes] owned by the enemy.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								DynamicVectorClass<TechnoTypeClass*> list = RulesExt::Global()->AITargetTypesLists.GetItem(pTrigger->Conditions[3].ComparatorOperand);
								bool isConditionMet = TeamExt::EnemyOwns(pTrigger, pHouse, targetHouse, false, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 10)
						{
							if (pTrigger->Conditions[3].ComparatorOperand < RulesExt::Global()->AITargetTypesLists.Count)
							{
								// New case 10: Like in case 1 but instead of 1 unit for comparisons there is a full list from [AITargetTypes] owned by the house.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								DynamicVectorClass<TechnoTypeClass*> list = RulesExt::Global()->AITargetTypesLists.GetItem(pTrigger->Conditions[3].ComparatorOperand);
								bool isConditionMet = TeamExt::HouseOwns(pTrigger, pHouse, false, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 11)
						{
							if (pTrigger->Conditions[3].ComparatorOperand < RulesExt::Global()->AITargetTypesLists.Count)
							{
								// New case 11: Like in case 7 but instead of 1 unit for comparisons there is a full list from [AITargetTypes] owned by the Civilians.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								DynamicVectorClass<TechnoTypeClass*> list = RulesExt::Global()->AITargetTypesLists.GetItem(pTrigger->Conditions[3].ComparatorOperand);
								bool isConditionMet = TeamExt::NeutralOwns(pTrigger, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 12)
						{
							if (pTrigger->Conditions[3].ComparatorOperand < RulesExt::Global()->AITargetTypesLists.Count)
							{
								// New case 12: Like in case 0 & 9 but instead of a specific enemy this checks in all enemies.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								DynamicVectorClass<TechnoTypeClass*> list = RulesExt::Global()->AITargetTypesLists.GetItem(pTrigger->Conditions[3].ComparatorOperand);
								bool isConditionMet = TeamExt::EnemyOwns(pTrigger, pHouse, nullptr, false, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 13)
						{
							if (pTrigger->Conditions[3].ComparatorOperand < RulesExt::Global()->AITargetTypesLists.Count)
							{
								// New case 13: Like in case 1 & 10 but instead checking the house now checks the allies.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								DynamicVectorClass<TechnoTypeClass*> list = RulesExt::Global()->AITargetTypesLists.GetItem(pTrigger->Conditions[3].ComparatorOperand);
								bool isConditionMet = TeamExt::HouseOwns(pTrigger, pHouse, true, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 14)
						{
							if (pTrigger->Conditions[3].ComparatorOperand < RulesExt::Global()->AITargetTypesLists.Count)
							{
								// New case 14: Like in case 9 but instead of meet any comparison now is required all.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								DynamicVectorClass<TechnoTypeClass*> list = RulesExt::Global()->AITargetTypesLists.GetItem(pTrigger->Conditions[3].ComparatorOperand);
								bool isConditionMet = TeamExt::EnemyOwnsAll(pTrigger, pHouse, targetHouse, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 15)
						{
							if (pTrigger->Conditions[3].ComparatorOperand < RulesExt::Global()->AITargetTypesLists.Count)
							{
								// New case 15: Like in case 10 but instead of meet any comparison now is required all.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								DynamicVectorClass<TechnoTypeClass*> list = RulesExt::Global()->AITargetTypesLists.GetItem(pTrigger->Conditions[3].ComparatorOperand);
								bool isConditionMet = TeamExt::HouseOwnsAll(pTrigger, pHouse, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 16)
						{
							if (pTrigger->Conditions[3].ComparatorOperand < RulesExt::Global()->AITargetTypesLists.Count)
							{
								// New case 16: Like in case 11 but instead of meet any comparison now is required all.
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								DynamicVectorClass<TechnoTypeClass*> list = RulesExt::Global()->AITargetTypesLists.GetItem(pTrigger->Conditions[3].ComparatorOperand);
								bool isConditionMet = TeamExt::NeutralOwnsAll(pTrigger, list);

								if (!isConditionMet)
									continue;
							}
						}
						else if ((int)pTrigger->ConditionType == 17)
						{
							if (pTrigger->Conditions[3].ComparatorOperand < RulesExt::Global()->AITargetTypesLists.Count)
							{
								// New case 17: Like in case 14 but instead of meet any comparison now is required all. Check all enemies
								// Caution: Little Endian hexadecimal value stored here: 00000000000000000000000000000000000000000000000000000000AABBCCDD; examples: 255 is 0xFF (in AA) and 256 is 0x0001 (in AABB)
								DynamicVectorClass<TechnoTypeClass*> list = RulesExt::Global()->AITargetTypesLists.GetItem(pTrigger->Conditions[3].ComparatorOperand);
								bool isConditionMet = TeamExt::EnemyOwnsAll(pTrigger, pHouse, nullptr, list);

								if (!isConditionMet)
									continue;
							}
						}
						else
						{
							// Other cases from vanilla game
							if (!pTrigger->ConditionMet(pHouse, targetHouse, hasReachedMaxDefensiveTeamsLimit))
								continue;
						}
					}

					// All triggers below 5000 in current weight will get discarded if this mode is enabled
					if (onlyCheckImportantTriggers)
					{
						if (pTrigger->Weight_Current < 5000)
							continue;
					}

					auto pTriggerTeam1Type = pTrigger->Team1;
					if (!pTriggerTeam1Type)
						continue;

					// No more defensive teams needed
					if (pTriggerTeam1Type->IsBaseDefense && hasReachedMaxDefensiveTeamsLimit)
						continue;

					// If this type of Team reached the max then skip it
					int count = 0;

					for (auto team : activeTeamsList)
					{
						if (team->Type == pTriggerTeam1Type)
							count++;
					}

					if (count >= pTriggerTeam1Type->Max)
						continue;

					teamCategory teamIsCategory = teamCategory::None;

					// Analyze what kind of category is this main team if the feature is enabled
					if (splitTriggersByCategory)
					{
						for (auto entry : pTriggerTeam1Type->TaskForce->Entries)
						{
							// If they team has mixed members there is no need to continue
							// Also, if the category was merged into another one
							if (teamIsCategory == teamCategory::Unclassified)
							{
								if (mergeUnclassifiedCategoryWith >= 0)
									teamIsCategory = (teamCategory)mergeUnclassifiedCategoryWith;

								break;
							}

							if (entry.Amount > 0)
							{
								if (entry.Type)
								{
									if (entry.Type->WhatAmI() == AbstractType::AircraftType
										|| entry.Type->ConsideredAircraft)
									{
										// For now the team is from air category
										teamIsCategory = teamIsCategory == teamCategory::None || teamIsCategory == teamCategory::Air ? teamCategory::Air : teamCategory::Unclassified;
									}
									else if (entry.Type->Naval
										&& (entry.Type->MovementZone != MovementZone::Amphibious
											&& entry.Type->MovementZone != MovementZone::AmphibiousDestroyer
											&& entry.Type->MovementZone != MovementZone::AmphibiousCrusher))
									{
										// For now the team is from naval category
										teamIsCategory = teamIsCategory == teamCategory::None || teamIsCategory == teamCategory::Naval ? teamCategory::Naval : teamCategory::Unclassified;
									}
									else if (teamIsCategory != teamCategory::Naval
										&& teamIsCategory != teamCategory::Air)
									{
										// For now the team doesn't belong to the previous categories
										teamIsCategory = teamIsCategory != teamCategory::Unclassified ? teamCategory::Ground : teamCategory::Unclassified;
									}
								}
							}
							else
							{
								break;
							}
						}
					}

					bool allObjectsCanBeBuiltOrRecruited = true;
					//Debug::Log("[%s] [%s] is evaluating unit prerequisites...\n", pTrigger->ID, pTriggerTeam1Type->ID);

					if (pTriggerTeam1Type->Autocreate)
					{
						for (auto entry : pTriggerTeam1Type->TaskForce->Entries)
						{
							// Check if each unit in the taskforce meets the structure prerequisites
							if (entry.Amount > 0)
							{
								if (!entry.Type)
									continue;

								TechnoTypeClass* object = entry.Type;
								//bool canBeBuilt = pHouse->AllPrerequisitesAvailable(object, ownedBuildingTypes, ownedBuildingTypes.Count); // Old

								// Experimental
								bool canBeBuilt = HouseExt::PrerequisitesMet(pHouse, object, ownedBuildingTypes);

								if (!canBeBuilt)
								{
									//Debug::Log("[%s] not ok...\n", object->ID);
									allObjectsCanBeBuiltOrRecruited = false;
									break;
								}
								else
								{
									//Debug::Log("[%s] build prerequisites met!\n", object->ID);
								}
							}
							else
							{
								break;
							}
						}
					}
					else
					{
						allObjectsCanBeBuiltOrRecruited = false;
					}

					//if (allObjectsCanBeBuiltOrRecruited)
						//Debug::Log("[%s] [%s] can build units!...\n", pTrigger->ID, pTriggerTeam1Type->ID);

					if (!allObjectsCanBeBuiltOrRecruited && pTriggerTeam1Type->Recruiter)
					{
						allObjectsCanBeBuiltOrRecruited = true;
						//Debug::Log("[%s] [%s] checking if it can recruit...\n", pTrigger->ID, pTriggerTeam1Type->ID);
						for (auto entry : pTriggerTeam1Type->TaskForce->Entries)
						{
							// Check if each unit in the taskforce has the available recruitable units in the map
							if (allObjectsCanBeBuiltOrRecruited && entry.Amount > 0)
							{
								bool canBeRecruited = false;
								//Debug::Log("[%s] checking if there are recruitable...\n", entry.Type->ID);

								for (auto item : recruitableUnits)
								{
									if (item.object == entry.Type)
									{
										if (item.count >= entry.Amount)
											canBeRecruited = true;

										break;
									}
								}

								if (!canBeRecruited)
								{
									//Debug::Log("[%s] %d can not be recruited!\n", entry.Type->ID, entry.Amount);
									allObjectsCanBeBuiltOrRecruited = false;
									break;
								}
								else
								{
									//Debug::Log("[%s] can recruit %d units!\n", entry.Type->ID, entry.Amount);
								}
							}
						}
					}

					// We can't let AI cheat in this trigger because doesn't have the required tech tree available
					if (!allObjectsCanBeBuiltOrRecruited)
						continue;

					// Special case: triggers become very important if they reach the max priority (value 5000)
					// They get stored in a different list and all previous triggers are discarded.
					if (pTrigger->Weight_Current >= 5000 && !onlyCheckImportantTriggers)
					{
						// First time only
						if (validTriggerCandidates.Count > 0)
						{
							validTriggerCandidates.Clear();
							validTriggerCandidatesGroundOnly.Clear();
							validTriggerCandidatesNavalOnly.Clear();
							validTriggerCandidatesAirOnly.Clear();
							validTriggerCandidatesUnclassifiedOnly.Clear();

							validCategory = teamCategory::None;
						}

						// Reset and now only add important triggers to the list
						onlyCheckImportantTriggers = true;
						totalWeight = 0.0;
						splitTriggersByCategory = false; // VIP teams breaks the categories logic (on purpose)
					}

					// Passed all checks, save this trigger for later.
					// The idea behind this is to simulate an ordered list of weights and once we throw the dice we'll know the winner trigger: The more weight means more possibilities to be selected.
					totalWeight += pTrigger->Weight_Current < 1.0 ? 1.0 : pTrigger->Weight_Current;
					TriggerElementWeight item;
					item.Trigger = pTrigger;
					item.Weight = totalWeight;
					item.Category = teamIsCategory;

					validTriggerCandidates.AddItem(item);

					if (splitTriggersByCategory)
					{
						TriggerElementWeight itemGroundOnly;
						TriggerElementWeight itemAirOnly;
						TriggerElementWeight itemNavalOnly;
						TriggerElementWeight itemUnclassifiedOnly;

						switch (teamIsCategory)
						{
						case teamCategory::Ground:
							totalWeightGroundOnly += pTrigger->Weight_Current < 1.0 ? 1.0 : pTrigger->Weight_Current;

							itemGroundOnly.Trigger = pTrigger;
							itemGroundOnly.Weight = totalWeightGroundOnly;
							itemGroundOnly.Category = teamIsCategory;

							validTriggerCandidatesGroundOnly.AddItem(itemGroundOnly);
							totalGroundCategoryTriggers++;
							break;

						case teamCategory::Air:
							totalWeightAirOnly += pTrigger->Weight_Current < 1.0 ? 1.0 : pTrigger->Weight_Current;

							itemAirOnly.Trigger = pTrigger;
							itemAirOnly.Weight = totalWeightAirOnly;
							itemAirOnly.Category = teamIsCategory;

							validTriggerCandidatesAirOnly.AddItem(itemAirOnly);
							totalAirCategoryTriggers++;
							break;

						case teamCategory::Naval:
							totalWeightNavalOnly += pTrigger->Weight_Current < 1.0 ? 1.0 : pTrigger->Weight_Current;

							itemNavalOnly.Trigger = pTrigger;
							itemNavalOnly.Weight = totalWeightNavalOnly;
							itemNavalOnly.Category = teamIsCategory;

							validTriggerCandidatesNavalOnly.AddItem(itemNavalOnly);
							totalNavalCategoryTriggers++;
							break;

						case teamCategory::Unclassified:
							totalWeightUnclassifiedOnly += pTrigger->Weight_Current < 1.0 ? 1.0 : pTrigger->Weight_Current;

							itemUnclassifiedOnly.Trigger = pTrigger;
							itemUnclassifiedOnly.Weight = totalWeightUnclassifiedOnly;
							itemUnclassifiedOnly.Category = teamIsCategory;

							validTriggerCandidatesUnclassifiedOnly.AddItem(itemUnclassifiedOnly);
							totalUnclassifiedCategoryTriggers++;
							break;

						default:
							break;
						}
					}
				}
			}
		}

		if (splitTriggersByCategory)
		{
			Debug::Log("DEBUG: Category percentages:\nMixed teams: %f\nGround teams: %f\nNaval teams: %f\nAir teams: %f\n", percentageUnclassifiedTriggers, percentageGroundTriggers, percentageNavalTriggers, percentageAirTriggers);

			switch (validCategory)
			{
			case teamCategory::Ground:
				Debug::Log("DEBUG: This time only will be picked GROUND teams.\n");
				break;

			case teamCategory::Unclassified:
				Debug::Log("DEBUG: This time only will be picked MIXED teams.\n");
				break;

			case teamCategory::Naval:
				Debug::Log("DEBUG: This time only will be picked NAVAL teams.\n");
				break;

			case teamCategory::Air:
				Debug::Log("DEBUG: This time only will be picked AIR teams.\n");
				break;

			default:
				Debug::Log("DEBUG: This time teams categories are DISABLED.\n");
				break;
			}
		}

		if (validTriggerCandidates.Count == 0)
		{
			Debug::Log("DEBUG: [%s] (idx: %d) No valid triggers for now. A new attempt will be done later...\n", pHouse->Type->ID, pHouse->ArrayIndex);
			return SkipCode;
		}

		if ((validCategory == teamCategory::Ground && totalGroundCategoryTriggers == 0) ||
			(validCategory == teamCategory::Unclassified && totalUnclassifiedCategoryTriggers == 0) ||
			(validCategory == teamCategory::Air && totalAirCategoryTriggers == 0) ||
			(validCategory == teamCategory::Naval && totalNavalCategoryTriggers == 0))
		{
			Debug::Log("DEBUG: [%s] (Idx: %d) No valid triggers of this category. A new attempt should be done later...\n", pHouse->Type->ID, pHouse->ArrayIndex);

			if (!isFallbackEnabled)
				return SkipCode;

			Debug::Log("... BUT fallback mode is enabled so now it will check all triggers available.\n");
			validCategory = teamCategory::None;
		}

		AITriggerTypeClass* selectedTrigger = nullptr;
		double weightDice = 0.0;
		double lastWeight = 0.0;
		bool found = false;

		switch (validCategory)
		{
		case teamCategory::None:
			weightDice = ScenarioClass::Instance->Random.RandomRanged(0, (int)totalWeight) * 1.0;
			//				Debug::Log("Possible values of the dice [0 - %f]. Dice says: %f:\n", totalWeight, weightDice);
			//				Debug::Log("Picking the trigger with highest weight from the list (No category set):\n");

			for (auto element : validTriggerCandidates)
			{
				//					Debug::Log("[%s] Lottery range values: %f - %f", element.Trigger->ID, lastWeight, element.Weight);
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					//						Debug::Log(" ... CANDIDATE!\n");
					selectedTrigger = element.Trigger;
					found = true;
				}
				else
				{
					//						Debug::Log(" ... \n");
				}
			}
			break;

		case teamCategory::Ground:
			weightDice = ScenarioClass::Instance->Random.RandomRanged(0, (int)totalWeightGroundOnly) * 1.0;
			//				Debug::Log("Possible values of the dice [0 - %f]. Dice says: %f:\n", totalWeightGroundOnly, weightDice);
			//				Debug::Log("Picking the trigger with highest weight from the list (Ground category):\n");

			for (auto element : validTriggerCandidatesGroundOnly)
			{
				//					Debug::Log("[%s] Lottery range values: %f - %f", element.Trigger->ID, lastWeight, element.Weight);
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					//						Debug::Log(" ... CANDIDATE!\n");
					selectedTrigger = element.Trigger;
					found = true;
				}
				else
				{
					//						Debug::Log(" ... \n");
				}
			}
			break;

		case teamCategory::Unclassified:
			weightDice = ScenarioClass::Instance->Random.RandomRanged(0, (int)totalWeightUnclassifiedOnly) * 1.0;
			//				Debug::Log("Possible values of the dice [0 - %f]. Dice says: %f:\n", totalWeightUnclassifiedOnly, weightDice);
			//				Debug::Log("Picking the trigger with highest weight from the list (Mixed category):\n");

			for (auto element : validTriggerCandidatesUnclassifiedOnly)
			{
				//					Debug::Log("[%s] Lottery range values: %f - %f", element.Trigger->ID, lastWeight, element.Weight);
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					//						Debug::Log(" ... CANDIDATE!\n");
					selectedTrigger = element.Trigger;
					found = true;
				}
				else
				{
					//						Debug::Log(" ... \n");
				}
			}
			break;

		case teamCategory::Naval:
			weightDice = ScenarioClass::Instance->Random.RandomRanged(0, (int)totalWeightNavalOnly) * 1.0;
			//				Debug::Log("Possible values of the dice [0 - %f]. Dice says: %f:\n", totalWeightNavalOnly, weightDice);
			//				Debug::Log("Picking the trigger with highest weight from the list (Naval category):\n");

			for (auto element : validTriggerCandidatesNavalOnly)
			{
				//					Debug::Log("[%s] Lottery range values: %f - %f", element.Trigger->ID, lastWeight, element.Weight);
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					//						Debug::Log(" ... CANDIDATE!\n");
					selectedTrigger = element.Trigger;
					found = true;
				}
				else
				{
					//						Debug::Log(" ... \n");
				}
			}
			break;

		case teamCategory::Air:
			weightDice = ScenarioClass::Instance->Random.RandomRanged(0, (int)totalWeightAirOnly) * 1.0;
			//				Debug::Log("Possible values of the dice [0 - %f]. Dice says: %f:\n", totalWeightAirOnly, weightDice);
			//				Debug::Log("Picking the trigger with highest weight from the list (Air category):\n");

			for (auto element : validTriggerCandidatesAirOnly)
			{
				//					Debug::Log("[%s] Lottery range values: %f - %f", element.Trigger->ID, lastWeight, element.Weight);
				lastWeight = element.Weight;

				if (weightDice < element.Weight && !found)
				{
					//						Debug::Log(" ... CANDIDATE!\n");
					selectedTrigger = element.Trigger;
					found = true;
				}
				else
				{
					//						Debug::Log(" ... \n");
				}
			}
			break;

		default:
			break;
		}
		/*
		Debug::Log("[%s] Lottery range values: %f - %f", element.Trigger->ID, lastWeight, element.Weight);
		lastWeight = element.Weight;

		if (weightDice < element.WeightOnlyNaval && !found)
		{
			Debug::Log(" ... CANDIDATE!\n");
			selectedTrigger = element.Trigger;
			found = true;
		}
		else
		{
			Debug::Log(" ... \n");
		}
		*/

		if (!selectedTrigger)
		{
			Debug::Log("DEBUG: House [%s] (Idx: %d) failed to select Trigger. A new attempt Will be done later...\n", pHouse->Type->ID, pHouse->ArrayIndex);
			return SkipCode;
		}

		if (selectedTrigger->Weight_Current >= 5000.0
			&& selectedTrigger->Weight_Minimum <= 4999.0)
		{
			// Next time this trigger will be out of the important triggers list
			selectedTrigger->Weight_Current = 4999.0;
		}

		// We have a winner
		Debug::Log("DEBUG: House [%s] (Idx: %d) selected trigger [%s].\n", pHouse->Type->ID, pHouse->ArrayIndex, selectedTrigger->ID);

		//for (auto entry : selectedTrigger->Team1->TaskForce->Entries)
		//{
			//if (entry.Amount > 0)
				//Debug::Log("[%s] = %d\n", entry.Type->ID, entry.Amount);
			//else
				//break;
		//}
		//Debug::Log("\n");

		auto pTriggerTeam1Type = selectedTrigger->Team1;
		if (pTriggerTeam1Type)
		{
			int count = 0;

			for (auto team : activeTeamsList)
			{
				if (team->Type == pTriggerTeam1Type)
					count++;
			}

			if (count < pTriggerTeam1Type->Max)
			{
				if (auto newTeam = pTriggerTeam1Type->CreateTeam(pHouse))
					newTeam->NeedsToDisappear = false;
			}
		}

		auto pTriggerTeam2Type = selectedTrigger->Team2;
		if (pTriggerTeam2Type)
		{
			int count = 0;

			for (auto team : activeTeamsList)
			{
				if (team->Type == pTriggerTeam2Type)
					count++;
			}

			if (count < pTriggerTeam2Type->Max)
			{
				if (auto newTeam = pTriggerTeam2Type->CreateTeam(pHouse))
					newTeam->NeedsToDisappear = false;
			}
		}

		//Debug::Log("TeamClass::Array->Count: %d\n", TeamClass::Array->Count);
	}


	//selectedTeams46 = newTeamsList;
	return SkipCode;
}

bool TeamExt::HouseOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, bool allies, DynamicVectorClass<TechnoTypeClass*> list)
{
	bool result = false;
	int counter = 0;

	// Count all objects of the list, like an OR operator
	for (auto pItem : list)
	{
		for (auto pObject : *TechnoClass::Array)
		{
			if (pObject
				&& pObject->IsAlive
				&& pObject->Health > 0
				&& ((!allies && pObject->Owner == pHouse) || (allies && pHouse != pObject->Owner && pHouse->IsAlliedWith(pObject->Owner)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& pObject->GetTechnoType() == pItem)
			{
				counter++;
			}
		}
	}

	switch (pThis->Conditions->ComparatorOperand)
	{
	case 0:
		result = counter < pThis->Conditions->ComparatorType;
		break;
	case 1:
		result = counter <= pThis->Conditions->ComparatorType;
		break;
	case 2:
		result = counter == pThis->Conditions->ComparatorType;
		break;
	case 3:
		result = counter >= pThis->Conditions->ComparatorType;
		break;
	case 4:
		result = counter > pThis->Conditions->ComparatorType;
		break;
	case 5:
		result = counter != pThis->Conditions->ComparatorType;
		break;
	default:
		break;
	}

	return result;
}

bool TeamExt::EnemyOwns(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, bool onlySelectedEnemy, DynamicVectorClass<TechnoTypeClass*> list)
{
	bool result = false;
	int counter = 0;

	if (pEnemy && pHouse->IsAlliedWith(pEnemy) && !onlySelectedEnemy)
		pEnemy = nullptr;

	// Count all objects of the list, like an OR operator
	for (auto pItem : list)
	{
		for (auto pObject : *TechnoClass::Array)
		{
			if (pObject
				&& pObject->IsAlive
				&& pObject->Health > 0
				&& pObject->Owner != pHouse
				&& (!pEnemy || (pEnemy && !pHouse->IsAlliedWith(pEnemy)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& pObject->GetTechnoType() == pItem)
			{
				counter++;
			}
		}
	}

	switch (pThis->Conditions->ComparatorOperand)
	{
	case 0:
		result = counter < pThis->Conditions->ComparatorType;
		break;
	case 1:
		result = counter <= pThis->Conditions->ComparatorType;
		break;
	case 2:
		result = counter == pThis->Conditions->ComparatorType;
		break;
	case 3:
		result = counter >= pThis->Conditions->ComparatorType;
		break;
	case 4:
		result = counter > pThis->Conditions->ComparatorType;
		break;
	case 5:
		result = counter != pThis->Conditions->ComparatorType;
		break;
	default:
		break;
	}

	return result;
}

bool TeamExt::NeutralOwns(AITriggerTypeClass* pThis, DynamicVectorClass<TechnoTypeClass*> list)
{
	bool result = false;
	int counter = 0;

	for (auto pHouse : *HouseClass::Array)
	{
		if (_stricmp(SideClass::Array->GetItem(pHouse->Type->SideIndex)->Name, "Civilian") != 0)
			continue;

		// Count all objects of the list, like an OR operator
		for (auto pItem : list)
		{
			for (auto pObject : *TechnoClass::Array)
			{
				if (pObject
					&& pObject->IsAlive
					&& pObject->Health > 0
					&& pObject->Owner == pHouse
					&& pObject->GetTechnoType() == pItem)
				{
					counter++;
				}
			}
		}
	}

	switch (pThis->Conditions->ComparatorOperand)
	{
	case 0:
		result = counter < pThis->Conditions->ComparatorType;
		break;
	case 1:
		result = counter <= pThis->Conditions->ComparatorType;
		break;
	case 2:
		result = counter == pThis->Conditions->ComparatorType;
		break;
	case 3:
		result = counter >= pThis->Conditions->ComparatorType;
		break;
	case 4:
		result = counter > pThis->Conditions->ComparatorType;
		break;
	case 5:
		result = counter != pThis->Conditions->ComparatorType;
		break;
	default:
		break;
	}

	return result;
}

bool TeamExt::HouseOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, DynamicVectorClass<TechnoTypeClass*> list)
{
	bool result = true;

	if (list.Count == 0)
		return false;

	// Count all objects of the list, like an AND operator
	for (auto pItem : list)
	{
		if (!result)
			break;

		int counter = 0;
		result = true;

		for (auto pObject : *TechnoClass::Array)
		{
			if (pObject &&
				pObject->IsAlive &&
				pObject->Health > 0 &&
				pObject->Owner == pHouse &&
				pObject->GetTechnoType() == pItem)
			{
				counter++;
			}
		}

		switch (pThis->Conditions->ComparatorOperand)
		{
		case 0:
			result = counter < pThis->Conditions->ComparatorType;
			break;
		case 1:
			result = counter <= pThis->Conditions->ComparatorType;
			break;
		case 2:
			result = counter == pThis->Conditions->ComparatorType;
			break;
		case 3:
			result = counter >= pThis->Conditions->ComparatorType;
			break;
		case 4:
			result = counter > pThis->Conditions->ComparatorType;
			break;
		case 5:
			result = counter != pThis->Conditions->ComparatorType;
			break;
		default:
			break;
		}
	}

	return result;
}

bool TeamExt::EnemyOwnsAll(AITriggerTypeClass* pThis, HouseClass* pHouse, HouseClass* pEnemy, DynamicVectorClass<TechnoTypeClass*> list)
{
	bool result = true;

	if (pEnemy && pHouse->IsAlliedWith(pEnemy))
		pEnemy = nullptr;

	if (list.Count == 0)
		return false;

	// Count all objects of the list, like an AND operator
	for (auto pItem : list)
	{
		if (!result)
			break;

		int counter = 0;
		result = true;

		for (auto pObject : *TechnoClass::Array)
		{
			if (pObject
				&& pObject->IsAlive
				&& pObject->Health > 0
				&& pObject->Owner != pHouse
				&& (!pEnemy || (pEnemy && !pHouse->IsAlliedWith(pEnemy)))
				&& !pObject->Owner->Type->MultiplayPassive
				&& pObject->GetTechnoType() == pItem)
			{
				counter++;
			}
		}

		switch (pThis->Conditions->ComparatorOperand)
		{
		case 0:
			result = counter < pThis->Conditions->ComparatorType;
			break;
		case 1:
			result = counter <= pThis->Conditions->ComparatorType;
			break;
		case 2:
			result = counter == pThis->Conditions->ComparatorType;
			break;
		case 3:
			result = counter >= pThis->Conditions->ComparatorType;
			break;
		case 4:
			result = counter > pThis->Conditions->ComparatorType;
			break;
		case 5:
			result = counter != pThis->Conditions->ComparatorType;
			break;
		default:
			break;
		}
	}

	return result;
}

bool TeamExt::NeutralOwnsAll(AITriggerTypeClass* pThis, DynamicVectorClass<TechnoTypeClass*> list)
{
	bool result = true;

	if (list.Count == 0)
		return false;

	// Any neutral house should be capable to meet the prerequisites
	for (auto pHouse : *HouseClass::Array)
	{
		if (!result)
			break;

		bool foundAll = true;

		if (_stricmp(SideClass::Array->GetItem(pHouse->Type->SideIndex)->Name, "Civilian") != 0)
			continue;

		// Count all objects of the list, like an AND operator
		for (auto pItem : list)
		{
			if (!foundAll)
				break;

			int counter = 0;

			for (auto pObject : *TechnoClass::Array)
			{
				if (pObject &&
					pObject->IsAlive &&
					pObject->Health > 0 &&
					pObject->Owner == pHouse &&
					pObject->GetTechnoType() == pItem)
				{
					counter++;
				}
			}

			switch (pThis->Conditions->ComparatorOperand)
			{
			case 0:
				foundAll = counter < pThis->Conditions->ComparatorType;
				break;
			case 1:
				foundAll = counter <= pThis->Conditions->ComparatorType;
				break;
			case 2:
				foundAll = counter == pThis->Conditions->ComparatorType;
				break;
			case 3:
				foundAll = counter >= pThis->Conditions->ComparatorType;
				break;
			case 4:
				foundAll = counter > pThis->Conditions->ComparatorType;
				break;
			case 5:
				foundAll = counter != pThis->Conditions->ComparatorType;
				break;
			default:
				break;
			}
		}

		if (!foundAll)
			result = false;
	}

	return result;
}
