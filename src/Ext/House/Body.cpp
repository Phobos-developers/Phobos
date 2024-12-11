#include "Body.h"

#include <Ext/SWType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

#include <ScenarioClass.h>

//Static init

HouseExt::ExtContainer HouseExt::ExtMap;

std::vector<int> HouseExt::AIProduction_CreationFrames;
std::vector<int> HouseExt::AIProduction_Values;
std::vector<int> HouseExt::AIProduction_BestChoices;
std::vector<int> HouseExt::AIProduction_BestChoicesNaval;

// Based on Ares' rewrite of 0x4FEA60 for 100 unit bugfix.
void HouseExt::ExtData::UpdateVehicleProduction()
{
	auto pThis = this->OwnerObject();
	auto const AIDifficulty = static_cast<int>(pThis->GetAIDifficultyIndex());
	bool skipGround = pThis->ProducingUnitTypeIndex != -1;
	bool skipNaval = this->ProducingNavalUnitTypeIndex != -1;

	if (skipGround && skipNaval)
		return;

	if (!skipGround && this->UpdateHarvesterProduction())
		return;

	auto& creationFrames = HouseExt::AIProduction_CreationFrames;
	auto& values = HouseExt::AIProduction_Values;
	auto& bestChoices = HouseExt::AIProduction_BestChoices;
	auto& bestChoicesNaval = HouseExt::AIProduction_BestChoicesNaval;

	auto const count = static_cast<unsigned int>(UnitTypeClass::Array->Count);
	creationFrames.assign(count, 0x7FFFFFFF);
	values.assign(count, 0);

	for (auto currentTeam : *TeamClass::Array)
	{
		if (!currentTeam || currentTeam->Owner != pThis)
			continue;

		int teamCreationFrame = currentTeam->CreationFrame;

		if ((!currentTeam->Type->Reinforce || currentTeam->IsFullStrength)
			&& (currentTeam->IsForcedActive || currentTeam->IsHasBeen))
		{
			continue;
		}

		DynamicVectorClass<TechnoTypeClass*> taskForceMembers;
		currentTeam->GetTaskForceMissingMemberTypes(taskForceMembers);

		for (auto currentMember : taskForceMembers)
		{
			if (currentMember->WhatAmI() != UnitTypeClass::AbsID
				|| (skipGround && !currentMember->Naval)
				|| (skipNaval && currentMember->Naval))
			{
				continue;
			}

			auto const index = static_cast<unsigned int>(currentMember->GetArrayIndex());
			++values[index];

			if (teamCreationFrame < creationFrames[index])
				creationFrames[index] = teamCreationFrame;
		}
	}

	for (auto unit : *UnitClass::Array)
	{
		auto const index = static_cast<unsigned int>(unit->GetType()->GetArrayIndex());

		if (values[index] > 0 && unit->CanBeRecruited(pThis))
			--values[index];
	}

	bestChoices.clear();
	bestChoicesNaval.clear();

	int bestValue = -1;
	int bestValueNaval = -1;
	int earliestTypenameIndex = -1;
	int earliestTypenameIndexNaval = -1;
	int earliestFrame = 0x7FFFFFFF;
	int earliestFrameNaval = 0x7FFFFFFF;

	for (auto i = 0u; i < count; ++i)
	{
		auto const type = UnitTypeClass::Array->Items[static_cast<int>(i)];
		int currentValue = values[i];

		if (currentValue <= 0
			|| pThis->CanBuild(type, false, false) == CanBuildResult::Unbuildable
			|| type->GetActualCost(pThis) > pThis->Available_Money())
		{
			continue;
		}

		bool isNaval = type->Naval;
		int* cBestValue = !isNaval ? &bestValue : &bestValueNaval;
		std::vector<int>* cBestChoices = !isNaval ? &bestChoices : &bestChoicesNaval;

		if (*cBestValue < currentValue || *cBestValue == -1)
		{
			*cBestValue = currentValue;
			cBestChoices->clear();
		}

		cBestChoices->push_back(static_cast<int>(i));

		int* cEarliestTypeNameIndex = !isNaval ? &earliestTypenameIndex : &earliestTypenameIndexNaval;
		int* cEarliestFrame = !isNaval ? &earliestFrame : &earliestFrameNaval;

		if (*cEarliestFrame > creationFrames[i] || *cEarliestTypeNameIndex == -1)
		{
			*cEarliestTypeNameIndex = static_cast<int>(i);
			*cEarliestFrame = creationFrames[i];
		}
	}

	int earliestOdds = RulesClass::Instance->FillEarliestTeamProbability[AIDifficulty];

	if (!skipGround)
	{
		if (ScenarioClass::Instance->Random.RandomRanged(0, 99) < earliestOdds)
		{
			pThis->ProducingUnitTypeIndex = earliestTypenameIndex;
		}
		else if (auto const size = static_cast<int>(bestChoices.size()))
		{
			int randomChoice = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);
			pThis->ProducingUnitTypeIndex = bestChoices[static_cast<unsigned int>(randomChoice)];
		}
	}

	if (!skipNaval)
	{
		if (ScenarioClass::Instance->Random.RandomRanged(0, 99) < earliestOdds)
		{
			this->ProducingNavalUnitTypeIndex = earliestTypenameIndexNaval;
		}
		else if (auto const size = static_cast<int>(bestChoicesNaval.size()))
		{
			int randomChoice = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);
			this->ProducingNavalUnitTypeIndex = bestChoicesNaval[static_cast<unsigned int>(randomChoice)];
		}
	}
}

