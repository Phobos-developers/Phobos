#include "Body.h"

#include <SuperClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include "Ext/Building/Body.h"
#include "Ext/House/Body.h"

// Too big to be kept in ApplyLimboDelivery
void LimboCreate(BuildingTypeClass* pType, HouseClass* pOwner, int ID)
{
	auto pOwnerExt = HouseExt::ExtMap.Find(pOwner);

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

	BuildingClass* pBuilding = abstract_cast<BuildingClass*>(pType->CreateObject(pOwner));

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

	// increment limbo build count
	if (pOwnerExt)
		pOwnerExt->OwnedLimboBuildingTypes.Increment(pType->ArrayIndex);

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

	// LimboKill init
	auto const pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);
	if (pBuildingExt && ID != -1)
		pBuildingExt->LimboID = ID;
}

void LimboDelete(BuildingClass* pBuilding, HouseClass* pTargetHouse)
{
	BuildingTypeClass* pType = static_cast<BuildingTypeClass*>(pBuilding->Type);

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

void SWTypeExt::ExtData::FireSuperWeapon(SuperClass* pSW, const CellStruct& cell)
{
	if (this->LimboDelivery_Types.size())
		ApplyLimboDelivery(pSW->Owner);

	if (this->LimboKill_IDs.size())
		ApplyLimboKill(pSW->Owner);
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
		size_t ids = this->LimboDelivery_IDs.size();

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
			size_t index = GeneralUtils::ChooseOneWeighted(this->RandomBuffer, &this->LimboDelivery_RandomWeightsData[j]);

			// extra weights are bound to automatically fail
			if (index >= this->LimboDelivery_Types.size())
				index = -1u;

			if (index != -1)
			{
				if (index < ids)
					id = this->LimboDelivery_IDs[index];

				LimboCreate(abstract_cast<BuildingTypeClass*>(this->LimboDelivery_Types[index]), pHouse, id);
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

			LimboCreate(abstract_cast<BuildingTypeClass*>(this->LimboDelivery_Types[i]), pHouse, id);
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
				for (const auto pBuilding : pTargetHouse->Buildings)
				{
					const auto pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);
					if (pBuildingExt->LimboID == limboKillID)
						LimboDelete(pBuilding, pTargetHouse);
				}
			}
		}
	}
}

//Ares 0.A helpers
bool SWTypeExt::IsInhibitor(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, TechnoClass* pTechno)
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

			return pSWType->SW_AnyInhibitor
				|| pSWType->SW_Inhibitors.Contains(pTechno->GetTechnoType());
		}
	}
	return false;
}

bool SWTypeExt::IsInhibitorEligible(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno)
{
	if (IsInhibitor(pSWType, pOwner, pTechno))
	{
		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

		// get the inhibitor's center
		auto center = pTechno->GetCoords();
		if (auto pBuilding = abstract_cast<BuildingClass*>(pTechno))
		{
			center = pBuilding->GetCoords();
			center.X += pBuilding->Type->GetFoundationWidth() / 2;
			center.Y += pBuilding->Type->GetFoundationHeight(false) / 2;
		}

		// has to be closer than the inhibitor range (which defaults to Sight)
		return Coords.DistanceFrom(CellClass::Coord2Cell(center)) <= pExt->InhibitorRange.Get(pType->Sight);
	}

	return false;
}

bool SWTypeExt::HasInhibitor(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct& Coords)
{
	// does not allow inhibitors
	if (pSWType->SW_Inhibitors.empty() && !pSWType->SW_AnyInhibitor)
		return false;

	// a single inhibitor in range suffices
	return std::any_of(TechnoClass::Array->begin(), TechnoClass::Array->end(),
		[=, &Coords](TechnoClass* pTechno)
		{ return IsInhibitorEligible(pSWType, pOwner, Coords, pTechno); }
	);
}
