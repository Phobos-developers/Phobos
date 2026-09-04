#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Utilities/Helpers.Alex.h>
#include <Misc/Hooks.DropshipLoadout.h>
#include <Ext/Techno/Body.h>
#include <Ext/UnitType/Body.h>

#include <TeamClass.h>
#include <TeamTypeClass.h>
#include <TaskForceClass.h>
#include <ScriptTypeClass.h>
#include <ScriptClass.h>
#include <MapClass.h>
#include <algorithm>

#include <unordered_set>

// ============= New SuperWeapon Effects================

void SWTypeExt::FireSuperWeaponExt(SuperClass* pSW, const CellStruct& cell)
{
	const auto pHouse = pSW->Owner;
	const auto pType = pSW->Type;
	auto const pTypeExt = SWTypeExt::Fetch(pType);

	if (pTypeExt->LimboDelivery_Types.size() > 0)
		pTypeExt->ApplyLimboDelivery(pHouse);

	if (pTypeExt->LimboKill_IDs.size() > 0)
		pTypeExt->ApplyLimboKill(pHouse);

	if (pTypeExt->Detonate_Warhead || pTypeExt->Detonate_Weapon)
		pTypeExt->ApplyDetonation(pHouse, cell);

	if (pTypeExt->SW_Next.size() > 0)
		pTypeExt->ApplySWNext(pSW, cell);

	if (pTypeExt->Convert_Pairs.size() > 0)
		pTypeExt->ApplyTypeConversion(pSW);

	if (pTypeExt->SW_Link.size() > 0)
		pTypeExt->ApplyLinkedSW(pSW);

	if (static_cast<int>(pType->Type) == 28 && !pTypeExt->EMPulse_TargetSelf) // Ares' Type=EMPulse SW
		pTypeExt->HandleEMPulseLaunch(pSW, cell);

	if (pTypeExt->DropshipLoadout_OpenWindow.Get(false) && pHouse->IsCurrentPlayer())
	{
		DropshipLoadoutClass::OpenInGameWindow(
			false, // bIgnoreFixedUnits
			pTypeExt->DropshipLoadout_PreloadCargo.Get(false), // bPreloadCargo
			0,     // allowableUnitsIndex
			pTypeExt->DropshipLoadout_Money.Get(-1), // startingMoney
			pTypeExt->DropshipLoadout_AddUnusedMoneyToPlayer,
			Nullable<bool>(pTypeExt->DropshipLoadout_RememberPurchasedCargo.Get()),
			pType
		);
	}

	if (pTypeExt->DropshipLoadout_Launch.Get())
		pTypeExt->ApplyDropshipLoadoutLaunch(pHouse, cell);

	pTypeExt->ApplyActivatedMessage(pSW);

	pTypeExt->ApplyActivatedEva(pSW);

	auto& sw_ext = HouseExt::Fetch(pHouse)->SuperExts[pType->ArrayIndex];
	sw_ext.ShotCount++;

	const auto pTags = &pHouse->RelatedTags;

	if (pTags->Count > 0)
	{
		auto RaiseEvent = [pTags](int nEvent, TechnoClass* pSource)
		{
			int index = 0;
			int tagCount = pTags->Count;

			while (tagCount > 0 && index < tagCount)
			{
				const auto pTag = pTags->Items[index];

				if (pTag->RaiseEvent(static_cast<TriggerEvent>(nEvent), nullptr, CellStruct::Empty, false, pSource))
				{
					if (tagCount != pTags->Count)
					{
						tagCount = pTags->Count;
						continue;
					}
				}

				++index;
			}
		};

		std::pair<SuperClass*, CellStruct> pass{ pSW, cell };

		RaiseEvent(77, reinterpret_cast<TechnoClass*>(&pass));
		RaiseEvent(75, reinterpret_cast<TechnoClass*>(pSW));
	}
}

// ====================================================

