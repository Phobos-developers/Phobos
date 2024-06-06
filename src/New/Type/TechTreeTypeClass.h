#pragma once

#include <functional>
#include <map>
#include <set>
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
	std::map<BuildingTypeClass*, int> BuildOtherCountMap;

	TechTreeTypeClass(const char* pTitle = NONE_STR) : Enumerable(pTitle) { }

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	static TechTreeTypeClass* GetForSide(int sideIndex);
	static TechTreeTypeClass* GetAnySuitable(HouseClass* pHouse);
	static size_t CountTotalOwnedBuildings(HouseClass* pHouse, BuildType buildType);

	bool IsSuitable(HouseClass* pHouse) const;
	bool IsCompleted(HouseClass* pHouse, std::function<bool(BuildingTypeClass*)> const& filter) const;
	size_t CountSideOwnedBuildings(HouseClass* pHouse, BuildType buildType) const;
	std::vector<BuildingTypeClass*> GetBuildable(BuildType buildType, std::function<bool(BuildingTypeClass*)> const& filter) const;
	BuildingTypeClass* GetRandomBuildable(BuildType buildType, std::function<bool(BuildingTypeClass*)> const& filter) const;

	static inline std::set<BuildingTypeClass*> TotalConstructionYards;
	static inline std::set<BuildingTypeClass*> TotalBuildPower;
	static inline std::set<BuildingTypeClass*> TotalBuildRefinery;
	static inline std::set<BuildingTypeClass*> TotalBuildBarracks;
	static inline std::set<BuildingTypeClass*> TotalBuildWeapons;
	static inline std::set<BuildingTypeClass*> TotalBuildRadar;
	static inline std::set<BuildingTypeClass*> TotalBuildHelipad;
	static inline std::set<BuildingTypeClass*> TotalBuildNavalYard;
	static inline std::set<BuildingTypeClass*> TotalBuildTech;
	static inline std::set<BuildingTypeClass*> TotalBuildAdvancedPower;
	static inline std::set<BuildingTypeClass*> TotalBuildDefense;
	static inline std::set<BuildingTypeClass*> TotalBuildOther;

	static void CalculateTotals();

private:
	template <typename T>
	void Serialize(T& Stm);
};
