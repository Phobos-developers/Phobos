#include "Body.h"

#include <SuperClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>

#include "Ext/House/Body.h"
#include <MessageListClass.h>
#include "Ext/WarheadType/Body.h"
#include "Ext/WeaponType/Body.h"

// ============= New SuperWeapon Effects================

void SWTypeExt::FireSuperWeaponExt(SuperClass* pSW, const CellStruct& cell)
{
	if (auto const pTypeExt = SWTypeExt::ExtMap.Find(pSW->Type))
	{
		if (pTypeExt->LimboDelivery_Types.size() > 0)
			pTypeExt->ApplyLimboDelivery(pSW->Owner);

		if (pTypeExt->LimboKill_IDs.size() > 0)
			pTypeExt->ApplyLimboKill(pSW->Owner);

		if (pTypeExt->Detonate_Warhead.isset() || pTypeExt->Detonate_Weapon.isset())
			pTypeExt->ApplyDetonation(pSW->Owner, cell);

		if (pTypeExt->SW_Next.size() > 0)
			pTypeExt->ApplySWNext(pSW, cell);

		if (pTypeExt->Convert_Pairs.size() > 0)
			pTypeExt->ApplyTypeConversion(pSW);

		if (pTypeExt->SW_GrantOneTime.size() > 0)
			pTypeExt->GrantOneTimeFromList(pSW);
	}
}

// ====================================================

#pragma region LimboDelivery
inline void LimboCreate(BuildingTypeClass* pType, HouseClass* pOwner, int ID)
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

		// For reasons beyond my comprehension, the discovery logic is checked for certain logics like power drain/output in campaign only.
		// Normally on unlimbo the buildings are revealed to current player if unshrouded or if game is a campaign and to non-player houses always.
		// Because of the unique nature of LimboDelivered buildings, this has been adjusted to always reveal to the current player in singleplayer
		// and to the owner of the building regardless, removing the shroud check from the equation since they don't physically exist - Starkku
		if (SessionClass::IsCampaign())
			pBuilding->DiscoveredBy(HouseClass::CurrentPlayer);

		pBuilding->DiscoveredBy(pOwner);

		pOwner->RegisterGain(pBuilding, false);
		pOwner->UpdatePower();
		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;
		pOwner->RecheckRadar = true;
		pOwner->Buildings.AddItem(pBuilding);

		// Different types of building logics
		if (pType->ConstructionYard)
			pOwner->ConYards.AddItem(pBuilding); // why would you do that????

		if (pType->SecretLab)
			pOwner->SecretLabs.AddItem(pBuilding);

		if (pType->FactoryPlant)
		{
			pOwner->FactoryPlants.AddItem(pBuilding);
			pOwner->CalculateCostMultipliers();
		}

		// BuildingClass::Place is already called in DiscoveredBy
		// it added OrePurifier and xxxGainSelfHeal to House counter already

		auto const pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);

		// LimboKill ID
		pBuildingExt->LimboID = ID;

		auto const pOwnerExt = HouseExt::ExtMap.Find(pOwner);

		// Add building to list of owned limbo buildings
		pOwnerExt->OwnedLimboDeliveredBuildings.push_back(pBuilding);

		if (!pBuilding->Type->Insignificant && !pBuilding->Type->DontScore)
			pOwnerExt->AddToLimboTracking(pBuilding->Type);

		auto const pTechnoExt = TechnoExt::ExtMap.Find(pBuilding);
		auto const pTechnoTypeExt = pTechnoExt->TypeExtData;

		if (pTechnoTypeExt->AutoDeath_Behavior.isset())
		{
			pOwnerExt->OwnedAutoDeathObjects.push_back(pTechnoExt);

			if (pTechnoTypeExt->AutoDeath_AfterDelay > 0)
				pTechnoExt->AutoDeathTimer.Start(pTechnoTypeExt->AutoDeath_AfterDelay);
		}

	}
}