bool HouseExt::ExtData::UpdateHarvesterProduction()
{
	auto pThis = this->OwnerObject();
	auto const AIDifficulty = static_cast<int>(pThis->GetAIDifficultyIndex());
	auto const idxParentCountry = pThis->Type->FindParentCountryIndex();
	auto const pHarvesterUnit = HouseExt::FindOwned(pThis, idxParentCountry, make_iterator(RulesClass::Instance->HarvesterUnit));

	if (pHarvesterUnit)
	{
		auto const harvesters = pThis->CountResourceGatherers;
		auto maxHarvesters = HouseExt::FindBuildable(
			pThis, idxParentCountry, make_iterator(RulesClass::Instance->BuildRefinery))
			? RulesClass::Instance->HarvestersPerRefinery[AIDifficulty] * pThis->CountResourceDestinations
			: RulesClass::Instance->AISlaveMinerNumber[AIDifficulty];

		if (pThis->IQLevel2 >= RulesClass::Instance->Harvester && !pThis->IsTiberiumShort
			&& !pThis->IsControlledByHuman() && harvesters < maxHarvesters
			&& pThis->TechLevel >= pHarvesterUnit->TechLevel)
		{
			pThis->ProducingUnitTypeIndex = pHarvesterUnit->ArrayIndex;
			return true;
		}
	}
	else
	{
		auto const maxHarvesters = RulesClass::Instance->AISlaveMinerNumber[AIDifficulty];

		if (pThis->CountResourceGatherers < maxHarvesters)
		{
			auto const pRefinery = HouseExt::FindBuildable(
				pThis, idxParentCountry, make_iterator(RulesClass::Instance->BuildRefinery));

			if (pRefinery)
			{
				if (auto const pSlaveMiner = pRefinery->UndeploysInto)
				{
					if (pSlaveMiner->ResourceDestination)
					{
						pThis->ProducingUnitTypeIndex = pSlaveMiner->ArrayIndex;
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool HouseExt::ExtData::OwnsLimboDeliveredBuilding(BuildingClass* pBuilding)
{
	if (!pBuilding)
		return false;

	auto& vec = this->OwnedLimboDeliveredBuildings;
	return std::find(vec.begin(), vec.end(), pBuilding) != vec.end();
}

size_t HouseExt::FindOwnedIndex(
	HouseClass const* const, int const idxParentCountry,
	Iterator<TechnoTypeClass const*> const items, size_t const start)
{
	auto const bitOwner = 1u << idxParentCountry;

	for (auto i = start; i < items.size(); ++i)
	{
		auto const pItem = items[i];

		if (pItem->InOwners(bitOwner))
			return i;
	}

	return items.size();
}

bool HouseExt::IsDisabledFromShell(
	HouseClass const* const pHouse, BuildingTypeClass const* const pItem)
{
	// SWAllowed does not apply to campaigns any more
	if (SessionClass::IsCampaign()
		|| GameModeOptionsClass::Instance->SWAllowed)
	{
		return false;
	}

	if (pItem->SuperWeapon != -1)
	{
		// allow SWs only if not disableable from shell
		auto const pItem2 = const_cast<BuildingTypeClass*>(pItem);
		auto const& BuildTech = RulesClass::Instance->BuildTech;

		if (BuildTech.FindItemIndex(pItem2) == -1)
		{
			auto const pSuper = pHouse->Supers[pItem->SuperWeapon];
			if (pSuper->Type->DisableableFromShell)
				return true;
		}
	}

	return false;
}

size_t HouseExt::FindBuildableIndex(
	HouseClass const* const pHouse, int const idxParentCountry,
	Iterator<TechnoTypeClass const*> const items, size_t const start)
{
	for (auto i = start; i < items.size(); ++i)
	{
		auto const pItem = items[i];

		if (pHouse->CanExpectToBuild(pItem, idxParentCountry))
		{
			auto const pBld = abstract_cast<const BuildingTypeClass*>(pItem);
			if (pBld && HouseExt::IsDisabledFromShell(pHouse, pBld))
				continue;

			return i;
		}
	}

	return items.size();
}

int HouseExt::ActiveHarvesterCount(HouseClass* pThis)
{
	int result = 0;

	for (auto pTechno : *TechnoClass::Array)
	{
		if (pTechno->Owner == pThis)
		{
			auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());
			result += pTypeExt->Harvester_Counted && TechnoExt::IsHarvesting(pTechno);
		}
	}

	return result;
}

int HouseExt::TotalHarvesterCount(HouseClass* pThis)
{
	int result = 0;

	for (auto pType : RulesExt::Global()->HarvesterTypes)
		result += pThis->CountOwnedAndPresent(pType);

	return result;
}

// This basically gets same cell that AI script action 53 Gather at Enemy Base uses, and code for that (0x6EF700) was used as reference here.
CellClass* HouseExt::GetEnemyBaseGatherCell(HouseClass* pTargetHouse, HouseClass* pCurrentHouse, CoordStruct defaultCurrentCoords, SpeedType speedTypeZone, int extraDistance)
{
	if (!pTargetHouse || !pCurrentHouse)
		return nullptr;

	auto targetCoords = CellClass::Cell2Coord(pTargetHouse->GetBaseCenter());

	if (targetCoords == CoordStruct::Empty)
		return nullptr;

	auto currentCoords = CellClass::Cell2Coord(pCurrentHouse->GetBaseCenter());

	if (currentCoords == CoordStruct::Empty)
		currentCoords = defaultCurrentCoords;

	int distance = (RulesClass::Instance->AISafeDistance + extraDistance) * Unsorted::LeptonsPerCell;
	auto newCoords = GeneralUtils::CalculateCoordsFromDistance(currentCoords, targetCoords, distance);

	auto cellStruct = CellClass::Coord2Cell(newCoords);
	cellStruct = MapClass::Instance->NearByLocation(cellStruct, speedTypeZone, -1, MovementZone::Normal, false, 3, 3, false, false, false, true, cellStruct, false, false);

	return MapClass::Instance->TryGetCellAt(cellStruct);
}

// Gets the superweapons used by AI for Chronoshift script actions.
void HouseExt::GetAIChronoshiftSupers(HouseClass* pThis, SuperClass*& pSuperCSphere, SuperClass*& pSuperCWarp)
{
	int idxCS = RulesExt::Global()->AIChronoSphereSW;
	int idxCW = RulesExt::Global()->AIChronoWarpSW;

	if (idxCS >= 0)
	{
		pSuperCSphere = pThis->Supers[idxCS];

		if (idxCW < 0)
		{
			auto const pSWTypeExt = SWTypeExt::ExtMap.Find(pSuperCSphere->Type);

			if (pSWTypeExt->SW_PostDependent >= 0)
				pSuperCWarp = pThis->Supers[pSWTypeExt->SW_PostDependent];
		}
	}

	if (idxCW >= 0)
		pSuperCWarp = pThis->Supers[idxCW];

	if (pSuperCSphere && pSuperCWarp)
		return;

	for (auto const pSuper : pThis->Supers)
	{
		if (pSuper->Type->Type == SuperWeaponType::ChronoSphere)
			pSuperCSphere = pSuper;

		if (pSuper->Type->Type == SuperWeaponType::ChronoWarp)
			pSuperCWarp = pSuper;
	}
}

// Ares
HouseClass* HouseExt::GetHouseKind(OwnerHouseKind const kind, bool const allowRandom, HouseClass* const pDefault, HouseClass* const pInvoker, HouseClass* const pVictim)
{
	switch (kind)
	{
	case OwnerHouseKind::Invoker:
	case OwnerHouseKind::Killer:
		return pInvoker ? pInvoker : pDefault;
	case OwnerHouseKind::Victim:
		return pVictim ? pVictim : pDefault;
	case OwnerHouseKind::Civilian:
		return HouseClass::FindCivilianSide();
	case OwnerHouseKind::Special:
		return HouseClass::FindSpecial();
	case OwnerHouseKind::Neutral:
		return HouseClass::FindNeutral();
	case OwnerHouseKind::Random:
		if (allowRandom)
		{
			auto& Random = ScenarioClass::Instance->Random;
			return HouseClass::Array->GetItem(
				Random.RandomRanged(0, HouseClass::Array->Count - 1));
		}
		else
		{
			return pDefault;
		}
	case OwnerHouseKind::Default:
	default:
		return pDefault;
	}
}

void HouseExt::ExtData::AddToLimboTracking(TechnoTypeClass* pTechnoType)
{
	if (pTechnoType)
	{
		int arrayIndex = pTechnoType->GetArrayIndex();

		switch (pTechnoType->WhatAmI())
		{
		case AbstractType::AircraftType:
			this->LimboAircraft.Increment(arrayIndex);
			break;
		case AbstractType::BuildingType:
			this->LimboBuildings.Increment(arrayIndex);
			break;
		case AbstractType::InfantryType:
			this->LimboInfantry.Increment(arrayIndex);
			break;
		case AbstractType::UnitType:
			this->LimboVehicles.Increment(arrayIndex);
			break;
		default:
			break;
		}
	}
}

void HouseExt::ExtData::RemoveFromLimboTracking(TechnoTypeClass* pTechnoType)
{
	if (pTechnoType)
	{
		int arrayIndex = pTechnoType->GetArrayIndex();

		switch (pTechnoType->WhatAmI())
		{
		case AbstractType::AircraftType:
			this->LimboAircraft.Decrement(arrayIndex);
			break;
		case AbstractType::BuildingType:
			this->LimboBuildings.Decrement(arrayIndex);
			break;
		case AbstractType::InfantryType:
			this->LimboInfantry.Decrement(arrayIndex);
			break;
		case AbstractType::UnitType:
			this->LimboVehicles.Decrement(arrayIndex);
			break;
		default:
			break;
		}
	}
}

int HouseExt::ExtData::CountOwnedPresentAndLimboed(TechnoTypeClass* pTechnoType)
{
	int count = this->OwnerObject()->CountOwnedAndPresent(pTechnoType);
	int arrayIndex = pTechnoType->GetArrayIndex();

	switch (pTechnoType->WhatAmI())
	{
	case AbstractType::AircraftType:
		count += this->LimboAircraft.GetItemCount(arrayIndex);
		break;
	case AbstractType::BuildingType:
		count += this->LimboBuildings.GetItemCount(arrayIndex);
		break;
	case AbstractType::InfantryType:
		count += this->LimboInfantry.GetItemCount(arrayIndex);
		break;
	case AbstractType::UnitType:
		count += this->LimboVehicles.GetItemCount(arrayIndex);
		break;
	default:
		break;
	}

	return count;
}

void HouseExt::ExtData::UpdateNonMFBFactoryCounts(AbstractType rtti, bool remove, bool isNaval)
{
	int* count = nullptr;

	switch (rtti)
	{
	case AbstractType::Aircraft:
	case AbstractType::AircraftType:
		count = &this->NumAirpads_NonMFB;
		break;
	case AbstractType::Building:
	case AbstractType::BuildingType:
		count = &this->NumConYards_NonMFB;
		break;
	case AbstractType::Infantry:
	case AbstractType::InfantryType:
		count = &this->NumBarracks_NonMFB;
		break;
	case AbstractType::Unit:
	case AbstractType::UnitType:
		count = isNaval ? &this->NumShipyards_NonMFB : &this->NumWarFactories_NonMFB;
		break;
	default:
		break;
	}

	if (count)
		*count += remove ? -1 : 1;
}

int HouseExt::ExtData::GetFactoryCountWithoutNonMFB(AbstractType rtti, bool isNaval)
{
	auto const pThis = this->OwnerObject();
	int count = 0;

	switch (rtti)
	{
	case AbstractType::Aircraft:
	case AbstractType::AircraftType:
		count = pThis->NumAirpads - this->NumAirpads_NonMFB;
		break;
	case AbstractType::Building:
	case AbstractType::BuildingType:
		count = pThis->NumConYards - this->NumConYards_NonMFB;
		break;
	case AbstractType::Infantry:
	case AbstractType::InfantryType:
		count = pThis->NumBarracks - this->NumBarracks_NonMFB;
		break;
	case AbstractType::Unit:
	case AbstractType::UnitType:
		if (isNaval)
			count = pThis->NumShipyards - this->NumShipyards_NonMFB;
		else
			count = pThis->NumWarFactories - this->NumWarFactories_NonMFB;
		break;
	default:
		break;
	}

	return Math::max(count, 0);
}

float HouseExt::ExtData::GetRestrictedFactoryPlantMult(TechnoTypeClass* pTechnoType) const
{
	float mult = 1.0;
	auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

	for (auto const pBuilding : this->RestrictedFactoryPlants)
	{
		auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

		if (pTypeExt->FactoryPlant_AllowTypes.size() > 0 && !pTypeExt->FactoryPlant_AllowTypes.Contains(pTechnoType))
			continue;

		if (pTypeExt->FactoryPlant_DisallowTypes.size() > 0 && pTypeExt->FactoryPlant_DisallowTypes.Contains(pTechnoType))
			continue;

		float currentMult = 1.0f;

		switch (pTechnoType->WhatAmI())
		{
		case AbstractType::BuildingType:
			if (((BuildingTypeClass*)pTechnoType)->BuildCat == BuildCat::Combat)
				currentMult = pBuilding->Type->DefensesCostBonus;
			else
				currentMult = pBuilding->Type->BuildingsCostBonus;
			break;
		case AbstractType::AircraftType:
			currentMult = pBuilding->Type->AircraftCostBonus;
			break;
		case AbstractType::InfantryType:
			currentMult = pBuilding->Type->InfantryCostBonus;
			break;
		case AbstractType::UnitType:
			currentMult = pBuilding->Type->UnitsCostBonus;
			break;
		default:
			break;
		}

		mult *= currentMult;
	}

	return 1.0f - ((1.0f - mult) * pTechnoTypeExt->FactoryPlant_Multiplier);
}

void HouseExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	const char* pSection = this->OwnerObject()->PlainName;

	INI_EX exINI(pINI);

	ValueableVector<bool> readBaseNodeRepairInfo;
	readBaseNodeRepairInfo.Read(exINI, pSection, "RepairBaseNodes");
	size_t nWritten = readBaseNodeRepairInfo.size();

	if (nWritten <= 3)
	{
		for (size_t i = 0; i < nWritten; i++)
		{
			this->RepairBaseNodes[i] = readBaseNodeRepairInfo[i];
		}
	}
}

// =============================
// load / save

template <typename T>
void HouseExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->PowerPlantEnhancers)
		.Process(this->OwnedLimboDeliveredBuildings)
		.Process(this->LimboAircraft)
		.Process(this->LimboBuildings)
		.Process(this->LimboInfantry)
		.Process(this->LimboVehicles)
		.Process(this->Factory_BuildingType)
		.Process(this->Factory_InfantryType)
		.Process(this->Factory_VehicleType)
		.Process(this->Factory_NavyType)
		.Process(this->Factory_AircraftType)
		.Process(this->AISuperWeaponDelayTimer)
		.Process(this->RepairBaseNodes)
		.Process(this->RestrictedFactoryPlants)
		.Process(this->LastBuiltNavalVehicleType)
		.Process(this->ProducingNavalUnitTypeIndex)
		.Process(this->NumAirpads_NonMFB)
		.Process(this->NumBarracks_NonMFB)
		.Process(this->NumWarFactories_NonMFB)
		.Process(this->NumConYards_NonMFB)
		.Process(this->NumShipyards_NonMFB)
		.Process(this->AIFireSaleDelayTimer)
		.Process(this->SuspendedEMPulseSWs)
		.Process(this->SuperExts)
		;
}

void HouseExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<HouseClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void HouseExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<HouseClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool HouseExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool HouseExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

void HouseExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(Factory_BuildingType, ptr);
	AnnounceInvalidPointer(Factory_InfantryType, ptr);
	AnnounceInvalidPointer(Factory_VehicleType, ptr);
	AnnounceInvalidPointer(Factory_NavyType, ptr);
	AnnounceInvalidPointer(Factory_AircraftType, ptr);

	if (ptr != nullptr)
	{
		if (!OwnedLimboDeliveredBuildings.empty())
		{
			auto& vec = this->OwnedLimboDeliveredBuildings;
			vec.erase(std::remove(vec.begin(), vec.end(), reinterpret_cast<BuildingClass*>(ptr)), vec.end());
		}

		if (!RestrictedFactoryPlants.empty())
		{
			auto& vec = this->RestrictedFactoryPlants;
			vec.erase(std::remove(vec.begin(), vec.end(), reinterpret_cast<BuildingClass*>(ptr)), vec.end());
		}
	}

}

// =============================
// container

HouseExt::ExtContainer::ExtContainer() : Container("HouseClass")
{ }

HouseExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4F6532, HouseClass_CTOR, 0x5)
{
	GET(HouseClass*, pItem, EAX);

	HouseExt::ExtMap.TryAllocate(pItem);

	if (RulesExt::Global()->EnablePowerSurplus)
		pItem->PowerSurplus = RulesClass::Instance->PowerSurplus;

	return 0;
}

