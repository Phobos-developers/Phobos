#include "TechTreeTypeClass.h"

#include <HouseClass.h>

#include <Utilities/TemplateDef.h>
#include "Utilities/Debug.h"

TechTreeTypeClass* TechTreeTypeClass::GetForSide(int sideIndex)
{
	if (Array.empty())
		Debug::FatalErrorAndExit("TechTreeTypeClass::GetForSide: Array is empty!");

	for (const auto& pType : Array)
	{
		if (pType->SideIndex == sideIndex)
		{
			return pType.get();
		}
	}

	Debug::Log("TechTreeTypeClass::GetForSide: Could not find tech tree for side %d, returning tech tree 0: %s", sideIndex, Array[0]->Name.data());
	return Array[0].get();
}

void TechTreeTypeClass::InitializeCache()
{
	Cache.ConstructionYards.clear();
	Cache.BuildPower.clear();
	Cache.BuildRefinery.clear();
	Cache.BuildBarracks.clear();
	Cache.BuildWeapons.clear();
	Cache.BuildRadar.clear();
	Cache.BuildHelipad.clear();
	Cache.BuildNavalYard.clear();
	Cache.BuildTech.clear();
	Cache.BuildOther.clear();

	for (const auto& pTree : Array)
	{
		Cache.ConstructionYards.insert(pTree->ConstructionYard);
		Cache.BuildPower.insert(pTree->BuildPower.begin(), pTree->BuildPower.end());
		Cache.BuildRefinery.insert(pTree->BuildRefinery.begin(), pTree->BuildRefinery.end());
		Cache.BuildBarracks.insert(pTree->BuildBarracks.begin(), pTree->BuildBarracks.end());
		Cache.BuildWeapons.insert(pTree->BuildWeapons.begin(), pTree->BuildWeapons.end());
		Cache.BuildRadar.insert(pTree->BuildRadar.begin(), pTree->BuildRadar.end());
		Cache.BuildHelipad.insert(pTree->BuildHelipad.begin(), pTree->BuildHelipad.end());
		Cache.BuildNavalYard.insert(pTree->BuildNavalYard.begin(), pTree->BuildNavalYard.end());
		Cache.BuildTech.insert(pTree->BuildTech.begin(), pTree->BuildTech.end());
		Cache.BuildOther.insert(pTree->BuildOther.begin(), pTree->BuildOther.end());
	}
}

size_t TechTreeTypeClass::CountTotalOwnedBuildings(HouseClass* pHouse, BuildType buildType)
{
	std::unordered_set<BuildingTypeClass*>* typeList;
	switch (buildType)
	{
	case BuildType::BuildPower:
		typeList = &Cache.BuildPower;
		break;
	case BuildType::BuildRefinery:
		typeList = &Cache.BuildRefinery;
		break;
	case BuildType::BuildBarracks:
		typeList = &Cache.BuildBarracks;
		break;
	case BuildType::BuildWeapons:
		typeList = &Cache.BuildWeapons;
		break;
	case BuildType::BuildRadar:
		typeList = &Cache.BuildRadar;
		break;
	case BuildType::BuildHelipad:
		typeList = &Cache.BuildHelipad;
		break;
	case BuildType::BuildNavalYard:
		typeList = &Cache.BuildNavalYard;
		break;
	case BuildType::BuildTech:
		typeList = &Cache.BuildTech;
		break;
	case BuildType::BuildAdvancedPower:
		typeList = &Cache.BuildAdvancedPower;
		break;
	case BuildType::BuildDefense:
		typeList = &Cache.BuildDefense;
		break;
	case BuildType::BuildOther:
		typeList = &Cache.BuildOther;
		break;
	}

	size_t count = 0;
	for (const auto pBuilding : *typeList)
	{
		count += pHouse->ActiveBuildingTypes.GetItemCount(pBuilding->ArrayIndex);
	}

	return count;
}