inline void LimboDelete(BuildingClass* pBuilding, HouseClass* pTargetHouse)
{
	auto pOwnerExt = HouseExt::ExtMap.Find(pTargetHouse);

	// Remove building from list of owned limbo buildings
	auto& vec = pOwnerExt->OwnedLimboDeliveredBuildings;
	vec.erase(std::remove(vec.begin(), vec.end(), pBuilding), vec.end());

	pBuilding->Stun();
	pBuilding->Limbo();
	pBuilding->RegisterDestruction(nullptr);
	pBuilding->UnInit();
}

void SWTypeExt::ExtData::ApplyLimboDelivery(HouseClass* pHouse)
{
	// random mode
	if (this->LimboDelivery_RandomWeightsData.size())
	{
		int id = -1;
		size_t idsSize = this->LimboDelivery_IDs.size();
		auto results = this->WeightedRollsHandler(&this->LimboDelivery_RollChances, &this->LimboDelivery_RandomWeightsData, this->LimboDelivery_Types.size());
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
		size_t idsSize = this->LimboDelivery_IDs.size();
		for (size_t i = 0; i < this->LimboDelivery_Types.size(); i++)
		{
			if (i < idsSize)
				id = this->LimboDelivery_IDs[i];

			LimboCreate(this->LimboDelivery_Types[i], pHouse, id);
		}
	}
}

void SWTypeExt::ExtData::ApplyLimboKill(HouseClass* pHouse)
{
	for (int limboKillID : this->LimboKill_IDs)
	{
		for (HouseClass* pTargetHouse : *HouseClass::Array)
		{
			if (EnumFunctions::CanTargetHouse(this->LimboKill_Affected, pHouse, pTargetHouse))
			{
				auto const pHouseExt = HouseExt::ExtMap.Find(pTargetHouse);

				for (const auto& pBuilding : pHouseExt->OwnedLimboDeliveredBuildings)
				{
					auto const pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);

					if (pBuildingExt->LimboID == limboKillID)
						LimboDelete(pBuilding, pTargetHouse);
				}
			}
		}
	}
}

#pragma endregion

void SWTypeExt::ExtData::ApplyDetonation(HouseClass* pHouse, const CellStruct& cell)
{
	if (!this->Detonate_Weapon.isset() && !this->Detonate_Warhead.isset())
		return;

	auto coords = MapClass::Instance->GetCellAt(cell)->GetCoords();
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

	const auto pWeapon = this->Detonate_Weapon.isset() ? this->Detonate_Weapon.Get() : nullptr;
	auto const mapCoords = CellClass::Coord2Cell(coords);

	if (!MapClass::Instance->CoordinatesLegal(mapCoords))
	{
		auto const ID = pWeapon ? pWeapon->get_ID() : this->Detonate_Warhead.Get()->get_ID();
		Debug::Log("ApplyDetonation: Superweapon [%s] failed to detonate [%s] - cell at %d, %d is invalid.\n", this->OwnerObject()->get_ID(), ID, mapCoords.X, mapCoords.Y);
		return;
	}

	HouseClass* pFirerHouse = nullptr;

	if (!pFirer)
		pFirerHouse = pHouse;

	if (pWeapon)
		WeaponTypeExt::DetonateAt(pWeapon, coords, pFirer, this->Detonate_Damage.Get(pWeapon->Damage), pFirerHouse);
	else
		WarheadTypeExt::DetonateAt(this->Detonate_Warhead.Get(), coords, pFirer, this->Detonate_Damage.Get(0), pFirerHouse);
}