#pragma region LimboDelivery
static inline void LimboCreate(BuildingTypeClass* pType, HouseClass* pOwner, int ID)
{
	// BuildLimit check goes before creation
	if (pType->BuildLimit > 0)
	{
		int sum = pOwner->CountOwnedNow(pType);

		// copy Ares' deployable units x build limit fix
		if (auto const pUndeploy = pType->UndeploysInto)
			sum += pOwner->CountOwnedNow(pUndeploy);

		if (sum >= pType->BuildLimit)
			return;
	}

	if (auto const pBuilding = static_cast<BuildingClass*>(pType->CreateObject(pOwner)))
	{
		// All of these are mandatory
		pBuilding->InLimbo = false;
		pBuilding->IsAlive = true;
		pBuilding->IsOnMap = true;

		// Jun 3, 2023 - Starkku: For reasons beyond my comprehension, the discovery logic is checked for certain logics like power drain/output in campaign only.
		// Normally on unlimbo the buildings are revealed to current player if unshrouded or if game is a campaign and to non-player houses always.
		// Because of the unique nature of LimboDelivered buildings, this has been adjusted to always reveal to the current player in singleplayer
		// and to the owner of the building regardless, removing the shroud check from the equation since they don't physically exist
		if (SessionClass::IsCampaign())
			pBuilding->DiscoveredBy(HouseClass::CurrentPlayer);

		pBuilding->DiscoveredBy(pOwner);

		pOwner->RegisterGain(pBuilding, false);
		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;
		pOwner->Buildings.AddItem(pBuilding);

		// Different types of building logics
		if (pType->ConstructionYard)
			pOwner->ConYards.AddItem(pBuilding); // why would you do that????

		if (pType->SecretLab)
			pOwner->SecretLabs.AddItem(pBuilding);

		auto const pBuildingExt = BuildingExt::Fetch(pBuilding);
		auto const pOwnerExt = HouseExt::Fetch(pOwner);

		if (!pBuildingExt->GetTypeExtData()->PowerPlantEnhancer_Buildings.empty()
			&& (pBuildingExt->GetTypeExtData()->PowerPlantEnhancer_Amount != 0 || pBuildingExt->GetTypeExtData()->PowerPlantEnhancer_Factor != 1.0f))
			pOwnerExt->PowerPlantEnhancers.push_back(pBuilding);

		if (pType->FactoryPlant)
		{
			if (pBuildingExt->GetTypeExtData()->FactoryPlant_AllowTypes.size() > 0 || pBuildingExt->GetTypeExtData()->FactoryPlant_DisallowTypes.size() > 0)
			{
				pOwnerExt->RestrictedFactoryPlants.push_back(pBuilding);
			}
			else
			{
				pOwner->FactoryPlants.AddItem(pBuilding);
				pOwner->CalculateCostMultipliers();
			}
		}

		// BuildingClass::Place is already called in DiscoveredBy
		// it added OrePurifier and xxxGainSelfHeal to House counter already

		// LimboKill ID
		pBuildingExt->LimboID = ID;

		// Add building to list of owned limbo buildings
		pOwnerExt->OwnedLimboDeliveredBuildings.push_back(pBuilding);
		auto const pBldType = pBuilding->Type;

		if (!pBldType->Insignificant && !pBldType->DontScore)
			pOwnerExt->AddToLimboTracking(pBldType);

		auto const pTechnoExt = TechnoExt::Fetch(pBuilding);
		auto const pTechnoTypeExt = pTechnoExt->TypeExtData;

		if (pTechnoTypeExt->AutoDeath_Behavior.isset())
		{
			ScenarioExt::Global()->AutoDeathObjects.push_back(pTechnoExt);

			if (pTechnoTypeExt->AutoDeath_AfterDelay > 0)
				pTechnoExt->AutoDeathTimer.Start(pTechnoTypeExt->AutoDeath_AfterDelay);
		}

	}
}

