#include "Body.h"

#include <SuperClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <MessageListClass.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>

#include "Ext/House/Body.h"
#include "Ext/WarheadType/Body.h"
#include "Ext/WeaponType/Body.h"

// ============= New SuperWeapon Effects================

void SWTypeExt::FireSuperWeaponExt(SuperClass* pSW, const CellStruct& cell)
{
	const auto pHouse = pSW->Owner;
	const auto pType = pSW->Type;
	auto const pTypeExt = SWTypeExt::ExtMap.Find(pType);

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

	auto& sw_ext = HouseExt::ExtMap.Find(pHouse)->SuperExts[pType->ArrayIndex];
	sw_ext.ShotCount++;

	const auto pTags = &pHouse->RelatedTags;
	if (pTags->Count > 0)
	{
		int index = 0;
		int TagCount = pTags->Count;

		while (TagCount > 0 && index < TagCount)
		{
			const auto pTag = pTags->GetItem(index);

			// don't be confused as to why (TechnoClass*)(pSW) is there, it's something very much needed..
			if (pTag->RaiseEvent(static_cast<TriggerEvent>(77), nullptr, CellStruct::Empty, false, (TechnoClass*)(pSW))
				|| pTag->RaiseEvent(static_cast<TriggerEvent>(75), nullptr, CellStruct::Empty, false, (TechnoClass*)(pSW)))
			{
				if (TagCount != pTags->Count)
				{
					TagCount = pTags->Count;
					continue;
				}
			}

			++index;
		}
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

	BuildingTypeExt::CreateLimboBuilding(nullptr, pType, pOwner, ID);
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
		for (HouseClass* pTargetHouse : HouseClass::Array)
		{
			if (EnumFunctions::CanTargetHouse(this->LimboKill_Affected, pHouse, pTargetHouse))
			{
				auto const pHouseExt = HouseExt::ExtMap.Find(pTargetHouse);
				auto& vec = pHouseExt->OwnedLimboDeliveredBuildings;

				for (auto it = vec.begin(); it != vec.end(); )
				{
					BuildingClass* const pBuilding = *it;

					if (BuildingTypeExt::DeleteLimboBuilding(pBuilding, limboKillID))
					{
						it = vec.erase(it);

						// Remove limbo buildings' tracking here because their are not truely InLimbo
						if (!pBuilding->Type->Insignificant && !pBuilding->Type->DontScore)
							HouseExt::ExtMap.Find(pBuilding->Owner)->RemoveFromLimboTracking(pBuilding->Type);

						pBuilding->Stun();
						pBuilding->Limbo();
						pBuilding->RegisterDestruction(nullptr);
						pBuilding->UnInit();
					}
					else
					{
						++it;
					}
				}
			}
		}
	}
}

#pragma endregion

void SWTypeExt::ExtData::ApplyDetonation(HouseClass* pHouse, const CellStruct& cell)
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
					if ((this->SW_Next_IgnoreInhibitors || !pNextTypeExt->HasInhibitor(pHouse, cell))
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
	for (const auto pTargetFoot : FootClass::Array)
		TypeConvertGroup::Convert(pTargetFoot, this->Convert_Pairs, pSW->Owner);
}

void SWTypeExt::ExtData::HandleEMPulseLaunch(SuperClass* pSW, const CellStruct& cell) const
{
	auto const& pBuildings = this->GetEMPulseCannons(pSW->Owner, cell);
	auto const count = this->SW_MaxCount >= 0 ? static_cast<size_t>(this->SW_MaxCount) : std::numeric_limits<size_t>::max();

	for (size_t i = 0; i < pBuildings.size(); i++)
	{
		auto const pBuilding = pBuildings[i];
		auto const pExt = BuildingExt::ExtMap.Find(pBuilding);
		pExt->EMPulseSW = pSW;

		if (i + 1 == count)
			break;
	}

	if (this->EMPulse_SuspendOthers)
	{
		auto const pHouseExt = HouseExt::ExtMap.Find(pSW->Owner);

		for (auto const& pSuper : pSW->Owner->Supers)
		{
			if (static_cast<int>(pSuper->Type->Type) != 28 || pSuper == pSW)
				continue;

			auto const pTypeExt = SWTypeExt::ExtMap.Find(pSW->Type);
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

				if (pHouseExt->SuspendedEMPulseSWs.count(pSW->Type->ArrayIndex))
					pHouseExt->SuspendedEMPulseSWs[pSW->Type->ArrayIndex].push_back(pSuper->Type->ArrayIndex);
				else
					pHouseExt->SuspendedEMPulseSWs.insert({ pSW->Type->ArrayIndex, std::vector<int>{pSuper->Type->ArrayIndex} });
			}
		}
	}
}

void SWTypeExt::ExtData::ApplyLinkedSW(SuperClass* pSW)
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
				else if (this->SW_Link_Ready || (granted && SWTypeExt::ExtMap.Find(pSuper->Type)->SW_InitialReady))
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
