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

TechTreeTypeClass* TechTreeTypeClass::GetAnySuitable(HouseClass* pHouse)
{
	for (const auto& pType : Array)
	{
		if (pHouse->ActiveBuildingTypes.GetItemCount(pType->ConstructionYard->ArrayIndex) > 0)
		{
			return pType.get();
		}
	}

	return nullptr;	
}

void TechTreeTypeClass::CalculateTotals()
{
	TotalConstructionYards.clear();
	TotalBuildPower.clear();
	TotalBuildRefinery.clear();
	TotalBuildBarracks.clear();
	TotalBuildWeapons.clear();
	TotalBuildRadar.clear();
	TotalBuildHelipad.clear();
	TotalBuildNavalYard.clear();
	TotalBuildTech.clear();
	TotalBuildDefense.clear();
	TotalBuildOther.clear();

	for (const auto& pTree : Array)
	{
		TotalConstructionYards.insert(pTree->ConstructionYard);
		TotalBuildPower.insert(pTree->BuildPower.begin(), pTree->BuildPower.end());
		TotalBuildRefinery.insert(pTree->BuildRefinery.begin(), pTree->BuildRefinery.end());
		TotalBuildBarracks.insert(pTree->BuildBarracks.begin(), pTree->BuildBarracks.end());
		TotalBuildWeapons.insert(pTree->BuildWeapons.begin(), pTree->BuildWeapons.end());
		TotalBuildRadar.insert(pTree->BuildRadar.begin(), pTree->BuildRadar.end());
		TotalBuildHelipad.insert(pTree->BuildHelipad.begin(), pTree->BuildHelipad.end());
		TotalBuildNavalYard.insert(pTree->BuildNavalYard.begin(), pTree->BuildNavalYard.end());
		TotalBuildTech.insert(pTree->BuildTech.begin(), pTree->BuildTech.end());
		TotalBuildDefense.insert(pTree->BuildDefense.begin(), pTree->BuildDefense.end());
		TotalBuildOther.insert(pTree->BuildOther.begin(), pTree->BuildOther.end());
	}
}

size_t TechTreeTypeClass::CountTotalOwnedBuildings(HouseClass* pHouse, BuildType buildType)
{
	std::set<BuildingTypeClass*>* typeList;
	switch (buildType)
	{
	case BuildType::BuildPower:
		typeList = &TotalBuildPower;
		break;
	case BuildType::BuildRefinery:
		typeList = &TotalBuildRefinery;
		break;
	case BuildType::BuildBarracks:
		typeList = &TotalBuildBarracks;
		break;
	case BuildType::BuildWeapons:
		typeList = &TotalBuildWeapons;
		break;
	case BuildType::BuildRadar:
		typeList = &TotalBuildRadar;
		break;
	case BuildType::BuildHelipad:
		typeList = &TotalBuildHelipad;
		break;
	case BuildType::BuildNavalYard:
		typeList = &TotalBuildNavalYard;
		break;
	case BuildType::BuildTech:
		typeList = &TotalBuildTech;
		break;
	case BuildType::BuildAdvancedPower:
		typeList = &TotalBuildAdvancedPower;
		break;
	case BuildType::BuildDefense:
		typeList = &TotalBuildDefense;
		break;
	case BuildType::BuildOther:
		typeList = &TotalBuildOther;
		break;
	}

	size_t count = 0;
	for (const auto pBuilding : *typeList)
	{
		count += pHouse->ActiveBuildingTypes.GetItemCount(pBuilding->ArrayIndex);
	}

	return count;
}

size_t TechTreeTypeClass::CountSideOwnedBuildings(HouseClass* pHouse, BuildType buildType) const
{
	const ValueableVector<BuildingTypeClass*>* typeList;
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

bool TechTreeTypeClass::IsSuitable(HouseClass* pHouse) const
{
	return pHouse->ActiveBuildingTypes.GetItemCount(this->ConstructionYard->ArrayIndex) > 0;
}

bool TechTreeTypeClass::IsCompleted(HouseClass* pHouse, std::function<bool(BuildingTypeClass*)> const& filter) const
{
	if (!GetBuildable(BuildType::BuildPower, filter).empty() && CountSideOwnedBuildings(pHouse, BuildType::BuildPower) < 1)
		return false;

	if (!GetBuildable(BuildType::BuildRefinery, filter).empty() && CountSideOwnedBuildings(pHouse, BuildType::BuildRefinery) < 1)
		return false;

	if (!GetBuildable(BuildType::BuildBarracks, filter).empty() && CountSideOwnedBuildings(pHouse, BuildType::BuildBarracks) < 1)
		return false;

	if (!GetBuildable(BuildType::BuildWeapons, filter).empty() && CountSideOwnedBuildings(pHouse, BuildType::BuildWeapons) < 1)
		return false;

	if (!GetBuildable(BuildType::BuildRadar, filter).empty() && CountSideOwnedBuildings(pHouse, BuildType::BuildRadar) < 1)
		return false;

	if (!GetBuildable(BuildType::BuildHelipad, filter).empty() && CountSideOwnedBuildings(pHouse, BuildType::BuildHelipad) < 1)
		return false;

	if (!GetBuildable(BuildType::BuildNavalYard, filter).empty() && CountSideOwnedBuildings(pHouse, BuildType::BuildNavalYard) < 1)
		return false;

	if (!GetBuildable(BuildType::BuildTech, filter).empty() && CountSideOwnedBuildings(pHouse, BuildType::BuildTech) < 1)
		return false;

	for (const auto& buildOtherPair : BuildOtherCountMap)
	{
		if (filter(buildOtherPair.first) && CountSideOwnedBuildings(pHouse, BuildType::BuildOther) < buildOtherPair.second)
			return false;
	}

	return true;
}

std::vector<BuildingTypeClass*> TechTreeTypeClass::GetBuildable(BuildType buildType, std::function<bool(BuildingTypeClass*)> const& filter) const
{
	const ValueableVector<BuildingTypeClass*>* typeList;
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

BuildingTypeClass* TechTreeTypeClass::GetRandomBuildable(BuildType buildType, std::function<bool(BuildingTypeClass*)> const& filter) const
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

	for (size_t i = 0; i < BuildOther.size(); i++)
	{
		if (i < BuildOtherCounts.size())
		{
			BuildOtherCountMap[BuildOther[i]] = BuildOtherCounts[i];
		}
		else
		{
			Debug::Log("TechTreeTypeClass::LoadFromINI: BuildOtherCounts is missing count for %s, setting to 0.", BuildOther[i]->Name);
			BuildOtherCountMap[BuildOther[i]] = 0;
		}
	}
}

template <typename T>
void TechTreeTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(SideIndex)
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
		.Process(BuildOtherCountMap)
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
