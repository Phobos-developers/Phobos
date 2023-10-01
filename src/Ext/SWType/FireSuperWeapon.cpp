#include "Body.h"

#include <SuperClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include "Ext/Building/Body.h"
#include "Ext/House/Body.h"

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
		if (SessionClass::Instance->GameMode == GameMode::Campaign)
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

		if (pType->OrePurifier)
			pOwner->NumOrePurifiers++;

		// BuildingClass::Place is where Ares hooks secret lab expansion
		// pTechnoBuilding->Place(false);
		// even with it no bueno yet, plus new issues
		// probably should just port it from Ares 0.A and be done

		auto const pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);

		// LimboKill ID
		pBuildingExt->LimboID = ID;

		auto const pOwnerExt = HouseExt::ExtMap.Find(pOwner);

		// Add building to list of owned limbo buildings
		pOwnerExt->OwnedLimboDeliveredBuildings.push_back(pBuilding);

		auto pTechExt = TechnoExt::ExtMap.Find(pBuilding);

		if (pTechExt->TypeExtData->AutoDeath_Behavior.isset() && pTechExt->TypeExtData->AutoDeath_AfterDelay > 0)
		{
			pTechExt->AutoDeathTimer.Start(pTechExt->TypeExtData->AutoDeath_AfterDelay);
			pOwnerExt->OwnedTimedAutoDeathObjects.push_back(pTechExt);
		}

	}
}

inline void LimboDelete(BuildingClass* pBuilding, HouseClass* pTargetHouse)
{
	BuildingTypeClass* pType = pBuilding->Type;

	auto pOwnerExt = HouseExt::ExtMap.Find(pTargetHouse);

	// Remove building from list of owned limbo buildings
	auto& vec = pOwnerExt->OwnedLimboDeliveredBuildings;
	vec.erase(std::remove(vec.begin(), vec.end(), pBuilding), vec.end());

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

void SWTypeExt::FireSuperWeaponExt(SuperClass* pSW, const CellStruct& cell)
{
	if (auto const pTypeExt = SWTypeExt::ExtMap.Find(pSW->Type))
	{
		if (pTypeExt->LimboDelivery_Types.size() > 0)
			pTypeExt->ApplyLimboDelivery(pSW->Owner);

		if (pTypeExt->LimboKill_IDs.size() > 0)
			pTypeExt->ApplyLimboKill(pSW->Owner);
	}
}

void SWTypeExt::ExtData::ApplyLimboDelivery(HouseClass* pHouse)
{
	// random mode
	if (this->LimboDelivery_RandomWeightsData.size())
	{
		bool rollOnce = false;
		int id = -1;
		size_t rolls = this->LimboDelivery_RollChances.size();
		size_t weights = this->LimboDelivery_RandomWeightsData.size();
		int ids = (int)this->LimboDelivery_IDs.size();

		// if no RollChances are supplied, do only one roll
		if (rolls == 0)
		{
			rolls = 1;
			rollOnce = true;
		}

		for (size_t i = 0; i < rolls; i++)
		{
			this->RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();
			if (!rollOnce && this->RandomBuffer > this->LimboDelivery_RollChances[i])
				continue;

			size_t j = rolls > weights ? weights : i;
			int index = GeneralUtils::ChooseOneWeighted(this->RandomBuffer, &this->LimboDelivery_RandomWeightsData[j]);

			// extra weights are bound to automatically fail
			if (index >= (int)this->LimboDelivery_Types.size())
				index = -1;

			if (index != -1)
			{
				if (index < ids)
					id = this->LimboDelivery_IDs[index];

				LimboCreate(this->LimboDelivery_Types[index], pHouse, id);
			}
		}
	}
	// no randomness mode
	else
	{
		int id = -1;
		size_t ids = this->LimboDelivery_IDs.size();

		for (size_t i = 0; i < this->LimboDelivery_Types.size(); i++)
		{
			if (i < ids)
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

// =============================
// Ares 0.A helpers
// Inhibitors check
bool SWTypeExt::ExtData::IsInhibitor(HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated)
	{
		if (!pOwner->IsAlliedWith(pTechno))
		{
			if (auto pBld = abstract_cast<BuildingClass*>(pTechno))
			{
				if (!pBld->IsPowerOnline())
					return false;
			}

			return this->SW_AnyInhibitor || this->SW_Inhibitors.Contains(pTechno->GetTechnoType());
		}
	}
	return false;
}

bool SWTypeExt::ExtData::IsInhibitorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const
{
	if (this->IsInhibitor(pOwner, pTechno))
	{
		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

		// get the inhibitor's center
		auto center = pTechno->GetCoords();
		if (auto pBuilding = abstract_cast<BuildingClass*>(pTechno))
		{
			//center = pBuilding->GetCoords();
			center.X += pBuilding->Type->GetFoundationWidth() / 2;
			center.Y += pBuilding->Type->GetFoundationHeight(false) / 2;
		}

		// has to be closer than the inhibitor range (which defaults to Sight)
		return coords.DistanceFrom(CellClass::Coord2Cell(center)) <= pExt->InhibitorRange.Get(pType->Sight);
	}

	return false;
}

bool SWTypeExt::ExtData::HasInhibitor(HouseClass* pOwner, const CellStruct& coords) const
{
	// does not allow inhibitors
	if (this->SW_Inhibitors.empty() && !this->SW_AnyInhibitor)
		return false;

	// a single inhibitor in range suffices
	return std::any_of(TechnoClass::Array->begin(), TechnoClass::Array->end(), [=, &coords](TechnoClass* pTechno)
		{ return this->IsInhibitorEligible(pOwner, coords, pTechno); }
	);
}

// Designators check
bool SWTypeExt::ExtData::IsDesignator(HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->Owner == pOwner && pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated)
		return this->SW_AnyDesignator || this->SW_Designators.Contains(pTechno->GetTechnoType());

	return false;
}

bool SWTypeExt::ExtData::IsDesignatorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const
{
	if (this->IsDesignator(pOwner, pTechno))
	{
		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

		// get the designator's center
		auto center = pTechno->GetCoords();
		if (auto pBuilding = abstract_cast<BuildingClass*>(pTechno))
		{
			//center = pBuilding->GetCoords();
			center.X += pBuilding->Type->GetFoundationWidth() / 2;
			center.Y += pBuilding->Type->GetFoundationHeight(false) / 2;
		}

		// has to be closer than the designator range (which defaults to Sight)
		return coords.DistanceFrom(CellClass::Coord2Cell(center)) <= pExt->DesignatorRange.Get(pType->Sight);
	}

	return false;
}

bool SWTypeExt::ExtData::HasDesignator(HouseClass* pOwner, const CellStruct& coords) const
{
	// does not require designators
	if (this->SW_Designators.empty() && !this->SW_AnyDesignator)
		return true;

	// a single designator in range suffices
	return std::any_of(TechnoClass::Array->begin(), TechnoClass::Array->end(), [=, &coords](TechnoClass* pTechno)
		{ return this->IsDesignatorEligible(pOwner, coords, pTechno); });
}

