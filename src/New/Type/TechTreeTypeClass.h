#pragma once

#include <functional>
#include <unordered_set>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class TechTreeTypeClass final : public Enumerable<TechTreeTypeClass>
{
public:
	enum class BuildType
	{
		BuildPower,
		BuildRefinery,
		BuildBarracks,
		BuildWeapons,
		BuildRadar,
		BuildHelipad,
		BuildNavalYard,
		BuildTech,
		BuildAdvancedPower,
		BuildDefense,
		BuildOther
	};

	Valueable<int> SideIndex;
	Valueable<BuildingTypeClass*> ConstructionYard;
	ValueableVector<BuildingTypeClass*> BuildPower;
	ValueableVector<BuildingTypeClass*> BuildRefinery;
	ValueableVector<BuildingTypeClass*> BuildBarracks;
	ValueableVector<BuildingTypeClass*> BuildWeapons;
	ValueableVector<BuildingTypeClass*> BuildRadar;
	ValueableVector<BuildingTypeClass*> BuildHelipad;
	ValueableVector<BuildingTypeClass*> BuildNavalYard;
	ValueableVector<BuildingTypeClass*> BuildTech;
	ValueableVector<BuildingTypeClass*> BuildAdvancedPower;
	ValueableVector<BuildingTypeClass*> BuildDefense;
	ValueableVector<BuildingTypeClass*> BuildOther;
	ValueableVector<int> BuildOtherCounts;

	TechTreeTypeClass(const char* pTitle = NONE_STR) : Enumerable(pTitle) { }

	virtual ~TechTreeTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	static TechTreeTypeClass* GetForSide(int sideIndex);
	static size_t CountTotalOwnedBuildings(HouseClass* pHouse, BuildType buildType);

	size_t CountSideOwnedBuildings(HouseClass* pHouse, BuildType buildType);
	std::vector<BuildingTypeClass*> GetBuildable(BuildType buildType, std::function<bool(BuildingTypeClass*)> const& filter);
	BuildingTypeClass* GetRandomBuildable(BuildType buildType, std::function<bool(BuildingTypeClass*)> const& filter);

	static struct
	{
		std::unordered_set<BuildingTypeClass*> ConstructionYards;
		std::unordered_set<BuildingTypeClass*> BuildPower;
		std::unordered_set<BuildingTypeClass*> BuildRefinery;
		std::unordered_set<BuildingTypeClass*> BuildBarracks;
		std::unordered_set<BuildingTypeClass*> BuildWeapons;
		std::unordered_set<BuildingTypeClass*> BuildRadar;
		std::unordered_set<BuildingTypeClass*> BuildHelipad;
		std::unordered_set<BuildingTypeClass*> BuildNavalYard;
		std::unordered_set<BuildingTypeClass*> BuildTech;
		std::unordered_set<BuildingTypeClass*> BuildAdvancedPower;
		std::unordered_set<BuildingTypeClass*> BuildDefense;
		std::unordered_set<BuildingTypeClass*> BuildOther;
	} Cache;

	static void InitializeCache();

private:
	template <typename T>
	void Serialize(T& Stm);
};