void SWTypeExt::ApplyLimboDelivery(HouseClass* pHouse)
{
	// random mode
	if (this->LimboDelivery_RandomWeightsData.size())
	{
		int id = -1;
		const size_t idsSize = this->LimboDelivery_IDs.size();
		const auto results = this->WeightedRollsHandler(&this->LimboDelivery_RollChances, &this->LimboDelivery_RandomWeightsData, this->LimboDelivery_Types.size());
		for (size_t result : results)
		{
			if (result < idsSize)
				id = this->LimboDelivery_IDs[result];

			LimboCreate(this->LimboDelivery_Types[result], pHouse, id);
		}
	}
	// no randomness mode
	else
	{
		int id = -1;
		const size_t idsSize = this->LimboDelivery_IDs.size();

		for (size_t i = 0; i < this->LimboDelivery_Types.size(); i++)
		{
			if (i < idsSize)
				id = this->LimboDelivery_IDs[i];

			LimboCreate(this->LimboDelivery_Types[i], pHouse, id);
		}
	}
}

void SWTypeExt::ApplyLimboKill(HouseClass* pHouse)
{
	const int idAmount = static_cast<int>(this->LimboKill_IDs.size());

	if (!idAmount)
		return;

	std::vector<BuildingClass*> limboKills;

	for (const auto pTargetHouse : HouseClass::Array)
	{
		if (!EnumFunctions::CanTargetHouse(this->LimboKill_AffectsHouse, pHouse, pTargetHouse))
			continue;

		const auto pHouseExt = HouseExt::Fetch(pTargetHouse);
		auto& buildings = pHouseExt->OwnedLimboDeliveredBuildings;

		if (buildings.empty())
			continue;

		std::unordered_set<int> removedID;
		std::unordered_set<BuildingClass*> removes;

		for (int idx = 0; idx < idAmount; ++idx)
		{
			const int id = this->LimboKill_IDs[idx];

			if (removedID.contains(id))
				continue;

			const int maxCount = idx < static_cast<int>(this->LimboKill_Counts.size()) ? this->LimboKill_Counts[idx] : std::numeric_limits<int>::max();
			auto IsEligible = [id](BuildingClass* pBuilding) { return BuildingExt::Fetch(pBuilding)->LimboID == id; };

			Helpers::Alex::for_each_if_n(buildings.begin(), buildings.end(), maxCount, IsEligible, [&limboKills, &removes](BuildingClass* pBuilding) {
				limboKills.emplace_back(pBuilding);
				removes.emplace(pBuilding);
			});

			removedID.emplace(id);
		}

		if (!buildings.empty())
			std::erase_if(buildings, [&removes](BuildingClass* pBuilding) { return removes.contains(pBuilding); });
	}

	for (const auto pBuilding : limboKills)
	{
		const auto pBuildingType = pBuilding->Type;

		// Remove limbo buildings' tracking here because their are not truely InLimbo
		if (!pBuildingType->Insignificant && !pBuildingType->DontScore)
			HouseExt::Fetch(pBuilding->Owner)->RemoveFromLimboTracking(pBuildingType);

		pBuilding->Stun();
		pBuilding->Limbo();
		pBuilding->RegisterDestruction(nullptr);
		pBuilding->UnInit();
	}
}

#pragma endregion

void SWTypeExt::ApplyDetonation(HouseClass* pHouse, const CellStruct& cell)
{
	auto coords = MapClass::Instance.GetCellAt(cell)->GetCoords();
	BuildingClass* pFirer = nullptr;

	for (auto const& pBld : pHouse->Buildings)
	{
		if (this->IsLaunchSiteEligible(cell, pBld, false))
		{
			pFirer = pBld;
			break;
		}
	}

	if (this->Detonate_AtFirer)
		coords = pFirer ? pFirer->GetCenterCoords() : CoordStruct::Empty;

	const auto pWeapon = this->Detonate_Weapon;
	auto const mapCoords = CellClass::Coord2Cell(coords);

	if (!MapClass::Instance.CoordinatesLegal(mapCoords))
	{
		auto const ID = pWeapon ? pWeapon->get_ID() : this->Detonate_Warhead->get_ID();
		Debug::Log("ApplyDetonation: Superweapon [%s] failed to detonate [%s] - cell at %d, %d is invalid.\n", this->OwnerObject()->get_ID(), ID, mapCoords.X, mapCoords.Y);
		return;
	}

	if (pWeapon)
		WeaponTypeExt::DetonateAt(pWeapon, coords, pFirer, this->Detonate_Damage.Get(pWeapon->Damage), pHouse);
	else
	{
		if (this->Detonate_Warhead_Full)
			WarheadTypeExt::DetonateAt(this->Detonate_Warhead, coords, pFirer, this->Detonate_Damage.Get(0), pHouse);
		else
			MapClass::DamageArea(coords, this->Detonate_Damage.Get(0), pFirer, this->Detonate_Warhead, true, pHouse);
	}
}