size_t TechTreeTypeClass::CountSideOwnedBuildings(HouseClass* pHouse, BuildType buildType)
{
	ValueableVector<BuildingTypeClass*>* typeList;
	switch (buildType)
	{
	case BuildType::BuildPower:
		typeList = &this->BuildPower;
		break;
	case BuildType::BuildRefinery:
		typeList = &this->BuildRefinery;
		break;
	case BuildType::BuildBarracks:
		typeList = &this->BuildBarracks;
		break;
	case BuildType::BuildWeapons:
		typeList = &this->BuildWeapons;
		break;
	case BuildType::BuildRadar:
		typeList = &this->BuildRadar;
		break;
	case BuildType::BuildHelipad:
		typeList = &this->BuildHelipad;
		break;
	case BuildType::BuildNavalYard:
		typeList = &this->BuildNavalYard;
		break;
	case BuildType::BuildTech:
		typeList = &this->BuildTech;
		break;
	case BuildType::BuildAdvancedPower:
		typeList = &this->BuildAdvancedPower;
		break;
	case BuildType::BuildDefense:
		typeList = &this->BuildDefense;
		break;
	case BuildType::BuildOther:
		typeList = &this->BuildOther;
		break;
	}

	size_t count = 0;
	for (const auto pBuilding : *typeList)
	{
		count += pHouse->ActiveBuildingTypes.GetItemCount(pBuilding->ArrayIndex);
	}

	return count;
}

std::vector<BuildingTypeClass*> TechTreeTypeClass::GetBuildable(BuildType buildType, std::function<bool(BuildingTypeClass*)> const& filter)
{
	ValueableVector<BuildingTypeClass*>* typeList;
	switch (buildType)
	{
	case BuildType::BuildPower:
		typeList = &this->BuildPower;
		break;
	case BuildType::BuildRefinery:
		typeList = &this->BuildRefinery;
		break;
	case BuildType::BuildBarracks:
		typeList = &this->BuildBarracks;
		break;
	case BuildType::BuildWeapons:
		typeList = &this->BuildWeapons;
		break;
	case BuildType::BuildRadar:
		typeList = &this->BuildRadar;
		break;
	case BuildType::BuildHelipad:
		typeList = &this->BuildHelipad;
		break;
	case BuildType::BuildNavalYard:
		typeList = &this->BuildNavalYard;
		break;
	case BuildType::BuildTech:
		typeList = &this->BuildTech;
		break;
	case BuildType::BuildAdvancedPower:
		typeList = &this->BuildAdvancedPower;
		break;
	case BuildType::BuildDefense:
		typeList = &this->BuildDefense;
		break;
	case BuildType::BuildOther:
		typeList = &this->BuildOther;
		break;
	}

	std::vector<BuildingTypeClass*> filtered;
	std::ranges::copy_if(*typeList, std::back_inserter(filtered), filter);
	return filtered;
}

BuildingTypeClass* TechTreeTypeClass::GetRandomBuildable(BuildType buildType, std::function<bool(BuildingTypeClass*)> const& filter)
{
	std::vector<BuildingTypeClass*> buildable = GetBuildable(buildType, filter);
	if (buildable.empty())
	{
		return nullptr;
	}

	return buildable[ScenarioClass::Instance->Random.RandomRanged(0, buildable.size() - 1)];
}

template<>
const char* Enumerable<TechTreeTypeClass>::GetMainSection()
{
	return "TechTreeTypes";
}

void TechTreeTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->SideIndex.Read(exINI, section, "SideIndex");
	this->ConstructionYard.Read(exINI, section, "ConstructionYard");
	this->BuildPower.Read(exINI, section, "BuildPower");
	this->BuildRefinery.Read(exINI, section, "BuildRefinery");
	this->BuildBarracks.Read(exINI, section, "BuildBarracks");
	this->BuildWeapons.Read(exINI, section, "BuildWeapons");
	this->BuildRadar.Read(exINI, section, "BuildRadar");
	this->BuildHelipad.Read(exINI, section, "BuildHelipad");
	this->BuildNavalYard.Read(exINI, section, "BuildNavalYard");
	this->BuildTech.Read(exINI, section, "BuildTech");
	this->BuildAdvancedPower.Read(exINI, section, "BuildAdvancedPower");
	this->BuildDefense.Read(exINI, section, "BuildDefense");
	this->BuildOther.Read(exINI, section, "BuildOther");
	this->BuildOtherCounts.Read(exINI, section, "BuildOtherCounts");
}

template <typename T>
void TechTreeTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(ConstructionYard)
		.Process(BuildPower)
		.Process(BuildRefinery)
		.Process(BuildBarracks)
		.Process(BuildRadar)
		.Process(BuildHelipad)
		.Process(BuildNavalYard)
		.Process(BuildTech)
		.Process(BuildAdvancedPower)
		.Process(BuildDefense)
		.Process(BuildOther)
		.Process(BuildOtherCounts)
		;
}

void TechTreeTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void TechTreeTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