DEFINE_HOOK(0x4F7371, HouseClass_DTOR, 0x6)
{
	GET(HouseClass*, pItem, ESI);

	HouseExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x504080, HouseClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x503040, HouseClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(HouseClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	HouseExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x504069, HouseClass_Load_Suffix, 0x7)
{
	HouseExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x5046DE, HouseClass_Save_Suffix, 0x7)
{
	HouseExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK(0x50114D, HouseClass_InitFromINI, 0x5)
{
	GET(HouseClass* const, pThis, EBX);
	GET(CCINIClass* const, pINI, ESI);

	HouseExt::ExtMap.LoadFromINI(pThis, pINI);

	return 0;
}
#pragma region BuildLimitGroup
int CountOwnedIncludeDeploy(const HouseClass* pThis, const TechnoTypeClass* pItem)
{
	int count = pThis->CountOwnedNow(pItem);
	count += pItem->DeploysInto ? pThis->CountOwnedNow(pItem->DeploysInto) : 0;
	count += pItem->UndeploysInto ? pThis->CountOwnedNow(pItem->UndeploysInto) : 0;
	return count;
}

CanBuildResult HouseExt::BuildLimitGroupCheck(const HouseClass* pThis, const TechnoTypeClass* pItem, bool buildLimitOnly, bool includeQueued)
{
	const auto pItemExt = TechnoTypeExt::ExtMap.Find(pItem);

	if (pItemExt->BuildLimitGroup_Types.empty())
		return CanBuildResult::Buildable;

	std::vector<int> limits = pItemExt->BuildLimitGroup_Nums;

	if (!pItemExt->BuildLimitGroup_ExtraLimit_Types.empty() && !pItemExt->BuildLimitGroup_ExtraLimit_Nums.empty())
	{
		for (size_t i = 0; i < pItemExt->BuildLimitGroup_ExtraLimit_Types.size(); i++)
		{
			int count = 0;
			auto pTmpType = pItemExt->BuildLimitGroup_ExtraLimit_Types[i];
			auto const pBuildingType = abstract_cast<BuildingTypeClass*>(pTmpType);

			if (pBuildingType && (BuildingTypeExt::ExtMap.Find(pBuildingType)->PowersUp_Buildings.size() > 0 || BuildingTypeClass::Find(pBuildingType->PowersUpBuilding)))
				count = BuildingTypeExt::GetUpgradesAmount(pBuildingType, const_cast<HouseClass*>(pThis));
			else
				count = pThis->CountOwnedNow(pTmpType);

			if (i < pItemExt->BuildLimitGroup_ExtraLimit_MaxCount.size() && pItemExt->BuildLimitGroup_ExtraLimit_MaxCount[i] > 0)
				count = Math::min(count, pItemExt->BuildLimitGroup_ExtraLimit_MaxCount[i]);

			for (auto& limit : limits)
			{
				if (i < pItemExt->BuildLimitGroup_ExtraLimit_Nums.size() && pItemExt->BuildLimitGroup_ExtraLimit_Nums[i] > 0)
				{
					limit += count * pItemExt->BuildLimitGroup_ExtraLimit_Nums[i];

					if (pItemExt->BuildLimitGroup_ExtraLimit_MaxNum > 0)
						limit = Math::min(limit, pItemExt->BuildLimitGroup_ExtraLimit_MaxNum);
				}
			}
		}
	}

	if (pItemExt->BuildLimitGroup_ContentIfAnyMatch.Get())
	{
		bool reachedLimit = false;

		for (size_t i = 0; i < std::min(pItemExt->BuildLimitGroup_Types.size(), pItemExt->BuildLimitGroup_Nums.size()); i++)
		{
			TechnoTypeClass* pType = pItemExt->BuildLimitGroup_Types[i];
			const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
			const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pType);
			int ownedNow = 0;

			if (pBuildingType && (BuildingTypeExt::ExtMap.Find(pBuildingType)->PowersUp_Buildings.size() > 0 || BuildingTypeClass::Find(pBuildingType->PowersUpBuilding)))
				ownedNow = BuildingTypeExt::GetUpgradesAmount(pBuildingType, const_cast<HouseClass*>(pThis));
			else
				ownedNow = CountOwnedIncludeDeploy(pThis, pType);

			ownedNow *= pTypeExt->BuildLimitGroup_Factor;

			if (ownedNow >= limits[i] + 1 - pItemExt->BuildLimitGroup_Factor)
				reachedLimit |= (includeQueued && FactoryClass::FindByOwnerAndProduct(pThis, pType)) ? false : true;
		}

		return reachedLimit ? CanBuildResult::TemporarilyUnbuildable : CanBuildResult::Buildable;
	}
	else
	{
		if (limits.size() == 1U)
		{
			int sum = 0;
			bool reachedLimit = false;

			for (auto& pType : pItemExt->BuildLimitGroup_Types)
			{
				const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
				const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pType);
				int owned = 0;

				if (pBuildingType && (BuildingTypeExt::ExtMap.Find(pBuildingType)->PowersUp_Buildings.size() > 0 || BuildingTypeClass::Find(pBuildingType->PowersUpBuilding)))
					owned = BuildingTypeExt::GetUpgradesAmount(pBuildingType, const_cast<HouseClass*>(pThis));
				else
					owned = CountOwnedIncludeDeploy(pThis, pType);

				sum += owned * pTypeExt->BuildLimitGroup_Factor;
			}

			if (sum >= limits[0] + 1 - pItemExt->BuildLimitGroup_Factor)
			{
				for (auto& pType : pItemExt->BuildLimitGroup_Types)
				{
					reachedLimit |= (includeQueued && FactoryClass::FindByOwnerAndProduct(pThis, pType)) ? false : true;
				}
			}

			return reachedLimit ? CanBuildResult::TemporarilyUnbuildable : CanBuildResult::Buildable;
		}
		else
		{
			for (size_t i = 0; i < std::min(pItemExt->BuildLimitGroup_Types.size(), limits.size()); i++)
			{
				TechnoTypeClass* pType = pItemExt->BuildLimitGroup_Types[i];
				const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
				const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pType);
				int ownedNow = 0;

				if (pBuildingType && (BuildingTypeExt::ExtMap.Find(pBuildingType)->PowersUp_Buildings.size() > 0 || BuildingTypeClass::Find(pBuildingType->PowersUpBuilding)))
					ownedNow = BuildingTypeExt::GetUpgradesAmount(pBuildingType, const_cast<HouseClass*>(pThis));
				else
					ownedNow = CountOwnedIncludeDeploy(pThis, pType);

				ownedNow *= pTypeExt->BuildLimitGroup_Factor;

				if ((pItem == pType && ownedNow < limits[i] + 1 - pItemExt->BuildLimitGroup_Factor) || includeQueued && FactoryClass::FindByOwnerAndProduct(pThis, pType))
					return CanBuildResult::Buildable;

				if ((pItem != pType && ownedNow < limits[i]) || includeQueued && FactoryClass::FindByOwnerAndProduct(pThis, pType))
					return CanBuildResult::Buildable;
			}

			return CanBuildResult::TemporarilyUnbuildable;
		}
	}
}