void SWTypeExt::ApplySWNext(SuperClass* pSW, const CellStruct& cell)
{
	// SW.Next proper launching mechanic
	auto LaunchTheSW = [=](const int swIdxToLaunch)
		{
			const auto pHouse = pSW->Owner;
			if (const auto pSuper = pHouse->Supers.GetItem(swIdxToLaunch))
			{
				const auto pNextTypeExt = SWTypeExt::Fetch(pSuper->Type);
				if (!this->SW_Next_RealLaunch
					|| (pSuper->IsPresent && pSuper->IsReady && !pSuper->IsSuspended && pHouse->CanTransactMoney(pNextTypeExt->Money_Amount)))
				{
					if ((this->SW_Next_IgnoreInhibitors || !pNextTypeExt->HasInhibitor(pHouse, cell))
						&& (this->SW_Next_IgnoreDesignators || pNextTypeExt->HasDesignator(pHouse, cell)))
					{
						const int oldstart = pSuper->RechargeTimer.StartTime;
						const int oldleft = pSuper->RechargeTimer.TimeLeft;
						pSuper->SetReadiness(true);
						pSuper->Launch(cell, pHouse->IsCurrentPlayer());
						pSuper->Reset();
						if (!this->SW_Next_RealLaunch)
						{
							pSuper->RechargeTimer.StartTime = oldstart;
							pSuper->RechargeTimer.TimeLeft = oldleft;
						}
					}
				}
			}
		};

	// random mode
	if (this->SW_Next_RandomWeightsData.size())
	{
		const auto results = this->WeightedRollsHandler(&this->SW_Next_RollChances, &this->SW_Next_RandomWeightsData, this->SW_Next.size());
		for (const int result : results)
			LaunchTheSW(this->SW_Next[result]);
	}
	// no randomness mode
	else
	{
		for (const auto swType : this->SW_Next)
			LaunchTheSW(swType);
	}
}

void SWTypeExt::ApplyTypeConversion(SuperClass* pSW)
{
	for (const auto pTargetFoot : FootClass::Array)
		TypeConvertGroup::Convert(pTargetFoot, this->Convert_Pairs, pSW->Owner);
}

