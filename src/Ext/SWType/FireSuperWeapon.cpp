#include "Body.h"

#include <SuperClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>

#include "Ext/House/Body.h"
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

		if (pType->OrePurifier)
			pOwner->NumOrePurifiers++;

		// BuildingClass::Place is where Ares hooks secret lab expansion
		// pTechnoBuilding->Place(false);
		// even with it no bueno yet, plus new issues
		// probably should just port it from Ares 0.A and be done

		auto const pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);

		if (pBuildingExt)
		{
			// LimboKill ID
			pBuildingExt->LimboID = ID;

			if (auto pOwnerExt = HouseExt::ExtMap.Find(pOwner))
			{	// Add building to list of owned limbo buildings
				pOwnerExt->OwnedLimboDeliveredBuildings.insert({ pBuilding, pBuildingExt });

				auto pTechExt = TechnoExt::ExtMap.Find(pBuilding);
				if (pTechExt->TypeExtData->AutoDeath_Behavior.isset() && pTechExt->TypeExtData->AutoDeath_AfterDelay > 0)
				{
					pTechExt->AutoDeathTimer.Start(pTechExt->TypeExtData->AutoDeath_AfterDelay);
					pOwnerExt->OwnedTimedAutoDeathObjects.push_back(pTechExt);
				}
			}
		}
	}
}

inline void LimboDelete(BuildingClass* pBuilding, HouseClass* pTargetHouse)
{
	BuildingTypeClass* pType = pBuilding->Type;

	auto pOwnerExt = HouseExt::ExtMap.Find(pTargetHouse);

	// Remove building from list of owned limbo buildings
	if (pOwnerExt)
		pOwnerExt->OwnedLimboDeliveredBuildings.erase(pBuilding);

	// Mandatory
	pBuilding->InLimbo = true;
	pBuilding->IsAlive = false;
	pBuilding->IsOnMap = false;
	pTargetHouse->RegisterLoss(pBuilding, false);
	pTargetHouse->UpdatePower();
	pTargetHouse->RecheckTechTree = true;
	pTargetHouse->RecheckPower = true;
	pTargetHouse->RecheckRadar = true;
	pTargetHouse->Buildings.Remove(pBuilding);

	// Building logics
	if (pType->ConstructionYard)
		pTargetHouse->ConYards.Remove(pBuilding);

	if (pType->SecretLab)
		pTargetHouse->SecretLabs.Remove(pBuilding);

	if (pType->FactoryPlant)
	{
		pTargetHouse->FactoryPlants.Remove(pBuilding);
		pTargetHouse->CalculateCostMultipliers();
	}

	if (pType->OrePurifier)
		pTargetHouse->NumOrePurifiers--;

	// Remove completely
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
				if (auto const pHouseExt = HouseExt::ExtMap.Find(pTargetHouse))
				{
					for (const auto& [pBuilding, pBuildingExt] : pHouseExt->OwnedLimboDeliveredBuildings)
					{
						if (pBuildingExt->LimboID == limboKillID)
							LimboDelete(pBuildingExt->OwnerObject(), pTargetHouse);
					}
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
				(pSuper->Granted && pSuper->IsCharged && !pSuper->IsOnHold && pHouse->CanTransactMoney(pNextTypeExt->Money_Amount)))
			{
				if (this->SW_Next_IgnoreInhibitors || !pNextTypeExt->HasInhibitor(pHouse, cell)
					&& (this->SW_Next_IgnoreDesignators || pNextTypeExt->HasDesignator(pHouse, cell)))
				{
					int oldstart = pSuper->RechargeTimer.StartTime;
					int oldleft = pSuper->RechargeTimer.TimeLeft;
					pSuper->SetReadiness(true);
					pSuper->Launch(cell, true);
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