int QueuedNum(const HouseClass* pHouse, const TechnoTypeClass* pType)
{
	const AbstractType absType = pType->WhatAmI();
	const FactoryClass* pFactory = pHouse->GetPrimaryFactory(absType, pType->Naval, BuildCat::DontCare);
	int queued = 0;

	if (pFactory)
	{
		queued = pFactory->CountTotal(pType);

		if (const auto pObject = pFactory->Object)
		{
			if (pObject->GetType() == pType)
				--queued;
		}
	}

	return queued;
}

void RemoveProduction(const HouseClass* pHouse, const TechnoTypeClass* pType, int num)
{
	const AbstractType absType = pType->WhatAmI();
	FactoryClass* pFactory = pHouse->GetPrimaryFactory(absType, pType->Naval, BuildCat::DontCare);
	if (pFactory)
	{
		int queued = pFactory->CountTotal(pType);
		if (num >= 0)
			queued = Math::min(num, queued);

		for (int i = 0; i < queued; i++)
		{
			pFactory->RemoveOneFromQueue(pType);
		}
	}
}

bool HouseExt::ReachedBuildLimit(const HouseClass* pHouse, const TechnoTypeClass* pType, bool ignoreQueued)
{
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->BuildLimitGroup_Types.empty() || pTypeExt->BuildLimitGroup_Nums.empty())
		return false;

	std::vector<int> limits = pTypeExt->BuildLimitGroup_Nums;

	if (!pTypeExt->BuildLimitGroup_ExtraLimit_Types.empty() && !pTypeExt->BuildLimitGroup_ExtraLimit_Nums.empty())
	{
		for (size_t i = 0; i < pTypeExt->BuildLimitGroup_ExtraLimit_Types.size(); i++)
		{
			auto pTmpType = pTypeExt->BuildLimitGroup_ExtraLimit_Types[i];
			const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTmpType);
			int count = 0;

			if (pBuildingType && (BuildingTypeExt::ExtMap.Find(pBuildingType)->PowersUp_Buildings.size() > 0 || BuildingTypeClass::Find(pBuildingType->PowersUpBuilding)))
				count = BuildingTypeExt::GetUpgradesAmount(pBuildingType, const_cast<HouseClass*>(pHouse));
			else
				count = pHouse->CountOwnedNow(pTmpType);

			if (i < pTypeExt->BuildLimitGroup_ExtraLimit_MaxCount.size() && pTypeExt->BuildLimitGroup_ExtraLimit_MaxCount[i] > 0)
				count = Math::min(count, pTypeExt->BuildLimitGroup_ExtraLimit_MaxCount[i]);

			for (auto& limit : limits)
			{
				if (i < pTypeExt->BuildLimitGroup_ExtraLimit_Nums.size() && pTypeExt->BuildLimitGroup_ExtraLimit_Nums[i] > 0)
				{
					limit += count * pTypeExt->BuildLimitGroup_ExtraLimit_Nums[i];

					if (pTypeExt->BuildLimitGroup_ExtraLimit_MaxNum > 0)
						limit = Math::min(limit, pTypeExt->BuildLimitGroup_ExtraLimit_MaxNum);
				}
			}
		}
	}

	if (limits.size() == 1)
	{
		int count = 0;
		int queued = 0;
		bool inside = false;

		for (TechnoTypeClass* pTmpType : pTypeExt->BuildLimitGroup_Types)
		{
			const auto pTmpTypeExt = TechnoTypeExt::ExtMap.Find(pTmpType);

			if (!ignoreQueued)
				queued += QueuedNum(pHouse, pTmpType) * pTmpTypeExt->BuildLimitGroup_Factor;

			int owned = 0;
			const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTmpType);

			if (pBuildingType && (BuildingTypeExt::ExtMap.Find(pBuildingType)->PowersUp_Buildings.size() > 0 || BuildingTypeClass::Find(pBuildingType->PowersUpBuilding)))
				owned = BuildingTypeExt::GetUpgradesAmount(pBuildingType, const_cast<HouseClass*>(pHouse));
			else
				owned = pHouse->CountOwnedNow(pTmpType);

			count += owned * pTmpTypeExt->BuildLimitGroup_Factor;

			if (pTmpType == pType)
				inside = true;
		}

		int num = count - limits.back();

		if (num + queued >= 1 - pTypeExt->BuildLimitGroup_Factor)
		{
			if (inside)
				RemoveProduction(pHouse, pType, (num + queued + pTypeExt->BuildLimitGroup_Factor - 1) / pTypeExt->BuildLimitGroup_Factor);
			else if (num >= 1 - pTypeExt->BuildLimitGroup_Factor || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
				RemoveProduction(pHouse, pType, -1);

			return true;
		}
	}
	else
	{
		size_t size = Math::min(limits.size(), pTypeExt->BuildLimitGroup_Types.size());
		bool reached = true;
		bool realReached = true;

		for (size_t i = 0; i < size; i++)
		{
			TechnoTypeClass* pTmpType = pTypeExt->BuildLimitGroup_Types[i];
			const auto pTmpTypeExt = TechnoTypeExt::ExtMap.Find(pTmpType);
			int queued = ignoreQueued ? 0 : QueuedNum(pHouse, pTmpType) * pTmpTypeExt->BuildLimitGroup_Factor;
			int num = 0;
			const auto pBuildingType = abstract_cast<BuildingTypeClass*>(pTmpType);

			if (pBuildingType && (BuildingTypeExt::ExtMap.Find(pBuildingType)->PowersUp_Buildings.size() > 0 || BuildingTypeClass::Find(pBuildingType->PowersUpBuilding)))
				num = BuildingTypeExt::GetUpgradesAmount(pBuildingType, const_cast<HouseClass*>(pHouse));
			else
				num = pHouse->CountOwnedNow(pTmpType);

			num *= pTmpTypeExt->BuildLimitGroup_Factor - limits[i];

			if (pType == pTmpType && num + queued >= 1 - pTypeExt->BuildLimitGroup_Factor)
			{
				if (pTypeExt->BuildLimitGroup_ContentIfAnyMatch)
				{
					if (num >= 1 - pTypeExt->BuildLimitGroup_Factor || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
						RemoveProduction(pHouse, pType, (num + queued + pTypeExt->BuildLimitGroup_Factor - 1) / pTypeExt->BuildLimitGroup_Factor);

					return true;
				}
				else if (num < 1 - pTypeExt->BuildLimitGroup_Factor)
				{
					realReached = false;
				}
			}
			else if (pType != pTmpType && num + queued >= 0)
			{
				if (pTypeExt->BuildLimitGroup_ContentIfAnyMatch)
				{
					if (num >= 0 || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
						RemoveProduction(pHouse, pType, -1);

					return true;
				}
				else if (num < 0)
				{
					realReached = false;
				}
			}
			else
			{
				reached = false;
			}
		}

		if (reached)
		{
			if (realReached || pTypeExt->BuildLimitGroup_NotBuildableIfQueueMatch)
				RemoveProduction(pHouse, pType, -1);

			return true;
		}
	}

	return false;
}
#pragma endregion