void SWTypeExt::HandleEMPulseLaunch(SuperClass* pSW, const CellStruct& cell) const
{
	auto const& pBuildings = this->GetEMPulseCannons(pSW->Owner, cell);
	auto const count = this->SW_MaxCount >= 0 ? static_cast<size_t>(this->SW_MaxCount) : std::numeric_limits<size_t>::max();

	for (size_t i = 0; i < pBuildings.size(); i++)
	{
		auto const pBuilding = pBuildings[i];
		auto const pExt = BuildingExt::Fetch(pBuilding);
		pExt->CurrentEMPulseSW = pSW;

		if (i + 1 == count)
			break;
	}

	if (this->EMPulse_SuspendOthers)
	{
		auto const pHouse = pSW->Owner;
		auto const pHouseExt = HouseExt::Fetch(pHouse);

		for (auto const& pSuper : pHouse->Supers)
		{
			if (static_cast<int>(pSuper->Type->Type) != 28 || pSuper == pSW)
				continue;

			auto const pTypeExt = SWTypeExt::Fetch(pSuper->Type);
			bool suspend = false;

			if (this->EMPulse_Cannons.empty() && pTypeExt->EMPulse_Cannons.empty())
			{
				suspend = true;
			}
			else
			{
				// Suspend if the two cannon lists share common items.
				suspend = std::find_first_of(this->EMPulse_Cannons.begin(), this->EMPulse_Cannons.end(),
					pTypeExt->EMPulse_Cannons.begin(), pTypeExt->EMPulse_Cannons.end()) != this->EMPulse_Cannons.end();
			}

			if (suspend)
			{
				pSuper->IsSuspended = true;
				const int arrayIndex = pSW->Type->ArrayIndex;

				if (pHouseExt->SuspendedEMPulseSWs.count(arrayIndex))
					pHouseExt->SuspendedEMPulseSWs[arrayIndex].push_back(arrayIndex);
				else
					pHouseExt->SuspendedEMPulseSWs.insert({ arrayIndex, std::vector<int>{pSuper->Type->ArrayIndex} });
			}
		}
	}
}