void SWTypeExt::ExtData::ApplySWNext(SuperClass* pSW, const CellStruct& cell)
{
	// SW.Next proper launching mechanic
	auto LaunchTheSW = [=](const int swIdxToLaunch)
	{
		HouseClass* pHouse = pSW->Owner;
		if (const auto pSuper = pHouse->Supers.GetItem(swIdxToLaunch))
		{
			const auto pNextTypeExt = SWTypeExt::ExtMap.Find(pSuper->Type);
			if (!this->SW_Next_RealLaunch ||
				(pSuper->IsPresent && pSuper->IsReady && !pSuper->IsSuspended && pHouse->CanTransactMoney(pNextTypeExt->Money_Amount)))
			{
				if (this->SW_Next_IgnoreInhibitors || !pNextTypeExt->HasInhibitor(pHouse, cell)
					&& (this->SW_Next_IgnoreDesignators || pNextTypeExt->HasDesignator(pHouse, cell)))
				{
					int oldstart = pSuper->RechargeTimer.StartTime;
					int oldleft = pSuper->RechargeTimer.TimeLeft;
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
		auto results = this->WeightedRollsHandler(&this->SW_Next_RollChances, &this->SW_Next_RandomWeightsData, this->SW_Next.size());
		for (int result : results)
			LaunchTheSW(this->SW_Next[result]);
	}
	// no randomness mode
	else
	{
		for (const auto swType : this->SW_Next)
			LaunchTheSW(swType);
	}
}

void SWTypeExt::ExtData::ApplyTypeConversion(SuperClass* pSW)
{
	if (this->Convert_Pairs.size() == 0)
		return;

	for (const auto pTargetFoot : *FootClass::Array)
		TypeConvertGroup::Convert(pTargetFoot, this->Convert_Pairs, pSW->Owner);
}

void SWTypeExt::ExtData::GrantOneTimeFromList(SuperClass* pSW)
{
	// SW.GrantOneTime proper SW granting mechanic
	HouseClass* pHouse = pSW->Owner;
	bool notObserver = !pHouse->IsObserver() || !pHouse->IsCurrentPlayerObserver();

	auto grantTheSW = [=](const int swIdxToAdd) -> bool
	{
		if (const auto pSuper = pHouse->Supers.GetItem(swIdxToAdd))
		{

			bool forceReadyness = pSuper->IsPresent && this->SW_GrantOneTime_ReadyIfExists.isset() && this->SW_GrantOneTime_ReadyIfExists.Get();
			bool granted = pSuper->Grant(true, false, false);

			if (granted || forceReadyness)
			{
				auto const pTypeExt = SWTypeExt::ExtMap.Find(pSuper->Type);
				bool isReady = this->SW_GrantOneTime_InitialReady.isset() && this->SW_GrantOneTime_InitialReady.Get() ? true : false;
				isReady = !this->SW_GrantOneTime_InitialReady.isset() && pTypeExt->SW_InitialReady ? true : isReady;
				isReady = forceReadyness ? true : isReady;

				if (isReady)
				{
					pSuper->RechargeTimer.TimeLeft = 0;
					pSuper->SetReadiness(true);
				}
				else
				{
					pSuper->Reset();
				}

				if (notObserver && pHouse->IsCurrentPlayer())
				{
					if (MouseClass::Instance->AddCameo(AbstractType::Special, swIdxToAdd))
						MouseClass::Instance->RepaintSidebar(1);
				}
			}

			return granted;
		}

		return false;
	};

	// random mode
	if (this->SW_GrantOneTime_RandomWeightsData.size())
	{
		auto results = this->WeightedRollsHandler(&this->SW_GrantOneTime_RollChances, &this->SW_GrantOneTime_RandomWeightsData, this->SW_GrantOneTime.size());
		for (int result : results)
			grantTheSW(this->SW_GrantOneTime[result]);
	}
	// no randomness mode
	else
	{
		for (const auto swType : this->SW_GrantOneTime)
			grantTheSW(swType);
	}

	if (notObserver && pHouse->IsCurrentPlayer())
	{
		if (this->EVA_GrantOneTimeLaunched.isset())
			VoxClass::PlayIndex(this->EVA_GrantOneTimeLaunched.Get(), -1, -1);

		MessageListClass::Instance->PrintMessage(this->Message_GrantOneTimeLaunched.Get(), RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
	}
}