void SWTypeExt::ApplyLinkedSW(SuperClass* pSW)
{
	const auto pHouse = pSW->Owner;
	const bool notObserver = !pHouse->IsObserver() || !pHouse->IsCurrentPlayerObserver();

	if (pHouse->Defeated || !notObserver)
		return;

	auto linkedSW = [=](const int swIdxToAdd)
	{
		if (const auto pSuper = pHouse->Supers.GetItem(swIdxToAdd))
		{
			const bool granted = this->SW_Link_Grant && !pSuper->IsPresent && pSuper->Grant(true, false, false);
			bool isActive = granted;

			if (pSuper->IsPresent)
			{
				// check SW.Link.Reset first
				if (this->SW_Link_Reset)
				{
					pSuper->Reset();
					isActive = true;
				}
				// check SW.Link.Ready, which will default to SW.InitialReady for granted superweapon
				else if (this->SW_Link_Ready || (granted && SWTypeExt::Fetch(pSuper->Type)->SW_InitialReady))
				{
					pSuper->RechargeTimer.TimeLeft = 0;
					pSuper->SetReadiness(true);
					isActive = true;
				}
				// reset granted superweapon if it doesn't meet above conditions
				else if (granted)
				{
					pSuper->Reset();
				}
			}

			if (granted && notObserver && pHouse->IsCurrentPlayer())
			{
				if (MouseClass::Instance.AddCameo(AbstractType::Special, swIdxToAdd))
					MouseClass::Instance.RepaintSidebar(1);
			}

			return isActive;
		}

		return false;
	};

	bool isActive = false;

	// random mode
	if (this->SW_Link_RandomWeightsData.size())
	{
		const auto results = this->WeightedRollsHandler(&this->SW_Link_RollChances, &this->SW_Link_RandomWeightsData, this->SW_Link.size());

		for (const int result : results)
		{
			if (linkedSW(this->SW_Link[result]))
				isActive = true;
		}
	}

	// no randomness mode
	else
	{
		for (const auto swType : this->SW_Link)
		{
			if (linkedSW(swType))
				isActive = true;
		}
	}

	if (isActive && notObserver && pHouse->IsCurrentPlayer())
	{
		if (this->EVA_LinkedSWAcquired.isset())
			VoxClass::PlayIndex(this->EVA_LinkedSWAcquired.Get(), -1, -1);

		MessageListClass::Instance.PrintMessage(this->Message_LinkedSWAcquired.Get(), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
	}
}

void ConfigureTemporarySWClass(int index, TechnoTypeClass* pTransporterType, const CellStruct& cell, const CellStruct& spawnCell)
{
	char scriptName[64];
	char tfName[64];
	char ttName[64];
	sprintf_s(scriptName, "PH_SW_TempScript_%d", index);
	sprintf_s(tfName, "PH_SW_TempTaskForce_%d", index);
	sprintf_s(ttName, "PH_SW_TempTeamType_%d", index);

	auto pScript = ScriptTypeClass::Find(scriptName);

	if (!pScript)
		pScript = GameCreate<ScriptTypeClass>(scriptName);

	pScript->ActionsCount = 5;
	pScript->ScriptActions[0].Action = 4; // Move to cell
	const int nDivisor = ScenarioClass::NewINIFormat < 4 ? 128 : 1000;
	pScript->ScriptActions[0].Argument = cell.Y * nDivisor + cell.X;

	pScript->ScriptActions[1].Action = 5; // Deploy
	pScript->ScriptActions[1].Argument = 6;

	pScript->ScriptActions[2].Action = 8; // Deliver payload
	pScript->ScriptActions[2].Argument = 1;

	pScript->ScriptActions[3].Action = 4; // Move to cell
	pScript->ScriptActions[3].Argument = spawnCell.Y * nDivisor + spawnCell.X;

	pScript->ScriptActions[4].Action = 37; // Delete team
	pScript->ScriptActions[4].Argument = 0;

	auto pTaskForce = TaskForceClass::Find(tfName);

	if (!pTaskForce)
		pTaskForce = GameCreate<TaskForceClass>(tfName);

	pTaskForce->CountEntries = 1;
	pTaskForce->Entries[0].Amount = 1;

	if (pTransporterType)
		pTaskForce->Entries[0].Type = pTransporterType;

	auto pTeamType = TeamTypeClass::Find(ttName);

	if (!pTeamType)
		pTeamType = GameCreate<TeamTypeClass>(ttName);

	pTeamType->ScriptType = pScript;
	pTeamType->TaskForce = pTaskForce;

	pTeamType->Max = 1;
	pTeamType->Full = true;
	pTeamType->OnTransOnly = true;
	pTeamType->Annoyance = false;
	pTeamType->GuardSlower = false;
	pTeamType->Recruiter = false;
	pTeamType->Autocreate = false;
	pTeamType->Prebuild = false;
	pTeamType->Reinforce = false;
	pTeamType->Whiner = false;
	pTeamType->Aggressive = false;
	pTeamType->LooseRecruit = false;
	pTeamType->Suicide = false;
	pTeamType->Droppod = false;
	pTeamType->UseTransportOrigin = false;
	pTeamType->Priority = 0;
	pTeamType->Owner = nullptr;
	pTeamType->idxHouse = 0;
	pTeamType->TechLevel = 0;
	pTeamType->AvoidThreats = false;
	pTeamType->IonImmune = false;
	pTeamType->TransportsReturnOnUnload = false;
	pTeamType->AreTeamMembersRecruitable = false;
	pTeamType->IsBaseDefense = false;
	pTeamType->OnlyTargetHouseEnemy = false;
}

void SWTypeExt::ApplyDropshipLoadoutLaunch(HouseClass* pHouse, const CellStruct& cell)
{
	auto const pHouseExt = HouseExt::Fetch(pHouse);

	TechnoTypeClass* pTransporterType = nullptr;
	std::vector<TechnoTypeClass*> pCargo;

	if (pHouseExt->DropshipLoadout_SWCargo.empty())
		return;

	if (this->DropshipLoadout_Carrier.isset())
		pTransporterType = this->DropshipLoadout_Carrier;
	else if (pHouseExt->DropshipLoadout_SWCarrier)
		pTransporterType = pHouseExt->DropshipLoadout_SWCarrier;
	else if (!pHouseExt->DropshipLoadout_Carriers.empty())
		pTransporterType = pHouseExt->DropshipLoadout_Carriers[0];

	if (!pTransporterType)
		return;

	pCargo = pHouseExt->DropshipLoadout_SWCargo;

	Edge spawnEdge = pHouse->StartingEdge;

	if (spawnEdge == Edge::None || spawnEdge == Edge::Air)
		spawnEdge = Edge::North;

	CellStruct spawnCell = MapClass::Instance.PickCellOnEdge(
		spawnEdge,
		cell,
		cell,
		pTransporterType->SpeedType,
		false,
		pTransporterType->MovementZone
	);

	// Offset the spawn cell outwards into the black margins (outside playable area)
	if (spawnEdge == Edge::North)
	{
		// North edge of visible area is at MapRect.Y. Move Y outside visible area (towards 0).
		for (int y = spawnCell.Y - 1; y >= 0; --y)
		{
			CellStruct nextCell = { spawnCell.X, static_cast<short>(y) };

			if (MapClass::Instance.CoordinatesLegal(nextCell))
				spawnCell = nextCell;
			else
				break;
		}
	}
	else if (spawnEdge == Edge::South)
	{
		// South edge of visible area is at MapRect.Y + MapRect.Height - 1. Move Y outside (towards MaxHeight).
		int maxHeight = MapClass::Instance.MaxHeight;

		for (int y = spawnCell.Y + 1; y < maxHeight; ++y)
		{
			CellStruct nextCell = { spawnCell.X, static_cast<short>(y) };

			if (MapClass::Instance.CoordinatesLegal(nextCell))
				spawnCell = nextCell;
			else
				break;
		}
	}
	else if (spawnEdge == Edge::West)
	{
		// West edge of visible area is at MapRect.X. Move X outside visible area (towards 0).
		for (int x = spawnCell.X - 1; x >= 0; --x)
		{
			CellStruct nextCell = { static_cast<short>(x), spawnCell.Y };

			if (MapClass::Instance.CoordinatesLegal(nextCell))
				spawnCell = nextCell;
			else
				break;
		}
	}
	else if (spawnEdge == Edge::East)
	{
		// East edge of visible area is at MapRect.X + MapRect.Width - 1. Move X outside (towards MaxWidth).
		int maxWidth = MapClass::Instance.MaxWidth;

		for (int x = spawnCell.X + 1; x < maxWidth; ++x)
		{
			CellStruct nextCell = { static_cast<short>(x), spawnCell.Y };

			if (MapClass::Instance.CoordinatesLegal(nextCell))
				spawnCell = nextCell;
			else
				break;
		}
	}

	CoordStruct startLocation = CellClass::Cell2Coord(spawnCell);

	auto pGlobal = ScenarioExt::Global();
	auto& activeSuffixes = pGlobal->DropshipLoadout_ActiveTeamSuffixes;

	// Clean up inactive suffixes from the active list
	activeSuffixes.erase(
		std::remove_if(activeSuffixes.begin(), activeSuffixes.end(), [](int s)
			{
				char ttName[64];
				sprintf_s(ttName, "PH_SW_TempTeamType_%d", s);
				auto pTeamType = TeamTypeClass::Find(ttName);

				if (pTeamType)
					return pTeamType->cntInstances == 0 && pTeamType->FindFirstInstance() == nullptr;

				return true;
			}),
		activeSuffixes.end()
	);

	// Find the first unused suffix
	int suffix = 0;
	while (true)
	{
		bool isInUse = std::find(activeSuffixes.begin(), activeSuffixes.end(), suffix) != activeSuffixes.end();

		if (!isInUse)
			break;

		suffix++;
	}

	activeSuffixes.push_back(suffix);

	ConfigureTemporarySWClass(suffix, pTransporterType, cell, spawnCell);

	char ttName[64];
	sprintf_s(ttName, "PH_SW_TempTeamType_%d", suffix);
	auto pTeamType = TeamTypeClass::Find(ttName);

	if (!pTeamType)
		return;

	auto pTeam = GameCreate<TeamClass>(pTeamType, pHouse, 0);
	if (!pTeam)
		return;

	pTeam->NeedsToDisappear = false;
	pTeam->IsTransient = false;
	pTeam->IsForcedActive = true;

	auto const pTransporter = static_cast<FootClass*>(pTransporterType->CreateObject(pHouse));

	if (!pTransporter)
	{
		GameDelete(pTeam);
		return;
	}

	FootClass* pGunner = nullptr;

	for (auto pObjectType : pCargo)
	{
		auto const pObject = static_cast<FootClass*>(pObjectType->CreateObject(pHouse));
		if (!pObject)
			continue;

		auto const pPayload = static_cast<FootClass*>(pObject);
		pPayload->SetLocation(startLocation);
		pPayload->Limbo();

		if (pPayload->GetTechnoType()->Trainable && this->DropshipLoadout_VeteranLevel.isset())
		{
			int targetVetLevel = this->DropshipLoadout_VeteranLevel.Get();
			float targetVeterancy = 0.0f;

			if (targetVetLevel == 2)
				targetVeterancy = 1.0f;
			else if (targetVetLevel == 3)
				targetVeterancy = 2.0f;

			if (targetVeterancy > pPayload->Veterancy.Veterancy)
				pPayload->Veterancy.Add(targetVeterancy - pPayload->Veterancy.Veterancy);
		}

		if (pTransporterType->OpenTopped)
			pTransporter->EnteredOpenTopped(pPayload);

		pPayload->Transporter = pTransporter;
		pGunner = pPayload;
		pTransporter->AddPassenger(pPayload);
	}

	if (pTransporterType->Gunner && pGunner)
		pTransporter->ReceiveGunner(pGunner);

	if (!this->DropshipLoadout_PersistentCargo)
		pHouseExt->DropshipLoadout_SWCargo.clear();

	// Remove only the spawned units from SW InitialUnits pool in HouseExt
	for (auto pObjectType : pCargo)
	{
		if (pObjectType)
		{
			auto& swInitialUnits = pHouseExt->DropshipLoadout_SWInitialUnits;
			auto it = std::find(swInitialUnits.begin(), swInitialUnits.end(), pObjectType);

			if (it != swInitialUnits.end())
				swInitialUnits.erase(it);
		}
	}

	int zCoord = 0;

	if (pTransporterType->ConsideredAircraft)
	{
		zCoord = RulesClass::Instance->FlightLevel;
	}
	else if (pTransporterType->IsSubterranean)
	{
		auto const pTypeExt = UnitTypeExt::Fetch(static_cast<UnitTypeClass*>(pTransporterType));
		zCoord += pTypeExt->SubterraneanHeight.Get(RulesExt::Global()->SubterraneanHeight);
		zCoord -= pTransporter->Location.Z;
	}

	startLocation.Z = zCoord;
	pTransporter->SetLocation(startLocation);

	++Unsorted::ScenarioInit;
	bool success = pTransporter->Unlimbo(startLocation, DirType::North);
	--Unsorted::ScenarioInit;

	if (!success)
	{
		GameDelete(pTeam);
		GameDelete(pTransporter);
		return;
	}

	pTeam->AddMember(pTransporter, true);
	pTransporter->SetDestination(pTransporter, true);
}

void SWTypeExt::ApplyActivatedMessage(SuperClass* pSW) const
{
	const auto pHouse = pSW->Owner;

	const auto pMessage = pHouse->IsControlledByCurrentPlayer()
		? &this->Message_Activated_Owner
		: (pHouse->IsAlliedWith(HouseClass::CurrentPlayer)
			? &this->Message_Activated_Allies
			: &this->Message_Activated_Enemies);

	if (pMessage->Get().empty())
		return;
		
	MessageListClass::Instance.PrintMessage(
		pMessage->Get(),
		RulesClass::Instance->MessageDelay,
		pHouse->ColorSchemeIndex,
		true
	);
}

void SWTypeExt::ApplyActivatedEva(SuperClass* pSW) const
{
	const auto pHouse = pSW->Owner;

	const auto pEva = pHouse->IsControlledByCurrentPlayer()
		? &this->EVA_Activated_Owner
		: (pHouse->IsAlliedWith(HouseClass::CurrentPlayer)
			? &this->EVA_Activated_Allies
			: &this->EVA_Activated_Enemies);

	if (pEva->Get() == -1)
		return;

	VoxClass::PlayIndex(pEva->Get(), -1, -1);
}
